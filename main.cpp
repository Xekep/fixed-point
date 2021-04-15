#include <cstdio>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "fixed.h"


int main()
{
  
 // fixed f2(1);
 // f = -12.0;

  //f = f / 5 + -2.0;
  //f = 3.4;//2.4 - 6;

  //f = fixed::fromString("7936");
 // uint32_t test = 0x80000000;
 // f = *(float*)&test;

  //std::cout << fixed::fromString("-12.6");
  fixed f1(-1.5f);
  std::cout << f1.toString() << std::endl;
  f1 /= 2;
  std::cout << std::setprecision(8) << f1.toString() << std::endl;


  if(f1 < fixed(0))
  {
    std::cout << std::setprecision(8) << f1 << " меньше"  << std::endl;
  }
  fixed f2(731.4f);
  std::cout << std::setprecision(8) << f2 << std::endl;

  fixed f3(-160.0314f);
  std::cout << std::setprecision(8) << f3 << std::endl;  

  try
  {
    fixed f4(32769);
    std::cout << std::setprecision(8) << f4 << std::endl;  
  }
  catch(std::string ex)
  {
    std::cout << ex << std::endl; 
  } 
  //std::cout << std::hex << *(uint32_t*)&test << std::endl;
  return 0;
}