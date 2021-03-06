#!/bin/sh
#========================================================================
# Simplified unit test framework for shell script which
# produces junit-style xml results file (for Jenkins/Hudson).
#
# version 1.0.0-20
#
# Include this file into your test shellscript using ". /usr/local/include/jshutest.inc"
# (or whatever path is appropriate to find this file).
#
# Create shell functions whose name ends with "Test" which will return
# ${jshuPASS} - (0) if the test passes
# ${jshuFAIL} - (1) if the test fails
# ${jshuERROR} - (2) if an error occured other than a test failure
# ${jshuFAILABORT} - (3) if test fails and test suite should abort (do not
#                          run following tests)
# ${jshuERRORABORT} - (4) if an error occurs and test suite should abort (do not
#                          run following tests)
# As of 1.0.0-15 it is now possible to specify a list of function names that
# do not end with "Test" which you wish to run as test functions.
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
#    jshu_errmsg="Script ./idontexist.sh was existed!!"
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
# * This script relies on the proper recognition of the %N format
#   character for the date(1) command. The version provided by
#   CentOS 5.x is broken in that respect.
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
jshuFAILABORT=3
jshuERRORABORT=4
jshuSKIP=12

jshuTEST_SKIP="return ${jshuSKIP}"

tests=0
errors=0
failed=0
total=0
skipped=0
content=""
date_cmd=$(which date)

CURPROC=$$
# If we are in a bash shell, we can use this to get subshell pids.
# Otherwise, we can only get the parent shell pid (making jshu
# not capable of nesting test scripts).
if [ -z ${BASHPID+x} ] || [ -z $BASHPID ]; then
  CURPROC=$BASHPID
fi

jshu_pkgname=""
jshu_suite=""
jshu_suitename=""
jshu_content_file=/var/tmp/jshu_content_${CURPROC}.txt
jshu_resultDir=""
ereg=""

jshuTestFunctions=""

