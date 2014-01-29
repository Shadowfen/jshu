#!/bin/sh
#========================================================================
# Simplified unit test framework for shell script which
# produces junit-style xml results file (for Jenkins/Hudson).
#
# Include this file into your test shellscript using ". /usr/local/include/jshutest.inc"
# (or whatever path is appropriate to find this file).
#
# Create shell functions whose name ends with "Test" which will return
# ${jshuPASS} - (0) if the test passes
# ${jshuFAIL} - (1) if the test fails
# ${jshuERROR} - (2) if an error occured other than a test failure
#
# The meat of the test shell function performs the test. It can do so locally
# (inside this shell script) or it can call external shell scripts and
# programs. 
#
# Standard out and standard err are captured and included in the
# xml result report. Also, you may define two variables inside your 
# test function to do a regular expression search of the combined
# stdout/stderr. If the regular expression is found inside the 
# captured stdout/stderr, then the test fails.
#     ereg - regular expression pattern for Linux egrep(1)
#     icase - either "" (don't ignore case) or "-i" (ignore case)
#
# Inside a test shell function, you may set the shell variable errmsg to
# some value (typically used for error conditions to describe what happened)
# if you do not want the default errmsg "Error found". For example:
#    errmsg="Script ./idontexist.sh was existed!!"
#
# Inside a test shell function, you may say:
#    ${jshuTEST_SKIP}
# if you want the test to be skipped (not passing or failing) for whatever
# reason. This would typically be used at the top of the function.
#
# At the bottom of the test shell script:
#    ##############################################################
#    # main
#    ##############################################################
#    # initialize testsuite
#    jshuInit
#    
#    # run unit tests in this script
#    jshuRunTests
#    
#    # result summary
#    jshuFinalize
#    
#    echo Done.
#    echo
#    let tot=failed+errors
#    exit $tot
#
# NOTES:
# * You may wish to set the jshu_pkgname variable to the name you want
#   it to display for the "package" name at the top of the test script,
#   otherwise it will default to the name of the directory that the 
#   test script is in (or one above it, if the current directory is
#   named "test" or "tests").
# * Junit-style XML Jenkins result files will be written to ./results. You
#   can change the directory that the results dir is in by defining
#   the BUILDDIR variable at the top of the script.
# * You can use a test shell script to test functions in other shell scripts
#   by sourcing the script to be tested as long as that script has the
#   following if-then around the "main" (non-function) code:
#        if [ ${0##*/} == "${SCRIPT_NAME}" ] ; then
#            # "main" code goes here
#        fi
#   where ${SCRIPT_NAME} evaluates to the name that you use to invoke
#   the script.
#========================================================================
#License: Simplified BSD:
#
# Copyright 2014, Dolores Scott. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice, this list of
#       conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright notice, this list
#       of conditions and the following disclaimer in the documentation and/or other materials
#       provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY Dolores Scott ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Dolores Scott OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#========================================================================
jshuPASS=0
jshuFAIL=1
jshuERROR=2
jshuSKIP=12

jshuTEST_SKIP="return ${jshuSKIP}"

tests=0; errors=0; failed=0; total=0; skipped=0; content=""
date_cmd=`which date`

jshu_pkgname=""
jshu_suite=""
jshu_content_file=/var/tmp/jshu_content_$$.txt
resultDir=""
ereg=""

jshuSetup() {
	echo >/dev/null
}

jshuTeardown() {
	echo >/dev/null
}

jshuInit() {
	#look for BUILDDIR variable
	_jshu_blddir=`pwd`
	if [ -z ${BUILDDIR+x} ]; then 
		# BUILDDIR is unset
		resultDir=${_jshu_blddir}/results
	else
		# BUILDDIR is set
		_jshu_blddir=$BUILDDIR
		resultDir=${_jshu_blddir}/results
	fi
	# create output folder
	mkdir -p "$resultDir" 
	
	if [ "${jshu_pkgname}" == "" ] ; then
		jshuGetPkgName
	fi
	jshuGetSuiteName $0
	>$jshu_content_file
	unset _jshu_blddir
}

jshuFinalize() {
	test_file="${resultDir}/TEST_${jshu_pkgname}.${jshu_suite}.xml"
	## testsuite block
	
	echo "<testsuite failures=\"${failed}\" errors=\"$errors\" tests=\"$tests\" skipped=\"${skipped}\" name=\"${jshu_pkgname}.${jshu_suite}\" time=\"${total}\">" >${test_file}
	cat $jshu_content_file >>$test_file
	echo "</testsuite>" >>$test_file
	rm -f $jshu_content_file
	echo
	let totfail=errors+failed
	echo "-- Number of tests         = ${tests}"
	echo "-- Number of failing tests = ${totfail}"
}

jshuRunTests() {
	jshuSetup
	_jshu_tests=`extractTestFunctions $0`
	for t in $_jshu_tests ; do
		jshu_run_test $t
	done
	jshuTeardown
	unset _jshu_tests
}

# package name is parent directory name unless overridden
# suite name is the name of the test script (without .sh)
# class name is the name of the unit test function

# Test result file name is TEST_${pkgname}.${suite}.xml

