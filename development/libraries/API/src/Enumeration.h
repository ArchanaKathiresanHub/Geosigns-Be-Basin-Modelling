//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Enumeration.h
/// @brief This file keeps declaration of top level class for handling categorical parameter values set

#ifndef CASA_API_ENUMERATION_H
#define CASA_API_ENUMERATION_H

namespace casa
{
   /// @brief Base class for handling values set of categorical parameters
   class Enumeration
   {
   public:
   protected:
      Enumeration() {;}
      virtual ~Enumeration() {;}
   };
}

#endif // CASA_API_ENUMERATION_H
