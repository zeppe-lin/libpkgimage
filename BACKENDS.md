libpkgimage backend contract
============================

Backend abstraction
-------------------

`archive_backend` separates archive transport from package-image semantics.

A backend must return the same normalized public model regardless of the
archive library used internally.

The public interface is:

```cpp
std::unique_ptr<package_archive>
open(const std::filesystem::path& filename) const;

package_image
inspect(const std::filesystem::path& filename) const;
```

`open()` retains stable payload access.

`inspect()` is a convenience operation that opens the archive, copies the
immutable image, and releases the stable source handle. The returned image
contains no replay capability.

A backend must not expose archive-library handles or types through the public
API.

Backend responsibilities
------------------------

A backend is responsible for:

* opening and validating the source;
* decoding the archive transport;
* mapping archive pathnames to `package_path`;
* mapping supported entry types;
* normalizing numeric metadata;
* constructing a validated `package_image`;
* retaining any source state required for replay; and
* reporting source, format, decoding, and I/O failures.

A backend is not responsible for:

* package identity inferred from filenames;
* build footprint interpretation;
* installation policy;
* destination path selection;
* filesystem mutation;
* rejected-object staging;
* installed-state updates; or
* complete package transactions.

Libarchive backend
------------------

`libarchive_backend` accepts regular tar archive files using filters enabled
explicitly by the implementation:

```text
none
gzip
bzip2
xz
lzip
zstd
```

The linked libarchive build must support the requested filter.

Only tar format is enabled.

The source pathname must open successfully and identify a regular file.
Directories, FIFOs, sockets, devices, and other non-regular sources are
rejected.

The backend reads through `pread(2)` from a retained descriptor and uses the
source size captured at inspection.

Entry normalization
-------------------

The backend accepts:

```text
regular files
explicit directories
symbolic links
hard links
FIFOs
character devices
block devices
```

Unsupported entry types are rejected with `manifest_error`.

Normalized fields include:

```text
canonical path
entry type
permission and special bits
numeric uid and gid
regular-file size
mtime and mtime nanoseconds
symlink target
hardlink target
device major and minor
```

Negative uid, gid, size, or device numbers are rejected.

Hard-link archive headers take hard-link semantics even when the underlying
archive file type is regular.

The completed `package_image` performs backend-independent validation after
normalization.

Source retention
----------------

The backend retains one open descriptor and one inspected source stamp.

See `REPLAY.md` and `pkgimage_replay(3)` for replay and source-change
semantics.

Backend replacement
-------------------

A future backend may use another archive library or transport, but it must
preserve:

* canonical package path semantics;
* supported public entry meanings;
* archive order;
* stable image-local identifiers;
* image validation;
* selection validation;
* sink event ordering;
* exact payload byte transport; and
* explicit source and replay failures.

Backend-specific extensions must not silently alter the normalized public
meaning.
