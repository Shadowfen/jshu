#!/bin/bash

# increment_build.sh
#
# This script allows for simple maintenance of an informational text 
# file containing information useful for specification in an rpm spec file.
#
# For example, one such file might contain:
#	ProductName: Alidnis Network Defense System
#	PkgName: alidnis
#	Version: 1.0.0
#	BuildNumber: 1
#	Epoch: 64
#	BuildHost: ananke.shadowguard
#	Date: Wed Sep  5 13:26:20 CDT 2012
#
# The BUILDNUM_TXT variable (below) has the name of your buildnumber.txt
# file (including any necessary directory info). If the
# BUILDNUM_TXT file does not exist, it will be created 
# with dummy values. You can then edit the BUILDNUM_TXT
# file to provide product and package names, etc.
#
# Usage:
#	increment_build.sh incr_option [alternate buildnumber.txt file]
#
# where incr_option is one of the following:
#   super - the first number of the version triple a.b.c
#   major - the second number of the version triple a.b.c
#   minor - the third number of the version triple a.b.c
#   build - the build number
# and the alternate buildnumber.txt file is the name of the file that
# you wish to keep your info in different from the default "buildnumber.txt".
#
# When the incr_option is:
#   super - the super number is incremented,
#           the major and minor numbers are reset to zero,
#           and build number is reset to one
#   major - the major number is incremented,
#           the minor number is reset to zero,
#           and build number is reset to one
#   minor - the minor number is incremented,
#           and build number is reset to one
#   build - the build number is incremented
#
# The epoch number is always incremented and never reset.
#
# One way you can use this is in the Makefile where you
# build your rpm. Define the following to get access to
# the values (add directory info to "buildnumber.txt" 
# if necessary):
#   PRODNAME = $(shell /bin/grep ^ProductName: buildnumber.txt | /bin/cut -d: -f2)
#   PKGNAME = $(shell /bin/grep ^PkgName: buildnumber.txt | /bin/awk '{print $$2}')
#   VERSION = $(shell /bin/grep ^Version: buildnumber.txt | /bin/awk '{print $$2}')
#   RELEASE = $(shell /bin/grep ^BuildNumber: buildnumber.txt | /bin/awk '{print $$2}')
#   EPOCH = $(shell /bin/grep ^Epoch: buildnumber.txt | /bin/awk '{print $$2}')
#
# Add the parameters to your rpmbuild command to pass them in. For example:
#	cd $(RPMBUILD)/SPECS; \
#	rpmbuild --define '_topdir $(RPMBUILD)' \
#		--define 'prodname $(PRODNAME)' \
#		--define 'pkgname $(PKGNAME)' \
#		--define 'version $(VERSION)' \
#		--define 'release $(RELEASE)' \
#		--define 'epoch $(EPOCH)' \
#		-bb $(PKGNAME).spec
#
# And finally, use the variables you defined in the rpmbuild
# command inside your spec file. For example:
#    Summary: %{prodname} installation package
#    Name: %{pkgname}
#    Version: %{version}
#    Release: %{release}
#    BuildArch: i686
#    Epoch: %{epoch}
#
#
#
#-----------------------------------------------------------
# License: Simplified BSD:
#
# Copyright 2012, Dolores Scott. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this list of
#      conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright notice, this list
#      of conditions and the following disclaimer in the documentation and/or other materials
#      provided with the distribution.

# THIS SOFTWARE IS PROVIDED BY Dolores Scott ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Dolores Scott OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BUILDNUM_TXT=./buildnumber.txt

# global variables used in the script:
#    PRODNAME, PKGNAME, VERSION, BUILDNUM, EPOCH,
#    BUILDHOST, BUILDDATE



