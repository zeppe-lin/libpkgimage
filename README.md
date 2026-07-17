libpkgimage
===========

`libpkgimage` is a C++17 library for reading package archives into a
backend-neutral, normalized package image.

It currently provides:

* canonical root-relative package paths;
* typed package entries for regular files, explicit directories, symbolic
  links, hard links, FIFOs, and device nodes;
* ordered package manifests with duplicate-path and type-specific invariant
  validation;
* an archive backend interface; and
* a tar reader implemented with `libarchive`.

`libpkgimage` describes archive contents.  It does not install files, apply
package policy, identify packages from archive filenames, mutate a package
database, or perform package transactions.

The implementation is original Zeppe-Lin code.  It is not derived from CRUX
`pkgutils` or from the former CRUX-derived implementation in `libpkgcore`.

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

pkgimage::libarchive_backend backend;
pkgimage::package_image image = backend.inspect("example#1.0-1.pkg.tar.xz");

for (const pkgimage::package_entry& entry : image.entries())
  std::cout << entry.path << '\n';
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
