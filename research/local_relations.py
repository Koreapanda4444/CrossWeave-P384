import argparse,json,subprocess,sys
from common import ROOT,samples,sample_input,identity,save
sys.path.insert(0,str(ROOT/'tests'))
from reference import f,rol
M=1<<63

def check(runner,r,i,n,dataset=0):
    raw=samples('local',r,i,dataset,n);pairs=b''.join(raw[j:j+16] for j in range(0,len(raw),48))
    got=json.loads(subprocess.check_output([str(runner),str(r),str(i),str(n)],input=pairs))
    ah=[0,0];bh=[0,0];bothh=[0]*4
    for j in range(0,len(pairs),16):
        a=int.from_bytes(pairs[j:j+8],'little');b=int.from_bytes(pairs[j+8:j+16],'little')
        base=f(a,b,r,i);da=f(a^M,b,r,i)^base;db=f(a,b^M,r,i)^base
        alpha=(7*i+3*r)%63+1;beta=(13*i+5*r)%63+1
        assert da==rol(M,alpha)^M^(M&rol(b,11))
        assert db==rol(M,beta)^M^(a&rol(M,11))
        ah[bool(da^(rol(M,alpha)^M))]+=1;bh[bool(db^(rol(M,beta)^M))]+=1
        if r==0 and i==0:
            both=f(a^M,b^M,r,i)^base;assert both in (0,1024,M,M^1024)
            bothh[2*bool(both&M)+bool(both&1024)]+=1
    assert got==dict(a=ah,b=bh,both=bothh)
    return dict(round=r,lane=i,n=n,dataset_index=dataset,generator_input=sample_input('local',r,i,dataset).hex(),counts=got)

def run(runner,full=False):
    n=256 if full else 8
    rows=[check(runner,r,i,n) for r in range(16) for i in range(6)]
    both=check(runner,0,0,16384,1) if full else None
    return dict(**identity(),status='complete',classification='exact local component identities',positions=96,pairs_per_position=n,rows=rows,simultaneous_msb=both,theory='Independent uniform a,b: each single MSB relation has two differences of probability 1/2; simultaneous at r0/i0 has 0,2^10,2^63,2^63 XOR 2^10 each 1/4. Dependent lane-event probabilities must not be multiplied.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('runner');p.add_argument('--full',action='store_true');p.add_argument('--output');a=p.parse_args();d=run(a.runner,a.full)
    if a.output:save(a.output,d)
    print(json.dumps({k:d[k] for k in ('status','positions','pairs_per_position')}))
