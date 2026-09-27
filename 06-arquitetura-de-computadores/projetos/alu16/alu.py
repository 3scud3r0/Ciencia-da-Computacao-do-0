MASK=0xFFFF;SIGN=0x8000
def u16(x):return x&MASK
def s16(x):
 x&=MASK;return x-0x10000 if x&SIGN else x
def alu(x,y,op):
 x=u16(x);y=u16(y);carry=overflow=False
 if op=="ADD":
  raw=x+y;result=u16(raw);carry=raw>MASK
  sx,sy,sr=bool(x&SIGN),bool(y&SIGN),bool(result&SIGN);overflow=sx==sy and sr!=sx
 elif op=="SUB":
  result=u16(x-y);carry=x>=y
  sx,sy,sr=bool(x&SIGN),bool(y&SIGN),bool(result&SIGN);overflow=sx!=sy and sr!=sx
 elif op=="AND":result=x&y
 elif op=="OR":result=x|y
 elif op=="XOR":result=x^y
 elif op=="NOT":result=u16(~x)
 elif op=="SHL":carry=bool(x&SIGN);result=u16(x<<1)
 elif op=="SHR":carry=bool(x&1);result=x>>1
 else:raise ValueError(op)
 return result,{"Z":result==0,"N":bool(result&SIGN),"C":carry,"V":overflow}
