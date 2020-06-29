#!/bin/bash

# Variables
jshu=$(dirname $(readlink -f "${BASH_SOURCE:-${0}}"))
result=1

# Wrapper modes
case "${1}" in

# Load mode
--load)
  # Configure bash
  set +e
  set +x

  # Import jshu
  source "${jshu}/jshutest.inc"

  # Configure tests package
  jshu_pkgname="${2}"

  # Start tests
  if [ ! -z "${WORKSPACE}" ] && [ -z "${BUILDDIR}" ]; then
    BUILDDIR=${WORKSPACE} jshuInit "${3}" "${4}"
  else
    jshuInit "${3}" "${4}"
  fi

  # Declare Test function
  function Test() {
    # Variables
    local result

    # Create test runner
    source <(
      cat <<END_OF_TEST
function TestRunner()
{
  # Test title
  test_title="${1}";

  # Test executions
  $(cat)
}
END_OF_TEST
    )

    # Run test
    jshu_run_test TestRunner
    result=${?}

    # Cleanup
    unset TestRunner

    # Result
    return "${result}"
  }

  # Result
  result=0
  ;;

# Test mode
--test)
  # Prepare test name for file name
  jshu_test_tag=${4//./_}
  jshu_test_tag=${jshu_test_tag// /_}

  # Load jshu wrapper
  source "${jshu}/wrapper.sh" --load "${2}" "${3}" "${jshu_test_tag}"

  # Run single test
  Test "${4}" <<§
  $(cat)
§
  result=${?}

  # Finish jshu wrapper
  source "${jshu}/wrapper.sh" --finish
  ;;

# Finish mode
--finish)
  # Finalize tests
  jshuFinalize
  result=${?}
  ;;

esac

# Result
(exit "${result}")
