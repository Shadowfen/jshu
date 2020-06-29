#!/bin/bash
set -e

# Load jshu wrapper
source ../wrapper.sh --load \
  'Name of the main stage' \
  'Name of the tests suite of this sequence'

# Test 1
Test "Test 1 - Echo" <<§

  # Test
  echo 'Test'
§

# Test 2
Test "Test 2 - Falsy" <<§

  # Test
  echo 'OK'
  [ ${?} -eq 0 ]
  echo 'NOK'
  [ ${?} -eq 1 ]
  echo 'NOT VISIBLE'
§

# Test 3
Test "Test 3 - Heredoc" <<§

  # Test
  bash <<EOF
echo 'Nested bash'
ls -la /root/
EOF
§

# Test 4
Test "Test 4 - Exit" <<§

  # Test
  exit 1
§

# Test 5
Test "Test 5 - Return" <<§

  # Test
  return 1
§

# Test 6
Test "Test 6 - Environment" <<§

  # Test
  echo "${jshu_ts_startTime}"
§

# Test 7
Test "Test 7 - Variables expanded" <<§

  # Test
  path='/root/'
  ls -la "${path}"
§

# Test 8
Test "Test 8 - Variables evaluated" <<\§

  # Test
  path='/root/'
  ls -la "${path}"
§

# Finish jshu wrapper
source ../wrapper.sh --finish