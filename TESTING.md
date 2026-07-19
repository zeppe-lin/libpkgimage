libpkgimage testing
===================

Doctrine
--------

Archive code is tested as an untrusted-input boundary and a transport
contract.

The suite must establish:

* package path normalization;
* entry and image validation;
* deterministic identifiers;
* duplicate rejection;
* hard-link invariants;
* selection binding;
* backend normalization;
* payload event ordering;
* exact binary replay;
* declared size enforcement;
* source-stability detection;
* sink exception behavior;
* independent public-header usability; and
* documentation consistency.

Path tests
----------

Path tests cover:

* ordinary relative paths;
* repeated separators;
* `.` components;
* trailing separators;
* empty input;
* absolute paths;
* `..` escape attempts;
* NUL bytes;
* carriage returns;
* newlines;
* parent queries;
* filename queries; and
* ancestor relations.

Image tests
-----------

Image tests cover:

* every supported entry type;
* archive-order preservation;
* consecutive identifiers;
* empty images;
* duplicate canonical paths;
* explicit empty directories;
* absent structural parents;
* symlink target requirements;
* hard-link target requirements;
* self-referential hard links;
* absent hard-link targets;
* non-regular hard-link targets;
* non-regular payload sizes;
* device number requirements; and
* modification-time nanosecond bounds.

Selection tests
---------------

Selection tests cover:

* all regular entries;
* explicit identifier sets;
* empty selections;
* duplicate identifiers;
* absent identifiers;
* non-regular identifiers;
* image-width mismatches;
* path mismatches; and
* input-order independence.

Backend tests
-------------

Libarchive tests cover:

* uncompressed tar;
* supported compression filters;
* every supported public entry type;
* unsupported archive entry types;
* invalid pathnames;
* negative numeric metadata;
* explicit directory retention;
* archive-order retention;
* duplicate paths;
* regular-file source enforcement;
* malformed and truncated archives; and
* inspect-versus-open behavior.

Replay tests
------------

Replay tests cover:

* selected and unselected entries;
* archive-order events;
* empty regular files;
* multiple payload blocks;
* exact binary bytes;
* begin/write/end sequencing;
* sink failure propagation;
* absence of `end()` after failed begin or write;
* declared-size overflow and underflow;
* manifest changes during replay;
* source replacement after open;
* pathname unlinking after open;
* metadata-only source changes;
* in-place source changes before replay;
* source changes during replay; and
* partial consumption before late failure.

Public-header tests
-------------------

Every installed public header is compiled independently.

This catches:

* missing direct includes;
* accidental include-order dependencies;
* undeclared public dependency requirements; and
* umbrella-header-only success that hides broken individual headers.

Documentation tests
-------------------

The documentation contract test checks:

* presence of every manual source;
* page names and sections;
* package-image versus footprint boundaries;
* replay non-transaction warnings;
* selection and sink event contracts;
* source-stability documentation;
* accepted libarchive filters and entry types; and
* inclusion of durable documents in Doxygen.

Build matrix
------------

CI runs:

* shared library and shared dependencies;
* static library and static dependencies;
* warnings as errors;
* the complete test suite;
* scdoc manual generation;
* Doxygen with warnings as errors; and
* staged installation.

A release should not proceed while a contract test is disabled merely to
obtain a green build.

Adding behavior
---------------

Every new entry type, path rule, metadata field, backend, selection rule,
replay branch, or observable error requires:

1. a direct positive test;
2. a direct negative or boundary test;
3. documentation in the relevant contract document;
4. manual-page coverage; and
5. deterministic-order tests where ordering is observable.
