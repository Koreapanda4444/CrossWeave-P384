from reference import encrypt,decrypt
vectors=[
    ('f38f855b4280efb6c827a2361a03979e','00'*16,'','','','7c7d20f684fe8f6d80ae7d20c78cc3cf'),
    ('00'*16,bytes(range(16)).hex(),bytes(range(17)).hex(),bytes(range(17)).hex(),
     'd9284e96da9b49763c268a2abc39abf2c0','c1157d9abecbade2ca62f2611adc0c4a')]
for row in vectors:
    key,nonce,ad,msg,c,t=map(bytes.fromhex,row)
    trace=[]; assert encrypt(key,nonce,ad,msg,trace)==(c,t)
    backward=[]; assert decrypt(key,nonce,ad,c,t,backward)==msg
    assert trace==backward
    assert [e['length'] for e in trace if e['event']=='ad-tail']==[len(ad)%16]
    assert [e['length'] for e in trace if e['event']=='message-tail']==[len(msg)%16]
for n in (0,16,32):
    trace=[]; encrypt(bytes(16),bytes(16),bytes(n),bytes(n),trace)
    assert sum(e['event']=='ad-tail' and e['length']==0 for e in trace)==1
    assert sum(e['event']=='message-tail' and e['length']==0 for e in trace)==1
try: decrypt(bytes(16),bytes(16),b'',b'',bytes(16))
except ValueError as e: assert str(e)=='authentication'
else: raise AssertionError('invalid tag accepted')
print('AEAD oracle: both normative KATs, encryption/decryption traces, mandatory empty tails')
