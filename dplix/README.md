# dplix: CUPS driver for the Dell 3010cn color laser printer

`dplix` is a CUPS raster driver for the Dell 3010cn color laser printer, which uses the DPL page description language. It was written by James L. Hammons in 2009 and is loosely based on the SpliX driver by Aurélien Croc (see `THANKS`).

Until now it was only archived in the OpenPrinting download space. This directory keeps the complete original release, as discussed in #36 and #28.

## Status

- **Source:** the complete `dplix-1.0.0.tar.bz2` release, untouched and byte-identical to the tarball. Nothing was removed.
- **Build:** `rastertodpl` builds unchanged with the current libcups, see [Build status](#build-status).
- **Hardware:** not tested.

## Directory layout

```
dplix/
├── original-source/
│   └── dplix-1.0.0/          # complete release tarball contents, untouched
│       ├── src/              # rastertodpl (CUPS filter) and pstodpl sources
│       ├── ppd/
│       │   ├── dell-color.drv    # PPD compiler source
│       │   └── ppd/dl3010cn.ppd  # PPD generated from it, as shipped
│       ├── tools/            # debugging and reverse-engineering helpers
│       ├── Makefile, module.mk, rules.mk
│       └── AUTHORS, ChangeLog, COPYING, INSTALL, README, THANKS, TODO
└── README.md
```

The top directory inside the tarball is named `dplix/`. It is stored here as `dplix-1.0.0/` to keep the version visible.

## Provenance record

| Field | Value |
|---|---|
| Driver | `dplix`, CUPS filter `rastertodpl` |
| Printer | Dell 3010cn (color laser, DPL) |
| Preserved version | 1.0.0 (`ChangeLog`: "Sat Apr 18 17:08:22 2009 ... 1.0.0: First usable version") |
| Source | https://openprinting.org/download/printing/dplix/dplix-1.0.0.tar.bz2 |
| SHA256 of the tarball | `63d487a42be820739a75d65beef255c26b893879d0f9ac42d9262a1b68550d34` |
| Files | 37, byte-identical to the tarball (`diff -r` shows no difference) |
| Original author | James L. Hammons, "(C) 2009 Underground Software", per `AUTHORS` and the source headers |
| License | GPL, see below |
| Hardware testing | Not tested |

### License

`COPYING` contains the GNU General Public License, version 2. The header of `src/rastertodpl.cpp` says the program may be redistributed and/or modified "under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 or later of the License". The other files in `src/` also refer to version 2 of the GPL in their headers.

## Build status

Built with `make` on Linux Mint 22 (Ubuntu 24.04 base) with the distribution's libcups development packages:

- Only `rastertodpl` is built. It builds without changes. There is one `-Wformat` warning in `src/printer.cpp:109`, where a non-literal format string is passed to `printf()`.
- `pstodpl` is already disabled in the original build (`#TARGETS` in `module.mk:15` and `#pstodpl_SRC` in `src/module.mk:14`). It does not modify PostScript: it forks `pstoraster` and pipes its output into `rastertodpl`. With the current PDF to CUPS Raster workflow `rastertodpl` alone is enough. It is kept here for reference.

Filter test without a printer: the CUPS test page was converted to CUPS Raster with `cupsfilter` using `ppd/ppd/dl3010cn.ppd`, then fed to `rastertodpl`. It exits with 0 and writes a 4.7 MB job that starts with a PJL header (`@PJL JOB MODE=PRINTER`, `COPIES`, `DUPLEX`, ...).

## PPD

The PPD source is `ppd/dell-color.drv`, with one filter line:

```
Filter application/vnd.cups-raster 100 rastertodpl
```

`ppdc` from `libppd-utils` 2.0.0 (libppd, not CUPS) generates `dl3010cn.ppd` from it and exits with 0. The generated PPD has the same `cupsFilter` line as the shipped `ppd/ppd/dl3010cn.ppd`. Build scripts should generate the PPD from the `.drv` file with libppd's `ppdc`, since CUPS 3.x no longer provides it.

## Other files

`tools/` contains helpers that are not needed for printing. They are kept because they are useful for reverse-engineering other printer models, especially from Dell.
