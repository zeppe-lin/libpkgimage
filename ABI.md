libpkgimage ABI
===============

0.3.0
-----

`libpkgimage 0.3.0` changes the shared-library ABI generation from 0 to 1.

The change is required because the release:

* extends the public `package_entry`, `package_image`, and `entry_selection`
  object layouts;
* adds identity and receipt value types;
* adds a virtual `package_archive::inspection_receipt()` member;
* replaces the canonical backend `open()` request contract;
* changes `archive_backend::inspect()` from a bare `package_image` to an
  `inspected_package_image`; and
* strengthens regular-entry construction and selection invariants.

Keeping soversion 0 would allow binaries built against 0.2.x to load an
incompatible object layout and virtual interface. The project version alone is
not a safe ABI boundary.

Shared and static builds remain separate. Static pkg-config closure includes
both `libarchive` and `libcrypto` through `Requires.private`; neither dependency
leaks an implementation type into installed public headers.
