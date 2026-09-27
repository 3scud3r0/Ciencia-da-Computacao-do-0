import socket
from dataclasses import dataclass
from datetime import datetime,timezone
@dataclass(slots=True)
class Request:
 method:str;target:str;version:str;headers:dict[str,str];body:bytes
def parse_request(data):
 head,sep,body=data.partition(b"\r\n\r\n")
 if not sep:raise ValueError("requisição incompleta")
 lines=head.decode("iso-8859-1").split("\r\n")
 try:method,target,version=lines[0].split(" ",2)
 except ValueError as exc:raise ValueError("request line inválida") from exc
 if not version.startswith("HTTP/"):raise ValueError("versão inválida")
 headers={}
 for line in lines[1:]:
  if ":" not in line:raise ValueError("header inválido")
  name,value=line.split(":",1);headers[name.strip().lower()]=value.strip()
 n=int(headers.get("content-length","0"))
 if len(body)<n:raise ValueError("body incompleto")
 return Request(method,target,version,headers,body[:n])
def response(status,body,headers=None):
 hs={"Content-Length":str(len(body)),"Content-Type":"text/plain; charset=utf-8","Connection":"close","Date":datetime.now(timezone.utc).strftime("%a, %d %b %Y %H:%M:%S GMT")}
 if headers:hs.update(headers)
 lines=[f"HTTP/1.1 {status}"]+[f"{k}: {v}" for k,v in hs.items()]
 return ("\r\n".join(lines)+"\r\n\r\n").encode("ascii")+body
def handle(req):
 if req.method!="GET":return response("405 Method Not Allowed",b"Use GET\n",{"Allow":"GET"})
 if req.target=="/":return response("200 OK",b"Ciencia da Computacao do 0\n")
 if req.target=="/health":return response("200 OK",b"ok\n")
 return response("404 Not Found",b"not found\n")
def serve(host="127.0.0.1",port=8080):
 with socket.create_server((host,port),reuse_port=False) as server:
  print(f"escutando em http://{host}:{port}")
  while True:
   connection,_=server.accept()
   with connection:
    data=b""
    while b"\r\n\r\n" not in data and len(data)<65536:
     chunk=connection.recv(4096)
     if not chunk:break
     data+=chunk
    try:payload=handle(parse_request(data))
    except (ValueError,UnicodeError):payload=response("400 Bad Request",b"bad request\n")
    connection.sendall(payload)
if __name__=="__main__":serve()
