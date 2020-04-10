#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

char line1[] = "G10  21408287.681    21408287.451                    21408287.261    21408286.671    21408283.731        2281.851        2281.849                        1778.077        1778.096        1703.992   112501305.764 8 112501302.764 8                  87663353.322 8  87663355.316 8  84010714.059 8        48.164          50.859                          50.859          49.672          52.625";
char line2[] = "G12  22923566.178    22923566.118                    22923563.878    22923564.778                        2302.304        2302.304                        1794.054        1793.999                   120464155.580 6 120464157.567 5                  93868175.973 5  93868172.968 7                        41.262          31.754                          31.754          43.688";
char line3[] = "G15  23805496.468    23805496.238                    23805493.938    23805494.578                       -2438.709       -2438.714                       -1900.267       -1900.306                   125098718.186 6 125098720.196 4                  97479519.720 4  97479520.715 6                        40.945          25.973                          25.973          40.848";
char line4[] = "G20  19920161.191    19920160.721                    19920158.031                                         306.276         306.276                         238.701                                   104681147.588 8 104681144.590 7                  81569722.543 7                                        48.145          46.176                          46.176";
char line5[] = "G21  22278898.577    22278896.977                    22278895.007                                       -2218.671       -2218.672                       -1728.829                                   117076389.188 7 117076388.184 5                  91228353.955 5                                        44.824          31.723                          31.723";
char line6[] = "G24  20948208.760    20948208.600                    20948208.750    20948208.500    20948206.970       -2178.523       -2178.525                       -1697.519       -1697.512       -1626.785   110083386.666 7 110083578.668 8                  85779411.482 8  85779415.478 8  82205269.236 8        47.402          52.625                          52.625          53.234          53.469";
char line7[] = "G25  23744234.305    23744233.595                    23744236.005    23744236.495    23744234.835        3268.512        3268.512                        2546.980        2546.896        2440.788   124776825.348 6 124776795.351 4                  97228672.730 4  97228671.728 7  93177481.169 7        38.004          28.230                          28.230          42.328          46.195";
char line8[] = "G32  22805724.844    22805724.914                    22805725.094    22805725.034    22805722.234        1760.842        1760.838                        1372.097        1372.097        1314.929   119844889.291 6 119844888.293 7                  93385627.723 7  93385627.723 7  89494560.498 8        41.363          42.598                          42.598          44.223          47.672";

inline bool
string_is_empty(const char* str, std::size_t stop=-1) noexcept
{
  std::size_t len = std::strlen(str);
  if (stop!=(std::size_t)-1 && stop<len) len = stop;
  for (std::size_t i=0; i<len; i++) if (*(str+i)!=' ') return false;
  return true;
}

template<int M>
class Str2DoubleConverter
{ 
public:
  static inline char __str[M+1]; 
};

template<int M> 
class SpacesAsZeros : public Str2DoubleConverter<M>
{
public:
  static bool
  char2double(const char* line, double* data, int N) noexcept
  {
    char* end;
    Str2DoubleConverter<M>::__str[M] = '\0';
    for (int i=0; i<N; i++) {
      if (string_is_empty(line, M)) {
        data[i] = 0e0;
      } else {
        std::memcpy(Str2DoubleConverter<M>::__str, line, M);
        std::cout<<"\n--> temp buffer: \""<<Str2DoubleConverter<M>::__str<<"\"";
        data[i] = std::strtod(Str2DoubleConverter<M>::__str, &end);
        if (line == end) return false;
      }
      line+=M;
    }
    return (errno) ? false : true;
  }
};

template<int M> 
class SpacesAreErrors : public Str2DoubleConverter<M>
{
public:
  static bool
  char2double(const char* line, double* data, int N) noexcept
  {
    char* end;
    Str2DoubleConverter<M>::__str[M] = '\0';
    for (int i=0; i<N; i++) {
      std::memcpy(Str2DoubleConverter<M>::__str, line, M);
      data[i] = std::strtod(Str2DoubleConverter<M>::__str, &end);
      if (Str2DoubleConverter<M>::__str == end) return false;
      line+=M;
    }
    return (errno) ? false : true;
  }
};

template<template<int> class P, int M>
bool
char2double(const char* line, double* data, int N) noexcept
{
  return P<M>::char2double(line, data, N);
}

int main()
{
  std::vector<const char*> vec {line1, line2, line3, line4, line5, 
    line6, line7, line8};
  double data[24];
  int status;
  for (const char* s : vec) {
    std::cout<<"\n"<<s;
    std::cout<<"\nresolved to:";
    // status = char2double<SpacesAsZeros, 16>(s+3, data, 24);
    status = char2double<SpacesAreErrors, 16>(s+3, data, 24);
    if (status) {
      for (int i=0;i<24;i++) std::cout<<"\n\t["<<i<<"] = "<<data[i];
    } else {
      std::cout<<" !!!!!!!!!!!!!! FAILURE !!!!!!!!!!!!";
    }
  }

  std::cout<<"\n";
  return 0;
}
