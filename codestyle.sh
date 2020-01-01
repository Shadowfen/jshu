#!/bin/sh

# Apply shfmt codestyle
shfmt -i 2 -bn -ci -w \
  ./codestyle.sh \
  ./jshutest.inc \
  ./sample/*.sh \
  ./tests/*.sh
