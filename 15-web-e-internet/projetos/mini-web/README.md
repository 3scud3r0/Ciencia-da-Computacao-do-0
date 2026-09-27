# Mini aplicação web completa

O navegador pede HTML, o HTML referencia JavaScript e o JavaScript chama uma API JSON. Tudo é servido por um processo Python sem framework.

Fluxo:

~~~text
GET / → HTML → GET /app.js → fetch('/api/status') → JSON → DOM
~~~

Execute `python server.py` e abra `http://127.0.0.1:8000`. Depois use DevTools → Network para observar cada request.
