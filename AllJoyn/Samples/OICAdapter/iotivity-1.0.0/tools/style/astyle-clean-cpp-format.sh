#!/bin/bash
# copy or move Artistic Style backup files to a backup directory

# CHANGE THE FOLLOWING 4 VARIABLES
# $indir is the input top level directory containing the .orig files
# $outdir is the output top level directory containing the moved files
# $fileext is the backup file extension
# $copyfiles is COPY (cp) or MOVE (mv) the files
# spaces ARE allowed in directory and file name (for Cygwin on Windows)

indir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
outdir="astyle_backup"
fileext=".orig"
fileext=".orig"
# USE ONE OF THE FOLLOWING TO COPY OR MOVE THE FILES
#copyfiles=cp
copyfiles=mv

# display the variables
echo
echo "\"$copyfiles\" Artistic Style backup files"
echo "FROM $indir"
echo "TO   $outdir"
echo

# validate input directory
if [ ! -d "$indir" ] ; then
	echo "Input directory does not exist!"
	echo
	exit
fi
if [ "$indir" == "$outdir" ]; then
	echo "Input and output directories are the same!"
	echo
	exit
fi

# optional statement to run Artistic Style
# astyle  -R  "%indir%\*.cpp"  "%indir%\*.h"
# if [ $? -ne 0 ] ; then  read -sn1 -p "Error executing astyle!"; fi

# variables for fle processing
#echo "processing files..."
fileHasSpaces=false         # a file or directory name has spaces
extLength=${#fileext}       # length of the file extension

# loop thru the input directory to find the .orig files
# then move the .orig file to the new directory
for file in `find  "$indir"  -type f  -name "*$fileext" ` ; do

	# allow spaces in file names for Cygwiin on Windows
	# test if this is a continuation line and build $infile
	if [ $fileHasSpaces = true ] ; then
		infile+=' '$file
	else
		infile=$file
	fi

	# test end of string for no file extension
	if [ ! "${infile:(-$extLength)}" = $fileext ] ; then
		fileHasSpaces=true
		continue
	fi

	fileHasSpaces=false
#	echo $infile

	# replace input file directory with output directory
	# ${string/substring/replacement}
	outfile=${infile/$indir/$outdir}
#	echo $outfile

	# strip filename from back of the output file
	# ${string%substring} - /* strips from last '/' to end
	outpath=${outfile%/*}
#	echo $outpath

	# create output directory
	[ ! -d  "$outpath" ] && mkdir  -p  "$outpath"

	#  copy or move the files
	$copyfiles  -f  "$infile"  "$outpath"
	if [ $? -ne 0 ] ; then
		read -sn1 -p "press any key to continue!"
		echo
	fi

	# echo for every 100 files processed
	let count++
	let mod=count%100
	[ $mod -eq 0 ] && echo $count files processed
done

echo $count files processed
echo
