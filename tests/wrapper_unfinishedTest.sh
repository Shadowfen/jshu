#!/bin/bash
set -e

# Load jshu wrapper
source ../wrapper.sh --load \
  'Name of the main stage' \
  'Unfinished test suite handled as failure'

# Test 1
Test "Test 1 - Echo" <<§

  # Test
  echo 'Test'
§
