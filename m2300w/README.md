# m2300w

Preservation of the legacy m2300w Linux printer driver for the
Konica Minolta magicolor 2300W and 2400W.

## Upstream source

- Project: m2300w
- Upstream version: 0.51
- Upstream archive naming documented by INSTALL: m2300w-<version>.tar.gz
- Verified source archive: m2300w_0.51.orig.tar.gz
- SHA-256:
  7a7dcede416bfd460f0d6528dd56b80fd8e63f055a57b87d93f3a0bcfbd2ca4f
- MD5:
  ba340a6ea545052aa2d1b634e9de1919

The Debian source archive was compared with the Git upstream/0.51
tree and the contents matched exactly.

The upstream source identifies version 0.51 in configure.in and
configure.

## License

The upstream source is licensed under the GNU General Public License,
version 2 or later.

The original COPYING file is preserved unchanged.

## Driver scope

The upstream README describes m2300w as an open-source Linux driver for
the Konica Minolta magicolor 2300W and 2400W, primarily intended for
use with Foomatic.

The upstream README documents limitations including:

- 1200x600 high resolution support
- no status monitor
- no duplex support
- no consumable replacement support
- limited error recovery

## Upstream status

Version 0.51 is the upstream version preserved here. The source was
originally distributed through the m2300w SourceForge project.

Debian continues to carry the driver as version 0.51-15. This represents
downstream maintenance and should not be interpreted as continued
upstream development.

## Debian patches

The Debian source contains five downstream patches, preserved separately
under patches/debian/:

1. Include CPPFLAGS in CFLAGS and LDFLAGS in LIBS.
2. Fix the Minolta magicolor 2400W PPD so it passes cupstestppd.
3. Add -dNOINTERPOLATE to the Ghostscript command line.
4. Search /usr/lib/cups/filter for foomatic-rip and update the
   dependency message.
5. Replace the Bash-specific let syntax with arithmetic expansion.

The Debian patches are kept separate from the original upstream source.

## OpenPrinting patch

Debian patch 0005 replaced the original shell command:

    let resX=$resX*$RESMUL;

with an arithmetic expansion used as a command. When the wrapper is
executed by /bin/sh (dash), this can result in a "not found" error.

The OpenPrinting patch changes this to a POSIX-compatible assignment:

    resX=$((resX * RESMUL))

The patch is stored under:

    patches/openprinting/0001-fix-shell-arithmetic-in-wrapper.patch

## Build

Prerequisites include:

- GCC
- GNU make
- Ghostscript
- Foomatic-RIP
- CUPS-related components as required by the build

Build the upstream source with:

    ./configure
    make

To apply the OpenPrinting patch to the original source:

    tar -xzf m2300w_0.51.orig.tar.gz
    cd m2300w-0.51
    patch -p1 < patches/openprinting/0001-fix-shell-arithmetic-in-wrapper.patch
    ./configure
    make

## Testing

The m2300w_0.51.orig.tar.gz archive was verified against the
upstream/0.51 Git tree and the contents matched exactly.

The OpenPrinting patch applies cleanly to the untouched upstream archive.

After applying the patch:

- ./configure succeeded.
- make -j$(nproc) succeeded.
- Existing compiler warnings were observed in the legacy C source,
  but the build completed successfully.
- The patched driver was tested through foomatic-rip using the
  magicolor_2300W-m2300w.ppd PPD.
- The test completed with exit status 0.
- Approximately 34 KiB of printer output was generated.
- No shell arithmetic "not found" error occurred during the test.

No physical m2300w or m2400w printer was available for hardware testing.
Therefore this preservation work does not claim hardware-level printing
compatibility.

## Directory layout

    original-source/
    patches/
      debian/
      openprinting/

The original upstream source is kept separate from Debian and
OpenPrinting modifications.
