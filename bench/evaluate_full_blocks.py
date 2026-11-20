"""Reproduce the discarded LE64 experiment in a generated build-only source tree."""
import argparse,pathlib,shutil,subprocess,hashlib,json
ROOT=pathlib.Path(__file__).resolve().parents[1]

def candidate(text):
    old='    for(size_t j=0;j<16;++j) s[j]^=block[j];'
    assert text.count(old)==1
    text=text.replace(old,'    uint64_t a=cw_load64(block),b=cw_load64(block+8);\n    cw_store64(s,cw_load64(s)^a);cw_store64(s+8,cw_load64(s+8)^b);')
    old='''    for(size_t j=0;j<n;++j) {
        uint8_t input=in[j], output=(uint8_t)(input^s[j]);
        out[j]=output; s[j]=decrypt?input:output;
    }'''
    new='''    if(full && n==16u) {
        uint64_t a=cw_load64(in),b=cw_load64(in+8);
        uint64_t x=a^cw_load64(s),y=b^cw_load64(s+8);
        cw_store64(out,x);cw_store64(out+8,y);
        cw_store64(s,decrypt?a:x);cw_store64(s+8,decrypt?b:y);
    } else {
        for(size_t j=0;j<n;++j) {
            uint8_t input=in[j], output=(uint8_t)(input^s[j]);
            out[j]=output; s[j]=decrypt?input:output;
        }
    }'''
    assert text.count(old)==1
    return text.replace(old,new)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--directory',type=pathlib.Path,default=ROOT/'build/full-block-trial');a=p.parse_args();dest=a.directory.resolve()
    if not dest.is_relative_to(ROOT/'build'):raise ValueError('experiment directory must be under build/')
    source=dest/'source';build=dest/'build';source.mkdir(parents=True,exist_ok=True)
    for name in ('src','include','tests','research','bench'):shutil.copytree(ROOT/name,source/name,dirs_exist_ok=True,ignore=shutil.ignore_patterns('__pycache__'))
    shutil.copyfile(ROOT/'CMakeLists.txt',source/'CMakeLists.txt')
    path=source/'src/crossweave_aead.c';path.write_text(candidate(path.read_text()))
    print(json.dumps({'candidate_aead_sha256':hashlib.sha256(path.read_bytes()).hexdigest()}),flush=True)
    commands=[['cmake','-S',str(source),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release','-DBUILD_TESTING=ON','-DCRYPTO_BUILD_RESEARCH=ON','-DCRYPTO_BUILD_BENCHMARKS=ON'],['cmake','--build',str(build),'--config','Release','--parallel','2'],['ctest','--test-dir',str(build),'-C','Release','--output-on-failure'],['cmake','--build',str(build),'--config','Release','--target','bench']]
    for command in commands:subprocess.run(command,check=True)
