# pocketjet: CUPS driver for the Pentax PocketJet II / 200 / 3 / 3plus

`pocketjet_CUPS` is a CUPS raster driver for the Pentax PocketJet portable thermal printers, written by Tyler Blessing (`AUTHORS`, `main.c`: "Copyright 2004 Tyler Blessing"). It consists of the CUPS filter `rastertopocketjet` and one PPD file per printer model. Its `README.TXT` calls it "Linux CUPS Driver for Pentax PocketJet II/200/3/3plus Printers, CUPS Driver Version 2.0.0, November 18, 2004".

Until now it was only archived in the OpenPrinting download space, in the `pentaxpj` directory next to the older LPD filter (now in [`../pentaxpj`](../pentaxpj)). This directory keeps everything from there, as discussed in #36 and #28.

## Status

- **Source:** the complete `pocketjet_CUPS.tar.gz`, `pocketjet-fix.patch` and `README.TXT` from the download space, untouched and byte-identical. Nothing was removed.
- **Build:** builds with the included patch, does not build without it, see [Build status](#build-status).
- **Filter test:** `rastertopocketjet` runs and produces output, see below.
- **Hardware:** not tested.

## Directory layout

```
pocketjet/
├── original-source/
│   ├── pocketjet_CUPS-2.0.0/       # contents of pocketjet_CUPS.tar.gz, untouched
│   │   ├── main.c, pocketjet.h     # rastertopocketjet sources
│   │   ├── Pentax-PocketJet_II.ppd, Pentax-PocketJet_200.ppd,
│   │   │   Pentax-PocketJet_3.ppd, Pentax-PocketJet_3plus.ppd
│   │   ├── configure, configure.ac, Makefile.am, Makefile.in, ...
│   │   └── AUTHORS, ChangeLog, COPYING, INSTALL, NEWS, README
│   ├── pocketjet-fix.patch
│   └── README.TXT
└── README.md
```

The top directory inside the tarball is named `pocketjet_CUPS/`. It is stored here as `pocketjet_CUPS-2.0.0/` to keep the version visible. The tarball itself contains some files that were probably included by accident (a copy of `pocketjet_CUPS.tar.gz`, `.DS_Store` and `autom4te.cache/`); they are kept as they are.

## Provenance record

| Field | Value |
|---|---|
| Driver | `pocketjet_CUPS`, CUPS filter `rastertopocketjet` |
| Printers | Pentax PocketJet II, 200, 3 and 3plus (one PPD each) |
| Preserved version | 2.0.0, November 18, 2004 (`README.TXT`) |
| Source | https://openprinting.org/download/printing/pentaxpj/pocketjet_cups/ |
| SHA256 of `pocketjet_CUPS.tar.gz` | `3c395dfebbdcd0c93b6cf6c714e16940b80ba2580728400238beb9a3486fc9f5` |
| SHA256 of `pocketjet-fix.patch` | `fe981425bda813d4e659a384efe0584446cc9dc89af1d1f7905ac87e92a402d9` |
| Original author | Tyler Blessing, per `AUTHORS` and `main.c` |
| License | GPL-2.0-or-later, see below |
| Hardware testing | Not tested |

### License

`COPYING` contains the GNU General Public License, version 2. The header of `main.c` says it may be redistributed and/or modified "under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version".

## Build status

Built on Linux Mint 22 (Ubuntu 24.04 base) with the distribution's libcups development packages:

```sh
cd pocketjet_CUPS-2.0.0
patch -p0 < ../pocketjet-fix.patch
./configure
make
```

- **Without the patch** the build fails (`make` exits with 2): `main.c:73: error: 'stderr' undeclared`, plus implicit declarations of `setbuf`, `fprintf` and `perror`.
- `pocketjet-fix.patch` (dated 2021-10-23) adds `#include <stdio.h>` to `pocketjet.h`. It must be applied with `-p0`.
- **With the patch** `make` exits with 0 and produces `rastertopocketjet`. The only warning is that `cupsRasterReadHeader()` is deprecated in favor of `cupsRasterReadHeader2()`.

Filter test without a printer: the CUPS test page was converted to CUPS Raster with `cupsfilter` using `Pentax-PocketJet_3.ppd`, then fed to `rastertopocketjet`. It exits with 0, writes a 139 KB job and logs "Ready to print".

## PPD

All four PPD files have the same filter line:

```
*cupsFilter: "application/vnd.cups-raster 10 rastertopocketjet"
```
