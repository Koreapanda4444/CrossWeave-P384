import math,pathlib,sys,tempfile,time,json,subprocess
from common import ROOT,samples,sample_input
from experiments import delta,count,statistics,search
sys.path.insert(0,str(ROOT/'tests'))
from reference import permute
assert len({delta(i) for i in range(55)})==55
assert delta(47)==bytes(47)+b'\x80' and delta(54)==b'\xff'*48
assert sample_input('discovery',2,47,0)!=sample_input('holdout',2,47,0)
for r in (0,1,2,12,16):
    for i in (0,47,54):
        raw=samples('fixed',r,i,0,8);counts=[0]*384
        for j in range(0,len(raw),48):
            x=raw[j:j+48];d=bytes(a^b for a,b in zip(x,delta(i)));diff=int.from_bytes(permute(x,r),'little')^int.from_bytes(permute(d,r),'little')
            for bit in range(384):counts[bit]+=(diff>>bit)&1
        assert count(sys.argv[1],'fixed',r,i,0,8,time.monotonic()+60)==counts
for n in range(1,33):
    for k in range(n+1):
        expected=min(1,2*sum(math.comb(n,j) for j in range(min(k,n-k)+1))/2**n)
        s=statistics(k,n,12);assert abs(s['p']-expected)<1e-14
        assert s['wilson_pointwise_99'][0]<=k/n+1e-14<=s['wilson_pointwise_99'][1]+1e-14
s=statistics(0,16384,40);assert s['p']==0 and s['log_p']< -10000 and math.isfinite(s['adjusted_log_p'])
with tempfile.TemporaryDirectory() as tmp:
    result=search(sys.argv[1],time.monotonic(),pathlib.Path(tmp)/'interrupted.json')
    assert result['status']=='incomplete' and result['completed']==dict(discovery=0,holdout=0)
print('55 deltas, 384-bit numbering, independent C/Python counts, seed separation, statistics boundaries and budget checkpoint passed')

raw=bytes(range(256))+bytes(128)
actual=json.loads(subprocess.check_output([sys.argv[1],'0','0','8'],input=raw))
assert actual==[8]+[0]*383

# A cancelled experiment must replace a stale complete result and retain partial counts.
from unittest.mock import patch
from argparse import Namespace
import local_relations
from run import run as integration_run
with tempfile.TemporaryDirectory() as tmp:
    path=pathlib.Path(tmp)/'result.json';path.write_text('{"status":"complete"}')
    worker=pathlib.Path(sys.argv[1]).resolve()
    args=Namespace(mode='full',budget=0,worker=worker,local_runner=worker,framing_runner=worker,omitted_ad=worker,aead_runner=worker,output=path)
    interrupted=integration_run(args)
    assert interrupted['status']=='incomplete' and json.loads(path.read_text())['status']=='incomplete'
    with patch('experiments.count',side_effect=[[0]*384,KeyboardInterrupt()]):
        interrupted=search(worker,time.monotonic()+60,path)
    assert interrupted['status']=='incomplete' and interrupted['completed']['discovery']==1
    interrupted=local_relations.run(worker,deadline=time.monotonic(),checkpoint=path)
    assert interrupted['status']=='incomplete' and interrupted['completed_positions']==0
