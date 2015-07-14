# jshu
Simplified unit test framework for shell script which produces junit-style xml results file (for Jenkins/Hudson).


The jshu archives contain the jshutest.inc shell script "include" file itself
and a sample directory containing a shell script (increment_build.sh) to increment
build numbers in a text file and two different test scripts for it:

* incrbuild_unitTest.sh - This test script is written more along the lines
    of classic developer unit-style tests where tests run individual functions
    inside of increment_build.sh to determine if they are correct.

* incrbuild_funcTest.sh - This test script is written more like a functional
    or integration test - where the internal components are presumed to already
    have been tested elsewhere and we now wish to test the operation of the script
    as a whole. Thus, this script provides increment_build.sh with data, runs it,
    and evaluates the resultant data after execution is done.
