
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <cstdio>

using namespace std;

typedef map<double, double> adcMap;

double adcInterpolate(const adcMap &data, double x);
void readAdcLut( string filename, adcMap &adc1, adcMap &adc2);

void adcTest( string filename);

