"""Deterministic AEAD result comparison, distinct from cryptanalytic experiments."""
import argparse,json,pathlib,subprocess
from reference import fixture,encrypt

def main():
    ap=argparse.ArgumentParser();ap.add_argument('runner');ap.add_argument('--output',type=pathlib.Path);args=ap.parse_args()
    lengths=(0,1,15,16,17,31,32,33,255,256,257,1024)
    pairs=[(a,m) for a in lengths for m in lengths]
    for i in range(144,400):
        raw=fixture('lengths',i,8)
        pairs.append((int.from_bytes(raw[:4],'little')%513,int.from_bytes(raw[4:],'little')%1025))
    records=[]
    for i,(a,m) in enumerate(pairs):
        key=fixture('key',i,16);nonce=i.to_bytes(16,'little');ad=fixture('ad',i,a);p=fixture('message',i,m)
        c,t=encrypt(key,nonce,ad,p)
        got=subprocess.check_output([args.runner,key.hex(),nonce.hex(),ad.hex(),p.hex()],text=True).splitlines()
        expected=[c.hex(),t.hex(),p.hex()]
        if got!=expected:
            first=next((j for j,(x,y) in enumerate(zip('|'.join(got),'|'.join(expected))) if x!=y),min(len('|'.join(got)),len('|'.join(expected))))
            raise AssertionError(dict(index=i,domain='CrossWeave/tests/',labels=['key','ad','message','lengths'],key=key.hex(),nonce=nonce.hex(),ad=ad.hex(),plaintext=p.hex(),expected=expected,actual=got,first_hex_offset=first))
        records.append(dict(index=i,labels={'key':'key','ad':'ad','message':'message','lengths':'lengths' if i>=144 else None},nonce=nonce.hex(),ad_len=a,message_len=m))
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(dict(status='complete',cases=records),indent=2)+'\n')
    print('400 deterministic AEAD cases passed: 144 boundary + 256 additional; C/Python, decrypt, in-place')
if __name__=='__main__':main()
