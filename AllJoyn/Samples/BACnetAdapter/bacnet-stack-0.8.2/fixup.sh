#!/bin/sh
# fix DOS/Unix names and Subversion EOL-Style, and remove backup files

#DOS2UNIX=/usr/bin/dos2unix
DOS2UNIX=/usr/bin/fromdos

# exit silently if utility is not installed
[ -x ${DOS2UNIX} ] || exit 0
[ -x /usr/bin/svn ] || exit 0

directory=${1-`pwd`}
for filename in $( find ${directory} -name '*.c' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/plain ${filename}
done

for filename in $( find ${directory} -name '*.h' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/plain ${filename}
done

for filename in $( find ${directory} -name '*.bat' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/plain ${filename}
done

for filename in $( find ${directory} -name '*.pl' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/x-script.perl ${filename}
done

for filename in $( find ${directory} -name '*.eww' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/xml ${filename}
done

for filename in $( find ${directory} -name '*.ewp' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/xml ${filename}
done

for filename in $( find ${directory} -name '*.cbp' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/xml ${filename}
done

for filename in $( find ${directory} -name '*.icf' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/plain ${filename}
done

for filename in $( find ${directory} -name '*.htm' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/html ${filename}
done

for filename in $( find ${directory} -name '*.txt' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/plain ${filename}
done

for filename in $( find ${directory} -name '*.lua' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/plain ${filename}
done

for filename in $( find ${directory} -name '*.sh' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/x-script.sh ${filename}
done

for filename in $( find ${directory} -name '*.b32' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/x-makefile ${filename}
done

for filename in $( find ${directory} -name '*.mak' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/x-makefile ${filename}
done

for filename in $( find ${directory} -name 'Makefile' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propset svn:eol-style native ${filename}
  /usr/bin/svn propset svn:mime-type text/x-makefile ${filename}
done

for filename in $( find ${directory} -name '*.xls' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propdel svn:eol-style ${filename}
  /usr/bin/svn propset svn:mime-type application/excel ${filename}
done

for filename in $( find ${directory} -name '*.ods' )
do
  echo Fixing DOS/Unix ${filename}
  ${DOS2UNIX} ${filename}
  echo Setting Subversion EOL Style for ${filename}
  /usr/bin/svn propdel svn:eol-style ${filename}
  /usr/bin/svn propset svn:mime-type application/vnd.oasis.opendocument.spreadsheet ${filename}
done

for filename in $( find ${directory} -name '*~' )
do
  echo Removing backup ${filename}
  rm ${filename}
done

