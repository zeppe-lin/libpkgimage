libpkgimage
===========

`libpkgimage` is a C++17 library for reading package archives into a
backend-neutral, normalized package image and replaying selected regular-file
payloads without extracting them into a live filesystem.

It currently provides:

* canonical root-relative package paths;
* typed package entries for regular files, explicit directories, symbolic
  links, hard links, FIFOs, and device nodes;
* ordered package images with stable entry identifiers, duplicate-path
  rejection, and type-specific invariant validation;
* immutable payload selections bound to an inspected image;
* a backend-neutral payload sink interface;
* stable package archive objects that retain their source independently of
  later pathname replacement; and
* a tar backend implemented with `libarchive`.

`libpkgimage` describes archive contents and transports selected payload
bytes.  It does not choose installation destinations, install files, apply
package policy, identify packages from archive filenames, mutate a package
database, stage rejected files, or perform package transactions.

The implementation is original Zeppe-Lin code.  It is not derived from CRUX
`pkgutils` or from the former CRUX-derived implementation in `libpkgcore`.

Model
-----

The public flow is deliberately narrow:

```text
archive backend
      |
      v
package_archive
      |-- immutable package_image
      `-- replay(entry_selection, payload_sink)
```

A `package_image` records archive truth.  It preserves archive order and
represents explicit empty directories as real entries.  It does not synthesize
parent directories merely because child paths require them.

An `entry_selection` may contain regular entries only.  Input selection order
has no effect on replay order: selected payloads are always emitted in archive
order.  Empty regular files produce `begin()` and `end()` events with no
intermediate `write()` call.

The libarchive backend retains an open descriptor for the inspected regular
file.  Atomic replacement of the original pathname therefore does not change
the source used for replay.  In-place changes to the retained source are
checked before and after replay and reported as `source_changed_error`.

Payload replay is a transport operation, not a transaction.  A sink may have
consumed data before a change detected at the end of replay is reported.
Installation code must stage data and provide its own transaction semantics.

Requirements
------------

Build-time requirements:

* a C++17 compiler;
* Meson 1.2.0 or later;
* Ninja;
* pkg-config; and
* libarchive headers and library.

Doxygen is optional and is only needed to build the generated API reference.

Building
--------

Shared library:

```sh
meson setup build
meson compile -C build
meson test -C build
```

Static library with static dependencies:

```sh
meson setup build-static \
  -Ddefault_library=static \
  -Dlink_mode=static
meson compile -C build-static
meson test -C build-static
```

Installation:

```sh
meson install -C build
```

The project intentionally rejects `default_library=both`.  Shared and static
artifacts are separate builds, and each build links the corresponding form of
its dependencies.

Tests may be disabled with:

```sh
meson setup build -Dtests=disabled
```

API documentation
-----------------

Public interfaces are documented with Doxygen comments under
`include/libpkgimage`.

Generate the HTML reference from the project root with:

```sh
doxygen Doxyfile
```

The output is written to `build/docs/html`.

Using the library
-----------------

```cpp
#include <libpkgimage/libarchive_backend.h>
#include <libpkgimage/entry_selection.h>
#include <libpkgimage/payload_sink.h>

class sink final : public pkgimage::payload_sink {
public:
  void begin(const pkgimage::package_entry& entry) override
  {
    // Prepare storage for entry.path.
  }

  void write(const pkgimage::package_entry& entry,
             const std::byte* data,
             std::size_t size) override
  {
    // Consume binary payload bytes.
  }

  void end(const pkgimage::package_entry& entry) override
  {
    // Finish storage for entry.path.
  }
};

pkgimage::libarchive_backend backend;
auto package = backend.open("example#1.0-1.pkg.tar.xz");

for (const pkgimage::package_entry& entry : package->image().entries())
  std::cout << entry.id << ' ' << entry.path << '\n';

sink destination;
package->replay(
    pkgimage::entry_selection::all_regular(package->image()),
    destination);
```

Compiler and linker flags are available through pkg-config:

```sh
pkg-config --cflags --libs libpkgimage
pkg-config --static --libs libpkgimage
```

Layout
------

* `include/libpkgimage/` — public API;
* `src/` — library implementation;
* `tests/` — unit and backend tests; and
* `.github/workflows/` — shared/static CI builds.

License
-------

`libpkgimage` is licensed under the GNU General Public License version 3 or
later.  See `COPYING` for the license terms and `COPYRIGHT` for notices.
