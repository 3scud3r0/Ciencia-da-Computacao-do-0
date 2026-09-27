from __future__ import annotations

OP_HALT=0x0; OP_LOADI=0x1; OP_ADD=0x2; OP_STORE=0x3; OP_LOAD=0x4
OP_JNZ=0x5; OP_JMP=0x6; OP_SUB=0x7; OP_XOR=0x8; OP_OUT=0x9

class Assembler:
    OPCODES={"HALT":OP_HALT,"LOADI":OP_LOADI,"ADD":OP_ADD,"STORE":OP_STORE,"LOAD":OP_LOAD,
             "JNZ":OP_JNZ,"JMP":OP_JMP,"SUB":OP_SUB,"XOR":OP_XOR,"OUT":OP_OUT}
    def assemble(self,source:str)->list[int]:
        cleaned=[]; labels={}; pc=0
        for raw in source.splitlines():
            line=raw.split("#",1)[0].strip()
            if not line: continue
            if line.endswith(":"):
                labels[line[:-1].strip()]=pc; continue
            cleaned.append(line); pc+=1
        words=[]
        for line in cleaned:
            parts=line.replace(","," ").split(); op=parts[0].upper()
            if op not in self.OPCODES: raise SyntaxError(f"opcode desconhecido: {op}")
            code=self.OPCODES[op]
            if op=="HALT": word=0
            elif op=="JMP":
                word=(code<<12)|(self._value(parts[1],labels)&0xFFF)
            elif op=="OUT":
                word=(code<<12)|(self._reg(parts[1])<<8)
            elif op in {"LOADI","STORE","LOAD","JNZ"}:
                reg=self._reg(parts[1]); value=self._value(parts[2],labels)
                if not 0<=value<=255: raise ValueError("imediato/endereço deve caber em 8 bits")
                word=(code<<12)|(reg<<8)|value
            elif op in {"ADD","SUB","XOR"}:
                rd=self._reg(parts[1]); rs=self._reg(parts[2]); word=(code<<12)|(rd<<8)|(rs<<4)
            words.append(word&0xFFFF)
        return words
    @staticmethod
    def _reg(token):
        if not token.upper().startswith("R"): raise SyntaxError("registrador esperado")
        reg=int(token[1:])
        if not 0<=reg<16: raise ValueError("registrador 0..15")
        return reg
    @staticmethod
    def _value(token,labels):
        if token in labels:return labels[token]
        return int(token,0)

class CPU16:
    def __init__(self,memory_words=256):
        self.reg=[0]*16; self.mem=[0]*memory_words; self.pc=0; self.halted=False; self.output=[]
    def load_program(self,words):
        if len(words)>len(self.mem):raise ValueError("programa grande")
        self.mem[:len(words)]=words; self.pc=0; self.halted=False; self.output=[]
    def step(self):
        if self.halted:return
        word=self.mem[self.pc]; self.pc=(self.pc+1)&0xFFFF
        op=(word>>12)&0xF; a=(word>>8)&0xF; b=(word>>4)&0xF; imm=word&0xFF
        if op==OP_HALT:self.halted=True
        elif op==OP_LOADI:self.reg[a]=imm
        elif op==OP_ADD:self.reg[a]=(self.reg[a]+self.reg[b])&0xFFFF
        elif op==OP_SUB:self.reg[a]=(self.reg[a]-self.reg[b])&0xFFFF
        elif op==OP_XOR:self.reg[a]^=self.reg[b]
        elif op==OP_STORE:self.mem[imm]=self.reg[a]
        elif op==OP_LOAD:self.reg[a]=self.mem[imm]
        elif op==OP_JNZ:
            if self.reg[a]!=0:self.pc=imm
        elif op==OP_JMP:self.pc=word&0xFFF
        elif op==OP_OUT:self.output.append(self.reg[a])
        else:raise RuntimeError(f"opcode inválido: {op}")
    def run(self,max_steps=10000):
        steps=0
        while not self.halted:
            if steps>=max_steps:raise RuntimeError("limite de passos excedido")
            self.step();steps+=1
        return self.output
