
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "AOExcept.h"

/*class BadConversionException : public std::runtime_error {
public:
BadConversionException(std::string s)
 : std::runtime_error(s)
 { }
};*/


#include <cmath>
#include <cstdlib> //strtod
#include <cerrno>

inline void convertToDouble(const std::string& s, double& value)
{
    const char *temp=s.c_str();   // = input.str().c_str();
    char *endptr;

    errno = 0;    // To distinguish success/failure after call 
    value = strtod(temp, &endptr);

    // Check for various possible errors 
    if (errno == ERANGE && (value == HUGE_VAL || value == -HUGE_VAL || value == 0)) {
        throw AOOutOfRangeException("convertToDouble(\"" + s + "\")", __FILE__, __LINE__);
    }
    if (endptr == temp) { // No digits were found
        value = NAN; 
        throw AOBadConversionException("convertToDouble(\"" + s + "\")",__FILE__, __LINE__);
    }
    // here means explicitly set to NaN
    if (isnan(value)) throw AONaNException("convertToDouble(\"" + s + "\")",__FILE__, __LINE__);

    return;
}

inline void convertToFloat(const std::string& s, float& value)
{
    const char *temp=s.c_str();   // = input.str().c_str();
    char *endptr;

    errno = 0;    // To distinguish success/failure after call 
    value = strtof(temp, &endptr);

    // Check for various possible errors 
    if (errno == ERANGE && (value == HUGE_VAL || value == -HUGE_VAL || value == 0)) {
        throw AOOutOfRangeException("convertToFloat(\"" + s + "\")", __FILE__, __LINE__);
    }
    if (endptr == temp) { // No digits were found
        value = NAN; 
        throw AOBadConversionException("convertToFloat(\"" + s + "\")",__FILE__, __LINE__);
    }
    // here means explicitly set to NaN
    if (isnan(value)) throw AONaNException("convertToFloat(\"" + s + "\")",__FILE__, __LINE__);

    return;
}

template<typename T> inline std::istream &ExtractFromStream(std::istream & input, T &v)
{
    input >> v;
    return input;
}

template<> inline std::istream &ExtractFromStream<double>(std::istream & input, double &v)
{
    std::string str;
    input >> str;
    convertToDouble(str,v);
    return input;
}

template<> inline std::istream &ExtractFromStream<float>(std::istream & input, float &v)
{
    std::string str;
    input >> str;
    convertToFloat(str,v);
    return input;
}
