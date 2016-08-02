#!/bin/bash 

# source the unit test for scripts functions
. ../jshutest.inc

# see to it that the setup function fails
jshuSetup() {
    return 0
}
##############################################################
# unit test functions
AbortingTest() {
    return ${jshuFAILABORT}
}

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
