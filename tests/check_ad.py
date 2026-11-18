import subprocess,sys
from reference import initialize,absorb_ad
for n in (0,1,15,16,17,31,32,33):
    trace=[]; state=initialize(bytes(range(16)),bytes(range(16,32)),trace)
    absorb_ad(state,bytes(range(n)),trace)
    actual=subprocess.check_output([sys.argv[1],str(n)],text=True).splitlines()
    assert actual==[row['state'] for row in trace],n
print('Initialization/key XOR and every AD stage match for lengths 0/1/15/16/17/31/32/33')
