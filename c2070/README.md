# c2070: driver for the Lexmark 2070 Color Jetprinter

`c2070` is a filter for the Lexmark 2070 Color Jetprinter, written by
Christian Kornblum. It reads Ghostscript `bitcmyk` output on standard input
and writes the printer's protocol to standard output, in CMYK at 300 dpi.

The driver is listed in OpenPrinting foomatic-db, but its homepage is gone.
This directory preserves release 0.99, taken from the Internet Archive
Wayback Machine.

## Status

- **Source:** preserved untouched, byte-identical to the contents of
  `c2070-0.99.tar.gz`.
- **Build:** `c2070.c` compiles, see [Build status](#build-status). No
  patches.
- **Hardware:** no printer was available for testing. Nothing here has been
  verified on a real printer.

## Directory layout

````
c2070/
├── original-source/
│   └── c2070-0.99/     # contents of c2070-0.99.tar.gz, untouched
│       ├── c2070.c     # driver source
│       ├── makefile
│       ├── README      # original documentation
│       └── LICENSE     # GPL version 2 text
└── README.md
````

The tarball has no top-level directory; its four files are stored here in
`original-source/c2070-0.99/`.

## Provenance record

| Field | Value |
|---|---|
| Driver | `c2070` |
| Version | 0.99, 06.10.1999 (from the `c2070.c` header) |
| Original archive | `c2070-0.99.tar.gz` |
| Release date | Not found. The gzip header of the tarball is dated 1999-10-09 |
| Original author | Christian Kornblum |
| Printer | Lexmark 2070 Color Jetprinter |
| Found through | OpenPrinting foomatic-db, `db/source/driver/c2070.xml` |
| Original project URL | http://www.kornblum.i-p.com/2070/Lexmark2070.html (host does not resolve now) |
| Download used | https://web.archive.org/web/20051220032623/http://www.kornblum.i-p.com:80/2070/c2070-0.99.tar.gz |
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
| `c2070-0.99.tar.gz` | `d836a813b7f9f5281b58c50444b0bb85071b1432fc6481f2677ba2f2b7e68f1f` |

Files in `original-source/c2070-0.99/`:

| File | SHA256 |
|---|---|
| `c2070.c` | `5a9f34e8616dfec7f1e9609a9d39731b2e975116c7f341b297fea4252c560a53` |
| `LICENSE` | `5c725770b5ab27b3110672b8aba1a1c4945d335367a3b972d6d3f3583fc4eaec` |
| `makefile` | `2b71ad7769085c53af114252e30f3499fc74ffbdbcba4b187900cdd51b8d3770` |
| `README` | `e6b3673c443c0bc92d9935f4f141c70298e39652deff154f9647ac7f884a549d` |

### License

The `c2070.c` header says "License: GPL (GNU Public License)" and names no
version. The README says the driver "is protected by the GNU Public License,
which you should have received a copy of along with this package". The
`LICENSE` file in the tarball is the GNU General Public License version 2
text.

## Build status

Tested on Linux Mint 22.3 (based on Ubuntu 24.04) with gcc 13.3.0. The `.c`
file was compiled on its own; the makefile was not run:

```sh
cd original-source/c2070-0.99
gcc -c c2070.c -o /dev/null
```

- `c2070.c`: compiles.
