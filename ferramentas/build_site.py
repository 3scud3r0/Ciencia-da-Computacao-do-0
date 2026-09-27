"""Gera páginas estáticas de aulas a partir do catálogo e dos arquivos Markdown."""

from __future__ import annotations

import html
import json
import re
from pathlib import Path
from urllib.parse import quote, urlparse


ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / "site"
REPO = "https://github.com/3scud3r0/Ciencia-da-Computacao-do-0/blob/main/"
LINK = re.compile(r"\[([^\]]+)\]\(([^)]+)\)")
CODE = re.compile(r"`([^`]+)`")


def page_url(lesson: dict) -> str:
    source = Path(lesson["path"])
    return f"aulas/{source.parent.parent.name}/{source.stem}/"


def source_url(source: Path, target: str) -> str:
    """Converte links relativos do Markdown em links para a fonte original."""
    parsed = urlparse(target)
    if parsed.scheme in {"https", "http"}:
        return target
    if parsed.scheme or target.startswith(("/", "//")):
        return "#"
    resolved = (source.parent / target).resolve()
    try:
        relative = resolved.relative_to(ROOT)
    except ValueError:
        return "#"
    if not resolved.is_file():
        raise ValueError(f"Link de arquivo inexistente: {source}: {target}")
    return REPO + quote(relative.as_posix(), safe="/")


def inline(value: str, source: Path) -> str:
    """Escapa o texto antes de inserir as poucas marcas usadas nas aulas."""
    parts = []
    offset = 0
    for match in LINK.finditer(value):
        parts.append(inline_plain(value[offset:match.start()]))
        label = inline_plain(match.group(1))
        href = html.escape(source_url(source, match.group(2)), quote=True)
        parts.append(f'<a href="{href}" target="_blank" rel="noopener noreferrer">{label}</a>')
        offset = match.end()
    parts.append(inline_plain(value[offset:]))
    return "".join(parts)


def inline_plain(value: str) -> str:
    escaped = html.escape(value)
    return CODE.sub(lambda m: f"<code>{m.group(1)}</code>", escaped)


def render_markdown(source: Path) -> tuple[str, list[tuple[str, str]]]:
    lines = source.read_text(encoding="utf-8").splitlines()
    output: list[str] = []
    headings: list[tuple[str, str]] = []
    list_type = None
    code: list[str] | None = None
    paragraph: list[str] = []

    def flush_paragraph() -> None:
        if paragraph:
            output.append("<p>" + inline(" ".join(paragraph), source) + "</p>")
            paragraph.clear()

    def close_list() -> None:
        nonlocal list_type
        if list_type:
            output.append(f"</{list_type}>")
            list_type = None

    for line in lines:
        if line.startswith(("~~~", "```")):
            flush_paragraph()
            close_list()
            if code is None:
                code = []
            else:
                output.append("<pre><code>" + html.escape("\n".join(code)) + "</code></pre>")
                code = None
            continue
        if code is not None:
            code.append(line)
            continue
        heading = re.match(r"^(#{1,3})\s+(.+)$", line)
        if heading:
            flush_paragraph()
            close_list()
            level = len(heading.group(1))
            label = heading.group(2)
            anchor = f"secao-{len(headings) + 1}"
            if level > 1:
                headings.append((anchor, label))
            output.append(f'<h{level} id="{anchor}">{inline(label, source)}</h{level}>')
            continue
        item = re.match(r"^(?:([-*])|\d+\.)\s+(.+)$", line)
        if item:
            flush_paragraph()
            kind = "ul" if item.group(1) else "ol"
            if list_type != kind:
                close_list()
                output.append(f"<{kind}>")
                list_type = kind
            output.append("<li>" + inline(item.group(2), source) + "</li>")
            continue
        if not line.strip():
            flush_paragraph()
            close_list()
        else:
            close_list()
            paragraph.append(line.strip())
    if code is not None:
        raise ValueError(f"Bloco de código não encerrado: {source}")
    flush_paragraph()
    close_list()
    return "\n".join(output), headings


def build() -> int:
    lessons = json.loads((SITE / "data/lessons.json").read_text(encoding="utf-8"))
    template = (SITE / "lesson-template.html").read_text(encoding="utf-8")
    paths = [page_url(lesson) for lesson in lessons]
    if len(paths) != len(set(paths)):
        raise ValueError("Duas aulas geram a mesma URL")
    for index, lesson in enumerate(lessons):
        source = (ROOT / lesson["path"]).resolve()
        source.relative_to(ROOT)
        if not source.is_file():
            raise ValueError(f"Aula inexistente: {lesson['path']}")
        body, headings = render_markdown(source)
        toc = "\n".join(
            f'<a href="#{anchor}">{html.escape(label)}</a>' for anchor, label in headings
        )
        previous = paths[index - 1] if index else None
        following = paths[index + 1] if index + 1 < len(paths) else None
        def nav(href: str | None, label: str) -> str:
            return f'<a href="../../../{href}">{html.escape(label)}</a>' if href else "<span></span>"
        values = {
            "TITLE": html.escape(lesson["title"]),
            "MODULE": html.escape(lesson["module"] + " · " + lesson["moduleTitle"]),
            "DESCRIPTION": html.escape(lesson["description"], quote=True),
            "BODY": body,
            "TOC": toc,
            "PREVIOUS": nav(previous, "← Aula anterior"),
            "NEXT": nav(following, "Próxima aula →"),
            "SOURCE": html.escape(REPO + quote(lesson["path"], safe="/"), quote=True),
            "PATH": html.escape(lesson["path"], quote=True),
        }
        if lesson["path"] == "01-programacao/aulas/04-recursao.md":
            values["ACTIVITY"] = (SITE / "activities/recursao.html").read_text(encoding="utf-8")
            values["ACTIVITY_SCRIPT"] = '<script src="../../../activities/recursao.js" defer></script>'
            values["HAS_EXERCISE"] = "true"
        else:
            values.update(ACTIVITY="", ACTIVITY_SCRIPT="", HAS_EXERCISE="false")
        page = template
        for key, value in values.items():
            page = page.replace("{{" + key + "}}", value)
        destination = SITE / paths[index] / "index.html"
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(page, encoding="utf-8")
    return len(lessons)


if __name__ == "__main__":
    print(f"{build()} páginas de aula geradas.")
