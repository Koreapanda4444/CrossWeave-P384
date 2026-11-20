import argparse,json,time
from common import identity,save
from experiments import count,generator,statistics

def run(worker,deadline,full=True,checkpoint=None):
    n=16384 if full else 512;rounds=(2,4,8,12,16) if full else (2,);datasets=range(8) if full else range(1)
    result=dict(**identity(),status='incomplete',classification='pre-registered permutation output bit hypothesis',input_bit=383,output_bit=1,planned=len(rounds)*len(datasets),completed=0,rows=[])
    if checkpoint:save(checkpoint,result)
    try:
        for r in rounds:
            for d in datasets:
                counts=count(worker,'fixed',r,47,d,n,deadline)
                result['rows'].append(dict(**generator('fixed',r,47,d,n),output_bit=1,**statistics(counts[1],n,40 if full else 1)))
                result['completed']+=1
                if checkpoint:save(checkpoint,result)
        result['status']='complete'
    except (TimeoutError,KeyboardInterrupt) as e:result['interruption']=str(e) or 'keyboard interruption'
    if checkpoint:save(checkpoint,result)
    return result
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('worker');p.add_argument('--smoke',action='store_true');p.add_argument('--output',required=True);p.add_argument('--budget',type=float,default=1800);a=p.parse_args()
    d=run(a.worker,time.monotonic()+min(1800,a.budget),not a.smoke,a.output)
    print(json.dumps({'status':d['status'],'completed':d['completed'],'two_round_counts':[r['count'] for r in d['rows'] if r['rounds']==2]}))