jshuGetPkgName() {
	# create a package name based on the directory your test script
	# is in (if the current directory is "test" or "tests", then
	# use the name of the directory above it
	_jshu_curdir=`pwd`
	_jshu_parentdir=${_jshu_curdir##*/}
	if [ $_jshu_parentdir == "test" -o $_jshu_parentdir == "tests" ] ; then
		_jshu_curdir=${_jshu_curdir%/*}
		_jshu_parentdir=${_jshu_curdir##*/}
	fi
	jshu_pkgname=${_jshu_parentdir//./_}
	# convert embedded ' ' to '_'
	jshu_pkgname=${jshu_pkgname// /_}
	unset _jshu_curdir _jshu_parentdir
}

jshu_suite=""
jshuGetSuiteName() {
	# strip off directories
	_jshu_script_name=${1##*/}
	# strip off ".sh"
	_jshu_base_name=${_jshu_script_name%.sh}
	# convert embedded '.' to '_'
	jshu_suite=${_jshu_base_name//./_}
	# convert embedded ' ' to '_'
	jshu_suite=${jshu_suite// /_}
	unset _jshu_base_name _jshu_script_name
}

jshu_test=""
jshuGetTestName() {
	# strip off directories
	_jshu_script_name=${1##*/}
	# strip off ".sh"
	_jshu_base_name=${_jshu_script_name%.sh}
	# convert embedded '.' to '_'
	jshu_test=${_jshu_base_name//./_}
	# convert embedded ' ' to '_'
	jshu_test=${jshu_test// /_}
	unset _jshu_base_name _jshu_script_name
}


jshuRecordResult() {
	_jshu_type=$1
    _jshu_test_name=$2
	_jshu_errMsg=$3
	jshu_failure_msg=""
	case ${_jshu_type} in
		${jshuPASS})
			printf " %-69s -> PASSED\n" ${_jshu_test_name}
			;;
		${jshuFAIL})
			jshu_failure_msg="<failure type=\"failure\" message=\"Test Failed\"></failure>"
			printf " %-69s -> FAILED\n" ${_jshu_test_name}
			let failed=failed+1
			;;
		${jshuERROR})
			[ -n "${_jshu_errMsg}" ] && _jshu_errMsg="Error found"
			jshu_failure_msg="<failure type=\"error\" message=\"${_jshu_errMsg}\"></failure>"
			printf " %-69s -> FAILED\n" ${_jshu_test_name}
			let errors=errors+1
			;;
		${jshuSKIP})
			jshu_failure_msg="<skipped/>"
			printf " %-69s -> SKIPPED\n" ${_jshu_test_name}
			let skipped=skipped+1
			;;
	esac
	unset _jshu_type _jshu_test_name _jshu_errMsg
}

# create the xml for a testcase result and save it
# to the content file.
writeTestContent()  {
	_jshu_test_name=$1
	_jshu_time=$2
	_jshu_fail=$3
	_jshu_outf=$4
	_jshu_out=$(<$_jshu_outf)
  ## testcase tag
  _jshu_content="
    <testcase assertions=\"1\" name=\"$_jshu_test_name\" time=\"$_jshu_time\">
    $_jshu_fail
    <system-out>
<![CDATA[
$_jshu_out
]]>
    </system-out>
    </testcase>
  "
  echo "$_jshu_content" >>$jshu_content_file
  unset _jshu_test_name _jshu_time _jshu_fail _jshu_outf _jshu_out _jshu_content
}

# Extract list of functions to run tests against.
#
# Args:
#   script: string: name of script to extract functions from
# Returns:
#   string: of function names
extractTestFunctions()
{
	_jshu_script_=$1
	
	# extract the lines with test function names, strip of anything besides the
	# function name, and output everything on a single line.
	_jshu_regex_='^[ 	]*(function )*[A-Za-z0-9_]*Test *\(\)'
	egrep "${_jshu_regex_}" "${_jshu_script_}" \
	  |sed 's/^[^A-Za-z0-9_]*//;s/^function //;s/\([A-Za-z0-9_]*\).*/\1/g' \
	  |xargs
	
	unset _jshu_regex_ _jshu_script_
}


########################################################################################
#
#	jshu_run_test func_name
#
# This function runs the shell function (internal) passed to it by name.
# It prints out a PASSED or FAILED based on the result code
# returned from the function.
#
# Stdout and stderr are redirected into a temp file.
#
########################################################################################
jshu_run_test()
{
	#default values
	jshu_failure_msg=""
	errmsg="Test failure"	# can be overridden in the tested function
	#function args
    _jshu_func_name=$1
    
	_jshu_script_output=/var/tmp/jshu$$.txt
	>$_jshu_script_output
	jshuGetTestName $_jshu_func_name
	test_name=$jshu_test
	
	# run the shell function
	ini=`$date_cmd +%s.%N`
	${_jshu_func_name} &>"${_jshu_script_output}"
    result=$?
    end=`$date_cmd +%s.%N`
    
    # look for regex in output for failure
    if [ $result -eq 0 ] ; then
		if [ -n "$ereg" ]; then
			H=`cat "${_jshu_script_output}" | sed -e 's/^\([^+]\)/| \1/g' | egrep $icase "$ereg"`
			[ -n "$H" ] && result=1
		fi
	fi
	
    # figure out the result and record it
    jshuRecordResult $result ${test_name} "${errmsg}"
    let tests=$tests+1
	time=`echo "$end - $ini" | bc -l`
	total=`echo "$total + $time" | bc -l`
	writeTestContent ${test_name} ${time} "${jshu_failure_msg}" "${_jshu_script_output}"
	rm -f "${_jshu_script_output}"
	
	unset _jshu_func_name _jshu_script_output
	return $result
}

########################################################################################
#
#	verify_directory dirname dirdesc funcname
#
# This function looks for the specified directory path and creates it
# if it is missing. It prints an ERROR message if the dirname argument is empty.
#
#   dirname = the name of the directory that you want to exist
#   dirdesc = what to call the directory when printing the error message
#   funcname = the name of the function/script that called this function (to print in
#              error message)
#
########################################################################################
verify_directory ()
{
	dirname=$1
	dirdesc=$2
	funcname=$3
	if [ "$dirname" == "" ] ; then
		echo "ERROR: Missing $dirdesc on call to $funcname"
		return 1
	fi
	[ -d $dirname ] || mkdir -p $dirname
}	
