# pentaxpj: LPD filter for the Pentax PocketJet II / 200

`pentaxpj` is a printer filter for the Pentax PocketJet portable thermal printers, written by Rildo Pragana in 2001 with contributions by Rob Roy of Pentax Technologies, who also sponsored the development (see the header of `pentaxpj.c`). Its `README` calls it the "Pentax PocketJetII/200 Linux Printer Driver" (October 25, 2002).

It is an LPD-era driver: it takes PostScript, renders it by calling Ghostscript, and is configured with the `pentaxsetup` tool, which writes `/etc/pentaxpj.conf` and edits `/etc/printcap`. It has no PPD files. For a CUPS driver for the same printer family see [`../pocketjet`](../pocketjet).

Until now it was only archived in the OpenPrinting download space. This directory keeps everything from there, as discussed in #36 and #28.

## Status

- **Source:** everything from https://openprinting.org/download/printing/pentaxpj/pentaxpj/ (except the separate `pocketjet_cups/` directory, which is in `../pocketjet`), untouched and byte-identical. Nothing was removed.
- **Build:** builds with the two included patches, see [Build status](#build-status).
- **Filter test:** not done.
- **Hardware:** not tested.

## Directory layout

```
pentaxpj/
├── original-source/
│   ├── pentaxpj-1.0.0/                 # source directory as in the download space, untouched
│   │   ├── pentaxpj.c, pentaxpj.h, Makefile
│   │   ├── pentaxsetup, Install, pentax.xpm
│   │   ├── README, ChangeLog, test-page.ps.gz
│   │   └── BWidget-1.3.1/              # bundled Tcl library
│   ├── pentaxpj.spec                   # RPM spec file (Version 1.0.0, Release alt2)
│   ├── pentaxpj-glibc28_fix.patch
│   └── pentaxpj-1.0.0-LDFLAGS.patch
└── README.md
```

In the download space the source directory is called `pentaxpj/`. It is stored here as `pentaxpj-1.0.0/`, the name used by `Source: %name-%version.tar` in the spec file.

## Provenance record

| Field | Value |
|---|---|
| Driver | `pentaxpj` (LPD filter) |
| Printers | Pentax PocketJet II and PocketJet 200, per `README` |
| Preserved version | 1.0.0 (spec file); latest `ChangeLog` entry 2003-07-15 |
| Source | https://openprinting.org/download/printing/pentaxpj/pentaxpj/ |
| Original author | Rildo Pragana, "(C) Copyright 2001", per `pentaxpj.c` |
| Upstream URL in spec | http://ww1.pragana.net/gdiprinters.html |
| SHA256 of `pentaxpj-glibc28_fix.patch` | `3744225da75219dc352aa885752d5c7b4effdcc7f1193f8d8eb3d2662e205166` |
| SHA256 of `pentaxpj-1.0.0-LDFLAGS.patch` | `f97e2f4358c7987888486949be97e07c92872f1260acaca9138662f61368fb21` |
| License | GPL, see below |
| Hardware testing | Not tested |

### License

The header of `pentaxpj.c` says "This is free software, released under the General Public License (GPL)" without naming a version. The spec file says `License: GPL`.

## Build status

Built on Linux Mint 22 (Ubuntu 24.04 base) with gcc:

```sh
cd pentaxpj-1.0.0
patch -p0 < ../pentaxpj-glibc28_fix.patch
patch -p0 < ../pentaxpj-1.0.0-LDFLAGS.patch
make
```

Both patches apply cleanly and `make` exits with 0, producing the `pentaxpj` binary. The only warnings are two ignored return values (`pipe()` at `pentaxpj.c:732` and `write()` at `pentaxpj.c:756`).

- `pentaxpj-glibc28_fix.patch` adds the missing mode argument `0666` to an `open()` call with `O_CREAT`.
- `pentaxpj-1.0.0-LDFLAGS.patch` adds an `LDFLAGS` variable to the `Makefile`.

The filter itself was not run, since it needs a configuration created by `pentaxsetup` and calls an external Ghostscript.