# Determine if date command can recognize %N format specifier.
_jshuDate() {
  if [ ! -n "${_jshuDateFormat+x}" ]; then
    testdate=$(date +%s.%N)
    nano=${testdate##*.}
    if [ "$nano" == "%N" ]; then
      # date is broken
      _jshuDateFormat="+%s.0"
    else
      _jshuDateFormat="+%s.%N"
    fi
  fi
  date ${_jshuDateFormat}
}

# package name is parent directory name unless overridden
# suite name is the name of the test script (without .sh)
# class name is the name of the unit test function

# Test result file name is TEST_${pkgname}.${suite}.xml

# Prepare the package name for the report from the current directory
jshuGetPkgName() {
  # create a package name based on the directory your test script
  # is in (if the current directory is "test" or "tests", then
  # use the name of the directory above it
  _jshu_curdir=$(pwd)
  _jshu_parentdir=${_jshu_curdir##*/}
  if [ $_jshu_parentdir == "test" -o $_jshu_parentdir == "tests" ]; then
    _jshu_curdir=${_jshu_curdir%/*}
    _jshu_parentdir=${_jshu_curdir##*/}
  fi
  jshu_pkgname=${_jshu_parentdir}
  unset _jshu_curdir _jshu_parentdir
}

# Prepare the suite name for the report from the script name
jshu_suite=""
jshuGetSuiteName() {
  # the "suite" name is simply the name of your script
  # (without the .sh extension) and embedded '.'s
  # converted to '_'s

  # Configure test suite name
  if [ ! "${2}" == "" ]; then
    jshu_suitename=${2}
  else
    jshu_suitename=${1}
  fi

  # strip off directories
  _jshu_script_name=${jshu_suitename##*/}
  # strip off ".sh"
  _jshu_base_name=${_jshu_script_name%.sh}
  # convert embedded '.' to '_'
  jshu_suite=${_jshu_base_name//./_}
  # convert embedded ' ' to '_'
  jshu_suite=${jshu_suite// /_}

  unset _jshu_base_name _jshu_script_name
}

# Prepare the test name for the report from the function name
jshu_test=""
jshuGetTestName() {
  _jshu_test_name=${1}
  # convert embedded '.' to '_'
  jshu_test=${_jshu_test_name//./_}
  # convert embedded ' ' to '_'
  jshu_test=${jshu_test// /_}
  unset _jshu_test_name
}

# Create function stub for one-time setup
jshuSetup() {
  :
}

# Create function stub for one-time teardown
jshuTeardown() {
  :
}

# Initialize the test suite
jshuInit() {
  # Create our results directory
  #look for BUILDDIR variable
  _jshu_blddir=$(pwd)
  if [ -z ${BUILDDIR+x} ]; then
    # BUILDDIR is unset
    jshu_resultDir=${_jshu_blddir}/results/xunit
  else
    # BUILDDIR is set
    _jshu_blddir=$BUILDDIR
    jshu_resultDir=${_jshu_blddir}/results/xunit
  fi
  # create output folder
  mkdir -p "$jshu_resultDir"

  # Store test suite start time
  jshu_ts_startTime=$(_jshuDate)

  # Set values for package name and suite name
  # for reporting
  if [ "${jshu_pkgname}" == "" ]; then
    jshuGetPkgName
  fi
  jshuGetSuiteName "$0" "$1"

  # Append test tag to suite name
  if [ ! -z "${2}" ]; then
    jshu_suite="${jshu_suite}.${2}"
  fi

  # Prepare JSHU result filename
  jshu_filename=${jshu_pkgname//./_}
  jshu_filename=${jshu_filename// /_}
  jshu_filename="Tests.${jshu_filename}.${jshu_suite}.xml"
  jshu_filename=$(echo "${jshu_filename}" | sed 's/\xC3\(\xA0\|\xA2\)/a/g;s/\xC3\(\xA8\|\xA9\|\xAA\|\xAB\)/e/g;s/\xC3\(\xB9\|\xBC\)/u/g')

  # Prepare faulty initial result
  {
    echo '<?xml version="1.0" encoding="UTF-8" ?>'
    echo "<testsuite failures=\"1\" errors=\"0\" tests=\"1\" skipped=\"0\" name=\"${jshu_pkgname}.${jshu_suitename}\">"
    echo "  <testcase name=\"${jshu_pkgname}.${jshu_suitename}\">"
    echo "    <failure type=\"failure\" message=\"Test failure\">"
    echo "    </failure>"
    echo '  </testcase>'
    echo "</testsuite>"
  } >"${jshu_resultDir}/${jshu_filename}"

  # Get rid of previous test results
  rm -f $jshu_content_file
  >$jshu_content_file
  unset _jshu_blddir
}

# Write up the tests result file
jshuFinalize() {
  jshu_filename=${jshu_pkgname//./_}
  jshu_filename=${jshu_filename// /_}
  jshu_filename="Tests.${jshu_filename}.${jshu_suite}.xml"
  jshu_filename=$(echo "${jshu_filename}" | sed 's/\xC3\(\xA0\|\xA2\)/a/g;s/\xC3\(\xA8\|\xA9\|\xAA\|\xAB\)/e/g;s/\xC3\(\xB9\|\xBC\)/u/g')
  tests_hostname=${HOSTNAME}
  tests_timestamp=$(date -u +%FT%TZ -d @"${jshu_ts_startTime%.}")

  ## testsuite block
  {
    echo '<?xml version="1.0" encoding="UTF-8" ?>'
    echo "<testsuite failures=\"${failed}\" errors=\"$errors\" tests=\"$tests\" skipped=\"${skipped}\" name=\"${jshu_pkgname}.${jshu_suitename}\" hostname=\"${tests_hostname}\" time=\"${total}\" timestamp=\"${tests_timestamp}\">"
    cat "${jshu_content_file}"
    echo "</testsuite>"
  } >"${jshu_resultDir}/${jshu_filename}"
  rm -f "${jshu_content_file}"

  echo ''
  let totfail=errors+failed
  echo "-- Duration of tests       = ${total} seconds"
  echo "-- Number of tests         = ${tests}"
  echo "-- Number of skipped tests = ${skipped}"
  echo "-- Number of failing tests = ${totfail}"
  echo ''

  unset jshu_ts_startTime

  return "${totfail}"
}

jshuRunTests() {
  _jshu_run_curdir=$(pwd)
  if ! jshu_run_test jshuSetup; then
    return 1
  fi
  _jshu_tests=$(jshuExtractTestFunctions $0)
  if [ -n "$jshuTestFunctions" ]; then
    _jshu_tests="$jshuTestFunctions $_jshu_tests"
  fi
  for t in $_jshu_tests; do
    if type -t $t &>/dev/null; then
      cd "${_jshu_run_curdir}"
      jshu_run_test $t
      rslt=$?
      if [ $rslt -eq ${jshuFAILABORT} -o $rslt -eq ${jshuERRORABORT} ]; then
        break
      fi
    else
      # named function does not exist - now what?
      jshuRecordResult "${jshuFAIL}" "${t}" "Failed to run ${t} - test function does not exist" ''
      touch /var/tmp/jshut_$$
      writeTestContent "${t}" '0' 'Test function does not exist in script' '' /var/tmp/jshut_$$
      rm -f /var/tmp/jshut_$$
    fi
  done
  if ! jshu_run_test jshuTeardown; then
    return 1
  fi
  unset _jshu_tests _jshu_run_curdir
}

# replace embedded quotes in error message with html entity
jshuSafeErrMsg() {
  _jshu_safe_errmsg="$1"
  _jshu_q="&quot;"
  # convert embedded '"' to '&quot;'
  _jshu_safe_errmsg=${_jshu_safe_errmsg//\"/$_jshu_q}
  echo "${_jshu_safe_errmsg}"
  unset _jshu_safe_errmsg
}

# filter out binary data from stdout/stderr contents
jshuSafeContent() {
  _jshu_sc_outf="$1"
  _jshu_sc_savf="$1.orig"
  /bin/cp $_jshu_sc_outf $_jshu_sc_savf
  tr -cd '\11\12\15\40-\176' <$_jshu_sc_savf >$_jshu_wtc_outf
  unset _jshu_sc_outf _jshu_sc_savf
}

# Record the result of a test for later inclusion in the test report
# (We also do a stdout summary of test name and PASSED/FAILED/SKIPPED)
jshuRecordResult() {
  _jshu_rr_type=$1
  _jshu__rr_test_name=$2
  _jshu_rr_errMsg=$(jshuSafeErrMsg "$3")
  jshu_failure_msg=""
  jshu_failure_end=""
  echo ''
  case ${_jshu_rr_type} in
    ${jshuPASS})
      printf " \033[1;33m%-69s \033[1;37m-> \033[1;32mPASSED\033[0m\n" "${_jshu__rr_test_name}"
      ;;
    ${jshuFAIL})
      if [ -z "$_jshu_rr_errMsg" ]; then
        _jshu_rr_errMsg="Test failure"
      fi
      jshu_failure_msg="<failure type=\"failure\" message=\"${_jshu_rr_errMsg}\">"
      jshu_failure_end='</failure>'
      printf " \033[1;33m%-69s \033[1;37m-> \033[1;31mFAILED\033[0m\n" "${_jshu__rr_test_name}"
      let failed=failed+1
      ;;
    ${jshuERROR})
      if [ -z "$_jshu_rr_errMsg" ]; then
        _jshu_rr_errMsg="Error found"
      fi
      jshu_failure_msg="<failure type=\"error\" message=\"${_jshu_rr_errMsg}\">"
      jshu_failure_end='</failure>'
      printf " \033[1;33m%-69s \033[1;37m-> \033[1;31mFAILED\033[0m\n" "${_jshu__rr_test_name}"
      let errors=errors+1
      ;;
    ${jshuFAILABORT})
      if [ -z "$_jshu_rr_errMsg" ]; then
        _jshu_rr_errMsg="Test failure"
      fi
      jshu_failure_msg="<failure type=\"failure\" message=\"${_jshu_rr_errMsg}\">"
      jshu_failure_end='</failure>'
      printf " \033[1;33m%-69s \033[1;37m-> \033[1;31mFAILED\033[0m\n" "${_jshu__rr_test_name}"
      let failed=failed+1
      ;;
    ${jshuERRORABORT})
      if [ -z "$_jshu_rr_errMsg" ]; then
        _jshu_rr_errMsg="Error found"
      fi
      jshu_failure_msg="<failure type=\"error\" message=\"${_jshu_rr_errMsg}\">"
      jshu_failure_end='</failure>'
      printf " \033[1;33m%-69s \033[1;37m-> \033[1;31mFAILED\033[0m\n" "${_jshu__rr_test_name}"
      let errors=errors+1
      ;;
    ${jshuSKIP})
      if [ ! -z "$_jshu_rr_errMsg" ]; then
        jshu_failure_msg="<skipped message=\"${_jshu_rr_errMsg}\">"
        jshu_failure_end='</skipped>'
      else
        jshu_failure_msg="<skipped/>"
        jshu_failure_end=''
      fi
      printf " \033[1;33m%-69s \033[1;37m-> \033[1;36mSKIPPED\033[0m\n" "${_jshu__rr_test_name}"
      let skipped=skipped+1
      ;;
  esac
  unset _jshu_rr_type _jshu__rr_test_name _jshu_rr_errMsg
}

# create the xml for a testcase result and save it
# to the content file.
writeTestContent() {
  _jshu_wtc_test_name=$1
  _jshu_wtc_time=$2
  _jshu_wtc_failure_msg=$3
  _jshu_wtc_failure_end=$4
  _jshu_wtc_outf=$5
  if [ -e $_jshu_wtc_outf ]; then
    _jshu_wtc_out=$(<$_jshu_wtc_outf)
  else
    _jshu_wtc_out="No such file ($_jshu_wtc_outf) found. Could not read test output."
  fi
  jshuSafeContent $_jshu_wtc_outf

  # testcase tag
  cat >>"${jshu_content_file}" <<EOF
  <testcase name="${_jshu_wtc_test_name}" time="${_jshu_wtc_time}">
EOF

  # failure log
  if [ ! -z "${_jshu_wtc_failure_msg}" ]; then
    cat >>"${jshu_content_file}" <<EOF
    ${_jshu_wtc_failure_msg}
EOF
    if [ ! -z "${_jshu_wtc_failure_msg}" ]; then
      cat >>"${jshu_content_file}" <<EOF
<![CDATA[
${_jshu_wtc_out}
]]>
    ${_jshu_wtc_failure_end}
EOF
    fi
  fi

  # testcase output
  cat >>"${jshu_content_file}" <<EOF
    <system-out>
<![CDATA[
${_jshu_wtc_out}
]]>
    </system-out>
  </testcase>
EOF
  unset _jshu_wtc_test_name _jshu_wtc_time _jshu_wtc_failure_msg _jshu_wtc_failure_end _jshu_wtc_outf _jshu_wtc_out
}

# Extract from script list of functions to run tests against.
# Returns list of function names
jshuExtractTestFunctions() {
  _jshu_etf_script_=$1 # name of script to extract functions from

  # extract the lines with test function names, strip of anything besides the
  # function name, and output everything on a single line.
  _jshu_etf_regex_='^[ 	]*(function )*[A-Za-z0-9_]*Test *\(\)'
  egrep "${_jshu_etf_regex_}" "${_jshu_etf_script_}" \
    | sed 's/^[^A-Za-z0-9_]*//;s/^function //;s/\([A-Za-z0-9_]*\).*/\1/g' \
    | xargs

  unset _jshu_etf_regex_ _jshu_etf_script_
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
jshu_run_test() {
  #default values
  jshu_failure_msg=""
  jshu_failure_end=""
  jshu_errmsg="" # can be overridden in the tested function
  test_title=""  # can be overridden in the tested function

  #function args
  _jshu_rt_func_name=$1

  _jshu_rt_script_output=/var/tmp/jshu${CURPROC}.txt
  rm -f "${_jshu_rt_script_output}"
  >"${_jshu_rt_script_output}"
  jshuGetTestName $_jshu_rt_func_name

  # run the shell function
  _jshu_rt_startTime=$(_jshuDate)
  (
    echo ''
    set -e
    ${_jshu_rt_func_name} 2>&1 | tee "${_jshu_rt_script_output}"
    return ${PIPESTATUS[0]}
  )
  [ $? -eq 0 ]
  _jshu_result=$?

  # trim the shell output
  sed -Ei 's/\x1B\[([0-9]{0,2}(;[0-9]{1,2})?)?[mGK]//g' "${_jshu_rt_script_output}"
  sed -Ei 's/\x1B\[[0-9]{0,4}[ABCD]//g' "${_jshu_rt_script_output}"
  sed -Ei 's/\x1B\[\?1034h//g' "${_jshu_rt_script_output}"
  sed -i 's/.*\x08//g' "${_jshu_rt_script_output}"
  sed -Ei 's/\x1C//g' "${_jshu_rt_script_output}"

  # acquire error and title outputs
  eval $(grep -m 1 'jshu_errmsg=' "${_jshu_rt_script_output}" | sed 's#^+* ##' || echo '')
  eval $(type "${_jshu_rt_func_name}" | grep -m 1 'test_title=' | sed 's#^+* ##' || echo '')
  _jshu_rt_endTime=$(_jshuDate)

  if [ ! "${test_title}" == "" ]; then
    _jshu_rt_test_name=${test_title}
  else
    _jshu_rt_test_name=${jshu_test}
    _jshu_rt_test_name=${_jshu_rt_test_name//_/ }
  fi

  # look for regex in output for failure
  if [ $_jshu_result -eq 0 ]; then
    if [ -n "$ereg" ]; then
      H=$(cat "${_jshu_rt_script_output}" | sed -e 's/^\([^+]\)/| \1/g' | egrep $icase "$ereg")
      [ -n "$H" ] && _jshu_result=1 && jshu_errmsg="Regex Test failure"
    fi
  fi

  # figure out the result and record it
  if [ "${_jshu_rt_test_name}" == "jshuSetup" -o "${_jshu_rt_test_name}" == "jshuTeardown" ]; then
    if [ "$_jshu_result" != "0" ]; then
      jshuRecordResult "${_jshu_result}" "${_jshu_rt_test_name}" "Failed to run ${_jshu_rt_test_name}" "${_jshu_rt_script_output}"
      time=$(echo "scale=3;($_jshu_rt_endTime - $_jshu_rt_startTime)/1" | bc -l)
      total=$(echo "scale=3;($total + $time)/1" | bc -l)
      writeTestContent "${_jshu_rt_test_name}" "${time}" "${jshu_failure_msg}" "${jshu_failure_end}" "${_jshu_rt_script_output}"
    fi
  else
    jshuRecordResult "${_jshu_result}" "${_jshu_rt_test_name}" "${jshu_errmsg}" "${_jshu_rt_script_output}"
    let tests=$tests+1
    time=$(echo "scale=3;($_jshu_rt_endTime - $_jshu_rt_startTime)/1" | bc -l)
    total=$(echo "scale=3;($total + $time)/1" | bc -l)
    writeTestContent "${_jshu_rt_test_name}" "${time}" "${jshu_failure_msg}" "${jshu_failure_end}" "${_jshu_rt_script_output}"
  fi
  rm -f "${_jshu_rt_script_output}"

  unset _jshu_rt_func_name _jshu_rt_script_output _jshu_rt_test_name
  unset _jshu_rt_startTime _jshu_rt_endTime
  unset jshu_errmsg test_title
  return $_jshu_result
}
