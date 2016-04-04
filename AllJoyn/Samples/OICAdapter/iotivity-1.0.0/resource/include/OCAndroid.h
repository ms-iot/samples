//******************************************************************
//
// Copyright 2014 MediaTek All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef _OCANDROID_H_
#define _OCANDROID_H_

#ifdef __ANDROID__
#include <string>

// FIXME : ugly hack
// Android missing functions
namespace std {
	int stoi(const std::string& s);
	double stod(const std::string& s);
	long long stoll(const std::string& s);
	unsigned long long stoull(const std::string& s);
	long double stold(const string& s);

	std::string to_string(int val);
	std::string to_string(long val);
	std::string to_string(long long val);
	std::string to_string(unsigned val);
	std::string to_string(unsigned long val);
	std::string to_string(unsigned long long val);
	std::string to_string(float val);
	std::string to_string(double val);
	std::string to_string(long double val);
}

#endif


#endif
