# cp50: Ghostscript driver for the Mitsubishi CP50 colour printer

`cp50` is a Ghostscript printer device for the Mitsubishi CP50 colour
printer. It was contributed to Ghostscript by a user (Michael Hu,
michael@ximage.com), was part of Ghostscript from 1998, and was removed in
Ghostscript 9.23.

This directory preserves the last copy of the driver, from the
Ghostscript 9.22 release.

## Status

- **Source:** preserved untouched, byte-identical to `devices/gdevcp50.c`
  in Ghostscript 9.22.
- **Build:** builds and runs as part of Ghostscript 9.22. It does **not**
  compile against current Ghostscript (10.08.0) without changes, see
  [Build status](#build-status).
- **Hardware:** the original printer was **not available** for testing.
  Nothing here has been verified on a real printer.

## Directory layout

````
cp50/
├── original-source/
│   └── ghostscript-9.22/
│       ├── gdevcp50.c          # driver source, untouched
│       └── cp50-contrib.mak    # cp50 build rule, extracted unchanged from devices/contrib.mak
└── README.md
````

`cp50-contrib.mak` contains only the cp50 section of `devices/contrib.mak`
from Ghostscript 9.22 (the lines removed by the removal commit), copied
without changes.

## Provenance record

| Field | Value |
|---|---|
| Driver | `cp50` (Ghostscript device), file `gdevcp50.c` |
| Preserved version | Copy shipped in Ghostscript 9.22 (`devices/gdevcp50.c`) |
| Ghostscript tag | `ghostpdl-9.22` (2017-10-04) |
| SHA256 of `gdevcp50.c` | `bac3f322b8f382c50033e3be9ce8e75e236915787602a1f2f31bf4c3798a75ea` |
| Original author | Contributed by a user, Michael Hu (michael@ximage.com), per the source |
| Added to Ghostscript | 1998, ArtifexSoftware/ghostpdl@eec0ef527 (`gs/src/gdevcp50.c`) |
| Removed from Ghostscript | ArtifexSoftware/ghostpdl@0c9d19b94 (Ken Sharp, 2018-02-02), first release without it: 9.23 |
| Separate upstream project | None found; Ghostscript's copy appears to be the original source |
| License | AGPL-3.0-or-later as part of GPL Ghostscript 9.22 (see below) |
| Hardware testing | Not tested on a real Mitsubishi CP50 |

### Reason for removal

From the removal commit: after recent changes the device entered clist
(banding) mode more often, and in clist mode it "calls a device method which
should never be called". The call could have been removed, but there was no
way to check the output, so the device was removed. The commit says it would
be reinstated if someone can test the output on the printer.

### License

The 9.22 file header refers to the file `LICENSE`. Ghostscript 9.22's
`LICENSE` says GPL Ghostscript is distributed under the GNU Affero General
Public License, version 3 or later. That file lists the directories that are
part of GPL Ghostscript but does not name `devices/` (created in 2013 by
ArtifexSoftware/ghostpdl@f1488c5af); it is treated here as AGPL-3.0-or-later
like the rest of Ghostscript 9.22. When the driver was added in 1998, it was
under the Aladdin Ghostscript Free Public License.

## Build status

Tested on Ubuntu 24.04.5 with gcc 13.3.0.

### With Ghostscript 9.22 (works)

cp50 is part of the default build of Ghostscript 9.22, so no extra files
are needed:

```sh
wget https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs922/ghostscript-9.22.tar.gz
# SHA256 7f5f4487c0df9dce37481e4c8f192c0322e4c69f5a2ba900a7833c992331bcf4
tar xzf ghostscript-9.22.tar.gz
cd ghostscript-9.22
./configure
make
./bin/gs -dBATCH -dNOPAUSE -dSAFER -sDEVICE=cp50 -sOutputFile=out.cp50 examples/tiger.eps
```

Results:
- `gdevcp50.c` compiles without errors and `gs -h` lists `cp50`.
- The command above exits with status 0 and writes 1,137,619 bytes.
- With `-dMaxBitmap=0` (forces banding/clist), Ghostscript prints
  `Unrecoverable error, exit code 255` and exits with status 1. The output
  file is byte-identical to the normal run, so the error happens after the
  page data is written. This matches the reason for removal above.

Note that Ghostscript 9.22 is from 2017 and no longer receives fixes.

### With current Ghostscript 10.08.0 (does not build)

Adding the untouched driver to Ghostscript 10.08.0 (copy `gdevcp50.c` to
`devices/`, append `cp50-contrib.mak` to `devices/dcontrib.mak`, then
`./configure && make DEVICE_DEVS_EXTRA='$(DD)cp50.dev'`) fails, because the
Ghostscript device API changed after 9.22:

````
devices/gdevcp50.c:50:3: warning: implicit declaration of function 'prn_color_procs'
devices/gdevcp50.c:50:3: error: invalid initializer
devices/gdevcp50.c:62:47: error: incompatible types when initializing type 'void (*)(gx_device *)' using type 'gx_device_procs'
devices/gdevcp50.c:74:1: error: conflicting types for 'cp50_print_page'; have 'int(gx_device_printer *, FILE *)'
devices/gdevcp50.c:193:16: error: 'gs_clist_device_procs' undeclared
````

Making it build with current Ghostscript would need a separate patch,
which is not part of this preservation.
