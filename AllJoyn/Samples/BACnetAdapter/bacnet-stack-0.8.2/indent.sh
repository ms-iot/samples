#!/bin/sh
# indent uses a local indent.pro file if it exists
# File must be in consistent unix format before indenting

#DOS2UNIX=/usr/bin/dos2unix
DOS2UNIX=/usr/bin/fromdos
INDENT=/usr/bin/indent
REMOVE=/bin/rm

# exit silently if utility is not installed
[ -x ${INDENT} ] || exit 0
[ -x ${DOS2UNIX} ] || exit 0

INDENTRC=".indent.pro"
if [ ! -e ${INDENTRC} ] 
then
  echo No ${INDENTRC} file found. Creating ${INDENTRC} file.
  echo "-kr -nut -nlp -ip4 -cli4 -bfda -nbc -nbbo -c0 -cd0 -cp0 -di0 -l79 -nhnl" > ${INDENTRC}
fi

directory=${1-`pwd`}
for filename in $( find $directory -name '*.c' )
do
  echo Fixing DOS/Unix $filename
  ${DOS2UNIX} $filename
  echo Indenting $filename
  ${INDENT} $filename
done

for filename in $( find $directory -name '*.h' )
do
  echo Fixing DOS/Unix $filename
  ${DOS2UNIX} $filename
  echo Indenting $filename
  ${INDENT} $filename
done

for filename in $( find $directory -name '*~' )
do
  echo Removing backup $filename
  ${REMOVE} $filename
done

