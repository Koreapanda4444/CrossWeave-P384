import subprocess,sys
from reference import fixture,permute
cases=[]

def add(data,rounds,label,index):
    fw=permute(data,rounds); inv=permute(data,rounds,True)
    for mode,state,expected in [('p',data,fw),('inverse',data,inv),('inverse',fw,data),('p',inv,data)]:
        cases.append((f'{mode} {rounds} {state.hex()}\n',expected.hex(),label,index,rounds,mode))
fixed=[bytes(48),bytes([255])*48,bytes(range(48)),bytes([128])*48]
for i,data in enumerate(fixed+[fixture('permutation',j,48) for j in range(1000)]):
    for n in (12,16): add(data,n,'permutation/fixed' if i<4 else 'permutation',i if i<4 else i-4)
for i in range(80):
    for n in range(17): add(fixture('prefix',i,48),n,'prefix',i)
out=subprocess.run([sys.argv[1]],input=''.join(x[0] for x in cases),capture_output=True,text=True,check=True).stdout.splitlines()
assert len(out)==len(cases),(len(out),len(cases))
for actual,case in zip(out,cases):
    assert actual==case[1],{'label':case[2],'index':case[3],'rounds':case[4],'mode':case[5],'input':case[0],'expected':case[1],'actual':actual}
print(f'1004 fixed/full states and 80 x 17 prefix states: {len(cases)} C/Python and bidirectional checks')
