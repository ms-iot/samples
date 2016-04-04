#include <sstream>
#include "android_cpp11_compat.h"

namespace OC {
    template <typename T>
    void from_string(const std::string& s, T& result) {
        std::stringstream ss(s);
        ss >> result;    // TODO handle errors
    }
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

    std::string to_string(int t) {
        std::ostringstream os;
            os << t;
        return os.str();
    }

    std::string to_string(long t) {
        std::ostringstream os;
            os << t;
        return os.str();
    }

    std::string to_string(double t) {
        std::ostringstream os;
            os << t;
        return os.str();
    }

    std::string to_string(uint32_t t)
    {
        std::ostringstream os;
            os << t;
        return os.str();
    }

} // std
