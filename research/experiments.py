import argparse,json,math,subprocess,time
from common import samples,sample_input,identity,save
Z=2.5758293035489004

def delta(index):
    bits=(0,1,7,15,31,32,47,63)
    if index<48:return (1<<((index//8)*64+bits[index%8])).to_bytes(48,'little')
    if index<54:return (b'\0'*(8*(index-48))+b'\xff'*8).ljust(48,b'\0')
    if index==54:return b'\xff'*48
    raise ValueError('delta index')

def statistics(k,n,family=1):
    if not 0<=k<=n or n<=0 or family<1:raise ValueError('statistics bounds')
    m=min(k,n-k);term=1;total=1
    for j in range(m):term=term*(n-j)//(j+1);total+=term
    numerator=2*total;denominator=1<<n
    logp=min(0.0,math.log(numerator)-n*math.log(2));p=min(1.0,numerator/denominator)
    phat=k/n;den=1+Z*Z/n;center=(phat+Z*Z/(2*n))/den
    half=Z*math.sqrt(phat*(1-phat)/n+Z*Z/(4*n*n))/den
    return dict(count=k,n=n,rate=phat,wilson_pointwise_99=[max(0.0,center-half),min(1.0,center+half)],p=p,log_p=logp,bonferroni_family=family,adjusted_p=min(1.0,p*family),adjusted_log_p=min(0.0,logp+math.log(family)))

def count(worker,label,rounds,index,dataset,n,deadline):
    left=deadline-time.monotonic()
    if left<=0:raise TimeoutError('research budget exhausted')
    raw=samples(label,rounds,index,dataset,n)
    try:r=subprocess.run([str(worker),str(rounds),str(index),str(n)],input=raw,capture_output=True,check=True,timeout=left)
    except subprocess.TimeoutExpired as e:raise TimeoutError('worker budget exhausted') from e
    result=json.loads(r.stdout)
    if len(result)!=384 or any(not 0<=x<=n for x in result):raise ValueError('invalid counts')
    return result

def generator(label,rounds,index,dataset,n):
    return dict(label=label,rounds=rounds,delta_index=index,delta_hex=delta(index).hex(),dataset_index=dataset,sample_count=n,generator_input=sample_input(label,rounds,index,dataset).hex())

def search(worker,deadline,checkpoint=None):
    result=dict(**identity(),status='incomplete',classification='permutation differential search and independent holdout',completed={'discovery':0,'holdout':0},planned={'discovery':330,'holdout':12},selected=[],holdout=[])
    def flush():
        if checkpoint:save(checkpoint,result)
    try:
        for rounds in (1,2,4,8,12,16):
            candidates=[]
            for index in range(55):
                counts=count(worker,'discovery',rounds,index,0,1024,deadline)
                candidates.extend((abs(2*k-1024),index,bit,k) for bit,k in enumerate(counts))
                result['completed']['discovery']+=1;flush()
            top=sorted(candidates,key=lambda x:(-x[0],x[1],x[2]))[:2]
            for bias,index,bit,k in top:result['selected'].append(dict(**generator('discovery',rounds,index,0,1024),output_bit=bit,count=k,absolute_bias=bias/2048))
            flush()
        for ordinal,candidate in enumerate(result['selected']):
            r=candidate['rounds'];i=candidate['delta_index'];bit=candidate['output_bit']
            counts=count(worker,'holdout',r,i,ordinal,16384,deadline)
            result['holdout'].append(dict(**generator('holdout',r,i,ordinal,16384),output_bit=bit,**statistics(counts[bit],16384,12)))
            result['completed']['holdout']+=1;flush()
        result['status']='complete'
    except TimeoutError as e:result['interruption']=str(e)
    flush();return result

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('worker');p.add_argument('--output',required=True);p.add_argument('--budget',type=float,default=1800);a=p.parse_args()
    result=search(a.worker,time.monotonic()+min(1800,a.budget),a.output)
    print(json.dumps({k:result[k] for k in ('status','completed','planned')}))
