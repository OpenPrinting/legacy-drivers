# dmprt: generic Ghostscript driver for dot matrix printers

`dmprt` is a Ghostscript printer device for dot matrix (and some page)
printers. It has no built-in printer model. Instead, the printer commands
for bitmap output are read at run time from a printer definition file in
the format of `dviprt`, a TeX DVI printer driver widely used on MS-DOS in
Japan. Both the text source form (`*.src`) and the compiled binary form
(`*.cfg`) of these files are accepted. `escp_24.src`, the definition for
Epson ESC/P 24-pin printers, is included and is the default.

The driver was written by ASAYAMA Kazunori (淺山和典) in 1992-1997, shipped
as part of Ghostscript in `contrib/japanese` from 2007, and removed from
Ghostscript on 2020-07-06.

This directory preserves the last upstream copy of the driver, from the
Ghostscript 9.52 release.

## Status

- **Source:** preserved untouched, byte-identical to Ghostscript 9.52.
- **Build:** not yet attempted (see [Build status](#build-status)).
- **Hardware:** the original printer hardware was **not available** for
  testing. Nothing in this directory has been verified on a real printer.

Not tested on real hardware.

## Supported printers

Any printer that can be described by a `dviprt` printer definition file.
Only `escp_24.src` (Epson ESC/P, 24 pins, 180 dpi) is included here.

According to `gdevdmpr.txt`, `dviprt` v2.42 shipped definition files for
about 50 printers in its utility package (`tex???ut.lzh`), among them Epson
ESC/P 8-pin and 24-pin, Epson AP-850/AP-900/HG-4800/LP-3000/LP-7000,
Canon BJ-10/BJ-130J/LBP-8/LIPS III, HP DeskJet 500/505J and LaserJet Plus,
NEC PC-PR series, Brother HL-8e, Kyocera L-880, Ricoh RPDL, Star JT-48,
Toshiba PR-48, and G3 fax / PBM / EPS image output. The author notes that
not all of them were tested. Those files are not part of Ghostscript and are
not included here.

Output encodings supported by the driver (`gdevdmpr.c`): none, ASCII hex,
CCITT fax, PCL mode 1 and PCL mode 2 compression.

## How it works

Summary of `gdevdmpr.txt` (Japanese, EUC-JP encoded):

- At start-up the device runs `dmp_init.ps` and then the site
  configuration file `dmp_site.ps` (or the file given with
  `-sDMPCONFIG=file`), both found on the Ghostscript library search path.
  If no site configuration is found, ESC/P 24-pin output is used.
- `dmp_site.ps` is a PostScript fragment between `BeginConfig` and
  `EndConfig`. Its entries, all optional:

  | Entry | Meaning |
  |---|---|
  | `/printer (file)` | `dviprt` printer definition file (`.src` or `.cfg`); searched on the library path, then in `$TEXCFG` |
  | `/resolution [x y]` | resolution in dpi; priority is `-r`, then this entry, then the definition file, then 180 dpi |
  | `/offset [x y]` | origin shift (length) |
  | `/defaultsize [w h]` | page size without `-g`/`-sPAPERSIZE` (default 8.3 x 11.7 in) |
  | `/margin [l b r t]` | non-printable margins (default 0) |
  | `/maxsize [w h]` | maximum printable area, 0 or negative means unlimited |
  | `/verbose bool` | print page count and output byte counts (default false) |

  Lengths accept the units `in` (default), `cm`, `mm`, `pt`, `bp` and `dot`.
- `dmp_init.ps` checks that its version (201) matches the driver's
  `Version` device parameter (`DEVICE_VERSION 201`, i.e. 2.01).
- Usage: `gs -sDEVICE=dmprt file.ps`

The documentation lists files of the author's original package that are
not in Ghostscript: `COPYING`, `gdevdmpr.sj` (the same document in
Shift JIS), `gdevdmpr.mak` and `testpage.ps` (alignment test page).

## Directory layout

```
dmprt/
├── original-source/
│   └── ghostscript-9.52/
│       ├── gdevdmpr.c        # driver source, untouched
│       ├── dviprlib.c        # dviprt printer definition library, untouched
│       ├── dviprlib.h
│       ├── dmp_init.ps       # device initialization file
│       ├── dmp_site.ps       # sample site configuration
│       ├── escp_24.src       # dviprt definition for Epson ESC/P 24-pin
│       └── gdevdmpr.txt      # author's documentation (Japanese, EUC-JP)
├── patches/
│   └── openprinting/
│       └── README.md         # placeholder; patch will follow separately
└── README.md
```

## Provenance record

| Field | Value |
|---|---|
| Driver | `dmprt` (Ghostscript device), files `gdevdmpr.c`, `dviprlib.c`, `dviprlib.h`, `dmp_init.ps` |
| Driver version | 2.01 (`DEVICE_VERSION 201` in `gdevdmpr.c`; "dmprt version 2.01 for Ghostscript version 4.03" in `gdevdmpr.txt`) |
| Preserved version | Copy shipped in Ghostscript 9.52 (`contrib/japanese/`, `contrib/japanese/doc/`) |
| Release tarball | `ghostscript-9.52.tar.gz`, https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/tag/gs952 |
| SHA-256 (tarball) | `c2501d8e8e0814c4a5aa7e443e230e73d7af7f70287546f7b697e5ef49e32176` |
| Size (tarball) | 49,722,607 bytes |
| Ghostscript tag / commit | `ghostscript-9.52`, `e49830f8efdbc3a9f4e8acaf708b68a742f515aa` (2020-03-19) |
| RCS keywords in files | `dviprlib.c` / `dviprlib.h`: `$Id: RCFG.C 1.1 1994/08/30 02:27:02 kaz Exp kaz $` and similar (from the `dviprt` library sources) |
| Original author | ASAYAMA Kazunori (K. Asayama), `TPM03937@pcvan.or.jp`, `GHF01532@niftyserve.or.jp`, `asayama@vsp.cpg.sony.co.jp` (per `gdevdmpr.txt`) |
| Other contributors | OkI (NEC PC-H98 high resolution mode, Dec 1992), hero.h (IBM PC AT, Mar 1993), FKR (386BSD, Mar 1993), per `gdevdmpr.c` header |
| First release | Nov 1992 (per `gdevdmpr.c` header) |
| Based on | `dviprt` printer definition library (`rcfg.c`, `s_cfg.h` etc.) by SHIMA and others (PC-VAN SSCIENCE); CCITT fax encoder based on the fax library by Godohoji and SHIMA (per `gdevdmpr.txt`) |
| Author's last known version | 2.01, 1997-05-13, for Ghostscript 4.03 (per `gdevdmpr.txt` history) |
| Original project pages | Not located |
| Original distribution | Not located (the author's package with `COPYING`, `gdevdmpr.sj`, `gdevdmpr.mak` and `testpage.ps` is described in `gdevdmpr.txt`) |
| License | GPL (version not stated), see [License](#license) |
| Last known upstream release | Ghostscript 9.52 (2020-03-19) |
| Hardware testing | None (hardware not available) |

### Source verification

The preserved files were copied unchanged from `contrib/japanese/` (and
`contrib/japanese/doc/`) of the official `ghostscript-9.52.tar.gz` release
tarball (checksum above). They were compared against two copies:

1. The extracted release tarball (`cmp` reports no differences).
2. `contrib/japanese/` at Ghostscript git tag `ghostscript-9.52`
   (commit `e49830f8e`): the git blob hash of each file matches the blob in
   the upstream tree.

| File | Path in tarball (under `ghostscript-9.52/`) | Size (bytes) | SHA-256 |
|---|---|---|---|
| `gdevdmpr.c` | `contrib/japanese/gdevdmpr.c` | 27,209 | `cbe2f30e535b4842c4128dd00226530c8b7e11db61fb0ae1f290f3ff652507f1` |
| `dviprlib.c` | `contrib/japanese/dviprlib.c` | 80,391 | `4de2196fa4df62aeb0c35e8453d9181ab7a2ff6b27d965155f49cefc02af7344` |
| `dviprlib.h` | `contrib/japanese/dviprlib.h` | 10,834 | `3cc3d2213cb82fa32a47411014ff8c0c585b4d9642be0e58a4ac062a96a80884` |
| `dmp_init.ps` | `contrib/japanese/dmp_init.ps` | 7,246 | `9e918e4739cbd11d3779b6dc66526eeff420bf236b51d3f51148faa711365ac4` |
| `dmp_site.ps` | `contrib/japanese/dmp_site.ps` | 459 | `aab47831317229c6dfcdb52e5e280400d6f0f50ce8e6a3067f20788f0a1f41a0` |
| `escp_24.src` | `contrib/japanese/escp_24.src` | 350 | `baf55e20954249060cf9249f7afcba7300c9d56d283040ed6efcf474f8a35ef2` |
| `gdevdmpr.txt` | `contrib/japanese/doc/gdevdmpr.txt` | 19,497 | `82c3253d7ab4c9aad977b247cecfd2ad10e4a2355407c7f084ba5b408173e033` |

**Note on `gdevdmpr.txt`:** the file is Japanese text in EUC-JP encoding.
Use `iconv -f EUC-JP -t UTF-8 gdevdmpr.txt` to read it. The document refers
to itself as `gdevdmpr.sj` (Shift JIS), so it was re-encoded at some point
before reaching Ghostscript.

**Note on `dviprlib.h`:** the removal commit `f2e2e3c` deleted the other six
files but not `dviprlib.h`, which is still present, unused, in
`contrib/japanese/` of Ghostscript 9.53.0 and later.

### History

| Date | Event |
|---|---|
| 1992-11 | First version by K. Asayama |
| 1992-12 | NEC PC-H98 high resolution mode added by OkI |
| 1993-03 | IBM PC AT support (hero.h), 386BSD fixes (FKR), port to Ghostscript 2.5.2 (K. Asayama) |
| 1993-10 | Port to Ghostscript 2.6.1 (K. Asayama) |
| 1993-11 | First version of `dmp_init.ps` (ASAYAMA Kazunori) |
| 1994-08-30 | Revision of the `dviprt` library files copied into `dviprlib.c` / `dviprlib.h` (RCS `$Id` lines) |
| 1997-05-13 | Version 2.01 for Ghostscript 4.03: last author release (per `gdevdmpr.txt`) |
| 2007-05-08 | First added to upstream Ghostscript (`gs/contrib/japanese/`) by Till Kamppeter (`33e99334f`), as part of adding the third-party drivers listed in the OpenPrinting database; backed out and re-merged several times (`ff4e77a33`, `36af9160f`, `e0bb089cd`) |
| 2007-06-06 | Merged for good with the ESP Ghostscript merge (`2bce78bce`, Till Kamppeter) |
| 2007-2019 | Maintenance-only upstream changes: `4dbf3a1c7` (`private` to `static`), `5f761d44d` (remove globals), `781969994` (whitespace), `821d4c00e` (non-GC memory), `773c69e46` (`fopen` to `gp_fopen`), `6948650ef` (moved to `contrib/japanese/`), `6b1b1d72e`, `9bb42182c` (error codes), `9de16a663`, `108dd018b` (`gp_file *` API) |
| 2020-03-19 | Ghostscript 9.52 released: last release containing the driver |
| 2020-07-06 | Removed from Ghostscript by Chris Liddell (`f2e2e3c1bb24887bce699821a3da1f8b3bbd12be`, "Remove the dmprt device") |
| 2020-09-10 | Ghostscript 9.53.0 released without the driver (only the orphaned `dviprlib.h` remains) |

### Evidence that upstream is abandoned

- No author release found after version 2.01 (1997-05-13). The author's
  history notes that support for DJ's GPP was frozen for lack of a
  development environment and time.
- No functional change upstream after 1997; all Ghostscript changes from
  2007 to 2019 are mechanical API cleanups.
- The removal commit states: "It has code which triggers security warnings,
  it has not built as it stands since before 8.71 (so >10 years) and has
  significant (segfaulting) problems when modified to successfully build.
  Since it cannot have been used (and no one has complained) in over ten
  years, we're removing it."
- `dmprt` was never added to any driver group in Ghostscript's `configure`
  (it is not in `JAPAN_DEVS`), so it was not built by `--with-drivers=ALL`.

## Distribution copies and patches

The source files were present in every Debian `ghostscript` source package
up to buster. Debian and Ubuntu built Ghostscript with `--with-drivers=ALL`,
which does not include `dmprt`, so **the device itself was never shipped**.
The data files (`dmp_init.ps`, `dmp_site.ps`, `escp_24.src`) were installed
anyway, so the packages shipped configuration for a device they did not
contain.

Checked binary packages:

| Distribution | Package | Data files in `/usr/share/ghostscript/<ver>/lib/` | `dmprt` in `libgs` |
|---|---|---|---|
| Debian lenny | `ghostscript` / `libgs8` 8.62.dfsg.1-3.2lenny5 | yes | no |
| Debian jessie | `libgs9-common` 9.06~dfsg-2+deb8u7 | yes | not checked |
| Debian buster | `libgs9-common` / `libgs9` 9.27~dfsg-2+deb10u5 | yes | no |
| Ubuntu 8.04 (hardy) | `ghostscript` / `libgs8` 8.61.dfsg.1-1ubuntu3 | yes | no |
| Ubuntu 10.04 (lucid) | `ghostscript` / `libgs8` 8.71.dfsg.1-0ubuntu5 | yes | no |
| Ubuntu 11.10 (oneiric) | `libgs9-common` 9.04~dfsg-0ubuntu11 | yes | not checked |
| Ubuntu 17.10 (artful) | `libgs9-common` / `libgs9` 9.21~dfsg+1-0ubuntu3 | yes | no |

"no" means the string `dmprt` does not occur in `libgs.so`, while other
Japanese contrib devices (e.g. `lips4v`) do. Debian bullseye (9.53.3) and
later, and Ubuntu releases with Ghostscript 9.53 or later, follow upstream
and no longer contain the files.

No Debian patch in `debian/patches/` touches the driver (all patch files of
lenny, squeeze, jessie and buster searched for the dmprt file names: 176
files, no matches), so there are no `patches/debian/` or other distribution patch
directories.

## License

`gdevdmpr.c`, `dviprlib.c`, `dviprlib.h`, `dmp_init.ps`:

- Header: "COPYRIGHT (C) 1990, 1992 Aladdin Enterprises" (`dmp_init.ps`:
  1989, 1992, 1993), with the old "Ghostscript General Public License"
  notice: "Everyone is granted permission to copy, modify and redistribute
  Ghostscript, but only under the conditions described in the Ghostscript
  General Public License."
- `gdevdmpr.txt` section 8: copyright is held by the author, ASAYAMA
  Kazunori; all programs and documents in the package come without warranty,
  and the conditions for warranty, redistribution and modification follow
  the GNU General Public License (GNU GPL), supplied as `COPYING` in the
  author's package. No GPL version is stated.

The Aladdin copyright notice appears to be the Ghostscript header that was
current in the early 1990s, copied by the author; the author's own copyright
statement is in `gdevdmpr.txt`.

The original license headers are preserved unmodified in the source files.

## Build status

### Original source against current Ghostscript

Not yet attempted.

Upstream's removal commit states that the driver "has not built as it
stands since before 8.71" and has "significant (segfaulting) problems when
modified to successfully build".

The build rule removed by `f2e2e3c` from `contrib/contrib.mak`, for
reference:

```
### ---------------- Dot matrix printer device ---------------- ###
dmprt_=$(DEVOBJ)gdevdmpr.$(OBJ) $(DEVOBJ)dviprlib.$(OBJ) $(DEVOBJ)gdevprn.$(OBJ)

$(DD)dmprt.dev : $(dmprt_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETDEV) $(DD)dmprt $(dmprt_)
	$(ADDMOD) $(DD)dmprt -ps dmp_init

$(DEVOBJ)gdevdmpr.$(OBJ) : $(JAPSRC)gdevdmpr.c $(JAPSRC)dviprlib.h $(PDEVH) \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(O_)$@ $(C_) $(JAPSRC)gdevdmpr.c

$(DEVOBJ)dviprlib.$(OBJ) : $(JAPSRC)dviprlib.c $(JAPSRC)dviprlib.h \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(O_)$@ $(C_) $(JAPSRC)dviprlib.c

extra-dmprt-install: install-libdata
	$(INSTALL_DATA) $(JAPSRC)dmp_init.ps $(DESTDIR)$(gsdatadir)$(D)lib || exit 1
	$(INSTALL_DATA) $(JAPSRC)dmp_site.ps $(DESTDIR)$(gsdatadir)$(D)lib || exit 1
	$(INSTALL_DATA) $(JAPSRC)escp_24.src $(DESTDIR)$(gsdatadir)$(D)lib || exit 1
```

### Patched source against current Ghostscript

In progress. The patch (`0001-port-to-current-ghostscript-api.patch`) will
be added to `patches/openprinting/` in a separate pull request.

### Original source within Ghostscript 9.52

Not yet attempted.

### Known issues

- Upstream reports security warnings and segfaults after modifying the
  driver to build (removal commit `f2e2e3c`). No bug number is given.

## Excluded files

None. All seven dmprt files from the Ghostscript 9.52 release are included.
The files of the author's original package that never reached Ghostscript
(`COPYING`, `gdevdmpr.sj`, `gdevdmpr.mak`, `testpage.ps`) were not located.

## References

- Ghostscript 9.52 release: https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/tag/gs952
- Ghostscript 9.52 source (git): https://github.com/ArtifexSoftware/ghostpdl/tree/ghostscript-9.52/contrib/japanese
- Removal commit: https://github.com/ArtifexSoftware/ghostpdl/commit/f2e2e3c1bb24887bce699821a3da1f8b3bbd12be
- Debian sources: https://sources.debian.org/src/ghostscript/
- Debian archive: http://archive.debian.org/debian/pool/main/g/ghostscript/
- Ubuntu archive: http://old-releases.ubuntu.com/ubuntu/pool/main/g/ghostscript/
- Issue: https://github.com/OpenPrinting/legacy-drivers/issues/25
