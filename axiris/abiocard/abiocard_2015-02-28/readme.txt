
-------------------------------------------------------------------------------
abiocard package                                                     2015-02-28

===============================================================================


Overview
--------

This package contains:

* Program abiocardserver (Linux, Windows).
* Program abiocardtime (Linux).

The package can be used on Linux and Windows.

The package is available in two archive formats:

* abiocard.tar.gz
* abiocard.zip

They both contain the same files. Format .tar.gz is meant for use in Linux as
it stores the correct file permissions.


Unpack in Windows
-----------------

1. Copy the .zip file to a local directory.

2. Right-click on the .zip or .tar.gz file and extract locally. You may have to
   install an extraction tool first if the file explorer doesn't offer an
   extraction command.


Unpack in Linux
---------------

1. Copy the .tar.gz file to a local directory in your home directory.

2. Unpack the archive file:

        $ tar -xzp -f abiocard.tar.gz

   If you're running a graphical interface like GNOME or KDE, the system likely
   offers an interactive way to unpack the archive (usually by right-clicking
   on the archive file).


Installation in Windows
-----------------------

There's no installation procedure in Windows.


Installation in Linux
---------------------

The package contains installation scripts for RPi computers. You should run an
installation script from a command shell using your regular login. The script
will ask for your sudo password to acquire root privileges.

install_abiocard_rpi.sh: Installs abiocardserver and abiocardtimer on your RPi.
both programs are added to the crontab of the root. Use this script if you're
using an AbioCard model A or B.

install_abiortc_rpi.sh: Installs abiocardtimer on your RPi. The program is
added to the crontab of the root. Use the script if you're using an AbioRTC or
AbioWire.


Executables
-----------

The Windows version of the abiocardserver program is included in the package.
The file (abiocardserver.exe) reside in the abiocard/ directory.

Note: There's no Windows version of program abiocardtime. The program is
designed to run specifically on Linux.

You need to build the programs in Linux. The makefiles are located in the
abiocard/ directory. First change to that directory, then build the programs:

    $ make -f abiocardserver.mk
    $ make -f abiocardtime.mk

The executable files (abiocardserver, abiocardtime) are created in the same
directory.

If you need to rebuild the programs from scratch, then first clean up the old
files:

    $ make -f abiocardserver.mk clean
    $ make -f abiocardtime.mk clean

You're supposed to run the programs from the command line prompt. 


References
----------

Please refer to the AbioCard user manual (pdf) for more information.
