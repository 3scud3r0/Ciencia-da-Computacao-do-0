#!/usr/bin/env python3
from __future__ import annotations
import json,re,sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
curriculum=json.loads((ROOT/"site/data/curriculum.json").read_text(encoding="utf-8"))
lessons=json.loads((ROOT/"site/data/lessons.json").read_text(encoding="utf-8"))
errors=[]

expected=sum(len(m["chapters"]) for m in curriculum)
if expected!=184:errors.append(f"ementa esperava 184 capítulos, encontrou {expected}")
if len(lessons)!=expected:errors.append(f"lessons.json tem {len(lessons)}, esperado {expected}")

required_sections=[
    "## Ideia central","## Modelo mental","## Código e laboratório",
    "## Experimento guiado","## Perguntas de domínio","## Exercícios","## Critério de conclusão"
]

for module in curriculum:
    base=ROOT/module["slug"]
    index=base/"AULAS.md"
    if not index.exists():errors.append(f"faltando {index.relative_to(ROOT)}")
    module_lessons=[x for x in lessons if x["module"]==module["id"]]
    if len(module_lessons)!=len(module["chapters"]):
        errors.append(f"módulo {module['id']}: {len(module_lessons)} aulas para {len(module['chapters'])} capítulos")
    for item in module_lessons:
        path=ROOT/item["path"]
        if not path.exists():
            errors.append(f"faltando {item['path']}");continue
        text=path.read_text(encoding="utf-8")
        for section in required_sections:
            if section not in text:errors.append(f"{item['path']}: faltando seção {section}")
        words=len(re.findall(r"\b\w+\b",text,flags=re.UNICODE))
        if words<180:errors.append(f"{item['path']}: aula curta demais ({words} palavras)")

project_roots=[
"00-como-estudar/projetos/mini-git","01-programacao/projetos/virtual-fs",
"02-c-e-memoria/projetos/vetor-dinamico","03-matematica-discreta/projetos/discrete-lab",
"04-estruturas-de-dados/projetos/hashmap-python","05-algoritmos/projetos/roteador-dijkstra",
"06-arquitetura-de-computadores/projetos/alu16","07-sistemas-operacionais/projetos/scheduler-simulator",
"08-redes/projetos/http-server","09-bancos-de-dados/projetos/bplus-tree",
"10-linguagens-e-compiladores/projetos/mini-lang","11-engenharia-de-software/projetos/instrumented-service",
"12-concorrencia-e-paralelismo/projetos/thread-pool","13-sistemas-distribuidos/projetos/consistent-hashing",
"14-seguranca/projetos/password-store","15-web-e-internet/projetos/mini-web",
"16-teoria-da-computacao/projetos/automatos","17-topicos-avancados/projetos/raytracer",
"18-projetos-finais/projetos/kv-http-service"
]
for relative in project_roots:
    p=ROOT/relative
    if not (p/"README.md").exists():errors.append(f"{relative}: sem README")
    if not list(p.glob("test_*.py")) and not (p/"test_vector.c").exists():
        errors.append(f"{relative}: sem teste")

capstones=[f"18-projetos-finais/projetos/{i:02d}-" for i in range(1,11)]
dirs=[p for p in (ROOT/"18-projetos-finais/projetos").iterdir() if p.is_dir()]
for prefix in capstones:
    name=prefix.split("/")[-1]
    matches=[p for p in dirs if p.name.startswith(name)]
    if len(matches)!=1:errors.append(f"capstone {name}: esperado 1 diretório, encontrou {len(matches)}")
    elif not (matches[0]/"README.md").exists() or not list(matches[0].glob("test_*.py")):
        errors.append(f"{matches[0].relative_to(ROOT)}: README/teste ausente")

if errors:
    print("\n".join("ERRO: "+x for x in errors));sys.exit(1)
print(f"OK: {len(curriculum)} módulos, {expected} aulas, {len(project_roots)} laboratórios principais e 10 capstones validados.")
