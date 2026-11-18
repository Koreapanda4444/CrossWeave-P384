import subprocess, sys
from reference import fixture,permute

def run(mode,n,data):
    return bytes.fromhex(subprocess.check_output([sys.argv[1],mode,str(n),data.hex()],text=True).strip())
for i in range(8):
    data=fixture('prefix',i,48)
    for n in range(17):
        forward=run('p',n,data); backward=run('inverse',n,data)
        assert forward==permute(data,n)
        assert backward==permute(data,n,True)
        assert run('inverse',n,forward)==data
        assert run('p',n,backward)==data
print('136 prefixes: C/Python forward/inverse and both round trips')
