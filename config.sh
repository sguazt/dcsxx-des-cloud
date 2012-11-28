#!/bin/bash

##
## Shell script for creating configuration include files.
##
## Authors:
##  Marco Guazzone, <marco.guazzone@mfn.unipmn.it>
##

##TODO:
## * we need to check if c++ (or other compiler??) is present


dtnow=$(date +'%F %T (%Z)')
logfile="$0.log"
basedir=$1
prjsubdir=dcs/des/cloud

CC=${CC:=cc}
CXX=${CXX:=c++}

## Declares user-related variables
USR_CPPFLAGS=
USR_CFLAGS=
USR_CXXFLAGS=
USR_LDFLAGS=


## Get the major version number of bash.
function dcs_bash_major_version()
{
	n=0
	found=0
	for ((i=0;i<${#BASH_VERSION} && !$found;i++))
	do
		ch=${BASH_VERSION:$i:1}
		case "$ch" in
			[0-9])
				n=$((n+$ch))
				;;
			'.')
				found=1
				;;
		esac
	done

	echo -n $n
}


## Convert to lower case
function dcs_tolower()
{
	ver=$(dcs_bash_major_version)

	s=
	if [ "$ver" -ge "4" ]; then
		declare -l s=$1
	else
		s=$(echo $1 | tr '[A-Z]' '[a-z]')
	fi
	echo -n $s
}


## Format a date
function dcs_format_date()
{
	echo $(date +'%F %T (%Z)')
}


## Initialize log file
function dcs_log_init()
{
	> $logfile
}


## Log an error to the log-file.
function dcs_log_error()
{
	date=$(dcs_format_date)
	echo "ERROR ($date)>> $1" >> $logfile
}


## Log an error to the log-file.
function dcs_log_info()
{
	date=$(dcs_format_date)
	echo "INFO ($date)>> $1" >> $logfile
}


## Log an error to the log-file.
function dcs_log_warn()
{
	date=$(dcs_format_date)
	echo "WARNING ($date)>> $1" >> $logfile
}


## Return yes or no
function dcs_yes_no()
{
	#declare -l v=$1
	v=$(dcs_tolower $1)
	if [ "$v" = "yes" ] || [ "$v" = "true" ] || [ "$v" = "1" ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the Boost libraries installed
function dcs_check_boost()
{
	src_temp_file=`mktemp tmp.XXXXXXXXXX`
	out_temp_file=`mktemp tmp.XXXXXXXXXX`
	cat > $src_temp_file <<EOT
#include <boost/version.hpp>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	compile_test=$($CXX $CXXFLAGS -x 'c++' -o $out_temp_file $src_temp_file 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the lp_solve libraries installed
function dcs_check_lpsolve()
{
	src_temp_file=`mktemp tmp.XXXXXXXXXX`
	out_temp_file=`mktemp tmp.XXXXXXXXXX`
	cat > $src_temp_file <<EOT
#include <lpsolve/lp_lib.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	compile_test=$($CXX $CXXFLAGS -x 'c++' -o $out_temp_file $src_temp_file 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Check if your system has the GLPK libraries installed
function dcs_check_glpk()
{
	src_temp_file=`mktemp tmp.XXXXXXXXXX`
	out_temp_file=`mktemp tmp.XXXXXXXXXX`
	cat > $src_temp_file <<EOT
#include <glpk/glpk.h>

int main(int argc, char* argv[])
{
	return 0;
}
EOT

	compile_test=$($CXX $CXXFLAGS -x 'c++' -o $out_temp_file $src_temp_file 2>&1 >/dev/null)
	ret=$?

	rm -f $out_temp_file $src_temp_file

	if [ $ret == 0 ]; then
		echo -n "yes"
	else
		echo -n "no"
	fi
}


## Script starts here

dcs_log_init
dcs_log_info "Starting configuration maker..."

## Control variables
have_boost=
have_boost_bool=
want_lpsolve=$(dcs_yes_no $USR_EESIM_USE_LPSOLVE)
have_lpsolve=
have_lpsolve_bool=
want_glpk=$(dcs_yes_no $USR_EESIM_USE_LPSOLVE)
have_glpk=
have_glpk_bool=
basesrcdir=


## Sanity checks
if [ -z "$basedir" ]; then
	basedir=.
fi
if [ -z "$basesrcdir" ]; then
	basesrcdir=$basedir/inc
fi


## Load user-related variables
if [ -e "$basedir/user-config.sh" ]; then
	source "$basedir/user-config.sh"
fi


## Set C/C++ environments
if [ ! -z "$USR_CPPFLAGS" ]; then
	CPPFLAGS="$CFLAGS $USR_CPPFLAGS"
fi
if [ ! -z "$USR_CFLAGS" ]; then
	CFLAGS="$CFLAGS $USR_CFLAGS"
fi
if [ ! -z "$USR_CXXFLAGS" ]; then
	CXXFLAGS="$CXXFLAGS $USR_CXXFLAGS"
fi
if [ ! -z "$USR_LDFLAGS" ]; then
	LDFLAGS="$LDFLAGS $USR_LDFLAGS"
fi


## Check for Boost
have_boost=$(dcs_check_boost)
if [ "$have_boost" = "yes" ]; then
	have_boost_bool='true'
else
	have_boost_bool='false'

	dcs_log_error "Wanted 'Boost' but 'Boost' not found."
	found_errors=yes
fi


## Check for lp_solve
have_lpsolve=$(dcs_check_lpsolve)
if [ "$have_lpsolve" = "yes" ]; then
	have_lpsolve_bool='true'
else
	have_lpsolve_bool='false'
fi

## Check for GLPK
have_lpsolve=$(dcs_check_lpsolve)
if [ "$have_glpk" = "yes" ]; then
	have_glpk_bool='true'
else
	have_glpk_bool='false'
fi

if [ "$want_lpsolve" = "yes" ] && [ "$have_lpsolve" != "yes" ]; then
	dcs_log_error "Wanted 'lp_solve' but 'lp_solve' not found."
	found_errors=yes
elif [ "$want_glpk" = "yes" ] && [ "$have_glpk" != "yes" ]; then
	dcs_log_error "Wanted 'GLPK' but 'GLPK' not found."
	found_errors=yes
elif [ "$want_lpsolve" = "yes" ] && [ "$want_glpk" = "yes" ]; then
	have_lpsolve_bool='true'
	have_glpk_bool='false'
elif [ "$want_lpsolve" != "yes" ] && [ "$want_glpk" != "yes" ]; then
	have_lpsolve_bool='true'
	have_glpk_bool='false'
fi

## Get version
version=$(cat $basedir/VERSION)


## Auto-generation of files.

if [ "$found_errors" = "yes" ]; then
	dcs_log_info "Configuration maker has finished."
	dcs_log_error "Exit with errors!"

	echo "There are some errors. Check the $logfile log-file."
	exit 1
fi


# Create base config dir
#mkdir -p "$basesrcdir/config"
mkdir -p $basesrcdir/$prjsubdir/detail

cat > $basedir/config.mk <<EOT
## \file config.mk
##
## \brief Configurations for the Make build system.
##
## [$dtnow]
## This is an autogenerated file. Do not edit.
##
## \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
##

CPPFLAGS_release+=$USR_CPPFLAGS
CFLAGS_release+=$USR_CFLAGS
CXXFLAGS_release+=$USR_CXXFLAGS
LDFLAGS_release+=$USR_LDFLAGS
CPPFLAGS_debug+=$USR_CPPFLAGS
CFLAGS_debug+=$USR_CFLAGS
CXXFLAGS_debug+=$USR_CXXFLAGS
LDFLAGS_debug+=$USR_LDFLAGS
CPPFLAGS_test+=$USR_CPPFLAGS
CFLAGS_test+=$USR_CFLAGS
CXXFLAGS_test+=$USR_CXXFLAGS
LDFLAGS_test+=$USR_LDFLAGS
CPPFLAGS_xmp+=$USR_CPPFLAGS
CFLAGS_xmp+=$USR_CFLAGS
CXXFLAGS_xmp+=$USR_CXXFLAGS
LDFLAGS_xmp+=$USR_LDFLAGS
EOT

cat > $basesrcdir/$prjsubdir/detail/config.hpp <<EOT
/** \file dcs/des/cloud/config.hpp
 *
 * \brief Configuration for this library.
 *
 * [$dtnow]
 * This is an autogenerated file. Do not edit.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_CONFIG_HPP
#define DCS_EESIM_CONFIG_HPP


#define DCS_EESIM_CONFIG_USE_GLPK $have_glpk_bool
#define DCS_EESIM_CONFIG_USE_LPSOLVE $have_lpsolve_bool


#endif // DCS_EESIM_CONFIG_HPP
EOT

cat > $basesrcdir/$prjsubdir/detail/version.hpp <<EOT
/** \file dcs/des/cloud/version.hpp
 *
 * \brief Version of this library.
 *
 * [$dtnow]
 * This is an autogenerated file. Do not edit.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_VERSION_HPP
#define DCS_EESIM_VERSION_HPP


#define DCS_EESIM_VERSION $version
#define DCS_EESIM_MAJOR_VERSION (DCS_EESIM_VERSION / 100000)
#define DCS_EESIM_MINOR_VERSION (DCS_EESIM_VERSION / 100 % 1000)
#define DCS_EESIM_PATCH_VERSION (DCS_EESIM_VERSION % 100)


#endif // DCS_EESIM_VERSION_HPP
EOT

dcs_log_info "Configuration maker has finished."
