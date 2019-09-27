//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_MOCKINTERFACEOUTPUT_H_
#define _CRUSTALTHICKNESS_MOCKINTERFACEOUTPUT_H_

//Parent
#include "../src/AbstractInterfaceOutput.h"

// DataAccess
#include "../../DataAccess/src/CrustalThicknessInterface.h"

#include <boost/multi_array.hpp>

/// @class MockInterfaceOutput Class which saves and get data in a 3d boost array
///        Should be used only in unit tests
class MockInterfaceOutput : public AbstractInterfaceOutput {

   public:
   
      MockInterfaceOutput();
      MockInterfaceOutput( unsigned int firstI,
                           unsigned int firstJ,
                           unsigned int lastI,
                           unsigned int lastJ );
      ~MockInterfaceOutput() {}

      double getMapValue ( CrustalThicknessInterface::outputMaps mapIndex, const unsigned int i, const unsigned int j ) const { return m_outputMaps[mapIndex][i - m_firstI][j - m_firstJ]; };
      
      /// @brief Set m_outputMaps[i,j] to value
      void setMapValue ( const CrustalThicknessInterface::outputMaps mapIndex,
         const unsigned int i,
         const unsigned int j,
         const double value ) final;

      /// @brief Set all m_outputMaps[i,j] to value
      void setMapValues( CrustalThicknessInterface::outputMaps mapIndex, const double value );

      /// @brief Clear the boost multi array by setting all values to 0;
      void clear();

   private:
   
      boost::multi_array<double, 3> m_outputMaps;
      const unsigned int m_firstI; ///< First i index on the array
      const unsigned int m_firstJ; ///< First j index on the array
      const unsigned int m_lastI;  ///< Last i index on the array
      const unsigned int m_lastJ;  ///< Last j index on the array

};

#endif