# Ciência da Computação do 0

Formação autodidata, orientada a código e projetos, cobrindo os fundamentos centrais de uma graduação em Ciência da Computação de forma mais direta.

## Comece aqui

- Leia [guia.md](./guia.md) para a super ementa.
- O site está em `site/` e é publicado via GitHub Pages. As aulas são lidas no próprio site, com navegação e progresso local por aula.
- O catálogo do site agrega todas as entradas detectadas nas listas públicas **Build Your Own X** e **Project Based Learning**, apontando para as fontes originais.

## Filosofia

**conceito → motivação → modelo mental → formalização → código → testes → laboratório → projeto**

O objetivo é entender computação por camadas: bits, C, memória, estruturas, algoritmos, assembly, CPU, sistemas operacionais, redes, bancos, linguagens, concorrência, distribuídos, segurança e teoria.

## Conteúdo externo e licenças

Este repositório não copia em massa código ou texto de outros projetos. Tutoriais externos são catalogados por link. Quando uma implementação de terceiros for incorporada, sua licença e atribuição devem ser preservadas; sempre que possível, serão produzidas implementações didáticas próprias.

## Status

- [x] Super ementa
- [x] Estrutura curricular
- [x] Site inicial
- [x] Catálogo de projetos externos
- [x] Capítulos completos
- [x] Laboratórios e testes
- [x] Projetos integradores completos
- [x] Leitor estático das 184 aulas, gerado dos arquivos Markdown

## Site local

Gere as páginas das aulas e inicie um servidor local:

```bash
python ferramentas/build_site.py
python -m http.server 8000 --directory site
```

Abra `http://localhost:8000/`. O script usa apenas a biblioteca padrão do Python; `site/aulas/` é gerado novamente durante a publicação e não precisa ser versionado. O catálogo (`site/data/lessons.json`) define a ordem das aulas, e cada campo `path` aponta para o Markdown que serve de fonte. Os links relativos de arquivos dentro das aulas são resolvidos para o repositório original. O progresso fica no `localStorage` deste navegador e não é sincronizado entre aparelhos.

## O que significa "completo"

O curso está completo **dentro do escopo educacional definido pela ementa**:

- 184 aulas autorais, uma para cada capítulo;
- 19 laboratórios principais, todos com implementação real e testes;
- 10 projetos finais integradores executáveis;
- CI que executa os projetos e valida a integridade curricular;
- biblioteca externa licenciada + catálogo de 818 projetos/tutoriais.

Os projetos são implementações educacionais completas dentro de seu escopo; não alegam equivalência operacional com Linux, Git, Redis, PostgreSQL, Docker ou outros sistemas de produção.
