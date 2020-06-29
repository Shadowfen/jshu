#!/bin/bash

# source the unit test for scripts functions
. ../jshutest.inc

jshuSetup() {
  # set this variable to a space-delimited list of function names
  # that you want to run as tests.
  jshuTestFunctions="AlwaysPass AlwaysPass2"
}

jshuTeardown() {
  return 0
}

##############################################################
# unit test functions
AlwaysPass() {
  return ${jshuPASS}
}

AlwaysPass2() {
  return ${jshuPASS}
}

NormalFuncNameTest() {
  return ${jshuPASS}
}

##############################################################
# main
##############################################################
# initialize testsuite
jshuInit

# run unit tests on script
jshuRunTests

# result summary
jshuFinalize

echo Done.
echo
let tot=failed+errors
exit $tot
