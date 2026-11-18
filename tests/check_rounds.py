import subprocess, sys
from reference import permute,round_state,lanes,serialize,fixture
for data in (bytes(48),bytes([255])*48,bytes(range(48)),fixture('prefix',0,48)):
    for r in (0,11,15):
        actual=subprocess.check_output([sys.argv[1],'round',str(r),data.hex()],text=True).strip()
        assert actual==serialize(round_state(lanes(data),r)).hex()
    for n in (12,16):
        actual=subprocess.check_output([sys.argv[1],'p',str(n),data.hex()],text=True).strip()
        assert actual==permute(data,n).hex()
print('12 round states and 8 fixed permutations match independent equations')
