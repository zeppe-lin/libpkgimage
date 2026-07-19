libpkgimage design
==================

Purpose
-------

`libpkgimage` normalizes package archive contents into backend-neutral facts
and transports selected regular-file payload bytes.

It owns:

```text
archive path normalization
entry type and metadata normalization
ordered image validation
stable entry identity
selected payload replay
```

It does not own package identity, build expectations, installed ownership,
installation policy, filesystem deployment, or package transactions.

Architectural boundary
----------------------

```text
archive transport
      |
      v
archive_backend
      |
      v
package_archive
      |-- package_image
      `-- payload replay
```

Archive libraries are replaceable behind `archive_backend`. Consumers see no
libarchive types and do not parse archive headers themselves.

Package path model
------------------

`package_path` is archive data interpreted under package-root semantics.

Parsing:

* rejects empty input;
* rejects absolute paths;
* rejects NUL, carriage-return, and newline bytes;
* removes empty components;
* removes `.` components;
* rejects every `..` component; and
* removes trailing separators.

The result is a canonical root-relative spelling.

The class deliberately does not use host `std::filesystem::path`
normalization. Package paths are not host paths and must not inherit host
platform interpretation.

Directory identity belongs to entry type, not to a trailing separator in the
canonical path.

Queries provide:

```text
string()
filename()
parent()
is_ancestor_of()
```

Entry model
-----------

A `package_entry` contains backend-neutral metadata for one explicit archive
entry:

```text
id
path
type
mode
uid
gid
size
mtime
mtime nanoseconds
optional symlink target
optional hardlink target
optional device number
```

Supported entry types are:

```text
regular
directory
symlink
hardlink
fifo
character_device
block_device
```

Sockets and other unsupported archive types are rejected.

The symbolic-link target is retained as uninterpreted text after line-safety
validation. It is not resolved against an installation root.

Hard-link targets are canonical package paths.

Image model
-----------

`package_image` is an immutable ordered collection after construction.

Construction:

* requires at least one explicit archive entry;
* preserves archive order;
* replaces incoming identifiers with consecutive identifiers from zero;
* validates type-specific metadata;
* rejects duplicate canonical paths;
* validates hard-link target existence; and
* requires every hard-link target to be a regular entry.

Explicit empty directories remain entries. Parent directories required only
to reach children are not synthesized.

Archive order is part of the model because hard-link and payload semantics can
depend on it.

Type-specific invariants
------------------------

Symlinks:

* require a non-empty target;
* reject NUL, carriage-return, and newline bytes; and
* reject a symlink target on any other entry type.

Hard links:

* require a target;
* may not target themselves;
* require the target path to exist in the same image; and
* require the target entry to be regular.

Payload size:

* is meaningful only for regular entries; and
* must be zero for every other entry type.

Device numbers:

* are required for character and block devices; and
* are rejected for every other type.

Modification time nanoseconds must be in the range 0 through 999999999.

Stable identifiers
------------------

`entry_id` is an image-local stable identifier.

Identifiers are consecutive in archive order and are valid only with the image
whose construction assigned them.

The invalid sentinel exists only before admission into an image.

A consumer must not treat an identifier as a package-global or
archive-file-global identity.

Errors
------

All public failures derive from `pkgimage::error`:

```text
path_error             invalid package path
manifest_error         invalid entry or image invariant
selection_error        invalid payload selection
archive_error          source, format, decoding, or I/O failure
source_changed_error   retained archive source changed
```

Consumers should select behavior from the exception type and not parse
diagnostic text.

Determinism
-----------

For the same normalized archive contents:

* entry order is archive order;
* identifiers are consecutive from zero;
* path lookup is exact;
* selection membership is deterministic; and
* replay event order is archive order.

The library does not sort archives into pathname order.
