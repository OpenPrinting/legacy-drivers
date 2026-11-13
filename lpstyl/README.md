# lpstyl: driver for Apple StyleWriter printers

`lpstyl` is a driver for Apple StyleWriter and Color StyleWriter printers,
written by Monroe Williams. It prints over a serial port. Version 0.9.9 also
has optional AppleTalk support (`adsp.c`, `Makefile.atalk`) for the Apple
StyleWriter EtherTalk Adapter.

The driver is listed in OpenPrinting foomatic-db, but its homepage is gone.
This directory preserves release 0.9.9, taken from the Internet Archive
Wayback Machine.

## Status

- **Source:** preserved untouched, byte-identical to the contents of
  `stylewriter.tar.gz`.
- **Build:** `lpstyl.c` and `adsp.c` do not compile, see
  [Build status](#build-status). No patches.
- **Hardware:** no printer was available for testing. Nothing here has been
  verified on a real printer.

### Supported printers

From the `lpstyl.c` header:

- Color StyleWriter 2500: "Fully supported in 360x360dpi, B/W and Color."
- Color StyleWriter 2400: "Fully supported in 360x360dpi, B/W and Color."
- Color StyleWriter 2200: "Supposed to work as of 0.9.9 (I haven't seen it, though.)"
- Color StyleWriter 1500: "Works in B/W and Color."
- StyleWriter III, StyleWriter 1200: "Works peachy."
- StyleWriter II: "Reported to work quite well."
- StyleWriter I: "Working again as of version 0.9.7."
- Other StyleWriters: "The code will treat any printer it doesn't recognize
  like a SWII, so it just might work."

## Directory layout

````
lpstyl/
├── original-source/
│   └── lpstyl-0.9.9/       # contents of stylewriter.tar.gz (directory stylewriter/), untouched
│       ├── lpstyl.c        # driver source
│       ├── adsp.c, adsp.h  # AppleTalk ADSP support
│       ├── Makefile.atalk  # build with AppleTalk support
│       ├── printcap, printcap.a4, styl.ppd
│       ├── scripts/        # stylps, stylascii and related filter scripts
│       └── README, README.atalk, README.protocol, README.troubleshooting
└── README.md
````

The original tarball is named `stylewriter.tar.gz` and unpacks into a
directory named `stylewriter/`. That directory is stored here as
`original-source/lpstyl-0.9.9/`; its contents are unchanged.

## Provenance record

| Field | Value |
|---|---|
| Driver | `lpstyl` |
| Version | 0.9.9 (from the `lpstyl.c` header) |
| Original archive | `stylewriter.tar.gz` (top-level directory `stylewriter/`) |
| Release date | Not found. The gzip header of the tarball is dated 2000-08-27 |
| Original author | Monroe Williams (Copyright 1996-2000) |
| Printers | Apple StyleWriter and Color StyleWriter models, see [Supported printers](#supported-printers) |
| Found through | OpenPrinting foomatic-db, `db/source/driver/lpstyl.xml` |
| Original project URL | http://homepage.mac.com/monroe/styl/ (host does not resolve now) |
| Download used | https://web.archive.org/web/20060720100136/http://homepage.mac.com:80/monroe/styl/stylewriter.tar.gz |
| Printer Applications | Not included in any |
| foomatic-db status | Not marked obsolete |
| Debian / Ubuntu | Not packaged (checked with apt-cache search on Ubuntu 24.04) |
| Other distributions | Not checked |
| License | BSD-2-Clause (see below) |
| Hardware testing | Not tested on a real printer |

### SHA256

Tarball:

| File | SHA256 |
|---|---|
| `stylewriter.tar.gz` | `52b787909cc1743205202ae85a3dd21b715889b8a72a81961feb555b733edd60` |

Files in `original-source/lpstyl-0.9.9/`:

| File | SHA256 |
|---|---|
| `adsp.c` | `b4019ce7dc4839715405a6bc7776cca6a17be30bf3481d74dec37a192e26cf9c` |
| `adsp.h` | `324796cac97f0fac6b8d5f7f3d240bb2cc38b9458e41b5ad934204723f495b18` |
| `lpstyl.c` | `dbcd2625ec04341714335f2a388e8a40095daed124629a21a1e834834a4fd631` |
| `Makefile.atalk` | `e1c89f8067d0a2358ff518c50b4e9408f0e57bdf95aac4446b62f1ddc77eaaad` |
| `printcap` | `07840562768d7caf55ee5978c0c9e5584be9733b6b4b53e7325830d22cf3b8e0` |
| `printcap.a4` | `7a6174f50e482451f7bb74006d75246cad7cf6d695928d46d8eb332a5b4f1755` |
| `README` | `f0e33833b2bd95e1b2da9a981361758a9d61a505a367639642a73adbfe39cd98` |
| `README.atalk` | `d49afd3998c100eff424751e6adb5dd92a74af49e09622bc2aed2b2b4b084d79` |
| `README.protocol` | `19966c142c90f933071e3923ec1934406eba160c71a7b5dfab2696dcb3da4ea5` |
| `README.troubleshooting` | `45034229f173998f3256df91d58e0a53e8895e4fd7925aab7033b650c274fc92` |
| `scripts/direct_stylpbm` | `15a33d4157da8b17bcb9a42524fc16d678e398719d02560dbc85e9f3c1adc4b3` |
| `scripts/direct_stylps` | `656aa8b1499b2ddf243bb566dad48721b50e25a5f5bc8147fc411e940fc8dfa4` |
| `scripts/stylascii` | `5764af8f266737f2e833fa89918c551cd292d178f9bf649e8b39775975006b05` |
| `scripts/stylascii-atalk` | `f7f0cbe59838ed4b7987cee3d00a8bd7bf8462f65c35b52059860105d618c2c2` |
| `scripts/stylascii.a4` | `cdbdd7e88bf618f7e87ecc855602b1d2e50b02b086df94da0b60c78f54dff8eb` |
| `scripts/stylps` | `8e773638ff439ef4696192dec2e0a1b4fb301a58fbf338f50025927fc74d6c0a` |
| `scripts/stylps-atalk` | `2cd42b4fde664f25b0f715951e44f6811ada452ac7e4badb9aace250d712af86` |
| `scripts/stylps-color` | `beedf3c5ace6113452e7c8529a1844d99596fee6e289d327aedcedd81cb6e367` |
| `scripts/stylps-color-atalk` | `a83527017f1c008e4a364fec31cb8a9e870c96f9f1c9ffb401c51860f7d4317a` |
| `scripts/stylps-color.a4` | `2390a6cf4fb5cc35fed8c5308ba5813f08b55e94bb685c8febbd059c93706ef0` |
| `scripts/stylps.a4` | `2254d3c657e54dcd1953d50fc0d11d9fa2f43a9d106173c684f3dfd15c9461dd` |
| `styl.ppd` | `dcf1418c2e491c63682458dae48573903b6761231190999e2dbdc7f41791f394` |

### License

The header of `lpstyl.c` says "Copyright (C) 1996-2000 Monroe Williams. All
rights reserved." followed by the two-clause BSD license text
(BSD-2-Clause). `adsp.c` and `adsp.h` carry the same notice.

## Build status

Tested on Linux Mint 22.3 (based on Ubuntu 24.04) with gcc 13.3.0. Each `.c`
file was compiled on its own; `Makefile.atalk` was not run:

```sh
cd original-source/lpstyl-0.9.9
gcc -c lpstyl.c -o /dev/null
gcc -c adsp.c -o /dev/null
```

- `lpstyl.c`: fails with 8 errors, at lines 1040-1043 and 1056-1059:
  `error: lvalue required as increment operand`. These lines use a cast as
  an lvalue, for example `*((unsigned char *)bufK)++ = ...`.
- `adsp.c`: fails with `fatal error: atalk/nbp.h: No such file or directory`.
  It needs the netatalk (AppleTalk) headers.
