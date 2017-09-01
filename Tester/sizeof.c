#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#ifdef __i386__
char *arch="i386";
char *fmt=" %20s -> %d\n";
#else
char *arch="x86_64";
char *fmt=" %20s -> %ld\n";
#endif

int main()
{
   printf("\n");
   printf("**************** SIZEOF *****************\n");
   printf("*  Running on an %6s architecture    *\n\n",arch);
   printf(fmt,"char", sizeof(char));
   printf(fmt,"unsigned char", sizeof(unsigned char));
   printf(fmt,"short", sizeof(short));
   printf(fmt,"unsigned short", sizeof(unsigned short));
   printf(fmt,"int", sizeof(int));
   printf(fmt,"unsigned int", sizeof(unsigned int));
   printf(fmt,"long", sizeof(long));
   printf(fmt,"unsigned long", sizeof(unsigned long));
   printf(fmt,"long long", sizeof(long long));
   printf(fmt,"unsigned long long", sizeof(unsigned long long));
   printf(fmt,"float", sizeof(float));
   printf(fmt,"double", sizeof(double));
   printf(fmt,"long double", sizeof(long double));
   printf(fmt,"size_t", sizeof(size_t));
   printf(fmt,"ptrdiff_t", sizeof(ptrdiff_t));
   printf(fmt,"wchar_t", sizeof(wchar_t));
   printf(fmt,"wint_t", sizeof(wint_t));
   printf(fmt,"intmax_t", sizeof(intmax_t));
   printf(fmt,"uintmax_t", sizeof(uintmax_t));
   printf(fmt,"int8_t", sizeof(int8_t));
   printf(fmt,"int16_t", sizeof(int16_t));
   printf(fmt,"int32_t", sizeof(int32_t));
   printf(fmt,"int64_t", sizeof(int64_t));
   printf(fmt,"uint8_t", sizeof(uint8_t));
   printf(fmt,"uint16_t", sizeof(uint16_t));
   printf(fmt,"uint32_t", sizeof(uint32_t));
   printf(fmt,"uint64_t", sizeof(uint64_t));
   printf(fmt,"int_least8_t", sizeof(int_least8_t));
   printf(fmt,"int_least16_t", sizeof(int_least16_t));
   printf(fmt,"int_least32_t", sizeof(int_least32_t));
   printf(fmt,"int_least64_t", sizeof(int_least64_t));
   printf(fmt,"uint_least8_t", sizeof(uint_least8_t));
   printf(fmt,"uint_least16_t", sizeof(uint_least16_t));
   printf(fmt,"uint_least32_t", sizeof(uint_least32_t));
   printf(fmt,"uint_least64_t", sizeof(uint_least64_t));
   printf(fmt,"int_fast8_t", sizeof(int_fast8_t));
   printf(fmt,"int_fast16_t", sizeof(int_fast16_t));
   printf(fmt,"int_fast32_t", sizeof(int_fast32_t));
   printf(fmt,"int_fast64_t", sizeof(int_fast64_t));
   printf(fmt,"uint_fast8_t", sizeof(uint_fast8_t));
   printf(fmt,"uint_fast16_t", sizeof(uint_fast16_t));
   printf(fmt,"uint_fast32_t", sizeof(uint_fast32_t));
   printf(fmt,"uint_fast64_t", sizeof(uint_fast64_t));
   printf(fmt,"intptr_t", sizeof(intptr_t));
   printf("*****************************************\n");
}
