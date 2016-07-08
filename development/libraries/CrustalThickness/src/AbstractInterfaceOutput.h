//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_ABSTRACTINTERFACEOUTPUT_H_
#define _CRUSTALTHICKNESS_ABSTRACTINTERFACEOUTPUT_H_

// DataAccess
#include "Interface/CrustalThicknessInterface.h"

/// @class InterfaceOutput The CTC abstract output interface
class AbstractInterfaceOutput {


public:
   AbstractInterfaceOutput()          {};
   virtual ~AbstractInterfaceOutput() {};

   virtual double getMapValue( CrustalThicknessInterface::outputMaps mapIndex, unsigned int i, unsigned int j ) const               = 0;
   virtual void   setMapValue( CrustalThicknessInterface::outputMaps mapIndex, unsigned int i, unsigned int j, const double value ) = 0;

};

#endif