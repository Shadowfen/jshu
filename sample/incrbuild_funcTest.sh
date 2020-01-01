#!/bin/bash

# source the unit test for scripts functions
. ../jshutest.inc

# helper function
# remove host and date lines from buildnumber.txt file
stripHostDate() {
  source="$1"
  dest="$2"
  grep -v '^BuildHost:' $source | grep -v '^Date' >$dest
}

# In this one, we don't source the other script because
# we're not going to test pieces of it. We're just going
# to run the script and evaluate what it does.
##############################################################
# unit test functions
emptyBldNumFile_Test() {
  tmpfiles_used="./ldempty_test.txt ./ldempty_test1.txt"
  /bin/rm -f $tmpfiles_used

  ./increment_build.sh build ./ldempty_test.txt

  stripHostDate ./ldempty_test.txt ./ldempty_test1.txt
  if ! diff ./ldempty_test1.txt data/empty.txt; then
    /bin/rm -f $tmpfiles_used
    return ${jshuFAIL}
  fi

  # cleanup files
  /bin/rm -f $tmpfiles_used
  return ${jshuPASS}
}

myprodFile_superTest() {
  tmpfiles_used="./myprod_test.txt ./myprod_test1.txt"
  /bin/rm -f $tmpfiles_used

  cp data/myprod.txt ./myprod_test.txt
  ./increment_build.sh super ./myprod_test.txt

  stripHostDate ./myprod_test.txt ./myprod_test1.txt
  if ! diff ./myprod_test1.txt data/myprod.sup.txt; then
    /bin/rm -f $tmpfiles_used
    return ${jshuFAIL}
  fi

  # cleanup files
  /bin/rm -f $tmpfiles_used
  return ${jshuPASS}
}

myprodFile_minorTest() {
  tmpfiles_used="./myprod_test.txt ./myprod_test1.txt"
  /bin/rm -f $tmpfiles_used

  cp data/myprod.txt ./myprod_test.txt
  ./increment_build.sh minor ./myprod_test.txt

  stripHostDate ./myprod_test.txt ./myprod_test1.txt
  if ! diff ./myprod_test1.txt data/myprod.min.txt; then
    /bin/rm -f $tmpfiles_used
    return ${jshuFAIL}
  fi

  # cleanup files
  /bin/rm -f $tmpfiles_used
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
