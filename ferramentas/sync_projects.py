#!/usr/bin/env python3
import json, re, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'site' / 'data' / 'projects.json'
BYOX = 'https://raw.githubusercontent.com/codecrafters-io/build-your-own-x/master/README.md'
PBL = 'https://raw.githubusercontent.com/practical-tutorials/project-based-learning/master/README.md'

def fetch(url):
    req = urllib.request.Request(url, headers={'User-Agent': 'cc0-project-sync'})
    with urllib.request.urlopen(req, timeout=30) as r:
        return r.read().decode('utf-8')

def link_from_line(line):
    line = line.strip()
    if not re.match(r'^[-*]\\s+', line):
        return None
    a = line.find('[')
    b = line.find('](', a + 1)
    c = line.rfind(')')
    if a < 0 or b < 0 or c <= b + 2:
        return None
    title = re.sub(r'[*_`]', '', line[a + 1:b]).strip()
    url = line[b + 2:c].strip()
    if not title or not re.match(r'^https?://', url, re.I):
        return None
    if re.search(r'badge|gitter|contributing', title, re.I):
        return None
    return title, url

def parse_byox(md):
    out, category = [], 'Outros'
    for raw in md.splitlines():
        line = raw.strip()
        h = re.match(r'^##\\s+Build your own\\s+(.+)$', line, re.I)
        if h:
            category = h.group(1).replace('&lt;', '<').replace('&gt;', '>').strip()
            continue
        item = link_from_line(line)
        if item:
            title, url = item
            out.append({'title': title, 'url': url, 'category': category, 'source': 'Build Your Own X'})
    return out

def parse_pbl(md):
    out, category, sub = [], 'Outros', ''
    for raw in md.splitlines():
        line = raw.strip()
        h2 = re.match(r'^##\\s+(.+?)(?::)?$', line)
        if h2 and 'Table of Contents' not in h2.group(1):
            category, sub = h2.group(1).rstrip(':').strip(), ''
            continue
        h3 = re.match(r'^###\\s+(.+?)(?::)?$', line)
        if h3:
            sub = h3.group(1).rstrip(':').strip()
            continue
        item = link_from_line(line)
        if item:
            title, url = item
            cat = category + (' · ' + sub if sub else '')
            out.append({'title': title, 'url': url, 'category': cat, 'source': 'Project Based Learning'})
    return out

def main():
    byox = parse_byox(fetch(BYOX))
    pbl = parse_pbl(fetch(PBL))
    seen, projects = set(), []
    for p in byox + pbl:
        key = (p['source'], p['url'])
        if key not in seen:
            seen.add(key)
            projects.append(p)
    payload = {
        'generatedFrom': [
            {'name': 'Build Your Own X', 'url': 'https://github.com/codecrafters-io/build-your-own-x'},
            {'name': 'Project Based Learning', 'url': 'https://github.com/practical-tutorials/project-based-learning'}
        ],
        'note': 'Índice de títulos, categorias e links públicos para navegação educacional; o conteúdo permanece nas fontes.',
        'counts': {'buildYourOwnX': len(byox), 'projectBasedLearning': len(pbl), 'total': len(projects)},
        'projects': projects
    }
    OUT.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + '\\n', encoding='utf-8')
    print('Catálogo atualizado:', len(projects), 'entradas')

if __name__ == '__main__':
    main()
