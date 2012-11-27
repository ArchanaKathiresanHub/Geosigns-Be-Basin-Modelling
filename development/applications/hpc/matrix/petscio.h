#ifndef HPC_PETSCIO_H
#define HPC_PETSCIO_H

#include <iosfwd>
#include "generalexception.h"

namespace hpc
{

  namespace petscio
  {
    typedef uint32_t SizeType;
    typedef double   ValueType;

    struct Header
    {
      static const uint32_t MATRIX = 1211216;
      static const uint32_t VECTOR =  1211214;
    };

    struct ReadException : BaseException< ReadException > {};

    void readIndices(std::istream &, SizeType * indices, SizeType n );
    void writeIndices(std::ostream & , const SizeType * indices, SizeType n);
    void readReal(std::istream & input, ValueType * values, SizeType n);
    void writeReal(std::ostream & output, ValueType value);

    bool isBigEndian();

    template <typename T> T convertHostToNetwork(const T & value)
    {
      if (isBigEndian())
      {
	return value;
      }
      else
      {
	union {
	  T value;
	  char bytes[sizeof(T)];
	} x = { value };

	union {
	  T value;
	  char bytes[sizeof(T)];
	} y ;

	for (uint32_t i = 0; i < sizeof(T); ++i)
	  y.bytes[i] = x.bytes[sizeof(T) - i - 1];

	return y.value;
      }
    }


    template <typename T> T convertNetworkToHost(const T & x)
    { return convertHostToNetwork(x); }


  }

}

#endif
