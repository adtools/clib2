#!/bin/sh

# Simple release archive build script for Unix systems.
# I typically build the library with a cross compiler.

# Set the name of the archive and the directory the
# files go into
export dir_name=clib2-1.`cat c.lib_rev.rev`

# Start with a clean slate
rm -rf $dir_name

# Create the directory, copy all the libraries, header files
# and release note files inside.
mkdir $dir_name
cp -a lib $dir_name
cp -a lib.threadsafe $dir_name
cp -a include $dir_name
cp changes $dir_name

# Delete the CVS data from the include directory
rm -rf ${dir_name}/include/CVS ${dir_name}/include/sys/CVS

# Finally, create an lha type 5 archive; the Amiga LhA port
# can unpack it.
lha ao5q ${dir_name}-gcc-os4.lha ${dir_name}

