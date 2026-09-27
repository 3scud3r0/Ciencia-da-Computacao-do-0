#!/usr/bin/env python3
"""Audita links GitHub do catálogo e registra licença/tamanho sem copiar conteúdo."""
from __future__ import annotations
import json, os, re, urllib.error, urllib.request
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
CATALOG=ROOT/"site"/"data"/"projects.json"
OUTPUT=ROOT/"site"/"data"/"source-audit.json"
PATTERN=re.compile(r"^https?://(?:www\.)?github\.com/([^/]+)/([^/?#]+)",re.I)

def api_json(url:str):
    headers={"Accept":"application/vnd.github+json","User-Agent":"cc0-source-audit"}
    token=os.environ.get("GITHUB_TOKEN")
    if token: headers["Authorization"]=f"Bearer {token}"
    req=urllib.request.Request(url,headers=headers)
    with urllib.request.urlopen(req,timeout=30) as r:
        return json.load(r)

def main():
    catalog=json.loads(CATALOG.read_text(encoding="utf-8"))
    found={}
    for item in catalog["projects"]:
        match=PATTERN.match(item["url"])
        if not match: continue
        owner=match.group(1)
        name=match.group(2).removesuffix(".git")
        if owner in {"topics","collections","settings","marketplace"}: continue
        full=f"{owner}/{name}"
        row=found.setdefault(full,{"repository":full,"references":0,"examples":[]})
        row["references"]+=1
        if len(row["examples"])<3: row["examples"].append(item["title"])

    results=[]
    for full,row in sorted(found.items(),key=lambda pair:pair[0].lower()):
        try:
            data=api_json("https://api.github.com/repos/"+full)
            license_data=data.get("license") or {}
            spdx=license_data.get("spdx_id")
            row.update({
                "html_url":data.get("html_url"),
                "default_branch":data.get("default_branch"),
                "size_kb":data.get("size"),
                "archived":data.get("archived",False),
                "fork":data.get("fork",False),
                "license_spdx":spdx,
                "license_name":license_data.get("name"),
                "has_declared_license":bool(spdx and spdx not in {"NOASSERTION","OTHER"}),
                "status":"ok",
            })
        except urllib.error.HTTPError as exc:
            row.update({"status":f"http-{exc.code}","has_declared_license":False})
        except Exception as exc:
            row.update({"status":"error","error":str(exc),"has_declared_license":False})
        results.append(row)

    payload={
        "generated_from":"site/data/projects.json",
        "policy":"Somente fontes com licença explicitamente identificada podem ser candidatas a snapshot. A licença deve ser preservada.",
        "counts":{
            "unique_github_repositories":len(results),
            "declared_license":sum(bool(x.get("has_declared_license")) for x in results),
            "without_verified_license":sum(not bool(x.get("has_declared_license")) for x in results),
        },
        "repositories":results,
    }
    OUTPUT.write_text(json.dumps(payload,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(payload["counts"],ensure_ascii=False))

if __name__=="__main__":
    main()
