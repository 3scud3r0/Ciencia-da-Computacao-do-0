from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer
import json
HTML=b"""<!doctype html><meta charset=utf-8><title>CC0</title><h1 id=t>Carregando...</h1><script src=/app.js></script>"""
JS=b"""fetch('/api/status').then(r=>r.json()).then(x=>document.querySelector('#t').textContent=x.message);"""
def route(path):
    if path=="/":return 200,"text/html; charset=utf-8",HTML
    if path=="/app.js":return 200,"text/javascript; charset=utf-8",JS
    if path=="/api/status":return 200,"application/json",json.dumps({"message":"Web do navegador ao backend"},ensure_ascii=False).encode()
    return 404,"text/plain; charset=utf-8",b"not found\n"
class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        status,content_type,body=route(self.path)
        self.send_response(status);self.send_header("Content-Type",content_type);self.send_header("Content-Length",str(len(body)));self.end_headers();self.wfile.write(body)
    def log_message(self,*_):pass
def serve(host="127.0.0.1",port=8000):ThreadingHTTPServer((host,port),Handler).serve_forever()
if __name__=="__main__":serve()
