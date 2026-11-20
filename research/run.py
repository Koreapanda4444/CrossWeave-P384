import argparse,hashlib,json,pathlib,subprocess,time
from common import identity,save
import local_relations,fixed_hypothesis,experiments,framing

def run(args):
    start=time.monotonic();deadline=start+min(1800,max(0,args.budget));full=args.mode=='full'
    folder=args.output.parent;folder.mkdir(parents=True,exist_ok=True)
    result=dict(**identity(),status='incomplete',mode=args.mode,budget_seconds=min(1800,max(0,args.budget)),components={},runner_sha256={})
    for p in (args.worker,args.local_runner,args.framing_runner,args.omitted_ad):result['runner_sha256'][p.name]=hashlib.sha256(p.read_bytes()).hexdigest()
    def flush():result['elapsed_seconds']=time.monotonic()-start;save(args.output,result)
    def remaining():
        left=deadline-time.monotonic()
        if left<=0:raise TimeoutError('overall research budget exhausted')
        return left
    try:
        result['components']['omitted_ad']=json.loads(subprocess.check_output([str(args.omitted_ad)],timeout=remaining()))
        result['components']['local']=local_relations.run(args.local_runner,full);remaining();flush()
        result['components']['fixed']=fixed_hypothesis.run(args.worker,deadline,full,folder/('fixed-'+args.mode+'.json'));flush()
        if full:
            result['components']['search']=experiments.search(args.worker,deadline,folder/'search-full.json');flush()
        framing.selftest();remaining()
        result['components']['framing']=framing.run(args.framing_runner,deadline,full,folder/('framing-'+args.mode+'.json'));flush()
        result['components']['nonce_reuse']=framing.nonce_reuse(args.aead_runner);remaining()
        result['status']='complete' if all(c.get('status','complete')=='complete' for c in result['components'].values()) else 'incomplete'
    except (TimeoutError,subprocess.TimeoutExpired) as e:result['interruption']=str(e)
    flush();return result

def selected(result):
    out=json.loads(json.dumps(result));local=out['components'].get('local',{})
    if 'rows' in local:
        rows=local.pop('rows');local['checked_positions']=len(rows)
        local['all_position_histogram_totals']={key:[sum(row['counts'][key][j] for row in rows) for j in range(2)] for key in ('a','b')}
        local['full_rows_location']='Reproduce with research-full; complete raw result is written under build/'
    return out
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--mode',choices=['smoke','full'],required=True)
    for name in ('worker','local-runner','framing-runner','omitted-ad','aead-runner'):p.add_argument('--'+name,type=pathlib.Path,required=True)
    p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--selected',type=pathlib.Path);p.add_argument('--budget',type=float,default=1800);a=p.parse_args()
    result=run(a)
    if a.selected:save(a.selected,selected(result))
    print(json.dumps({'status':result['status'],'mode':a.mode,'elapsed_seconds':result['elapsed_seconds'],'components':list(result['components'])}))
    raise SystemExit(0 if result['status']=='complete' else 2)
