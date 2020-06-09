#include "petscio.h"

namespace hpc
{

  namespace petscio
  {

   
    bool isBigEndian()
    {
      union {
	uint32_t i;
	char c[4];
      } integer = { 0x01020304 };
      
      return integer.c[0] == 1;
    }

    void readIndices( std::istream & input, SizeType * indices, SizeType n)
    {
      input.read( reinterpret_cast<char *>(indices), sizeof(indices[0]) * n);
      const std::streamsize indicesRead = input.gcount() / sizeof(indices[0]);
      if (indicesRead != n)
	throw ReadException() << "Expected '" << n << "'  index elements, while only '" << indicesRead 
	  << "' were encountered.";

      for (SizeType i = 0; i < n; ++i)
	indices[i] = convertNetworkToHost(indices[i]);
    }

    void writeIndices( std::ostream & output, const SizeType * indices, SizeType n)
    {
      for (SizeType i = 0; i < n; ++i)
      {
	SizeType x = convertHostToNetwork(indices[i]);
	output.write(reinterpret_cast<const char *>(&x), sizeof(x));
      }
    }

    void readReal(std::istream & input, ValueType * values, SizeType n )
    {
      input.read( reinterpret_cast<char *>(values), sizeof(values[0])*n);

      const std::streamsize valuesRead = input.gcount() / sizeof(values[0]);
      if (valuesRead != n)
	throw ReadException() << "Expected '" << n << "'  real values, while only '" << valuesRead 
	  << "' were encountered.";

      for (SizeType i = 0; i < n; ++i)
	values[i] = convertNetworkToHost(values[i]);
    }

    void writeReal( std::ostream & output, ValueType value)
    {
      ValueType x = convertHostToNetwork(value);
      output.write( reinterpret_cast<const char * >(&x), sizeof(x));
    }


  }

}
