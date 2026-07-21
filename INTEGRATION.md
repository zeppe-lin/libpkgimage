libpkgimage integration boundaries
================================

Archive truth
-------------

`libpkgimage` describes what one package archive contains.

Its normalized facts are:

```text
ordered entries
canonical paths
entry types
archive metadata
regular-content identities
package-image identity
complete-archive digest and inspection receipt
selected regular payload bytes
```

These facts are useful to build comparison, installation planning, audit
tools, and diagnostics, but they are not those higher-level decisions.

Footprints
----------

A `.footprint` is not a `package_image`.

A footprint is a build expectation and comparison contract. It may define what
a package build is permitted or expected to produce and how mismatch policy is
reported.

`libpkgimage` does not:

* parse `.footprint`;
* serialize `.footprint`;
* compare an archive against `.footprint`;
* decide whether a mismatch is fatal;
* update a footprint; or
* provide the inherited `pkginfo -f` command contract.

Those concerns belong to the package build layer, currently `libpkgbuild` and
its eventual frontend integration.

A build layer may inspect a completed archive through `libpkgimage` and convert
the image into its own normalized footprint model. That conversion and policy
must remain outside `libpkgimage`.

Installed state
---------------

`libpkgstate` records what durable package state says is installed.

```text
libpkgimage::package_image       archive truth
libpkgstate::installed_package   installed ownership truth
```

The two manifests need not match.

Installation policy may preserve an existing file, reject an incoming object,
omit an archive entry, or record only the ownership that remains after a
completed operation.

Copying an archive image directly into installed state would collapse that
policy boundary.

Package identity
----------------

`libpkgimage` does not parse package names or versions from archive filenames.

The pathname:

```text
name#version-release.pkg.tar.xz
```

is transport naming policy owned by a higher layer.

An archive may be inspected without assigning it package identity.

Installation
------------

`libpkgimage` does not choose destinations or mutate the installed root.

A transaction layer may:

1. inspect an archive;
2. evaluate policy;
3. select regular payloads;
4. replay them into private staging;
5. construct links, directories, and special objects;
6. publish filesystem changes; and
7. update installed state.

The library owns only steps 1 and the payload transport used by step 4.

Reconciliation
--------------

Rejected-object staging and later reconciliation belong to
`libpkgreconcile`.

`libpkgimage` does not classify installed versus staged objects and does not
choose keep, install, merge, restore, relocate, or delete dispositions.

Auditing
--------

Filesystem integrity auditing belongs to `libpkgaudit`.

Archive metadata may inform future audit expectations, but `libpkgimage` does
not probe a filesystem or classify integrity findings.

Identity boundaries
-------------------

```text
artifact identity                 producer/provider control
complete_archive_digest           exact archive bytes
regular_content_digest            decoded bytes of one regular entry
package_image_identity             ordered normalized archive semantics
archive_inspection_receipt         exact bytes -> normalized image evidence
future payload-equivalence digest  unresolved broader equivalence relation
```

`libpkgimage` owns the complete-archive, regular-content, package-image, and
inspection-receipt facts. The future payload-equivalence digest is deliberately
out of scope. It does not
create artifact identity or interpret an artifact manifest. An upstream caller
may compare a manifest's complete-archive digest with the digest observed in the
inspection receipt to bind artifact control to archive truth.
