#include "mex.h"


void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
  int dataLength,i;  
  double *x;
  char *data;
  char fldata[4];
  
  data=(char *)mxGetPr(prhs[0]);
  plhs[0]=mxCreateDoubleMatrix(1,1,mxREAL);
  x=mxGetPr(plhs[0]);
  /*rotate data*/
  for(i=0;i<4;i++)
  {
   fldata[3-i]=data[i];
  }
  x[0]=*((float *)fldata); 
}



