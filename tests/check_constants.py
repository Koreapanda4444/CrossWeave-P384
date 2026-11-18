import pathlib, subprocess, sys
from generate_constants import constants, iv
actual=subprocess.check_output([sys.argv[1]],text=True).splitlines()
expected=[x.to_bytes(8,'little').hex() for row in constants() for x in row]+[iv().hex()]
assert actual==expected
assert iv().hex()=='8d65b4d7917d81bbd9803c03a1164043'
generated=subprocess.check_output([sys.executable,str(pathlib.Path(__file__).with_name('generate_constants.py'))],text=True)
assert generated==pathlib.Path(__file__).parents[1].joinpath('src/crossweave_constants.c').read_text()
print('272 bytes: Python/OpenSSL/committed C arrays match; domain and NUL negatives pass')
