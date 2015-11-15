#!/bin/sh
# Release helper for this project

USERNAME=skarg
PROJECT=bacnet
SVN_BASE_URL=https://svn.code.sf.net/p/${PROJECT}/code
SVN_MODULE=bacnet-stack
FRS_URL=${USERNAME},${PROJECT}@frs.sourceforge.net:/home/frs/project/b/ba/bacnet/bacnet-stack

if [ -z "$1" ]
then
  echo "Usage: `basename $0` 0.0.0"
  echo "Creates the ChangeLog."
  echo "Creates the release files."
  echo "Tags the current version in subversion."
  exit 1
fi

DOTTED_VERSION="$1"
echo "Creating the release files for version ${DOTTED_VERSION}"

CHANGELOG=ChangeLog-${DOTTED_VERSION}
echo "Creating the ${PROJECT} change log ${CHANGELOG}"
if [ -e "${CHANGELOG}" ]
then
rm ${CHANGELOG}
fi
svn update
svn log --xml --verbose | xsltproc svn2cl.xsl - > ${CHANGELOG}
if [ -e "${CHANGELOG}" ]
then
  echo "${CHANGELOG} created."
else
  echo "Failed to create ${CHANGELOG}"
  exit 1
fi

ARCHIVE_NAME=${SVN_MODULE}-${DOTTED_VERSION}

SVN_TRUNK_NAME=${SVN_BASE_URL}/trunk/${SVN_MODULE}
SVN_TAGGED_NAME=${SVN_BASE_URL}/tags/${ARCHIVE_NAME}
echo "Setting a tag on the ${SVN_MODULE} module called ${ARCHIVE_NAME}"
TAG_COMMENT="Created version ${ARCHIVE_NAME}"
svn copy --username=${USERNAME} ${SVN_TRUNK_NAME} ${SVN_TAGGED_NAME} -m "${TAG_COMMENT}"
echo "done."

if [ -d "${ARCHIVE_NAME}" ]
then
  echo "removing old ${ARCHIVE_NAME}..."
  rm -rf ${ARCHIVE_NAME}
  echo "done."
fi

echo "Getting a clean version out of subversion for Linux gzip"
svn export --username=${USERNAME} ${SVN_TAGGED_NAME} ${ARCHIVE_NAME} > /dev/null
echo "done."

GZIP_FILENAME=${ARCHIVE_NAME}.tgz
echo "tar and gzip the clean directory"
if [ -e "${GZIP_FILENAME}" ]
then
  echo "removing old ${GZIP_FILENAME}..."
  rm ${GZIP_FILENAME}
  echo "done."
fi
tar -cvvzf ${GZIP_FILENAME} ${ARCHIVE_NAME}/ > /dev/null
echo "done."
if [ -e "${GZIP_FILENAME}" ]
then
  echo "${GZIP_FILENAME} created."
else
  echo "Failed to create ${GZIP_FILENAME}"
  exit 1
fi

if [ -d "${ARCHIVE_NAME}" ]
then
  echo "removing old ${ARCHIVE_NAME}..."
  rm -rf ${ARCHIVE_NAME}
  echo "done."
fi
echo "Getting another clean version out of subversion for Windows zip"
svn export --username=${USERNAME} --native-eol CRLF ${SVN_TAGGED_NAME} ${ARCHIVE_NAME} > /dev/null
ZIP_FILENAME=${ARCHIVE_NAME}.zip
echo "done."
echo "Zipping the directory exported for Windows."
zip -r ${ZIP_FILENAME} ${ARCHIVE_NAME} > /dev/null

if [ -e "${ZIP_FILENAME}" ]
then
  echo "${ZIP_FILENAME} created."
else
  echo "Failed to create ${ZIP_FILENAME}"
  exit 1
fi

# remove SVN files
if [ -d "${ARCHIVE_NAME}" ]
then
  echo "removing ${ARCHIVE_NAME}..."
  rm -rf ${ARCHIVE_NAME}
  echo "done."
fi

echo "Creating ${ARCHIVE_NAME}"
mkdir ${ARCHIVE_NAME}
mv ${ZIP_FILENAME} ${ARCHIVE_NAME}
mv ${GZIP_FILENAME} ${ARCHIVE_NAME}
mv ${CHANGELOG} ${ARCHIVE_NAME}
cp readme.txt ${ARCHIVE_NAME}

echo "Sending ${ARCHIVE_NAME} to SourceForge using scp..."
scp -r ${ARCHIVE_NAME} ${FRS_URL}

echo "Complete!"
