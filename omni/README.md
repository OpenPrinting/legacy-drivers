# omni: IBM Omni Driver Framework driver for Ghostscript

`omni` is the IBM Omni Driver Framework driver for Ghostscript, a contrib
device named `omni`. It needs the external Omni library. It was removed from
Ghostscript in commit f659ff57a; the first release without it is 9.21.

This directory keeps the last copy of the driver, from the Ghostscript 9.20
release, for reference and archival. It does not build on current
Ghostscript.

## Status

- **Source:** preserved untouched, byte-identical to the files in the
  Ghostscript 9.20 release tarball.
- **Build:** does not build on Ghostscript 9.20, and has been broken since
  9.07, see [Build status](#build-status).
- **Hardware:** not tested.

## Directory layout

````
omni/
├── original-source/
│   └── ghostscript-9.20/
│       ├── gomni.c             # driver source, untouched
│       ├── defs.h              # header for gomni.c, untouched
│       ├── omni-contrib.mak    # omni build rule, extracted unchanged from contrib/contrib.mak
│       └── omni-configure.ac   # --with-omni option, extracted unchanged from configure.ac
└── README.md
````

`omni-contrib.mak` is lines 677-687 of `contrib/contrib.mak` and
`omni-configure.ac` is lines 1588-1602 of `configure.ac`, both from
Ghostscript 9.20, copied without changes.

## Provenance record

| Field | Value |
|---|---|
| Driver | `omni` (Ghostscript contrib device), files `gomni.c`, `defs.h` |
| Preserved version | Copy shipped in Ghostscript 9.20 (`ghostscript-9.20.tar.gz`) |
| `gomni.c` | from `contrib/gomni.c` |
| `defs.h` | from `contrib/defs.h` |
| `omni-contrib.mak` | from `contrib/contrib.mak`, lines 677-687 |
| `omni-configure.ac` | from `configure.ac`, lines 1588-1602 |
| SHA256 of `gomni.c` | `f04b25adec1b54b0db41dbcb0c309a4864e80e55ea72be77dc7a45289d77b010` |
| SHA256 of `defs.h` | `aaef66fd30f5cc1d8e4e3de8468f65c96c170439cab7fa8188bd1b99c6e7c3f0` |
| SHA256 of `omni-contrib.mak` | `2d44b78c9d2f6a8e68066d40cd517be36c7b3eb121c4919112f6e291ccde00e5` |
| SHA256 of `omni-configure.ac` | `164144db5c9d0b23697c89e2d3fa1217ed8e69d3bcad54383776280b4ff7e0c0` |
| Original author | International Business Machines Corp., per the source |
| Removed from Ghostscript | commit f659ff57a, first release without it: 9.21 |
| External dependency | Omni library, reference version `Omni-0.9.2.tar.gz` (not included) |
| License | LGPL-2.1-or-later, per the file headers (see below) |
| Hardware testing | Not tested |

### License

The headers of `gomni.c` and `defs.h` say "Copyright (c) International
Business Machines Corp., 2000" and that the file may be redistributed and/or
modified under the terms of the GNU Lesser General Public License, version
2.1 or (at your option) any later version. Both also say "Portions of this
file are used with permission granted by Aladdin Enterprises". `gomni.c`
additionally carries "Copyright (C) 1998, 1999, 2000 Aladdin Enterprises"
under the same LGPL 2.1-or-later terms.

## Build status

Build tested with the omni driver enabled on Ghostscript 9.20 and 9.19. Both fail:

- Ghostscript 9.20: does not build. `gomni.c:115` includes `gdevprna.h`, which
  was removed by commit a1c3ce81d.
- Ghostscript 9.19 (and every release since 9.07): `gomni.c:533` fails
  with `'struct _DevStruct' has no member named 'memory'`, caused by
  commit 821d4c00e.
- Ghostscript 9.06 is probably the last release where it built. This is
  untested.

The device also needs the external Omni library. Reference version:

```
Omni-0.9.2.tar.gz
# SHA256 e8f36dfea952ad7a3eebf6d8d08aafd191906ba6b8992c5f56166bc7e67532a2
```

The tarball is not part of this repository.
