#ifndef _ANDRDIO_STRING_H_
#define _ANDRDIO_STRING_H_

#ifdef __ANDROID__
#include <string>

#ifndef ANDROID_C11_COMPAT
using std::to_string;
#else
namespace std {
    int stoi(const std::string& s);
    double stod(const std::string& s);
    long long stoll(const std::string& s);
    unsigned long long stoull(const std::string& s);
    long double stold(const string& s);

    std::string to_string(int i);
    std::string to_string(long t);
    std::string to_string(uint32_t i);
    std::string to_string(double d);
}
#endif

#endif

#endif