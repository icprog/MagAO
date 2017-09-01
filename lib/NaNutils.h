// Support for  NaN (integer NaN implemented as <int>max())

#ifndef NANUTILS_H_INCLUDE
#define NANUTILS_H_INCLUDE


// Root namespace
namespace Arcetri {

class NaNutils {
    public:
        static bool isNaN(double x);
        static bool isNaN(int x);
        static bool isNaN(long x);
        static double dNaN ();
        static int iNaN();
        static int lNaN();
	
};

};

#endif
