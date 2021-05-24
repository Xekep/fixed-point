#include <cstdio>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "fixed.h"

int main()
{ 
  fixed f1(0.8);
  while(f1 < 1.2)
  {
    f1 += 0.001;
  std::cout << f1.toString(1) << " " << f1.toFloat() << std::endl;
  }
  /*
  fixed f12 = (1 + f1 * 2);
  f12 = (f12 * -4.1) + "20.5";
  f12++;
  f12 -= fixed(7);
  std::cout << std::setprecision(8) << f12 << std::endl;

  if(f1 > 12)
  {
    std::cout << std::setprecision(8) << f1 << " больше" << f12  << std::endl;
  }
  
  fixed f2(731.4f);
  std::cout << std::setprecision(8) << f2 << std::endl;

  fixed f3(-160.0314f);
  std::cout << std::setprecision(8) << f3 << std::endl;  

  fixed f5(-0.25f);
  std::cout << std::setprecision(8) << f5 << std::endl;  

  try
  {
    fixed f4(1.1f); // ??
    std::cout << std::setprecision(8) << f4 << std::endl;

    std::cout << std::setprecision(8) << fixed::fromString("-23l.5") << std::endl;
  }
  catch(std::string ex)
  {
    std::cout << ex << std::endl; 
  }
  */
  return 0;
}