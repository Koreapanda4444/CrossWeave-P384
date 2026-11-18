"""Print the normative public C arrays; never changes the tracked files."""
import hashlib

def constants():
    return [[int.from_bytes(hashlib.shake_256(f'cw-p384/round-{r}/lane-{j}'.encode('ascii')).digest(8),'little') for j in (0,3)] for r in range(16)]

def iv():
    return hashlib.shake_256(b'CrossWeave/IV').digest(16)

if __name__=='__main__':
    print('#include "crossweave_internal.h"')
    print('const uint64_t cw_rc[16][2] = {')
    for pair in constants(): print('    {'+', '.join(f'UINT64_C(0x{x:016x})' for x in pair)+'},')
    print('};\nconst uint8_t cw_iv[16] = {')
    print('    '+', '.join(f'0x{x:02x}' for x in iv()))
    print('};')
