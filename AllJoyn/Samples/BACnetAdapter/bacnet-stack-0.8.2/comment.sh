#!/bin/bash
# This script converts any C++ comments to C comments
# using the ccmtcnvt tool from the liwc package

CONVERTER=/usr/bin/ccmtcnvt
# silent fail if the tool is not installed
[ -x ${CONVERTER} ] || exit 0

directory=${1-`pwd`}
for filename in $( find ${directory} -name '*.c' )
do
  echo Converting ${filename}
  TEMPFILE="/tmp/ccmtcnvt.$RANDOM.txt"
  ${CONVERTER} ${filename} > ${TEMPFILE}
  mv ${TEMPFILE} ${filename}
done

for filename in $( find ${directory} -name '*.h' )
do
  echo Converting ${filename}
  TEMPFILE="/tmp/ccmtcnvt.$RANDOM.txt"
  ${CONVERTER} ${filename} > ${TEMPFILE}
  mv ${TEMPFILE} ${filename}
done

