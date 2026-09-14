## Summary

Describe the problem, implementation, observable effects, and verification.

## Checklist

- [ ] Debug and Release builds pass on the applicable supported toolchains.
- [ ] The complete CTest suite passes.
- [ ] Project warnings remain at zero.
- [ ] Deterministic semantics and FIFO ordering are preserved.
- [ ] No hidden allocation was introduced.
- [ ] No hidden time or randomness was introduced.
- [ ] Architecture and compatibility impact is documented.
- [ ] New behaviour or a bug fix is covered by tests.
- [ ] Public API changes are documented.
- [ ] Frozen primitive semantics are unchanged, or this is explicitly marked as an architecture proposal.
- [ ] Validation guarantees were not weakened or bypassed.
