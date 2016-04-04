//-----------------------------------------------------------------------------
//
//      ZWOptions.cpp
//
//      Cli/C++ wrapper for the C++ OpenZWave Options class
//
//		Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//      SOFTWARE NOTICE AND LICENSE
//
//      This file is part of OpenZWave.
//
//      OpenZWave is free software: you can redistribute it and/or modify
//      it under the terms of the GNU Lesser General Public License as published
//      by the Free Software Foundation, either version 3 of the License,
//      or (at your option) any later version.
//
//      OpenZWave is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU Lesser General Public License for more details.
//
//      You should have received a copy of the GNU Lesser General Public License
//      along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------
#include "ZWOptions.h"

using namespace OpenZWaveDotNet;
using namespace OpenZWave;
using namespace Runtime::InteropServices;

//-----------------------------------------------------------------------------
//	<ZWOptions::Create>
//	Create the unmanaged Options singleton object
//-----------------------------------------------------------------------------
void ZWOptions::Create
(
	String^ _configPath,
	String^	_userPath,
	String^	_commandLine
)
{
	// Create the Manager singleton
	const char* config = (const char*)(Marshal::StringToHGlobalAnsi(_configPath)).ToPointer();
	const char* user = (const char*)(Marshal::StringToHGlobalAnsi(_userPath)).ToPointer();
	const char* command = (const char*)(Marshal::StringToHGlobalAnsi(_commandLine)).ToPointer();
	Options::Create( config, user, command );
}

//-----------------------------------------------------------------------------
// <ZWOptions::AddOptionBool>
// Add a boolean option to the program
//-----------------------------------------------------------------------------
bool ZWOptions::AddOptionBool
( 
	String^ _name,
	bool _default
)
{ 
	const char* name = (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer();
	return Options::Get()->AddOptionBool( name, _default ); 
}

//-----------------------------------------------------------------------------
// <ZWOptions::AddOptionInt>
// Add an integer option to the program
//-----------------------------------------------------------------------------
bool ZWOptions::AddOptionInt
( 
	String^ _name,
	int32 _default
)
{ 
	const char* name = (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer();
	return Options::Get()->AddOptionInt( name, _default ); 
}

//-----------------------------------------------------------------------------
// <ZWOptions::AddOptionString>
// Add a string option to the program
//-----------------------------------------------------------------------------
bool ZWOptions::AddOptionString
( 
	String^ _name,
	String^ _default,
	bool _append
)
{ 
	const char* name = (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer();
	const char* defaultStr = (const char*)(Marshal::StringToHGlobalAnsi(_default)).ToPointer();
	return Options::Get()->AddOptionString( name, defaultStr, _append ); 
}

//-----------------------------------------------------------------------------
// <ZWOptions::GetOptionAsBool>
// Gets the value of a boolean option
//-----------------------------------------------------------------------------
bool ZWOptions::GetOptionAsBool
( 
	String^ _name,
	[Out] System::Boolean %o_value
)
{ 
	bool value;
	if( Options::Get()->GetOptionAsBool( (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer(), &value ) )
	{
		o_value = value;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// <ZWOptions::GetOptionAsInt>
// Gets the value of an integer option
//-----------------------------------------------------------------------------
bool ZWOptions::GetOptionAsInt
( 
	String^ _name,
	[Out] System::Int32 %o_value
)
{ 
	int32 value;
	if( Options::Get()->GetOptionAsInt( (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer(), &value ) )
	{
		o_value = value;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// <ZWOptions::GetOptionAsString>
// Gets the value of a string option
//-----------------------------------------------------------------------------
bool ZWOptions::GetOptionAsString
( 
	String^ _name,
	[Out] String^ %o_value 
)
{ 
	string value;
	if( Options::Get()->GetOptionAsString( (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer(), &value ) )
	{
		o_value = gcnew String(value.c_str());
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// <ZWOptions::GetOptionType>
// Gets the type of the value stored by the option
//-----------------------------------------------------------------------------
ZWOptionType ZWOptions::GetOptionType
(
	String^ _name
)
{
	const char* name = (const char*)(Marshal::StringToHGlobalAnsi(_name)).ToPointer();
	return (ZWOptionType)Enum::ToObject( ZWOptionType::typeid, Options::Get()->GetOptionType( name ) );
}





