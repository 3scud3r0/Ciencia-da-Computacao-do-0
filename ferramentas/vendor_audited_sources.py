#!/usr/bin/env python3
"""Cria snapshots de repositórios GitHub catalogados que declaram licença.

Não executa código das fontes. O limite de tamanho protege a usabilidade do
repositório principal; fontes maiores continuam registradas no manifesto.
"""
from __future__ import annotations
import json, shutil, subprocess, tempfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
AUDIT=ROOT/"site"/"data"/"source-audit.json"
DEST=ROOT/"vendor"/"community"
MAX_SIZE_KB=50_000

def safe_name(full:str)->str:
    return full.replace("/","__")

def clone_snapshot(url:str,destination:Path)->str:
    with tempfile.TemporaryDirectory(prefix="cc0-vendor-") as tmp:
        source=Path(tmp)/"repo"
        subprocess.run(
            ["git","clone","--depth","1","--filter=blob:none",url,source],
            check=True,
            stdout=subprocess.DEVNULL,
        )
        commit=subprocess.check_output(["git","-C",source,"rev-parse","HEAD"],text=True).strip()
        if destination.exists(): shutil.rmtree(destination)
        shutil.copytree(source,destination,ignore=shutil.ignore_patterns(".git"),symlinks=True)
        return commit

def main():
    audit=json.loads(AUDIT.read_text(encoding="utf-8"))
    DEST.mkdir(parents=True,exist_ok=True)
    imported=[];skipped=[];failed=[]
    for row in audit["repositories"]:
        if not row.get("has_declared_license"):
            skipped.append({**row,"reason":"sem licença verificada"})
            continue
        size=int(row.get("size_kb") or 0)
        if size>MAX_SIZE_KB:
            skipped.append({**row,"reason":f"tamanho reportado acima de {MAX_SIZE_KB} KB"})
            continue
        full=row["repository"]
        url=row.get("html_url") or ("https://github.com/"+full)
        destination=DEST/safe_name(full)
        try:
            commit=clone_snapshot(url+".git",destination)
            imported.append({
                "repository":full,
                "url":url,
                "license_spdx":row.get("license_spdx"),
                "size_kb":size,
                "upstream_commit":commit,
                "path":str(destination.relative_to(ROOT)),
            })
            print("importado",full,row.get("license_spdx"),size,"KB")
        except Exception as exc:
            failed.append({**row,"reason":str(exc)})
            print("falhou",full,exc)

    manifest={
        "policy":{
            "max_size_kb":MAX_SIZE_KB,
            "rule":"snapshot somente quando GitHub identifica licença explícita; licença original permanece dentro da cópia",
            "execution":"nenhum código de terceiro é executado durante a importação",
        },
        "counts":{"imported":len(imported),"skipped":len(skipped),"failed":len(failed)},
        "imported":imported,
        "skipped":skipped,
        "failed":failed,
    }
    (DEST/"manifest.json").write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(manifest["counts"]))

if __name__=="__main__":
    main()
