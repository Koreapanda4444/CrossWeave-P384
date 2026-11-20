import argparse,json,pathlib,platform,statistics,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'research'))
from common import identity,save

def command(args):
    try:return subprocess.check_output(args,text=True,stderr=subprocess.STDOUT).strip()
    except (OSError,subprocess.CalledProcessError):return 'unavailable'

def run(executable,build,algorithms):
    flags=(build/'CMakeFiles/crossweave.dir/flags.make')
    cpu=platform.processor()
    cpuinfo=pathlib.Path('/proc/cpuinfo')
    if cpuinfo.exists():cpu=next((s.split(':',1)[1].strip() for s in cpuinfo.read_text().splitlines() if s.startswith('model name')),cpu)
    d=dict(**identity(),status='complete',environment=dict(cpu=cpu,os=platform.platform(),compiler=command(['cc','--version']).splitlines()[0],flags=flags.read_text() if flags.exists() else 'See CMake configuration',openssl=command(['openssl','version','-a']),openssl_cpu_settings=command(['openssl','info','-cpusettings']),hardware_acceleration='OpenSSL default CPU dispatch; selected instruction path not independently measured; CrossWeave portable C with compiler optimizations',warmup_calls={'aead':8,'permutation':32},samples=5,timer='monotonic platform clock',key_nonce_policy='Fresh random key per benchmark invocation; strictly increasing counter nonces across warmup and every encryption; generation outside timed loops'),rows=[])
    for name in ('P12','P16'):
        row=json.loads(subprocess.check_output([str(executable),name,'0','0'],text=True));d['rows'].append(row)
    for algorithm in algorithms:
        for a in (0,32,1024):
            for m in (0,16,64,1024,16384,1048576):
                row=json.loads(subprocess.check_output([str(executable),algorithm,str(a),str(m)],text=True))
                for op in ('encrypt','decrypt','failure'):
                    values=row[op+'_ns'];median=statistics.median(values)
                    row[op+'_summary']=dict(median_ns=median,min_ns=min(values),max_ns=max(values),message_mib_s=m/(median/1e9)/1048576 if m else None)
                d['rows'].append(row)
    archive=build/'libcrossweave.a';d['object_size']=dict(tool='GNU size Berkeley text column includes code/read-only data; archive size also includes metadata',raw=command(['size',str(archive)]),archive_bytes=archive.stat().st_size if archive.exists() else None)
    return d
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('executable',type=pathlib.Path);p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--algorithms',nargs='+',default=['CrossWeave']);a=p.parse_args()
    d=run(a.executable.resolve(),a.build.resolve(),a.algorithms);save(a.output,d);print(json.dumps({'status':d['status'],'rows':len(d['rows']),'output':str(a.output)}))
