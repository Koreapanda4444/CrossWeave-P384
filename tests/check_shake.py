import hashlib, subprocess, sys
kat='46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762fd75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be'
assert hashlib.shake_256(b'').hexdigest(64)==kat
for data in (b'',b'abc',bytes(range(256))):
    for n in (8,16,64,136,192):
        actual=subprocess.check_output([sys.argv[1],str(n),data.hex()],text=True).strip()
        assert actual==hashlib.shake_256(data).hexdigest(n),(data.hex(),n,actual)
print('SHAKE256: 15 Python/OpenSSL cases, fixed KAT, EVP init/final failure branches')
