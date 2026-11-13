# ml85p: driver for the Samsung ML-85G laser printer

`ml85p` is a driver for the Samsung ML-85G GDI laser printer, written by
Rildo Pragana. Ghostscript output is piped into it, and it drives the printer
directly over the parallel port. Its `.lsm` file describes it as working at
600x600 dpi and as "alpha-level software". The [`lbp660`](../lbp660/) driver
is based on it.

The driver is listed in OpenPrinting foomatic-db, but its homepage is gone.
This directory preserves release 0.2.0, taken from the Internet Archive
Wayback Machine.

## Status

- **Source:** preserved untouched, byte-identical to the contents of
  `ml85p-0.2.0.tar.gz`.
- **Build:** `cmpanal.c` compiles, `ml85p.c` does not, see
  [Build status](#build-status). No patches.
- **Hardware:** no printer was available for testing. Nothing here has been
  verified on a real printer.

## Directory layout

````
ml85p/
├── original-source/
│   └── ml85p-0.2.0/        # contents of ml85p-0.2.0.tar.gz, untouched
│       ├── ml85p.c         # driver source
│       ├── ml85p.h
│       ├── cmpanal.c       # compression analysis tool, mentioned in NEWS
│       ├── ml85p           # prebuilt 32-bit x86 Linux binary, shipped in the tarball
│       ├── ml85-print, ml85-test, printcap
│       ├── Makefile, README, NEWS, THANKS
│       ├── ml85p-0.2.0.lsm # Linux Software Map entry
│       └── COPYING         # GPL version 2 text
└── README.md
````

## Provenance record

| Field | Value |
|---|---|
| Driver | `ml85p` |
| Version | 0.2.0 |
| Original archive | `ml85p-0.2.0.tar.gz` |
| Release date | Not found. The `.lsm` file says `Entered-date: 03/11/2002`, the last `NEWS` entry is 2002Mar11, and the gzip header of the tarball is dated 2002-03-12 |
| Original author | Rildo Pragana (Copyleft 2000) |
| Printer | Samsung ML-85G |
| Found through | OpenPrinting foomatic-db, `db/source/driver/ml85p.xml` |
| Original project URL | http://www.pragana.net/gdiprinters.html (host does not resolve now) |
| Download used | https://web.archive.org/web/20051029103954/http://www.pragana.net:80/ml85p-0.2.0.tar.gz |
| Historical mirrors | The `.lsm` file lists `ibiblio.org/pub/Linux/hardware/drivers` as primary site. Not checked |
| Printer Applications | Not included in any |
| foomatic-db status | Not marked obsolete |
| Debian / Ubuntu | Not packaged (checked with apt-cache search on Ubuntu 24.04) |
| Other distributions | Not checked |
| License | GPL, version not stated in the source (see below) |
| Hardware testing | Not tested on a real printer |

### SHA256

Tarball:

| File | SHA256 |
|---|---|
| `ml85p-0.2.0.tar.gz` | `18192cd0217b46516c155f2f2c6f5cf71bbe0994d9cfda5cb8151f8ff301fd7b` |

Files in `original-source/ml85p-0.2.0/`:

| File | SHA256 |
|---|---|
| `cmpanal.c` | `5d47edb73493e91309c0c8d6df030aa14e5fd61db2aac76c0ee5f70c10ea2b5f` |
| `COPYING` | `91df39d1816bfb17a4dda2d3d2c83b1f6f2d38d53e53e41e8f97ad5ac46a0cad` |
| `Makefile` | `517b8347ce6e5c7c8f8bb1a1f09006f9854ed0a7d7a3c974b75c189e02ddf600` |
| `ml85-print` | `cd2bf3665638ff13e3d40633c649a986f7b966d8c189b8da77812a86230b0788` |
| `ml85-test` | `d1799d203355cbc6b535d9b790b4114dfa3f61202470510a89e2fdb8d2740d24` |
| `ml85p` | `3f410f5085ac74fe086801a44a365bffdb69ce10deab303209ccd9e37343e2fb` |
| `ml85p-0.2.0.lsm` | `df6246d5510526835e9100a3635f44e8afcf39b50266101d83b0375d584892cb` |
| `ml85p.c` | `e7360dfa3368cdcbe50b902668c1bf5e06f3262b434522b6dc6c9f3444b46890` |
| `ml85p.h` | `efab23b8961b53e529ef33983149a381b4deb2bcbe3c882be5c673a652c057d5` |
| `NEWS` | `7e8dbdaf0b7584d9bf89ab587b12e35f4fb9761bbce5539fe1471f83fc05d89c` |
| `printcap` | `1030beabc873489f94d7017da894c674b5449671a65a7d728d319becb4e64076` |
| `README` | `626e1e245533ac9c3abf2a5b969e8a040a6b5caaeb17174530519078c83157cf` |
| `THANKS` | `06037ff02239b323fd1e56c02074ac7a38b80741d119a4c8904eef073a6e9a19` |

### License

The headers of `ml85p.c` and `ml85p.h` say "(C) Copyleft, 2000 Rildo Pragana"
and "This program is released under GPL", without a version. The README says
the driver is released under GPL. The `.lsm` file says `Copying-policy: GPL`.
`COPYING` is the GNU General Public License version 2 text. `cmpanal.c` has no
copyright or license header.

## Build status

Tested on Linux Mint 22.3 (based on Ubuntu 24.04) with gcc 13.3.0. Each `.c`
file was compiled on its own; the Makefile was not run:

```sh
cd original-source/ml85p-0.2.0
gcc -c cmpanal.c -o /dev/null
gcc -c ml85p.c -o /dev/null
```

- `cmpanal.c`: compiles.
- `ml85p.c`: fails with `fatal error: asm/io.h: No such file or directory`.
  The driver accesses the parallel port directly.
