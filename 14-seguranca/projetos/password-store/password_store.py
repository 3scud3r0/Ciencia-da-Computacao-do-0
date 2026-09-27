import base64,hashlib,hmac,os
N=2**14;R=8;P=1;DKLEN=32
def hash_password(password):
    if not isinstance(password,str) or not password:raise ValueError("senha não vazia")
    salt=os.urandom(16)
    derived=hashlib.scrypt(password.encode(),salt=salt,n=N,r=R,p=P,dklen=DKLEN)
    return "$".join(["scrypt",str(N),str(R),str(P),base64.b64encode(salt).decode(),base64.b64encode(derived).decode()])
def verify_password(password,encoded):
    try:
        algorithm,n,r,p,salt_b64,hash_b64=encoded.split("$")
        if algorithm!="scrypt":return False
        salt=base64.b64decode(salt_b64,validate=True);expected=base64.b64decode(hash_b64,validate=True)
        actual=hashlib.scrypt(password.encode(),salt=salt,n=int(n),r=int(r),p=int(p),dklen=len(expected))
        return hmac.compare_digest(actual,expected)
    except (ValueError,TypeError):
        return False
