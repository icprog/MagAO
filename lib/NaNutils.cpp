#include "NaNutils.h"

#include <limits>

using namespace std;
using namespace Arcetri;

bool NaNutils::isNaN(double x)  { return x!=x; };
bool NaNutils::isNaN(int x) { return x==numeric_limits<int>::max(); };
bool NaNutils::isNaN(long x) { return x==numeric_limits<long>::max(); };
double NaNutils::dNaN () { return numeric_limits<double>::quiet_NaN(); };
int NaNutils::iNaN() { return numeric_limits<int>::max(); };
int NaNutils::lNaN() { return numeric_limits<long>::max(); };

