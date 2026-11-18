import json,pathlib,subprocess,sys
from reference import encrypt,decrypt,fixture
path=pathlib.Path(__file__).with_name('vectors.json');before=path.read_bytes();v=json.loads(before)
lengths=(0,1,15,16,17,31,32,33,255,256,257,1024)
assert len(v['aead_boundary'])==144
assert {(len(bytes.fromhex(r['ad'])),len(bytes.fromhex(r['plaintext']))) for r in v['aead_boundary']}=={(a,m) for a in lengths for m in lengths}
assert len({r['nonce'] for r in v['aead_boundary']})==144
for row in v['aead_fixed']+v['aead_boundary']:
    key,nonce,ad,p,c,t=(bytes.fromhex(row[x]) for x in ('key','nonce','ad','plaintext','ciphertext','tag'))
    if 'index' in row:
        i=row['index'];assert nonce==i.to_bytes(16,'little')
        for field,label in row['labels'].items(): assert bytes.fromhex(row[field])==fixture(label,i,len(bytes.fromhex(row[field])))
    trace=[]; assert encrypt(key,nonce,ad,p,trace)==(c,t)
    assert decrypt(key,nonce,ad,c,t)==p
    for prefix,n in [('ad',len(ad)),('message',len(p))]:
        assert len([e for e in trace if e['event']==prefix+'-full'])==n//16
        assert [e['length'] for e in trace if e['event']==prefix+'-tail']==[n%16]
    actual=subprocess.check_output([sys.argv[1],key.hex(),nonce.hex(),ad.hex(),p.hex()],text=True).splitlines()
    assert actual==[c.hex(),t.hex(),p.hex()],row.get('index',row.get('name'))
assert path.read_bytes()==before
print('146 immutable AEAD KATs: 144 boundary pairs + 2 normative; C/Python, decrypt, in-place, padding traces')
