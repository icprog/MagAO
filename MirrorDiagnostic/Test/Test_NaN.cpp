#include <iostream>
#include <float.h>
#include <math.h>
#include <cstdlib> //strtod
#include <limits>
#include <errno.h>

using namespace std;


template<typename T> void NumericLimitsInfo()
{
    numeric_limits<T> info;
    cout << " is specialized?     "<< boolalpha << info.is_specialized << '\n';
    if (info.is_specialized) {
    cout << " is iec559/ieee754 ? "<< boolalpha << info.is_iec559 << '\n';
    cout << " is bounded?         "<< boolalpha << info.is_bounded << '\n';
    cout << " is signed?          "<< boolalpha << info.is_signed << '\n';
    cout << " is integer?         "<< boolalpha << info.is_integer << '\n';
    cout << " is modulo?          "<< boolalpha << info.is_modulo << '\n';
    cout << " is exact?           "<< boolalpha << info.is_exact << '\n';
    cout << " radix             : "<< info.radix << endl;
    cout << " digits            : "<< info.digits << endl;
    cout << " digits10          : "<< info.digits10 << endl;
    cout << " min_exponent      : "<< info.min_exponent << endl;
    cout << " min_exponent10    : "<< info.min_exponent10 << endl;
    cout << " max_exponent      : "<< info.max_exponent << endl;
    cout << " max_exponent10    : "<< info.max_exponent10 << endl;
    cout << " min()             : "<< info.min() << endl;
    cout << " max()             : "<< info.max() << endl;
    cout << " epsilon()         : "<< info.epsilon() << endl;
    cout << " round_error()     : "<< info.round_error() << endl;
    cout << " denorm_min()      : "<< info.denorm_min() << endl;
    cout << " has_infinity      : "<< boolalpha << info.has_infinity << endl; 
    if (info.has_infinity)       cout << " infinity()        : "<< info.infinity() << endl; 
    cout << " has_quiet_NaN     : "<< boolalpha << info.has_quiet_NaN << endl; 
    if (info.has_quiet_NaN)      cout << " quiet_NaN()       : "<< info.quiet_NaN() << endl;
    cout << " has_signaling_NaN : "<< boolalpha << info.has_signaling_NaN << endl;
    if (info.has_signaling_NaN)  cout << " signaling_NaN()   : "<< info.signaling_NaN() << endl;
    cout << " has_denorm        : "<< info.has_denorm << endl; 
    cout << " rouns_style       : "<< info.round_style << endl; 
    cout << " tinyness_before   : "<< boolalpha << info.tinyness_before << endl; 
    cout << " traps             : "<< boolalpha << info.traps << endl; 
    }
    return;
}


