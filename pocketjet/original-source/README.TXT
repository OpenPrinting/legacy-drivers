Linux CUPS Driver for Pentax PocketJet II/200/3/3plus Printers
CUPS Driver Version 2.0.0
November 18, 2004

-------------------------------------------------------------------------------
A) Installing the Driver
-------------------------------------------------------------------------------
1. Unzip the driver package by entering:
tar xvzf pocketjet_CUPS.tar.gz

2. cd into the directory created

3. enter "./configure".

4. enter "make"

5. Normally, the next "standard" step would be to enter "make install".
However, this is ***NOT SUPPORTED*** by our installation at this time.

Instead, you will need to perform two additional, manual steps to copy the
driver to the proper location.

 a) To install the package copy the `rastertopocketjet' file to the
     CUPS filter directory indicated by `cups-config'. Typically the
     following command should work, but you will likely need root
     access:

     cp rastertopocketjet `cups-config --serverbin`/filter

 b)   Next, copy the PPD files to the CUPS PPD directory indicated by
    `cups-config --datadir'.  Typically the following command should
     work, but again you will likely need root access:

     cp *.ppd `cups-config --datadir`/model

For additional details about this process, please view the "Install" file
in the package.

6. Restart CUPS

Log in as root and enter **ONE** of the following commands: 

killall -HUP cupsd 

/etc/init.d/cups restart 

/etc/software/init.d/cups restart 

7. Use the CUPS tools provided by your distribution to add the print queue.

If that doesn't work, you should use the standard "web interface" tool
provided with CUPS. To use this web interface tool, open your web browser to
the following page:

http://localhost:631/admin 

Then, you should use the Add printer tool and follow the instructions.

8. The "web interface" can be used to set default printer settings, such as
Density, FormFeed mode, PaperSize, and several other custom features.
This is the recommended place to select the printer options, because the 
services provided with your distribution may not provide access to these
printer features.

-------------------------------------------------------------------------------
B) Additional Information
-------------------------------------------------------------------------------

For more information about CUPS, please visit http://cups.org

If you are having problems, please contact Technical Support at (800)543-6144