# get the name of the buildnumber file to use
getBldNumFilename() {
	if [ $# -lt 1 ] ; then
		filename="buildnumber.txt"
	else
		filename="$1"
	fi
	if [ ! -n "$filename" ] ; then
		filename="buildnumber.txt"
	fi
	echo $filename
	unset filename
}

writeBldNumFile() {
	filename="$1"
	
	echo "ProductName:$PRODNAME" >"${filename}"
	echo "PkgName: $PKGNAME" >>"${filename}"
	if [ -n "$VERSION_MINOR" ] ; then
		ver="$VERSION_SUPER.$VERSION_MAJOR.$VERSION_MINOR"
	else
		ver="$VERSION_SUPER.$VERSION_MAJOR"
	fi
	echo "Version: $ver" >>"${filename}"
	echo "BuildNumber: $BUILDNUM" >>"${filename}"
	echo "Epoch: $EPOCH" >>"${filename}"
	echo "BuildHost: $BUILDHOST" >>"${filename}"
	echo "Date: $BUILDDATE" >>"${filename}"
	unset filename ver
}


parseVersion() {
	VERSION_SUPER=${VERSION%%.*}
	VERSION_MINOR=${VERSION##*.}
	vermajmin=${VERSION#*.}
	VERSION_MAJOR=${vermajmin%.*}
	if [ "$vermajmin" = "$VERSION_MAJOR" ] ; then
		# we don't actually HAVE a minor version
		VERSION_MINOR=""
	fi
}

# only creates a file if it does not already exist!!
# will not overwrite an existing file
createBldNumFile() {
	filename="$1"
	if [ ! -n "$filename" ] ; then
		return 1
	fi
	if [ ! -e "$filename" ] ; then
		/bin/echo "Could not find $filename"
		BLDIR=${filename%/*}
		if [ "$BLDIR" != "$filename" ] ; then
			if [ ! -d $BLDIR ]; then
				echo "$BLDIR directory does not exist... aborting" >&2
				return 1
			fi
		fi
		/bin/echo "   Creating it."
		PRODNAME=" Unknown product" # yes, the initial space is deliberate
		PKGNAME="unknown_pkg"
		VERSION="0.0.0"
		parseVersion
		BUILDNUM="0"
		EPOCH="0"
		BUILDHOST=`/bin/uname -a | /bin/awk '{ print $2}'`
		BUILDDATE=`/bin/date`
		writeBldNumFile "$filename"
	fi
	unset filename
}

loadBldNumFile() {
	filename="$1"
	if [ ! -e "$filename" ] ; then
		return 1
	fi

	PRODNAME=`/bin/grep ^ProductName "$filename" | /bin/cut -d: -f2`
	PKGNAME=`/bin/grep ^PkgName "$filename" | /bin/awk '{print $2}'`
	VERSION=`/bin/grep ^Version "$filename" | /bin/awk '{print $2}'`
	BUILDNUM=`/bin/grep ^BuildNumber "$filename" | /bin/awk '{print $2}'`
	EPOCH=`/bin/grep "^Epoch" "$filename" | /bin/awk '{print $2}'`
	unset filename
}

incrSuper() {
	echo -e "\nIncrementing version super...\n"
	let VERSION_SUPER=$VERSION_SUPER+1
	VERSION_MAJOR=0
	VERSION_MINOR=0
	let BUILDNUM=1
	let EPOCH=$EPOCH+1
}

incrMajor() {
	echo -e "\nIncrementing version major...\n"
	let VERSION_MAJOR=$VERSION_MAJOR+1
	VERSION_MINOR=0
	let BUILDNUM=1
	let EPOCH=$EPOCH+1
}

incrMinor() {
	set -x -a -v
	echo -e "\nIncrementing version minor...\n"
	if [ -z "VERSION_MINOR" ]; then
		VERSION_MINOR=0
	else
		let VERSION_MINOR=$VERSION_MINOR+1
	fi
	let BUILDNUM=1
	let EPOCH=$EPOCH+1
	set +x +a +v
}

incrBuild() {
	echo -e "\nIncrementing build number...\n";
	let BUILDNUM=$BUILDNUM+1
	let EPOCH=$EPOCH+1
}

usage() {
	echo -e "USAGE:\n\t$0 super|major|minor|build [alternate buildnumber.txt file]"
}
#############################################################################################
# main
#    (Only execute the following if we were called by our real name - i.e. as a
#     main script. Otherwise assume we've been sourced for testing of the functions.)
#
if [ ${0##*/} == "increment_build.sh" ] ; then
	echo $@

	if [ -z "$1" ] || [ "$1" != "major" ] \
		&& [ "$1" != "minor" ] && [ "$1" != "build" ]\
		&& [ "$1" != "super" ]; then
		/bin/echo -e "\nERROR!  Invalid or missing parameter."
		usage
		exit 1
	fi
	opt="$1"
	if [ $# -lt 2 ]; then
		BUILDNUM_TXT=$(getBldNumFilename)
	else
		BUILDNUM_TXT=$(getBldNumFilename $2)
	fi
	createBldNumFile $BUILDNUM_TXT
	loadBldNumFile $BUILDNUM_TXT
	
	VERSION_SUPER=${VERSION%%.*}
	VERSION_MINOR=${VERSION##*.}
	vertmp=${VERSION#*.}
	VERSION_MAJOR=${vertmp%.*}
	case $opt in
	"super") 
		incrSuper
		;;
	"major")
		incrMajor
		;;
	"minor")
		incrMinor
		;;
	"build")
		incrBuild
		;;
	*)
		echo "ERROR: Unknown option \"$opt\""
		usage
		exit 1
		;;
	esac

	BUILDHOST=`/bin/uname -a | /bin/awk '{ print $2}'`
	BUILDDATE=`/bin/date`
	
	writeBldNumFile ${BUILDNUM_TXT}
fi
