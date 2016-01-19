#!/bin/bash 

# source the unit test for scripts functions
. ../jshutest.inc

jshuSetup() {
    return 0
}
# see to it that the teardown function fails
jshuTeardown() {
    return 1
}

##############################################################
# unit test functions
AlwaysPassTest() {
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
