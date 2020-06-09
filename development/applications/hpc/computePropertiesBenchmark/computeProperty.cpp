#include <cassert>
#include <time.h>
#include <iostream>
#include <string>
#include <utility> 
#include <stdlib.h>
#include <math.h>
#include "omp.h"

#include "computeProperty.h"

using namespace std;

Properties::Properties ( size_t bufSize ) {

   m_vector1 = 0; 
   m_vector2 = 0; 

   m_salinity = 0.0; 
   m_arraySize = 0;   
   m_initialized = false;

   allocateVectors( bufSize, true );
}

Properties::~Properties() {

   clean();
}

void Properties::clean() {
   
   if( !m_vector1 ) {
      delete [] m_vector1;  m_vector1 = 0; 
   }
   if( !m_vector2 ) {
      delete [] m_vector2;  m_vector2 = 0; 
   }

   m_salinity  = 0.0; 

   m_arraySize = 0;
}

void Properties::allocateVectors ( size_t newSize, bool initialize ) {

   if( newSize > m_arraySize ) {
      clean();

      //allocate memory for vectors
      m_arraySize = newSize;
      
      m_vector1 = new double [m_arraySize];
      m_vector2 = new double [m_arraySize];
      
      // vector1 = static_cast<double*>( _mm_malloc( 8 * sizeof ( double ) * arraySize, 32 ) );
      // vector2 = static_cast<double*>( _mm_malloc( 8 * sizeof ( double ) * arraySize, 32 ) );
      
      assert ( m_vector1 );
      assert ( m_vector2 );

      m_initialized = false;
   }

   if( initialize && !m_initialized ) {
      m_initialized = true;

      m_salinity = drand48();

      for( int i = 0; i < m_arraySize; ++ i ) {
         m_vector1[i] = drand48() * 1000;
         m_vector2[i] = drand48() * 200;
      }
   } 
 }

void Properties::computeDensity ( size_t bufferSize ) {

   allocateVectors( bufferSize, true );

   double * pressure    = m_vector1;
   double * temperature = m_vector2;
  
   double * density = new double [m_arraySize];
   assert( density );

   const double& s = m_salinity;
   int i;

//#pragma omp parallel shared( pressure, temperature, density ) private (i) 
   {
//  #pragma omp parallelfor nowait
//  #pragma omp parallel

      for ( i = 0; i < m_arraySize; ++ i ) {
      
         const double& p = pressure[i];
         const double& t = temperature[i];
         
         density[i] = 1000.0 * ( s * s * ( 0.44 - 0.0033 * t ) + p * p * ( -3.33e-7 - 2.0e-9 * t )+
                                 1.75e-9 * ( -1718.91 + t ) * ( -665.977 + t ) * ( 499.172 + t ) +
                                 s * ( 0.668 + 0.00008 * t + 3.0e-6 * t * t ) + 
                                 p * ( s * ( 0.0003 - 0.000013 * t ) + s * s * ( -0.0024 + 0.000047 * t ) -
                                       1.3e-11 * ( -1123.64 + t ) * ( 33476.2 - 107.125 * t + t * t )));
      }
   }
   //   std::cout << density[25] << " \n";
   delete [] density;
   
}

void Properties::computePermeability ( size_t bufferSize, Mixing mixingType ) {
   
   allocateVectors( bufferSize, false );

   double * permeabilityN = m_vector1;
   double * permeabilityP = m_vector2;

  // 20%, 70%, 10% lithology mixing
   std::vector<double> percentContainer;
   std::vector<CompoundLithology> lithoComponents;

   lithoComponents.push_back( CompoundLithology ( 1e-5, 1 ));
   percentContainer.push_back( 20 );

   lithoComponents.push_back( CompoundLithology ( 0.01, 1 ));
   percentContainer.push_back( 10 );
   
   lithoComponents.push_back( CompoundLithology ( 1e-4, 1 ));
   percentContainer.push_back( 70 );
 
   if( mixingType == Homogeneous ) {
      for ( int i = 0; i < bufferSize; ++ i ) {
         
         std::vector<double>::const_iterator percentIter = percentContainer.begin();
         std::vector<CompoundLithology>::const_iterator lithoIter = lithoComponents.begin();
         
         permeabilityN[i] = 1.0;
         permeabilityP[i] = 1.0;
         
         while( percentContainer.end() != percentIter ) {
            
            double volFrac  = (double)(*percentIter) * 0.01;
            double permVal  = lithoIter->m_permeability;
            double anisoVal = lithoIter->m_anisotropy;
            
            permeabilityN[i] *= pow( permVal, volFrac );
            permeabilityP[i] *= pow( permVal * anisoVal, volFrac );
            
            ++ lithoIter;
            ++ percentIter;
         }
         
      }
   } else {
      for ( int i = 0; i < bufferSize; ++ i ) {
         
         std::vector<double>::const_iterator percentIter = percentContainer.begin();
         std::vector<CompoundLithology>::const_iterator lithoIter = lithoComponents.begin();
         
         permeabilityN[i] = 1.0;
         permeabilityP[i] = 1.0;
         
         while( percentContainer.end() != percentIter ) {
            
            double volFrac  = (double)(*percentIter) * 0.01;
            double permVal  = lithoIter->m_permeability;
            double anisoVal = lithoIter->m_anisotropy;
            
            permeabilityN[i] = permeabilityN[i] + volFrac / permVal;
            permeabilityP[i] = permeabilityN[i] + volFrac * anisoVal * permVal;
            
            ++ lithoIter;
            ++ percentIter;
         }
         
      }
   }
   lithoComponents.clear();
   percentContainer.clear();

   //   std::cout << permeabilityN[25] << " " << permeabilityP[25] <<  "\n";
}
