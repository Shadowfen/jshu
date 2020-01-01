#!/bin/bash
set -e

# First standalone test
source ../wrapper.sh --test \
  'Name of the main stage' \
  'Name of the tests suite of this sequence' \
  'Name of the first single test' <<\§

  # Test
  echo 'Test'

  # Test
  ls -la /root/
§

# Second standalone test
source ../wrapper.sh --test \
  'Name of the main stage' \
  'Name of the tests suite of this sequence' \
  'Name of the second single test' <<\§

  # Test
  echo 'Test'

  # Test
  ls -la /missing/
§

# Third standalone test
source ../wrapper.sh --test \
  'Name of the main stage' \
  'Name of the tests suite of this sequence' \
  'Name of the third single test' <<\§

  # Test
  echo 'Test'
§