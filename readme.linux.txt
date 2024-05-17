CTESK 2.9
============

System Requirements
-------------------

1. Core utilities + gcc, make, gdb
2. Sun J2SE 6.0 or higher (http://java.sun.com)
3. Eclipse RCP/Platform Runtime Binary 3.4 or higher (http://eclipse.org)
4. Eclipse C/C++ Development Tools 5.0.1 or higher (http://eclipse.org)
   (Eclipse CDT Testing Feature depends on org.junit plug-in.
    If org.junit isn't installed CDT Testing Feature should not be installed)
5. UniTESK Reports 2.7 or higher (http://forge.ispras.ru/projects/utt)


Installation
------------

1. There is 'dropins' subdirectory in Eclipse installation directory.
   CTESK distribution should be unpacked to this subdirectory.
   For easy future updates it's recommended
   to rename enclosed directory ctesk-${version} to ctesk
   (or some other name without version number).
   The shell script 'bin/sec.sh' starts CTESK translator from the command line.
2. Set environment variable CTESK_HOME - path to unpacked archive.
3. Add directory %CTESK_HOME%/bin to PATH.
4. Install UniTESK Trace Tools to generate reports on test results.


Update
------
1. Close Eclipse before update to new version of CTESK.
2. Remove the contents of directory where CTESK is installed.
3. Start and close Eclipse.
4. Before installation, check System Requirements section
   in readme file of the new version and update your system if needed.
5. Install the new version of CTESK as described in Installation section.


Contacts
--------

Project page: http://forge.ispras.ru/projects/ctesk
Technical support is available through:
http://forge.ispras.ru/projects/ctesk/issues

UniTESK Lab 
www:    http://unitesk.com
e-mail: info@unitesk.com