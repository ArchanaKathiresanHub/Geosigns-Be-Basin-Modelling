#ifndef PRINTVECTOR_HH
#define PRINTVECTOR_HH
#include <vector>
#include <iostream>

template<typename T>
void printV(std::vector<T>& v)
{
  for(size_t i = 0; i< v.size(); ++i)
	std::cout<<v[i]<<" ";
  std::cout<<std::endl;
}

#endif
