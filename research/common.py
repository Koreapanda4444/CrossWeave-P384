import hashlib,json,pathlib,struct,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
SEED=0x43574e4f56303926

def sample_input(label,rounds,delta_index,dataset_index):
    return b'CrossWeave/research/'+label.encode('ascii')+b'\0'+struct.pack('<QIII',SEED,rounds,delta_index,dataset_index)

def samples(label,rounds,delta_index,dataset_index,n):
    return hashlib.shake_256(sample_input(label,rounds,delta_index,dataset_index)).digest(48*n)

def identity():
    paths=sorted(list((ROOT/'src').glob('*'))+list((ROOT/'include').glob('*.h')))
    digest=hashlib.sha256()
    for p in paths:digest.update(str(p.relative_to(ROOT)).encode()+b'\0'+p.read_bytes())
    return {'core_git_sha':subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip(),'core_source_sha256':digest.hexdigest(),'seed':hex(SEED)}

def save(path,data):
    path=pathlib.Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(data,indent=2,allow_nan=False)+'\n')
