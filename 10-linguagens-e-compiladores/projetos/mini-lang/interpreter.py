from dataclasses import dataclass
import re
TOKEN_RE=re.compile(r"(?P<NUMBER>\d+(?:\.\d+)?)|(?P<NAME>[A-Za-z_]\w*)|(?P<OP>[+\-*/=();])|(?P<WS>\s+)|(?P<MISMATCH>.)")
@dataclass(frozen=True,slots=True)
class Token:kind:str;value:str
def tokenize(source):
 out=[]
 for m in TOKEN_RE.finditer(source):
  kind=m.lastgroup;value=m.group()
  if kind=="WS":continue
  if kind=="MISMATCH":raise SyntaxError(value)
  out.append(Token(kind,value))
 out.append(Token("EOF",""));return out
class Parser:
 def __init__(self,tokens):self.tokens=tokens;self.position=0
 @property
 def current(self):return self.tokens[self.position]
 def advance(self):t=self.current;self.position+=1;return t
 def accept(self,value):
  if self.current.value==value:self.advance();return True
  return False
 def expect(self,value):
  if not self.accept(value):raise SyntaxError(f"esperado {value}")
 def program(self):
  out=[]
  while self.current.kind!="EOF":out.append(self.statement());self.accept(";")
  return ("program",out)
 def statement(self):
  if self.current.kind=="NAME" and self.current.value=="let":
   self.advance();name=self.advance()
   if name.kind!="NAME":raise SyntaxError("nome esperado")
   self.expect("=");return ("let",name.value,self.expression())
  if self.current.kind=="NAME" and self.current.value=="print":
   self.advance();return ("print",self.expression())
  return ("expr",self.expression())
 def expression(self):
  node=self.term()
  while self.current.value in {"+","-"}:op=self.advance().value;node=(op,node,self.term())
  return node
 def term(self):
  node=self.unary()
  while self.current.value in {"*","/"}:op=self.advance().value;node=(op,node,self.unary())
  return node
 def unary(self):return ("neg",self.unary()) if self.accept("-") else self.primary()
 def primary(self):
  t=self.advance()
  if t.kind=="NUMBER":return ("number",float(t.value))
  if t.kind=="NAME":return ("name",t.value)
  if t.value=="(":node=self.expression();self.expect(")");return node
  raise SyntaxError(t.value)
class Interpreter:
 def __init__(self):self.environment={};self.output=[]
 def run(self,source):self.execute(Parser(tokenize(source)).program());return list(self.output)
 def execute(self,node):
  kind=node[0]
  if kind=="program":
   result=None
   for s in node[1]:result=self.execute(s)
   return result
  if kind=="let":value=self.execute(node[2]);self.environment[node[1]]=value;return value
  if kind=="print":
   value=self.execute(node[1]);self.output.append(str(int(value)) if value.is_integer() else str(value));return value
  if kind=="expr":return self.execute(node[1])
  if kind=="number":return node[1]
  if kind=="name":
   if node[1] not in self.environment:raise NameError(node[1])
   return self.environment[node[1]]
  if kind=="neg":return -self.execute(node[1])
  a=self.execute(node[1]);b=self.execute(node[2])
  if kind=="+":return a+b
  if kind=="-":return a-b
  if kind=="*":return a*b
  if kind=="/":return a/b
  raise RuntimeError(node)
