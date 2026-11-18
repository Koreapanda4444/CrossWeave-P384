import hashlib,json,pathlib,subprocess,sys
from reference import permute
path=pathlib.Path(__file__).with_name('vectors.json')
before=path.read_bytes(); data=json.loads(before)
for row in data['permutation']:
    state=bytes.fromhex(row['input'])
    for n in (12,16):
        expected=row[f'p{n}']
        assert permute(state,n).hex()==expected,row['name']
        got=subprocess.check_output([sys.argv[1],'p',str(n),state.hex()],text=True).strip()
        assert got==expected,(row['name'],n,got)
assert path.read_bytes()==before
print('8 immutable permutation KATs passed; vectors SHA256='+hashlib.sha256(before).hexdigest())
