rem Indent the C and H files with specific coding standard
rem requires 'indent.exe' from MSYS (MinGW).
rem See http://www.gnu.org/software/indent/manual/indent.pdf
set OPTIONS=-kr -nut -nlp -ip4 -cli4 -bfda -nbc -nbbo -c0 -cd0 -cp0 -di0 -l79 -nhnl
rem -kr The Kernighan & Ritchie style, corresponds to the following options:
rem -nbad -bap -bbo -nbc -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0
rem -cp33 -cs -d0 -di1 -nfc1 -nfca -hnl -i4 -ip0 -l75 -lp -npcs
rem -nprs -npsl -saf -sai -saw -nsc -nsob -nss
rem -nut Use spaces instead of tabs.
rem -nlp Do not line up parentheses.
rem -ip4 Indent parameter types in old-style function definitions by n spaces.
rem -cli4 Case label indent of n spaces.
rem -bfda Break the line before all arguments in a declaration.
rem -nbc Do not force newlines after commas in declarations.
rem -nbbo Do not prefer to break long lines before boolean operators.
rem -c0 Put comments to the right of code in column n.
rem -cd0 Put comments to the right of the declarations in column n.
rem -cp0 Put comments to the right of #else and #endif statements in column n.
rem -di0 Put variables in column n.
rem -l79 Set maximum line length for non-comment lines to n.
rem -nhnl Do not prefer to break long lines at the position of newlines in the input.

call :treeProcess
goto :eof

:treeProcess
rem perform the indent on all the files of this subdirectory:
for %%f in (*.c) do (
    indent.exe "%%f" -o "%%f" %OPTIONS%
)
for %%f in (*.h) do (
    indent.exe "%%f" -o "%%f" %OPTIONS%
)
rem loop over all directories and sub directories
for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)
exit /b

