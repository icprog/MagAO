
#include "adcLut.h"
#include "AOExcept.h"

double adcInterpolate(const adcMap &data, double x)
{
  typedef adcMap::const_iterator iter_t;

  iter_t i=data.upper_bound(x);
  if(i==data.end())
  {
    return (--i)->second;
  }
  if (i==data.begin())
  {
    return i->second;
  }
  iter_t l=i; --l;

  double delta=(x- l->first)/(i->first - l->first);
  return delta*i->second +(1-delta)*l->second;
}



void readAdcLut( string filename, adcMap &adc1, adcMap &adc2) {

 string line;
 string buf;

 ifstream f(filename.c_str());
 if (f.is_open()) {
   while (f.good()) {
     getline(f, line);
     stringstream ss(line);
     vector<double> tokens;

     while (ss >> buf) {
	tokens.push_back(strtod(buf.c_str(), NULL));
     }
     if ((tokens.size() >= 3) && (tokens[0]>0)) {
        adc1[tokens[0]] = tokens[1]; 
        adc2[tokens[0]] = tokens[2]; 
        printf("ADC lut: inserted %f %f %f\n", tokens[0], tokens[1], tokens[2]);
     }
   }
   f.close();
 }
 else {
    throw AOException("Cannot read ADC lut file", ARB_INIT_ERROR);
 }

}

void adcTest( string filename) {

  adcMap adc1, adc2;

  readAdcLut(filename, adc1, adc2);

  for (int i=0; i<=90; i++)
   printf("Values for el=%d: %f %f\n", i, adcInterpolate(adc1, i), adcInterpolate(adc2, i));

}


