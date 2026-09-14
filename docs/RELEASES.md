# Release policy

Nexum intends to use Semantic Versioning once releases begin:

* **Major**: incompatible changes to a declared stable public contract.
* **Minor**: backward-compatible capabilities or newly stabilized interfaces.
* **Patch**: backward-compatible fixes and documentation/build corrections.

Release tags use `vX.Y.Z`. Pre-release suffixes may be used when explicitly planned.
No release exists merely because a version appears in CMake.

Before creating a tag, maintainers must verify:

* CI is green on supported desktop toolchains in Debug and Release.
* ASan and UBSan workflows are green.
* The bounded fuzz workflow is green.
* Project warnings are zero and all tests pass.
* `CHANGELOG.md` is updated.
* `LICENSE` is present and packaging includes it.
* Security-sensitive changes received an appropriate review.
* Public API and compatibility effects are documented.

The prepared release workflow triggers only for version tags, tests before packaging,
and creates simple source/header/static-library artifacts. It does not publish to a
package manager. Tag creation and release approval remain deliberate maintainer acts.
