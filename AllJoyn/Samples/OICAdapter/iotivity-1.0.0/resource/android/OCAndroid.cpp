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

#include <sstream>
#include "OCAndroid.h"

namespace OC {
	template <typename T>
	void from_string(const std::string& s, T& result) {
		std::stringstream ss(s);
		ss >> result;    // TODO handle errors
	}

	/*
	template <typename T>
	std::string to_string(T value)
	{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
	}
	*/

}

namespace std {

	int stoi(const string& s)
	{
		int ret;
		int &ref = ret;
		OC::from_string(s, ref);
		return ret;
	}

	double stod(const std::string& s)
	{
		double ret;
		double &ref = ret;
		OC::from_string(s, ref);
		return ret;
	}

	long long stoll(const std::string& s)
	{
		long long ret;
		long long int &ref = ret;
		OC::from_string(s, ref);
		return ret;
	}

	unsigned long long stoull(const std::string& s)
	{
		unsigned long long ret;
		unsigned long long  &ref = ret;
		OC::from_string(s, ref);
		return ret;
	}

	long double stold(const string& s)
	{
		long double ret;
		long double &ref = ret;
		OC::from_string(s, ref);
		return ret;
	}

	#define TO_STRING(_t) { \
		std::ostringstream os; \
		os << _t; \
		return os.str(); \
	} \

	std::string to_string(int val)
	{
		TO_STRING(val)
	}

	std::string to_string(long val)
	{
		TO_STRING(val)
	}

	std::string to_string(long long val)
	{
		TO_STRING(val)
	}

	std::string to_string(unsigned val)
	{
		TO_STRING(val)
	}

	std::string to_string(unsigned long val)
	{
		TO_STRING(val)
	}

	std::string to_string(unsigned long long val)
	{
		TO_STRING(val)
	}

	std::string to_string(float val)
	{
		TO_STRING(val)
	}

	std::string to_string(double val)
	{
		TO_STRING(val)
	}

	std::string to_string(long double val)
	{
		TO_STRING(val)
	}
} // std
