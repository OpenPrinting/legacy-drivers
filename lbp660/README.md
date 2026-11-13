# lbp660: driver for Canon LBP-660 and LBP-460 laser printers

`lbp660` is a Linux driver for the Canon LBP-660 and LBP-460 GDI laser
printers, written by Nicolas Boichat. It talks to the printer directly over
the parallel port. Per its README and source headers, it is based on Rildo
Pragana's driver for the Samsung ML-85G (see [`ml85p`](../ml85p/)). The
LBP-460 version (`lbp460.c`) was adapted by Indan Zupancic.

The driver is listed in OpenPrinting foomatic-db, but its homepage is gone.
This directory preserves release 0.3.1, taken from the Internet Archive
Wayback Machine.

## Status

- **Source:** preserved untouched, byte-identical to the contents of
  `lbp660-0.3.1.tar.gz`.
- **Build:** `lbp660.c` and `lbp460.c` compile, `smpllbp660.c` does not, see
  [Build status](#build-status). No patches.
- **Hardware:** no printer was available for testing. Nothing here has been
  verified on a real printer.

## Directory layout

````
lbp660/
├── original-source/
│   └── lbp660-0.3.1/           # contents of lbp660-0.3.1.tar.gz, untouched
│       ├── lbp660.c            # LBP-660 driver
│       ├── lbp660.h
│       ├── lbp460.c            # LBP-460 driver
│       ├── smpllbp660.c        # not built by the Makefile, no copyright header
│       ├── lbp660-print, lbp460-print, lbp660-test, lbp460-test, restartcups.sh
│       ├── ppd/                # Canon-LBP-460-lbp460.ppd, Canon-LBP-660-lbp660.ppd
│       ├── foomatic-rip        # third-party helper script, see below
│       ├── foomatic-gswrapper  # third-party helper script, see below
│       ├── Makefile, README, NEWS, THANKS, TODO
│       └── COPYING             # GPL version 2 text
└── README.md
````

`foomatic-rip` and `foomatic-gswrapper` are third-party Foomatic helper
scripts that were shipped in the tarball. The Makefile can also download them
from linuxprinting.org. They are kept untouched and are not part of the
driver.

## Provenance record

| Field | Value |
|---|---|
| Driver | `lbp660` (also builds `lbp460`) |
| Version | 0.3.1 (`RELEASE` in `Makefile`; the last `NEWS` entry is 0.2.2, 16-Aug-04) |
| Original archive | `lbp660-0.3.1.tar.gz` |
| Release date | Not found. The gzip header of the tarball is dated 2005-08-16 |
| Original author | Nicolas Boichat (Copyright 2004); LBP-460 adaptation by Indan Zupancic |
| Printers | Canon LBP-660, Canon LBP-460 |
| Found through | OpenPrinting foomatic-db, `db/source/driver/lbp660.xml` |
| Original project URL | http://www.boichat.ch/nicolas/lbp660/ (returns 404 now) |
| Download used | https://web.archive.org/web/20051029141006/http://www.boichat.ch:80/nicolas/lbp660/lbp660-0.3.1.tar.gz |
| Printer Applications | Not included in any |
| foomatic-db status | Not marked obsolete |
| Debian / Ubuntu | Not packaged (checked with apt-cache search on Ubuntu 24.04) |
| Other distributions | Not checked |
| License | GPL-2.0-or-later (see below) |
| Hardware testing | Not tested on a real printer |

### SHA256

Tarball:

| File | SHA256 |
|---|---|
| `lbp660-0.3.1.tar.gz` | `d7fa08b2df402cba19e37a4d7dc0d65d384847ffa449050a6db8feb173ab733d` |

Files in `original-source/lbp660-0.3.1/`:

| File | SHA256 |
|---|---|
| `COPYING` | `91df39d1816bfb17a4dda2d3d2c83b1f6f2d38d53e53e41e8f97ad5ac46a0cad` |
| `foomatic-gswrapper` | `16a084a6f2d20cbf32b5eca5807077da5408496119700607033f1972f7787cfa` |
| `foomatic-rip` | `956e4e0838a3718da728764f9eab67ffcb7aa32ddcf65b357b15d677bd348f55` |
| `lbp460-print` | `d0fb399a04cbf9ef1bff6dead82e6be79b6f85141001aafcf30c0d66e8efca1f` |
| `lbp460-test` | `f2f3ad711c39aa9d75435f7f412edb7bbf841b06334352b5829010c4bead89e3` |
| `lbp460.c` | `b34bcae56ef6ec6be50bdb72b39e1c4000317f0536710ba768326c3c13e985b7` |
| `lbp660-print` | `75e7c744bca7cea565b20c284afacb58f7258746dc61597c522c62cc4580ac6c` |
| `lbp660-test` | `e93d290de014b2f5bcb612b592fdc1784fa287d689e3cd38d8763ad9e6725562` |
| `lbp660.c` | `4855c503f3351bb8ae976fa2cda57457a7253ac2f0392d29b317ea0a85c8ab5e` |
| `lbp660.h` | `2d2fee236bf1a3f5d37cf05a29bf3e1688b6a4997dc8eb285b5d7b9610a5c28b` |
| `Makefile` | `e1e7cc08958c52f68b57684706aeb3733f56dfecb81927f574893232eae64a61` |
| `NEWS` | `93fd2c3ddc3b866c653a121b01b7b4b90ed67e90d354ecd5bb0f2c888403dc4a` |
| `ppd/Canon-LBP-460-lbp460.ppd` | `47d467e3a13a044d35382440226279dee2d60426f58686fd69ba0e518b0b268f` |
| `ppd/Canon-LBP-660-lbp660.ppd` | `eb9b87404a6a5526f683c645796b51a70237e9f911cb70ce070e5912d8d6a4b8` |
| `README` | `575c3328df2180e07bbf33874073e21707ca0f502d86e168c03f9cec23a7486b` |
| `restartcups.sh` | `c0a33d6349eba8c52c157fcf0adac93e042d8004edd7a72d4d3f2bd6e2d33738` |
| `smpllbp660.c` | `a001be0cf0432238bbc5f6539e0037d03579662aea9f18538081c49683533e48` |
| `THANKS` | `a75a654b88fa4038724fa0ec3134ea872b7f8709c689a812c0b4be42895a65c9` |
| `TODO` | `e0aa9509432faf421e9844a8adaa40f0bbbaba94c3be83659c9596e7fd9aa905` |

### License

The headers of `lbp660.c`, `lbp660.h` and `lbp460.c` say "Copyright (C) 2004
Nicolas Boichat" and that the program may be redistributed and/or modified
under the GNU General Public License, version 2 or (at your option) any later
version. `COPYING` is the GPL version 2 text. The two PPD files say they are
published under the GNU General Public License, without a version.
`smpllbp660.c` has no copyright or license header.

Third-party scripts: `foomatic-rip` says "Copyright 2002 Grant Taylor & Till
Kamppeter", GPL version 2 or later. `foomatic-gswrapper` has no copyright or
license notice.

## Build status

Tested on Linux Mint 22.3 (based on Ubuntu 24.04) with gcc 13.3.0. Each `.c`
file was compiled on its own; the Makefile was not run:

```sh
cd original-source/lbp660-0.3.1
gcc -c lbp660.c -o /dev/null
gcc -c lbp460.c -o /dev/null
gcc -c smpllbp660.c -o /dev/null
```

- `lbp660.c`: compiles.
- `lbp460.c`: compiles.
- `smpllbp660.c`: fails with
  `fatal error: asm/io.h: No such file or directory`.
  This file is not built by the Makefile.
