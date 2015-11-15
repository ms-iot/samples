#!/bin/sh
# splint is a static code checker

SPLINT=/usr/bin/splint

[ -x ${SPLINT} ] || exit 0

DEFINES="-D__signed__=signed -D__gnuc_va_list=va_list"
INCLUDES="-Iinclude -Idemo/object -Iports/linux"
SETTINGS="-castfcnptr -fullinitblock -initallelements -weak -warnposixheaders"

if [ ! -e .splintrc ]
then
  echo ${DEFINES} ${INCLUDES} ${SETTINGS} > .splintrc
fi

directory=${1-`pwd`}/src
rm -f splint_output.txt
touch splint_output.txt
for filename in $( find $directory -name '*.c' )
do
  echo splinting ${filename}
  ${SPLINT} ${filename} >> splint_output.txt 2>&1
done

