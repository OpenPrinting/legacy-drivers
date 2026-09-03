# m2300w

Preserved legacy Linux printer driver for the Konica Minolta magicolor
2300W and 2400W.

## Upstream source

- Project: m2300w
- Upstream version: 0.51
- License: GNU General Public License, version 2 or later
- Source archive: `m2300w_0.51.orig.tar.gz`

The preserved driver is based on the original upstream 0.51 source.

The verified source archive matches the `upstream/0.51` source tree exactly.

The upstream documentation identifies version 0.51 and describes the driver
as an open-source Linux driver for the Konica Minolta magicolor 2300W and
2400W, primarily intended for use with Foomatic.

## Preservation process

The preserved source follows this transformation:

```text
upstream 0.51
    ↓
Debian patches 0001–0005
    ↓
OpenPrinting compatibility fix
