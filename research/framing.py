import argparse,hashlib,itertools,json,subprocess,sys,time
from common import ROOT,identity,save
sys.path.insert(0,str(ROOT/'tests'))
from reference import encrypt

def pattern(n,index):return bytes(n) if index==0 else b'\xff'*n if index==1 else bytes(range(n))
def expected_encoding(ad,p):
    encoded=b''
    for data,full,tail in ((ad,0x11,0x12),(p,0x21,0x22)):
        for j in range(0,len(data)//16*16,16):encoded+=bytes([full])+data[j:j+16]
        t=data[len(data)//16*16:];encoded+=bytes([tail])+t+b'\x80'+bytes(15-len(t))
    return encoded

def decode(encoded):
    if len(encoded)%17:raise ValueError('frame width')
    parts=[bytearray(),bytearray()];phase=0
    for j in range(0,len(encoded),17):
        domain=encoded[j];block=encoded[j+1:j+17]
        if phase>1:raise ValueError('extra event')
        full,tail=((0x11,0x12),(0x21,0x22))[phase]
        if domain==full:parts[phase].extend(block)
        elif domain==tail:
            end=len(block.rstrip(b'\0'))-1
            if end<0 or block[end]!=0x80:raise ValueError('padding')
            parts[phase].extend(block[:end]);phase+=1
        else:raise ValueError('phase domain')
    if phase!=2:raise ValueError('missing empty/final event')
    return tuple(map(bytes,parts))

def run(worker,deadline,full=True,checkpoint=None):
    lengths=range(34) if full else (0,1,15,16,17,31,32,33)
    fixtures=[(pattern(a,x),pattern(m,y)) for a,m,x,y in itertools.product(lengths,lengths,range(3),range(3))]
    d=dict(**identity(),status='incomplete',planned=len(fixtures),completed=0,patterns=['zero','ff','incrementing'],pattern_cross_product=True,unique_encodings=0,classification='framing event regression, not a mode security proof')
    seen={};digest=hashlib.sha256()
    try:
        for start in range(0,len(fixtures),128):
            left=deadline-time.monotonic()
            if left<=0:raise TimeoutError('framing budget exhausted')
            batch=fixtures[start:start+128]
            try:out=subprocess.run([str(worker)],input=''.join(a.hex()+','+p.hex()+'\n' for a,p in batch),text=True,capture_output=True,check=True,timeout=left).stdout.splitlines()
            except subprocess.TimeoutExpired as e:raise TimeoutError('framing worker budget exhausted') from e
            assert len(out)==len(batch)
            for (ad,p),line in zip(batch,out):
                row=json.loads(line);trace=[];c,t=encrypt(bytes(16),bytes(16),ad,p,trace)
                assert row['ciphertext']==c.hex() and row['tag']==t.hex()
                stages=[e for e in trace if e['event'].startswith(('ad-','message-'))]
                assert [e['after'] for e in row['events']]==[e['state'] for e in stages]
                encoded=b''
                for e in row['events']:
                    before=bytes.fromhex(e['before']);pre=bytes.fromhex(e['pre']);diff=bytes(a^b for a,b in zip(before,pre))
                    assert diff[16:47]==bytes(31) and diff[47]==e['domain']
                    encoded+=bytes([e['domain']])+diff[:16]
                assert encoded==expected_encoding(ad,p) and decode(encoded)==(ad,p)
                if encoded in seen:assert seen[encoded]==(ad,p)
                seen[encoded]=(ad,p);digest.update(line.encode()+b'\n');d['completed']+=1
            d['unique_encodings']=len(seen)
            if checkpoint:save(checkpoint,d)
        d['status']='complete'
    except TimeoutError as e:d['interruption']=str(e)
    d['transcript_sha256']=digest.hexdigest()
    if checkpoint:save(checkpoint,d)
    return d

def nonce_reuse(aead_runner):
    key=bytes(range(16));nonce=bytes(range(16,32));ad=bytes(range(33));p0=bytes(range(32));p1=bytes(x^0x55 for x in p0)
    rows=[subprocess.check_output([str(aead_runner),key.hex(),nonce.hex(),ad.hex(),p.hex()],text=True).splitlines() for p in (p0,p1)]
    c0,c1=(bytes.fromhex(row[0]) for row in rows)
    assert bytes(a^b for a,b in zip(c0[:16],c1[:16]))==bytes(a^b for a,b in zip(p0[:16],p1[:16]))
    return dict(classification='forbidden nonce-reuse control',queries=2,plaintext_bytes=32,first_block_xor_relation=True,unique_nonce_attack=False)

def selftest():
    for bad in (b'',bytes([0x12])+bytes(16),expected_encoding(b'',b'')[:-17],bytes([0x22])+expected_encoding(b'',b'')[1:]):
        try:decode(bad)
        except ValueError:pass
        else:raise AssertionError('malformed phase/padding accepted')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('worker');p.add_argument('aead_runner');p.add_argument('--smoke',action='store_true');p.add_argument('--output',required=True);a=p.parse_args()
    selftest();d=run(a.worker,time.monotonic()+1800,not a.smoke,a.output);d['nonce_reuse']=nonce_reuse(a.aead_runner);save(a.output,d)
    print(json.dumps({k:d[k] for k in ('status','planned','completed','unique_encodings')}))