int main()
{
   
    cout << endl << "--------------- double ------------------" << endl; NumericLimitsInfo<double>();
    cout << endl << "--------------- long double -------------" << endl; NumericLimitsInfo<long double>();
    cout << endl << "--------------- float  ------------------" << endl; NumericLimitsInfo<float>();
    cout << endl << "--------------- bool     ----------------" << endl; NumericLimitsInfo<bool>();
    cout << endl << "--------------- char    -----------------" << endl; NumericLimitsInfo<char>();
    cout << endl << "--------------- short    ----------------" << endl; NumericLimitsInfo<short>();
    cout << endl << "--------------- unsigned short    -------" << endl; NumericLimitsInfo<unsigned short>();
    cout << endl << "--------------- int    ------------------" << endl; NumericLimitsInfo<int>();
    cout << endl << "--------------- unsigned int    ---------" << endl; NumericLimitsInfo<unsigned int>();
    cout << endl << "--------------- long            ---------" << endl; NumericLimitsInfo<long>();
    cout << endl << "--------------- unsigned long    --------" << endl; NumericLimitsInfo<unsigned long>();
    cout << endl << "--------------- long long       ---------" << endl; NumericLimitsInfo<long long>();
    cout << endl << "--------------- unsigned long long   ----" << endl; NumericLimitsInfo<unsigned long long>();
    cout << endl << "--------------- string      --------" << endl; NumericLimitsInfo<string>();

    cout << endl;
    cout << "asin(1.5)              = " << asin( 1.5 ) << endl;
    cout << "double(0)/double(0)    = " << double(0)/double(0) << endl;
    cout << "double(0)/int(0)       = " << double(0)/int(0) << endl;
    cout << "int(0)/double(0)       = " << int(0)/double(0) << endl;
    cout << "int(0)/int(0)          = " << double(0)/double(0) << endl;
    cout << endl;

    cout << endl;
    cout << "double(1)/double(0)    = " << double(1)/double(0) << endl;
    cout << "double(1)/int(0)       = " << double(1)/int(0) << endl;
    cout << "int(1)/double(0)       = " << int(1)/double(0) << endl;
    cout << "int(1)/int(0)          = " << double(1)/double(0) << endl;
    cout << endl;

    cout << endl;
    cout << "isnan( numeric_limits<float>::infinity() )           = " << boolalpha << isnan( numeric_limits<float>::infinity() ) << endl;
    cout << "isnan( numeric_limits<double>::infinity() )          = " << boolalpha << isnan( numeric_limits<double>::infinity() ) << endl;
    cout << "isnan( numeric_limits<long double>::infinity() )     = " << boolalpha << isnan( numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan( numeric_limits<float>::quiet_NaN() )          = " << boolalpha << isnan( numeric_limits<float>::quiet_NaN() ) << endl;
    cout << "isnan( numeric_limits<double>::quiet_NaN() )         = " << boolalpha << isnan( numeric_limits<double>::quiet_NaN() ) << endl;
    cout << "isnan( numeric_limits<long double>::quiet_NaN() )    = " << boolalpha << isnan( numeric_limits<long double>::quiet_NaN() ) << endl;
    cout << "isnan( numeric_limits<float>::signaling_NaN() )      = " << boolalpha << isnan( numeric_limits<float>::signaling_NaN() ) << endl;
    cout << "isnan( numeric_limits<double>::signaling_NaN() )     = " << boolalpha << isnan( numeric_limits<double>::signaling_NaN() ) << endl;
    cout << "isnan( numeric_limits<long double>::signaling_NaN() )= " << boolalpha << isnan( numeric_limits<long double>::signaling_NaN() ) << endl;
    cout << endl;

    cout << endl;
    cout << "isinf( numeric_limits<float>::infinity() )           = " << boolalpha << isinf( numeric_limits<float>::infinity() ) << endl;
    cout << "isinf( numeric_limits<double>::infinity() )          = " << boolalpha << isinf( numeric_limits<double>::infinity() ) << endl;
    cout << "isinf( numeric_limits<long double>::infinity() )     = " << boolalpha << isinf( numeric_limits<long double>::infinity() ) << endl;
    cout << "isinf( numeric_limits<float>::quiet_NaN() )          = " << boolalpha << isinf( numeric_limits<float>::quiet_NaN() ) << endl;
    cout << "isinf( numeric_limits<double>::quiet_NaN() )         = " << boolalpha << isinf( numeric_limits<double>::quiet_NaN() ) << endl;
    cout << "isinf( numeric_limits<long double>::quiet_NaN() )    = " << boolalpha << isinf( numeric_limits<long double>::quiet_NaN() ) << endl;
    cout << "isinf( numeric_limits<float>::signaling_NaN() )      = " << boolalpha << isinf( numeric_limits<float>::signaling_NaN() ) << endl;
    cout << "isinf( numeric_limits<double>::signaling_NaN() )     = " << boolalpha << isinf( numeric_limits<double>::signaling_NaN() ) << endl;
    cout << "isinf( numeric_limits<long double>::signaling_NaN() )= " << boolalpha << isinf( numeric_limits<long double>::signaling_NaN() ) << endl;
    cout << endl;

    cout << endl;
    cout << "isnan(  inf *  inf ) <float>                         = " << isnan( numeric_limits<float>::infinity() *  numeric_limits<float>::infinity() ) << endl;
    cout << "isnan(  inf * -inf ) <float>                         = " << isnan( numeric_limits<float>::infinity() * -numeric_limits<float>::infinity() ) << endl;
    cout << "isnan( -inf *  inf ) <float>                         = " << isnan(-numeric_limits<float>::infinity() *  numeric_limits<float>::infinity() ) << endl;
    cout << "isnan( -inf * -inf ) <float>                         = " << isnan(-numeric_limits<float>::infinity() * -numeric_limits<float>::infinity() ) << endl;
    cout << "isnan(  0 *  inf )   <float>                         = " << isnan( 0  * numeric_limits<float>::infinity() ) << endl;
    cout << "isnan(  0 * -inf )   <float>                         = " << isnan( 0  * -numeric_limits<float>::infinity() ) << endl;
    cout << "isnan(  inf +  inf ) <float>                         = " << isnan( numeric_limits<float>::infinity() +  numeric_limits<float>::infinity() ) << endl;
    cout << "isnan(  inf + -inf ) <float>                         = " << isnan( numeric_limits<float>::infinity() + -numeric_limits<float>::infinity() ) << endl;
    cout << endl;

    cout << endl;
    cout << "isnan(  inf *  inf ) <double>                        = " << isnan( numeric_limits<double>::infinity() *  numeric_limits<double>::infinity() ) << endl;
    cout << "isnan(  inf * -inf ) <double>                        = " << isnan( numeric_limits<double>::infinity() * -numeric_limits<double>::infinity() ) << endl;
    cout << "isnan( -inf *  inf ) <double>                        = " << isnan(-numeric_limits<double>::infinity() *  numeric_limits<double>::infinity() ) << endl;
    cout << "isnan( -inf * -inf ) <double>                        = " << isnan(-numeric_limits<double>::infinity() * -numeric_limits<double>::infinity() ) << endl;
    cout << "isnan(  0 *  inf )   <double>                        = " << isnan( 0  * numeric_limits<double>::infinity() ) << endl;
    cout << "isnan(  0 * -inf )   <double>                        = " << isnan( 0  * -numeric_limits<double>::infinity() ) << endl;
    cout << "isnan(  inf +  inf ) <double>                        = " << isnan( numeric_limits<double>::infinity() +  numeric_limits<double>::infinity() ) << endl;
    cout << "isnan(  inf + -inf ) <double>                        = " << isnan( numeric_limits<double>::infinity() + -numeric_limits<double>::infinity() ) << endl;
    cout << endl;

    cout << endl;
    cout << "isnan(  inf *  inf ) <long double>                   = " << isnan( numeric_limits<long double>::infinity() *  numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan(  inf * -inf ) <long double>                   = " << isnan( numeric_limits<long double>::infinity() * -numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan( -inf *  inf ) <long double>                   = " << isnan(-numeric_limits<long double>::infinity() *  numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan( -inf * -inf ) <long double>                   = " << isnan(-numeric_limits<long double>::infinity() * -numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan(  0 *  inf )   <long double>                   = " << isnan( 0  * numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan(  0 * -inf )   <long double>                   = " << isnan( 0  * -numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan(  inf +  inf ) <long double>                   = " << isnan( numeric_limits<long double>::infinity() +  numeric_limits<long double>::infinity() ) << endl;
    cout << "isnan(  inf + -inf ) <long double>                   = " << isnan( numeric_limits<long double>::infinity() + -numeric_limits<long double>::infinity() ) << endl;
    cout << endl;



    return 0;
}
