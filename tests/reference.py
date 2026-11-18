"""Equation-based Python oracle. Never loads or calls the C implementation."""
import hashlib
MASK=(1<<64)-1
ORDER=(0,2,4,1,3,5)
PERM=(2,5,1,4,0,3)
ROT=(1,7,19,31,43,57)

def rol(x,n):
    n%=64
    return ((x<<n)|(x>>((64-n)%64)))&MASK

def f(a,b,r,i):
    alpha=(7*i+3*r)%63+1
    beta=(13*i+5*r)%63+1
    return rol(a,alpha)^rol(b,beta)^(a&rol(b,11))^((a+b)&MASK)

def rc(r,lane):
    text=f'cw-p384/round-{r}/lane-{lane}'.encode('ascii')
    return int.from_bytes(hashlib.shake_256(text).digest(8),'little')

def lanes(data):
    if len(data)!=48: raise ValueError('state length')
    return [int.from_bytes(data[j:j+8],'little') for j in range(0,48,8)]

def serialize(s):
    return b''.join(x.to_bytes(8,'little') for x in s)

def updates(s,r):
    s=s.copy(); trace=[]
    for i in ORDER:
        s[i]^=f(s[(i+1)%6],s[(i+3)%6],r,i)
        trace.append(serialize(s))
    return s,trace

def round_state(s,r,inverse=False):
    if not 0<=r<16: raise ValueError('round index')
    if inverse:
        old=[0]*6
        for i in range(6): old[PERM[i]]=rol(s[i],64-ROT[i])
        old[0]^=rc(r,0); old[3]^=rc(r,3)
        for i in reversed(ORDER): old[i]^=f(old[(i+1)%6],old[(i+3)%6],r,i)
        return old
    t,_=updates(s,r)
    t[0]^=rc(r,0); t[3]^=rc(r,3)
    return [rol(t[PERM[i]],ROT[i]) for i in range(6)]

def permute(data,rounds=16,inverse=False):
    if not 0<=rounds<=16: raise ValueError('round count')
    s=lanes(data)
    for r in (reversed(range(rounds)) if inverse else range(rounds)):
        s=round_state(s,r,inverse)
    return serialize(s)

def fixture(label,index,n):
    seed=b'CrossWeave/tests/'+label.encode('ascii')+b'\0'+index.to_bytes(8,'little')
    return hashlib.shake_256(seed).digest(n)

def selftest():
    assert permute(bytes(48),12).hex()=='f98e26352d066afa483b96a5fad30f0963895e163d529708102e9e8728393b8bb113373987dbf0066981e2fb0ffc2630'
    assert permute(bytes(48),16).hex()=='59e853991a8c3b868074ee51104a750d54ca24a15315468d72233a7ed2126b5110b0ef086692e1cc7ba01d0648b15c8f'
    for i in range(32):
        data=fixture('permutation',i,48)
        for n in range(17):
            assert permute(permute(data,n),n,True)==data
            assert permute(permute(data,n,True),n)==data
    print('Python permutation oracle: fixed KATs and 1088 bidirectional prefix inversions')

if __name__=='__main__': selftest()

# The oracle models equations and authentication, not C memory ownership or cleansing.
def record(trace,event,state,length):
    if trace is not None: trace.append({'event':event,'length':length,'state':bytes(state).hex()})

def initialize(key,nonce,trace=None):
    if len(key)!=16 or len(nonce)!=16: raise ValueError('key/nonce length')
    iv=hashlib.shake_256(b'CrossWeave/IV').digest(16)
    s=bytearray(permute(nonce+key+iv,16))
    for j in range(16): s[16+j]^=key[j]
    record(trace,'init',s,0)
    return s

def absorb_ad(s,ad,trace=None):
    full=len(ad)//16*16
    for start in range(0,full,16):
        for j in range(16): s[j]^=ad[start+j]
        s[47]^=0x11; s[:]=permute(s,12)
        record(trace,'ad-full',s,16)
    tail=ad[full:]
    for j,value in enumerate(tail): s[j]^=value
    s[len(tail)]^=0x80; s[47]^=0x12; s[:]=permute(s,12)
    record(trace,'ad-tail',s,len(tail))

def crypt_message(s,data,decrypt=False,trace=None):
    result=bytearray()
    full=len(data)//16*16
    for start in range(0,full,16):
        block=bytes(data[start+j]^s[j] for j in range(16))
        result.extend(block); s[:16]=data[start:start+16] if decrypt else block
        s[47]^=0x21; s[:]=permute(s,12)
        record(trace,'message-full',s,16)
    tail=data[full:]
    block=bytes(value^s[j] for j,value in enumerate(tail))
    result.extend(block); s[:len(tail)]=tail if decrypt else block
    s[len(tail)]^=0x80; s[47]^=0x22; s[:]=permute(s,12)
    record(trace,'message-tail',s,len(tail))
    return bytes(result)

def finalize(s,key,trace=None):
    for j in range(16): s[16+j]^=key[j]
    s[47]^=0x31; s[:]=permute(s,16)
    record(trace,'final',s,0)
    return bytes(s[j]^key[j] for j in range(16))

def encrypt(key,nonce,ad,plaintext,trace=None):
    if len(ad)>16777216 or len(plaintext)>16777216: raise ValueError('input limit')
    s=initialize(key,nonce,trace); absorb_ad(s,ad,trace)
    ciphertext=crypt_message(s,plaintext,False,trace)
    return ciphertext,finalize(s,key,trace)

def decrypt(key,nonce,ad,ciphertext,tag,trace=None):
    import hmac
    if len(tag)!=16: raise ValueError('tag length')
    if len(ad)>16777216 or len(ciphertext)>16777216: raise ValueError('input limit')
    s=initialize(key,nonce,trace); absorb_ad(s,ad,trace)
    plaintext=crypt_message(s,ciphertext,True,trace)
    expected=finalize(s,key,trace)
    if not hmac.compare_digest(tag,expected): raise ValueError('authentication')
    return plaintext
