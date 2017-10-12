//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_PWDRCALCULATOR_H_
#define _CRUSTALTHICKNESS_PWDRCALCULATOR_H_

// CBMGenerics library
#include "Polyfunction.h"

// Geophysics library
#include "Local2DArray.h"

// forward declarations
class InterfaceInput;
class AbstractValidator;
class AbstractInterfaceOutput;

using namespace DataAccess;

namespace CrustalThickness
{
   /// @class PaleowaterdepthResidualCalculator The PWDR calculator
   class PaleowaterdepthResidualCalculator {

      typedef GeoPhysics::Local2DArray<CBMGenerics::Polyfunction> PolyFunction2DArray;

   public:

      /// @brief Constructs the PWDR calculator in order to compute the paleowaterdepth residual
      /// @param surfaceDepthHistory The user defined paleobathymetrie (loaded from the project handle)
      PaleowaterdepthResidualCalculator( InterfaceInput&            inputData,
         AbstractInterfaceOutput&   outputData,
         AbstractValidator&         validator,
         const double               age,
         const PolyFunction2DArray& surfaceDepthHistory
      );

      ~PaleowaterdepthResidualCalculator() {};

      /// @brief Computes the paleowaterdepth residual map
      void compute();

      /// @return The paleowaterdepth residual
      double calculatePWDR( const double  PWD,
         const double  surfaceDepthHistory ) const;

   private:

      const unsigned int m_firstI; ///< First i index on the map
      const unsigned int m_firstJ; ///< First j index on the map
      const unsigned int m_lastI;  ///< Last i index on the map
      const unsigned int m_lastJ;  ///< Last j index on the map

      const double m_age; ///< Age of the snapshot at which the PWDR is computed

      const PolyFunction2DArray& m_surfaceDepthHistory; ///< The user defined paleobathymetrie (loaded from the project handle)

      AbstractInterfaceOutput& m_outputData; ///< The global interface output object (contains the output maps)
      AbstractValidator&       m_validator;  ///< The validator to check if a node (i,j) is valid or not
   };
} // End namespace  CrustalThickness
#endif

