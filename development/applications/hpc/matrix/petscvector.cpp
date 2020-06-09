#include "petscvector.h"
#include "petscio.h"

namespace hpc
{

PetscVector
PetscVector
  :: load( std::istream & input)
{
  SizeType header, n;
  
  petscio::readIndices( input, &header, 1);
  if (header != petscio::Header::VECTOR)
    throw LoadException() << "File is not Petsc Vector";

  // read number of rows
  petscio::readIndices(input, &n,  1);
	
  PetscVector result;
  result.m_values.resize(n);

  petscio::readReal( input, &result.m_values[0], n);
  return result;
}

void
PetscVector
  :: save( std::ostream & output ) const
{
  const SizeType header = petscio::Header::VECTOR;
  const SizeType n = m_values.size();
  petscio::writeIndices( output, &header, 1);
  petscio::writeIndices( output, &n, 1);
 
  for (SizeType i = 0 ; i < n; ++i)
    petscio::writeReal( output, m_values[i] );
}


}

