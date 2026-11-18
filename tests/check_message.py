import subprocess,sys
from reference import encrypt
for a in (0,1,15,16,17,31,32,33):
    for m in (0,1,15,16,17,31,32,33):
        trace=[]; c,t=encrypt(bytes(range(16)),bytes(range(16,32)),bytes(range(a)),bytes((7*i+3)%256 for i in range(m)),trace)
        expected=[c.hex(),t.hex()]+[row['state'] for row in trace if row['event'].startswith('message-') or row['event']=='final']
        actual=subprocess.check_output([sys.argv[1],str(a),str(m)],text=True).splitlines()
        assert actual==expected,(a,m)
print('64 boundary pairs: ciphertext, tag, all feedback/final states; both decryption paths')
