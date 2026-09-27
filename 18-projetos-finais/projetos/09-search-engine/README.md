# Capstone 09 — Search engine

Tokenizer, inverted index termo→documentos, term frequency, document frequency, ranking BM25 e persistência/reconstrução.

A consulta não percorre todos os textos: acessa postings das palavras consultadas. O ranking combina frequência no documento com raridade global e normalização pelo tamanho do documento.

Próximos níveis: stemming, campos, phrase queries, posições, segment merge, compressão de postings e avaliação MAP/NDCG.
