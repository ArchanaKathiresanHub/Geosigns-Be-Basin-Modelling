#ifndef COMPUTE_PROPERTY_H
#define COMPUTE_PROPERTY_H

#include <vector>

enum Mixing { Homogeneous, Layered };

class Properties
{
public:
   Properties( size_t bufSize );
   ~Properties();
   
   // resize vectors and init them random values
   void allocateVectors( size_t newSize, bool fillin );
   
   void computeDensity( size_t bufSize );
   void computePermeability ( size_t bufferSize, Mixing mixingType );
   
private:
   size_t m_arraySize;
   bool   m_initialized;
   double m_salinity;
   
   double * m_vector1;   
   double * m_vector2;   
   
   void clean();

};


class CompoundLithology 
{
public:
   CompoundLithology( double aPerm, double aAniso) {
      m_permeability = aPerm;
      m_anisotropy   = aAniso;
   };
                  
   double m_permeability;
   double m_anisotropy;
};
#endif
