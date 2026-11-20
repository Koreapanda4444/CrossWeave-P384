# CrossWeave-P384

C11 implementation and reproducible research baseline for a 384-bit permutation
and one AEAD candidate. The 36-step implementation/research plan is complete;
[progress and verification limits](docs/progress_ko.md) distinguish local tests,
remote CI, performance measurements, exact component properties and open questions.
Passing tests is not a cryptographic security proof or approval for real secrets.

## Build and reproduce

Requires C11, CMake 3.20+, OpenSSL 3.x development files and Python 3.11+ for the
validation/research drivers. Production code is a static library with no Python
runtime dependency. OpenSSL supplies SHAKE comparison, tag comparison, cleansing
and benchmark baselines. Constants are committed public arrays, not regenerated
for each key/message. Tests never overwrite the committed vectors.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCRYPTO_BUILD_RESEARCH=ON -DCRYPTO_BUILD_BENCHMARKS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target check-differential
cmake --build build --config Release --target research-smoke
cmake --build build --config Release --target research-full
cmake --build build --config Release --target bench
```

The complete configuration has 29 CTest entries. `check-differential` checks
13,472 deterministic permutation results/inversions and 400 AEAD cases. It is
implementation comparison, distinct from differential cryptanalysis.
The long research and benchmark targets are explicit, not default CTest steps.
`research-full` has a 1,800-second budget and saves completed counts/checkpoints;
a timeout is incomplete. Raw generated data stays under the chosen build directory.

```sh
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCRYPTO_BUILD_RESEARCH=ON -DCRYPTO_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

This workspace prevents LeakSanitizer process inspection. Its local ASan/UBSan
run uses `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`; this is a
local limit, not a project default. CI runs default leak detection on unrestricted
Linux runners. Windows MSVC, macOS AppleClang and Linux GCC/Clang run the actual
vectors/contracts/corpora, research smoke and benchmark correctness smoke.
See the [CI workflow](.github/workflows/ci.yml) and its matching commit's
[Actions run](https://github.com/Koreapanda4444/CrossWeave-P384/actions/workflows/ci.yml).

For the production library alone, set `BUILD_TESTING=OFF` and leave the two
research/benchmark options OFF. On single-configuration Unix builds the result
is `build/libcrossweave.a`; Windows uses the normal CMake configuration output.
Windows multi-configuration commands use `--config Release` and CTest `-C Release`.
If OpenSSL is outside standard search paths, supply `OPENSSL_ROOT_DIR` at configure
time. No OS-specific absolute dependency path is committed to the library build.

## Public API

`include/crossweave.h` declares exactly four functions: fixed P12/P16 permutation
calls and one-shot `crossweave_encrypt` / `crossweave_decrypt`. State is 48 bytes;
key, nonce and tag are 16 bytes each; rate is 16 bytes. AD and message lengths
are independently limited to 16 MiB. Ciphertext length equals plaintext length.

The API supports exact in-place data processing, rejects partial overlaps and
preserves all caller outputs on failure. Decryption releases plaintext after
full tag verification. Callers own their buffers and must provide a fresh nonce
for every encryption under a key; nonce management and replay protection are
outside this API. Full pointer/capacity/alias/error priorities are in the spec
and API document. Research hooks, inverse rounds and fault controls are private
to their targets. There is no streaming, GUI, server or file-encryption product.

## Evidence and scope

- [Normative specification and 36-step plan](docs/spec_ko.md)
- [API, ownership, cleanup and side-channel review limits](docs/api_ko.md)
- [Bounded security research and unresolved questions](docs/security_ko.md)
- [Measured performance, samples and comparison limits](docs/benchmark_ko.md)
- [Discarded full-block optimization and reproduction](docs/optimization_ko.md)
- [Plan numbers, actual commits and verification status](docs/progress_ko.md)

Selected small research results live in `research/results/`. Performance samples
and the rejected optimization decision live in `bench/results/`. Each record
includes a core source fingerprint; Git commit identity is also retained when
available. A source archive without Git can run the drivers and reports a null
Git identity alongside the actual source fingerprint.

The LE64 full-block candidate passed correctness/sanitizer/research checks but
was slower on the selected same-host workloads, so the bytewise production core
is retained. Benchmark wrappers include temporary payload buffering and OpenSSL
context costs. Their different key/nonce sizes and acceleration levels are stated.
Full-round security, key-lifetime/forgery bounds and protocol suitability remain
unresolved; low-round observations are not claimed as full AEAD attacks.

Source and Git history were written anew. No owner-selected distribution license
has been declared. OpenSSL 3 is an external Apache-2.0-licensed dependency; its
[license](https://github.com/openssl/openssl/blob/master/LICENSE.txt) is separate
from this project's licensing. No OpenSSL implementation is vendored here.
