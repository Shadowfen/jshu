#!/bin/bash

# source the unit test for scripts functions
. ../jshutest.inc

# source our script that we want to test features of
. ./increment_build.sh

jshuSetup() {
  # set this variable to a space-delimited list of non-standard test function
  # names that you want to run as tests. They will run before the *Test
  # functions. It will be used by the jshuRunTests function, so if you
  # want to use it, you should assign the value to it before calling
  # jshuRunTests in the bottom (boilerplate) section.
  jshuTestFunctions="parseVersionFunc getBldFileName_noParam"
}

##############################################################
# unit test functions
parseVersionFunc() {
  VERSION="1.5.7"
  parseVersion
  if [ "$VERSION_SUPER" != "1" ]; then
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MAJOR" != "5" ]; then
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MINOR" != "7" ]; then
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

parseVersionShortTest() {
  VERSION="1.5"
  parseVersion
  echo $VERSION_SUPER.$VERSION_MAJOR.$VERSION_MINOR
  if [ "$VERSION_SUPER" != "1" ]; then
    jshu_errmsg="\$VERSION_SUPER ($VERSION_SUPER) != 1"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MAJOR" != "5" ]; then
    jshu_errmsg="\$VERSION_MAJOR ($VERSION_MAJOR) != 5"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MINOR" != "" ]; then
    jshu_errmsg="\$VERSION_MINOR ($VERSION_MINOR) != \"\""
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

incrSuperTest() {
  VERSION="1.5.7"
  BUILDNUM=3
  EPOCH=7003
  parseVersion
  incrSuper
  if [ ${VERSION_SUPER} -ne 2 ]; then
    jshu_errmsg="\$VERSION_SUPER ($VERSION_SUPER) != 1"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MAJOR" != "0" ]; then
    jshu_errmsg="\$VERSION_MAJOR ($VERSION_MAJOR) != 0"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MINOR" != "0" ]; then
    jshu_errmsg="\$VERSION_MINOR ($VERSION_MINOR) != 0"
    return ${jshuFAIL}
  fi
  if [ "$BUILDNUM" != "1" ]; then
    jshu_errmsg="\$BUILDNUM ($BUILDNUM) != 1"
    return ${jshuFAIL}
  fi
  if [ "$EPOCH" != "7004" ]; then
    jshu_errmsg="\$EPOCH ($EPOCH) != 7004"
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

incrMajorTest() {
  VERSION="1.5.7"
  BUILDNUM=3
  EPOCH=7003
  parseVersion
  incrMajor
  if [ ${VERSION_SUPER} -ne 1 ]; then
    jshu_errmsg="\$VERSION_SUPER ($VERSION_SUPER) != 1"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MAJOR" != "6" ]; then
    jshu_errmsg="\$VERSION_MAJOR ($VERSION_MAJOR) != 6"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MINOR" != "0" ]; then
    jshu_errmsg="\$VERSION_MINOR ($VERSION_MINOR) != 0"
    return ${jshuFAIL}
  fi
  if [ "$BUILDNUM" != "1" ]; then
    jshu_errmsg="\$BUILDNUM ($BUILDNUM) != 1"
    return ${jshuFAIL}
  fi
  if [ "$EPOCH" != "7004" ]; then
    jshu_errmsg="\$EPOCH ($EPOCH) != 7004"
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

incrMinorTest() {
  VERSION="1.5.7"
  BUILDNUM=3
  EPOCH=7003
  parseVersion
  incrMinor
  if [ ${VERSION_SUPER} -ne 1 ]; then
    jshu_errmsg="\$VERSION_SUPER ($VERSION_SUPER) != 1"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MAJOR" != "5" ]; then
    jshu_errmsg="\$VERSION_MAJOR ($VERSION_MAJOR) != 5"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MINOR" != "8" ]; then
    jshu_errmsg="\$VERSION_MINOR ($VERSION_MINOR) != 8"
    return ${jshuFAIL}
  fi
  if [ "$BUILDNUM" != "1" ]; then
    jshu_errmsg="\$BUILDNUM ($BUILDNUM) != 1"
    return ${jshuFAIL}
  fi
  if [ "$EPOCH" != "7004" ]; then
    jshu_errmsg="\$EPOCH ($EPOCH) != 7004"
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

incrBuildTest() {
  VERSION="1.5.7"
  BUILDNUM=3
  EPOCH=7003
  parseVersion
  incrBuild
  if [ ${VERSION_SUPER} -ne 1 ]; then
    jshu_errmsg="\$VERSION_SUPER ($VERSION_SUPER) != 1"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MAJOR" != "5" ]; then
    jshu_errmsg="\$VERSION_MAJOR ($VERSION_MAJOR) != 5"
    return ${jshuFAIL}
  fi
  if [ "$VERSION_MINOR" != "7" ]; then
    jshu_errmsg="\$VERSION_MINOR ($VERSION_MINOR) != 7"
    return ${jshuFAIL}
  fi
  if [ "$BUILDNUM" != "4" ]; then
    jshu_errmsg="\$BUILDNUM ($BUILDNUM) != 4"
    return ${jshuFAIL}
  fi
  if [ "$EPOCH" != "7004" ]; then
    jshu_errmsg="\$EPOCH ($EPOCH) != 7004"
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

getBldFileName_noParam() {
  bfname=$(getBldNumFilename)
  if [ "$bfname" != "buildnumber.txt" ]; then
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

getBldFileName_paramTest() {
  bfname=$(getBldNumFilename mybldnum.txt)
  if [ "$bfname" != "mybldnum.txt" ]; then
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

createBldNumFile_newTest() {
  tmpfiles_used="./crbldfile_test.txt"
  /bin/rm -f $tmpfiles_used

  createBldNumFile ./crbldfile_test.txt
  if [ ! -e ./crbldfile_test.txt ]; then
    jshu_errmsg="[ ! -e ./crbldfile_test.txt ] failed"
    return ${jshuFAIL}
  fi
  /bin/rm -f $tmpfiles_used
  return ${jshuPASS}
}

loadBldNumFile_emptyTest() {
  tmpfiles_used="./ldbldfile_test.txt"
  /bin/rm -f $tmpfiles_used

  createBldNumFile ./ldbldfile_test.txt
  parseVersion
  [ ! -e ./ldbldfile_test.txt ] && return ${jshuFAIL}
  loadBldNumFile ./ldbldfile_test.txt
  /bin/rm -f $tmpfiles_used

  if [ "${PRODNAME}" = "Unknown product" ]; then
    jshu_errmsg="\$PRODNAME ($PRODNAME) != Unknown product"
    return ${jshuFAIL}
  fi
  if [ "$PKGNAME" != "unknown_pkg" ]; then
    jshu_errmsg="\$PKGNAME ($PKGNAME) != unknown_pkg"
    return ${jshuFAIL}
  fi
  if [ "$VERSION" != "0.0.0" ]; then
    jshu_errmsg="\$VERSION ($VERSION) != 0.0.0"
    return ${jshuFAIL}
  fi
  if [ "$BUILDNUM" != "0" ]; then
    jshu_errmsg="\$BUILDNUM ($BUILDNUM) != 0"
    return ${jshuFAIL}
  fi
  if [ "$EPOCH" != "0" ]; then
    jshu_errmsg="\$EPOCH ($EPOCH) != 0"
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

loadBldNumFileTest() {
  tmpfiles_used="./ldbldfile1_test.txt"
  /bin/rm -f $tmpfiles_used

  PRODNAME="My sample product"
  PKGNAME="sample_prod"
  VERSION="1.2.3"
  parseVersion
  BUILDNUM="145"
  EPOCH="7009125"
  BUILDHOST="ananke"
  BUILDDATE="Sun Jan 26 15:57:13 CST 2014"
  writeBldNumFile ./ldbldfile1_test.txt
  unset PRODNAME PKGNAME VERSION BUILDNUM EPOCH

  loadBldNumFile ldbldfile1_test.txt
  /bin/rm -f $tmpfiles_used
  if [ "${PRODNAME}" != "My sample product" ]; then
    jshu_errmsg="\$PRODNAME ($PRODNAME) != My sample product"
    return ${jshuFAIL}
  fi
  if [ "${PKGNAME}" != "sample_prod" ]; then
    jshu_errmsg="\$PKGNAME (${PKGNAME}) != sample_prod"
    return ${jshuFAIL}
  fi
  if [ "${VERSION}" != "1.2.3" ]; then
    jshu_errmsg="\$VERSION ($VERSION) != 1.2.3"
    return ${jshuFAIL}
  fi
  if [ "${BUILDNUM}" != "145" ]; then
    jshu_errmsg="\$BUILDNUM ($BUILDNUM) != 145"
    return ${jshuFAIL}
  fi
  if [ "${EPOCH}" != "7009125" ]; then
    jshu_errmsg="\$EPOCH ($EPOCH) != 7009125"
    return ${jshuFAIL}
  fi
  return ${jshuPASS}
}

badErrorMsgTest() {
  jshu_errmsg="This is a \" quote mark"
  ${jshuTEST_SKIP}
}

unfinishedTest() {
  jshu_errmsg="This test has not been written yet"
  ${jshuTEST_SKIP}
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
