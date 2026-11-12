# dmprt: Ghostscript driver for dot matrix printers

`dmprt` is a generic Ghostscript printer device for dot matrix printers,
written by ASAYAMA Kazunori. Instead of supporting one printer model, it reads
the printer commands from printer definition files of `dviprt`, a TeX printer
driver that was widely used on MS-DOS in Japan. A sample definition for 24-pin
Epson ESC/P printers (`escp_24.src`) is included.

It was added to Ghostscript in 2007 and removed in July 2020. This directory
preserves the last copy, from the Ghostscript 9.52 release.

## Status

- **Source:** preserved untouched, byte-identical to the files in
  `contrib/japanese/` of Ghostscript 9.52.
- **Build:** does **not** compile, not even with Ghostscript 9.52 itself, see
  [Build status](#build-status). This matches the removal commit, which says
  it had not built since before Ghostscript 8.71.
- **Hardware:** no dot matrix printer was available for testing. Nothing here
  has been verified on a real printer.

## Directory layout

````
dmprt/
├── original-source/
│   └── ghostscript-9.52/
│       ├── gdevdmpr.c          # driver source, untouched
│       ├── dviprlib.c          # dviprt definition file library, untouched
│       ├── dviprlib.h          # untouched
│       ├── dmp_init.ps         # PostScript init file, untouched
│       ├── dmp_site.ps         # sample site configuration, untouched
│       ├── escp_24.src         # sample dviprt definition (ESC/P 24 pin), untouched
│       ├── doc/
│       │   └── gdevdmpr.txt    # original documentation (Japanese, EUC-JP), untouched
│       └── dmprt-contrib.mak   # dmprt build rules, extracted unchanged from contrib/contrib.mak
└── README.md
````

`dmprt-contrib.mak` contains only the dmprt lines of `contrib/contrib.mak`
from Ghostscript 9.52 (the device list comment and the "Dot matrix printer
device" section), copied without changes. In 9.52 the target
`install-contrib-extras` also depended on `extra-dmprt-install`.

## Provenance record

| Field | Value |
|---|---|
| Driver | `dmprt` (Ghostscript device), files `gdevdmpr.c`, `dviprlib.c`, `dviprlib.h` |
| Driver version | 2.01, for Ghostscript 4.03 (from `doc/gdevdmpr.txt`) |
| Preserved version | Copy shipped in Ghostscript 9.52 (`contrib/japanese/`) |
| Ghostscript tag | `ghostpdl-9.52` (ArtifexSoftware/ghostpdl@e49830f8e, 2020-03-19) |
| Original author | ASAYAMA Kazunori (淺山和典), with NEC PC-H98, IBM PC/AT and 386BSD changes by others, per the source header |
| Last upstream change | 1997-05-13, per the history in `doc/gdevdmpr.txt` |
| Added to Ghostscript | 2007, by Till Kamppeter, when merging ESP Ghostscript into Ghostscript (ArtifexSoftware/ghostpdl@2bce78bce, 2007-06-06, after earlier attempts in May 2007 were backed out) |
| Removed from Ghostscript | ArtifexSoftware/ghostpdl@f2e2e3c1b (Chris Liddell, 2020-07-06), last release with it: 9.52 |
| Separate upstream project | Not found. A web search only found copies inside Ghostscript source trees (for example the Debian ghostscript 9.06 source package and the DragonFly BSD ghostscript 9.52 port). The original package also contained files that Ghostscript did not ship (`COPYING`, `gdevdmpr.mak`, `testpage.ps`) |
| License | GPL (see below) |
| Hardware testing | Not tested on a real printer |

### SHA256

| File | SHA256 |
|---|---|
| `gdevdmpr.c` | `cbe2f30e535b4842c4128dd00226530c8b7e11db61fb0ae1f290f3ff652507f1` |
| `dviprlib.c` | `4de2196fa4df62aeb0c35e8453d9181ab7a2ff6b27d965155f49cefc02af7344` |
| `dviprlib.h` | `3cc3d2213cb82fa32a47411014ff8c0c585b4d9642be0e58a4ac062a96a80884` |
| `dmp_init.ps` | `9e918e4739cbd11d3779b6dc66526eeff420bf236b51d3f51148faa711365ac4` |
| `dmp_site.ps` | `aab47831317229c6dfcdb52e5e280400d6f0f50ce8e6a3067f20788f0a1f41a0` |
| `escp_24.src` | `baf55e20954249060cf9249f7afcba7300c9d56d283040ed6efcf474f8a35ef2` |
| `doc/gdevdmpr.txt` | `82c3253d7ab4c9aad977b247cecfd2ad10e4a2355407c7f084ba5b408173e033` |

### Reason for removal

From the removal commit: the device "has code which triggers security
warnings, it has not built as it stands since before 8.71 (so >10 years) and
has significant (segfaulting) problems when modified to successfully build".
Since no one had complained in over ten years, it was removed.

`dviprlib.h` was not deleted by that commit and is still in the Ghostscript
source tree.

### License

Section 8 of `doc/gdevdmpr.txt` (in Japanese) says the copyright belongs to
the author, that the package comes with no warranty, and that the conditions
for redistribution and modification follow the GNU General Public License,
described in the file `COPYING` of the original package. No GPL version is
named.

The C and PostScript files carry an older header that refers to the
"Ghostscript General Public License" (Aladdin Enterprises). As part of
Ghostscript 9.52, they were distributed under the GNU Affero General Public
License, version 3 or later, like the rest of Ghostscript.

## Build status

Tested with Ghostscript 9.52 on Linux Mint 22.3 (based on Ubuntu 24.04) with
gcc 13.3.0. dmprt is not part of the default build, so only its object files
were built:

```sh
wget https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs952/ghostscript-9.52.tar.gz
# SHA256 c2501d8e8e0814c4a5aa7e443e230e73d7af7f70287546f7b697e5ef49e32176
tar xf ghostscript-9.52.tar.gz
cd ghostscript-9.52
./configure --without-x --disable-cups --disable-gtk --disable-fontconfig --disable-dbus
make obj/arch.h
make obj/dviprlib.o
make obj/gdevdmpr.o
```

- `dviprlib.c` compiles, with warnings only.
- `gdevdmpr.c` fails with 4 errors:

```
./base/gxstdio.h:34:16: error: 'gs_stderr' undeclared (first use in this function); did you mean 'stderr'?
./base/gxstdio.h:34:16: error: 'gs_stderr' undeclared (first use in this function); did you mean 'stderr'?
./contrib/japanese/gdevdmpr.c:850:3: error: 'gp' undeclared (first use in this function); did you mean 'fp'?
./contrib/japanese/gdevdmpr.c:873:12: error: 'gs_errpr_ioerror' undeclared (first use in this function); did you mean 'gs_error_ioerror'?
```

The errors at lines 850 and 873 look like typos made when the file was last
updated for Ghostscript API changes (`gp` instead of `fp`, `gs_errpr_ioerror`
instead of `gs_error_ioerror`). They were left as they are, because the
removal commit says the driver still crashes after being made to build, and
no printer is available to test a fix.
