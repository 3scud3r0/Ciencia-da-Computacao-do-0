from __future__ import annotations
import base64,hashlib,hmac,secrets,sqlite3,time

class App:
    def __init__(self,path=":memory:",clock=time.time,token_factory=None):
        self.db=sqlite3.connect(path);self.clock=clock;self.token_factory=token_factory or (lambda:secrets.token_urlsafe(24))
        self.db.execute("PRAGMA foreign_keys=ON")
        self.db.executescript("""
        CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY,email TEXT UNIQUE NOT NULL,password TEXT NOT NULL);
        CREATE TABLE IF NOT EXISTS sessions(token TEXT PRIMARY KEY,user_id INTEGER NOT NULL,expires REAL NOT NULL,FOREIGN KEY(user_id) REFERENCES users(id));
        CREATE TABLE IF NOT EXISTS notes(id INTEGER PRIMARY KEY,user_id INTEGER NOT NULL,body TEXT NOT NULL,created REAL NOT NULL,FOREIGN KEY(user_id) REFERENCES users(id));
        """)
    @staticmethod
    def _hash(password):
        salt=secrets.token_bytes(16);digest=hashlib.scrypt(password.encode(),salt=salt,n=2**14,r=8,p=1,dklen=32)
        return base64.b64encode(salt+digest).decode()
    @staticmethod
    def _verify(password,encoded):
        raw=base64.b64decode(encoded);salt,expected=raw[:16],raw[16:]
        actual=hashlib.scrypt(password.encode(),salt=salt,n=2**14,r=8,p=1,dklen=len(expected))
        return hmac.compare_digest(actual,expected)
    def register(self,email,password):
        if "@" not in email or len(password)<8:raise ValueError("credenciais inválidas")
        cur=self.db.execute("INSERT INTO users(email,password) VALUES(?,?)",(email.lower(),self._hash(password)));self.db.commit()
        return cur.lastrowid
    def login(self,email,password,ttl=3600):
        row=self.db.execute("SELECT id,password FROM users WHERE email=?",(email.lower(),)).fetchone()
        if row is None or not self._verify(password,row[1]):raise PermissionError("login inválido")
        token=self.token_factory();self.db.execute("INSERT INTO sessions VALUES(?,?,?)",(token,row[0],self.clock()+ttl));self.db.commit();return token
    def _user(self,token):
        row=self.db.execute("SELECT user_id,expires FROM sessions WHERE token=?",(token,)).fetchone()
        if row is None or row[1]<=self.clock():raise PermissionError("sessão inválida")
        return row[0]
    def create_note(self,token,body):
        user=self._user(token)
        cur=self.db.execute("INSERT INTO notes(user_id,body,created) VALUES(?,?,?)",(user,body,self.clock()));self.db.commit();return cur.lastrowid
    def list_notes(self,token):
        user=self._user(token)
        return [{"id":r[0],"body":r[1],"created":r[2]} for r in self.db.execute("SELECT id,body,created FROM notes WHERE user_id=? ORDER BY id",(user,))]
    def route(self,method,path,headers=None,json_body=None):
        headers=headers or {};json_body=json_body or {}
        try:
            if method=="POST" and path=="/register":
                return 201,{"user_id":self.register(json_body["email"],json_body["password"])}
            if method=="POST" and path=="/login":
                return 200,{"token":self.login(json_body["email"],json_body["password"])}
            token=headers.get("Authorization","").removeprefix("Bearer ")
            if method=="POST" and path=="/notes":
                return 201,{"note_id":self.create_note(token,json_body["body"])}
            if method=="GET" and path=="/notes":
                return 200,{"notes":self.list_notes(token)}
            return 404,{"error":"not found"}
        except (ValueError,PermissionError,KeyError) as exc:
            return 400 if isinstance(exc,(ValueError,KeyError)) else 401,{"error":str(exc)}
