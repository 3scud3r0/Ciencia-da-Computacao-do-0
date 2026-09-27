from __future__ import annotations
import re
TOKEN=re.compile(r"(?P<NUM>\d+)|(?P<NAME>[A-Za-z_]\w*)|(?P<OP>[+\-*/=();])|(?P<WS>\s+)|(?P<BAD>.)")

def tokenize(source):
    out=[]
    for m in TOKEN.finditer(source):
        kind=m.lastgroup;value=m.group()
        if kind=="WS":continue
        if kind=="BAD":raise SyntaxError(value)
        out.append((kind,value))
    out.append(("EOF",""));return out

class Parser:
    def __init__(self,tokens):self.t=tokens;self.i=0
    @property
    def cur(self):return self.t[self.i]
    def pop(self):x=self.cur;self.i+=1;return x
    def accept(self,v):
        if self.cur[1]==v:self.pop();return True
        return False
    def expect(self,v):
        if not self.accept(v):raise SyntaxError("esperado "+v)
    def program(self):
        out=[]
        while self.cur[0]!="EOF":out.append(self.statement());self.accept(";")
        return ("program",out)
    def statement(self):
        if self.cur==("NAME","let"):
            self.pop();kind,name=self.pop()
            if kind!="NAME":raise SyntaxError("nome")
            self.expect("=");return ("let",name,self.expr())
        if self.cur==("NAME","print"):
            self.pop();return ("print",self.expr())
        return ("expr",self.expr())
    def expr(self):
        n=self.term()
        while self.cur[1] in {"+","-"}:op=self.pop()[1];n=(op,n,self.term())
        return n
    def term(self):
        n=self.unary()
        while self.cur[1] in {"*","/"}:op=self.pop()[1];n=(op,n,self.unary())
        return n
    def unary(self):
        return ("neg",self.unary()) if self.accept("-") else self.primary()
    def primary(self):
        kind,value=self.pop()
        if kind=="NUM":return ("num",int(value))
        if kind=="NAME":return ("name",value)
        if value=="(":n=self.expr();self.expect(")");return n
        raise SyntaxError(value)

class Compiler:
    def compile(self,source):
        ast=Parser(tokenize(source)).program();code=[]
        def emit(node):
            kind=node[0]
            if kind=="program":
                for s in node[1]:emit(s)
            elif kind=="num":code.append(("CONST",node[1]))
            elif kind=="name":code.append(("LOAD",node[1]))
            elif kind=="neg":emit(node[1]);code.append(("NEG",))
            elif kind in {"+","-","*","/"}:
                emit(node[1]);emit(node[2]);code.append(({"+":"ADD","-":"SUB","*":"MUL","/":"DIV"}[kind],))
            elif kind=="let":emit(node[2]);code.append(("STORE",node[1]))
            elif kind=="print":emit(node[1]);code.append(("PRINT",))
            elif kind=="expr":emit(node[1]);code.append(("POP",))
            else:raise RuntimeError(node)
        emit(ast);code.append(("HALT",));return code

class VM:
    def __init__(self):self.stack=[];self.env={};self.output=[]
    def run(self,code):
        ip=0
        while True:
            ins=code[ip];ip+=1;op=ins[0]
            if op=="HALT":break
            if op=="CONST":self.stack.append(ins[1])
            elif op=="LOAD":
                if ins[1] not in self.env:raise NameError(ins[1])
                self.stack.append(self.env[ins[1]])
            elif op=="STORE":self.env[ins[1]]=self.stack.pop()
            elif op=="NEG":self.stack.append(-self.stack.pop())
            elif op in {"ADD","SUB","MUL","DIV"}:
                b=self.stack.pop();a=self.stack.pop()
                self.stack.append(a+b if op=="ADD" else a-b if op=="SUB" else a*b if op=="MUL" else a//b)
            elif op=="PRINT":self.output.append(self.stack.pop())
            elif op=="POP":self.stack.pop()
            else:raise RuntimeError(op)
        return self.output
