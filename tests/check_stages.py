import subprocess, sys
from reference import updates,lanes,serialize,f,fixture
cases=[bytes(48),bytes([255])*48,bytes(range(48)),bytes([128])*48]+[fixture('permutation',i,48) for i in range(8)]
different=0
for data in cases:
    for r in (0,11,15):
        actual=subprocess.check_output([sys.argv[1],str(r),data.hex()],text=True).splitlines()
        final,trace=updates(lanes(data),r)
        assert actual==[x.hex() for x in trace],(r,data.hex())
        s=lanes(data)
        simultaneous=serialize([s[i]^f(s[(i+1)%6],s[(i+3)%6],r,i) for i in range(6)])
        different+=simultaneous!=serialize(final)
assert different>0
print('216 lane intermediate states; wrapping addition; simultaneous-update negative control')
