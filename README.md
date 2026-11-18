# CrossWeave-P384

C11 research implementation of a 384-bit permutation and a single AEAD construction.
The normative Korean specification and the ordered 36-step development plan are in
[docs/spec_ko.md](docs/spec_ko.md). Implementation and verification currently stop at
the checkpoint recorded in [docs/progress_ko.md](docs/progress_ko.md).

This candidate has no established full-round security guarantee. Passing vectors
and implementation tests does not establish suitability for protecting secrets.

The project starts from new source and an independent Git root. No source, commits,
or license grant from an older implementation are inherited. No distribution
license has been selected by the owner.

## Build and verify

Requires a C11 compiler, CMake 3.20+, OpenSSL 3.x development files, and Python
3.11+ for tests. The production static library does not require Python.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The static library is `build/libcrossweave.a` on single-configuration Unix
builds. Its public header is `include/crossweave.h`: fixed P12/P16 calls and
one-shot authenticated encryption/decryption. Buffer and error contracts are
specified in section 5 of the normative specification. Callers must use a
unique 16-byte nonce for every encryption under the same key.

The step-20 checkpoint includes 20 CTest entries, immutable permutation and
AEAD vectors, 13,472 deterministic permutation comparisons, 144 boundary AEAD
pairs plus the two normative AEAD vectors, 799 argument cases, and isolated
allocation-failure/cleanup instrumentation. Instrumentation is linked only
into a separate test library; production code has no mutable test controls.

```sh
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DCRYPTO_ENABLE_SANITIZERS=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

This workspace blocks LeakSanitizer process inspection. Here, the initial
sanitizer run failed at that environmental check; repeating with
`ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1` passed all 20 entries.
Allocation accounting passed separately, but a successful LeakSanitizer run is
still pending. Do not disable leak detection by default on unrestricted hosts.

Steps 21–36 remain pending, including the additional AEAD differential corpus,
maximum-length execution, cross-platform CI, bounded cryptanalysis and
benchmarks. Their targets are not advertised as implemented at this checkpoint.
