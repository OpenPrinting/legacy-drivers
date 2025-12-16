# OpenPrinting Legacy Printer Driver Repository

In this repository we are hosting printer drivers which are not maintained any more by their original authors. This way they stay available and printers keep working to not turn into e-waste. This is an important contribution to sustainability.

If we discover in some form that a printer driver is not maintained any more by its original author, ideally they told it to us, but also in case of being abandoned and especially their web site having gone away, we add the source code of that driver to this repository, in a directory named by the driver's name. We apply the patches of the Debian package of the driver to it, as these are usually adapting the code to the current compilers and systems, fixing bugs, and sometimes even adding extra printer models.

## License

All the drivers collected here are free software, but they are published under different licenses. See the license information in each driver's subdirectory.

## How to use

Free software printer drivers can be used with many different operating systems and so legacy printers which do not receive support any more by their manufacturers or by operating system publishers do not need to get thrown away. If there is a Linux/free software driver available for them, they can be continued to be used.

### Linux, Unix, *BSD, ...

These operating systems use CUPS as print environment. 

#### CUPS 2.x or older

To make the drivers in this repository work with CUPS, install your operating systems's packages of the driver needed, or if there is no such package, build the driver from this repository get a PPD file for your printer with that driver from our [printer/driver database](https://www.openprinting.org/printers/). Then you can set up a print queue.

#### CUPS 3.x or newer

CUPS 3.x does not support classic printer drivers and PPD files any more, but for using legacy printers which need drivers you use Printer Applications, software emulations of driverless IPP printers.All free/open source printer drivers available can be used with Printer Applications. If your OS distribution supports the [Snap](https://snapcraft.io/) package format, you can find all available Printer Applications in the Snap Store: [from OpenPrinting](https://snapcraft.io/publisher/openprinting) and also [LPrint](https://snapcraft.io/lprint) and [HP Printer App](https://snapcraft.io/hp-printer-app).

Want to know more details and alternatives to get them, here is more about [Printer Applications](https://openprinting.github.io/achievements/#all-free-drivers-in-a-ppd-less-world---or---all-free-drivers-in-snaps).

All the drivers here are available in the Printer Application Snaps and you can also use them with Printer Applications built from source.

### Windows

All printer drivers which can be used under Linux, especially also the ones provided in this repository can also be used under Windows, by installing Ubuntu under WSL and installing the appropriate Printer Application from the Snap Store. See these [instructions](https://openprinting.github.io/wsl-printer-app/).

### macOS

Similar to the way how it works under Windows it should also work under macOS. You install Ubuntu in a virtual machine and install the Printer Applications from the Snap Store.

## Available drivers

### c2050

Printer driver for **Lexmark 2050 Color Jetprinter**

Filter to convert a Postscript file to Lexmark 2050 format. This driver allows you to print at 300dpi in color on A4 paper.

### min12xxw

Printer driver for **(Konica) Minolta PagePro 1[234]xxW**

A printer driver for the (Konica) Minolta PagePro 1200W, 1250W, 1300W, 1350W, and 1400W.

