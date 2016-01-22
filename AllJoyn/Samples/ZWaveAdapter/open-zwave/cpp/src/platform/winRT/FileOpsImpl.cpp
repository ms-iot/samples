//-----------------------------------------------------------------------------
//
//	FileOpsImpl.cpp
//
//	WinRT implementation of file operations
//
//	Copyright (c) 2015 Microsoft Corporation
//	All rights reserved.
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include "FileOpsImpl.h"

using namespace OpenZWave;

//-----------------------------------------------------------------------------
//	<FileOpsImpl::FileOpsImpl>
//	Constructor
//-----------------------------------------------------------------------------
FileOpsImpl::FileOpsImpl
(
)
{
}

//-----------------------------------------------------------------------------
//	<FileOpsImpl::~FileOpsImpl>
//	Destructor
//-----------------------------------------------------------------------------
FileOpsImpl::~FileOpsImpl
(
)
{
}

//-----------------------------------------------------------------------------
//	<FileOpsImpl::FolderExists>
//	Determine if a folder exists and is accessible by the calling App
//-----------------------------------------------------------------------------
bool FileOpsImpl::FolderExists(
	const string &_folderName
)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	wstring wFolderName(_folderName.begin(), _folderName.end());

	if (0 == GetFileAttributesEx(wFolderName.c_str(), GetFileExInfoStandard, &fad))
		return false;

	return (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? true: false;
}
