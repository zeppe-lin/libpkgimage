libpkgimage
===========

`libpkgimage` is the Zeppe-Lin C++17 library for inspecting package archives
as normalized package images and replaying selected regular-file payloads.

It provides:

* canonical root-relative package paths;
* typed archive entries;
* ordered and validated package images;
* stable entry identifiers;
* typed complete-archive and regular-content digests;
* canonical package-image identities;
* immutable archive-inspection receipts;
* immutable regular-file selections;
* backend-neutral payload sinks;
* stable inspected archive objects; and
* a tar backend implemented with `libarchive`.

The central distinction is:

```text
exact package archive bytes
        |
        v
archive inspection
        |-- immutable package_image
        |-- package_image_identity
        `-- archive_inspection_receipt
```

A package image states what an archive contains. It does not state what is
installed, what should be installed, or whether a build footprint matches.

`libpkgimage` does not:

* identify packages from archive filenames;
* parse or write `.footprint` files;
* decide footprint mismatches;
* evaluate installation policy;
* choose installation destinations;
* mutate a live filesystem;
* stage rejected objects;
* update installed package state; or
* coordinate complete package transactions.

The implementation is original Zeppe-Lin code. It is not derived from CRUX
`pkgutils` or from the former CRUX-derived `libpkgcore`.

Contracts
---------

The public flow is:

```text
archive_backend::open()
          |
          v
    package_archive
      |-- immutable package_image
      |-- archive_inspection_receipt
      `-- replay(entry_selection, payload_sink)
```

Important invariants:

* package paths are canonical and relative to the package root;
* archive order is preserved;
* explicit directories remain explicit entries;
* structural parent directories are not synthesized;
* canonical paths are unique inside one image;
* entry identifiers are consecutive in archive order;
* hard links target regular entries in the same image;
* selections contain regular entries only;
* replay order follows archive order, not selection input order; and
* payload replay is transport, not a transaction.

The libarchive backend retains an open descriptor for the inspected regular
file. Pathname replacement or unlinking does not redirect later replay to a
different inode. Source stamps, complete-byte checks around replay, normalized
header comparison, and selected-content identities enforce the documented
retained-source boundary. The library does not claim adversarial immutability
for a mutable open file descriptor.

See:

* `DESIGN.md` for the package path, entry, image, and error contracts;
* `IDENTITY.md` for archive, content, image, and receipt identity records;
* `REPLAY.md` for selection, sink, and source-stability semantics;
* `BACKENDS.md` for archive backend and libarchive behavior;
* `INTEGRATION.md` for boundaries with footprints, build, and installed state;
* `TESTING.md` for the verification doctrine; and
* `HISTORY.md` for project lineage.

Manual pages
------------

The installed manual suite is:

* `libpkgimage(3)` — library overview and boundaries;
* `pkgimage_model(3)` — paths, entries, images, and identifiers;
* `pkgimage_identity(3)` — digest domains and inspection evidence;
* `pkgimage_archive(3)` — archive backends and inspected archives;
* `pkgimage_replay(3)` — selections, payload sinks, and partial consumption;
* `pkgimage_libarchive_backend(3)` — tar backend and source-stability rules.

Requirements
------------

Build-time requirements:

* a C++17 compiler;
* Meson 1.2.0 or later;
* Ninja;
* pkg-config; and
* libarchive headers and library;
* OpenSSL libcrypto headers and library.

Optional documentation dependencies:

* `scdoc` for manual pages; and
* Doxygen for the generated API reference.

Building
--------

Shared library:

```sh
meson setup build
meson compile -C build
meson test -C build --print-errorlogs
```

Static library with static dependencies:

```sh
meson setup build-static \
  -Ddefault_library=static \
  -Dlink_mode=static
meson compile -C build-static
meson test -C build-static --print-errorlogs
```

The project rejects `default_library=both`. Shared and static artifacts are
separate builds, and each build links the corresponding form of its
dependencies.

Useful options:

```sh
meson setup build -Dtests=disabled
meson setup build -Dman_pages=disabled
```

Manual pages use `man_pages=auto` by default and are built when `scdoc` is
available.

Installation:

```sh
meson install -C build
```

Using the library
-----------------

```cpp
#include <cstddef>
#include <iostream>

#include <libpkgimage/entry_selection.h>
#include <libpkgimage/libarchive_backend.h>
#include <libpkgimage/payload_sink.h>

class sink final : public pkgimage::payload_sink {
public:
  void begin(const pkgimage::package_entry& entry) override
  {
    // Prepare staged storage for entry.path.
  }

  void write(const pkgimage::package_entry&,
             const std::byte* data,
             std::size_t size) override
  {
    // Consume exact binary payload bytes.
  }

  void end(const pkgimage::package_entry&) override
  {
    // Complete this staged payload.
  }
};

pkgimage::libarchive_backend backend;
auto archive = backend.open("example#1.0-1.pkg.tar.xz");

std::cout << archive->inspection_receipt().archive_digest().string() << '\n';
std::cout << archive->image().identity().string() << '\n';

for (const pkgimage::package_entry& entry : archive->image().entries())
  std::cout << entry.id << ' ' << entry.path << '\n';

sink destination;
archive->replay(
    pkgimage::entry_selection::all_regular(archive->image()),
    destination);
```

A sink may have consumed bytes before replay fails. Consumers that require
all-or-nothing installation must stage payloads and provide their own
transaction boundary.

Compiler and linker flags:

```sh
pkg-config --cflags --libs libpkgimage
pkg-config --static --libs libpkgimage
```

API documentation
-----------------

Public interfaces are documented with Doxygen comments under
`include/libpkgimage`.

```sh
doxygen Doxyfile
```

Generated HTML is written to `build/docs/html`.

Layout
------

* `include/libpkgimage/` — installed public API;
* `src/` — library and libarchive backend;
* `tests/` — model, backend, replay, header, and documentation tests;
* `man/` — scdoc manual sources; and
* `.github/workflows/` — build and documentation gates.

License
-------

`libpkgimage` is licensed under the GNU General Public License version 3 or
later. See `COPYING` for the license terms and `COPYRIGHT` for notices.
