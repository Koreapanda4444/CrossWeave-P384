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
