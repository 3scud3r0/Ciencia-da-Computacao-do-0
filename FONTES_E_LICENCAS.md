# Fontes de terceiros e licenças

Este repositório contém duas classes de material:

1. **Conteúdo original do projeto** — explicações, implementações, exercícios e projetos escritos especificamente para `Ciencia-da-Computacao-do-0`.
2. **Snapshots de terceiros** — cópias claramente isoladas em `vendor/`, preservando a licença e a autoria da fonte.

## Matriz verificada em 27/09/2026

| Fonte | Licença declarada | Tratamento |
|---|---|---|
| TheAlgorithms/Python | MIT | Snapshot permitido em `vendor/`, preservando LICENSE |
| TheAlgorithms/Algorithms-Explanation | MIT | Snapshot permitido em `vendor/`, preservando LICENSE |
| practical-tutorials/project-based-learning | MIT | Snapshot permitido em `vendor/`, preservando LICENSE |
| davecom/ComputerScienceFromScratch | Apache-2.0 | Snapshot permitido em `vendor/`, preservando LICENSE/NOTICE aplicáveis |
| jwasham/coding-interview-university | CC BY-SA 4.0 | Snapshot separado, com atribuição e licença preservadas |
| codecrafters-io/build-your-own-x | nenhuma licença declarada no repositório | **não copiar**; manter índice, links e resumos originais |

> Uma lista como Build Your Own X ou Project Based Learning aponta para centenas de sites e repositórios independentes. A licença da lista **não concede automaticamente direitos sobre os tutoriais externos**. Cada fonte externa precisa ser tratada individualmente.

## Regras de importação

- Nunca executar código de terceiros durante a importação.
- Preservar `LICENSE`, `COPYING`, `NOTICE` e arquivos equivalentes.
- Registrar repositório, URL, licença e commit de origem.
- Não misturar arquivos de terceiros com implementações originais sem indicar proveniência.
- Material sem licença explícita permanece como link + metadados + resumo original.
- Quando um tutorial externo tiver licença permissiva verificável, poderá ser adicionado a uma importação futura.
- Códigos essenciais do curso devem, sempre que possível, possuir uma implementação original pedagógica no diretório curricular correspondente.

## Estrutura

~~~text
vendor/
├── README.md
├── manifest.json
├── thealgorithms-python/
├── algorithms-explanation/
├── project-based-learning/
├── computer-science-from-scratch/
└── coding-interview-university/
~~~

Cada pasta conserva os termos da respectiva fonte. Esses termos podem ser diferentes dos termos do restante deste projeto.
