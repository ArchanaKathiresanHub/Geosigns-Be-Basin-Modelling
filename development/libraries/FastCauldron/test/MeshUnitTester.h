//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON_UNIT_TESTING__MESH_UNIT_TESTER__H
#define FASTCAULDRON_UNIT_TESTING__MESH_UNIT_TESTER__H

#include <string>

/// \brief A helper class for the fastcauldron dof counting unit tests.
class MeshUnitTester {

public :

   /// \brief Set the age of the fastcauldron model.
   bool setTime ( const double age ) const;

   /// \brief Compare two text files.
   ///
   /// In the unit tests the files will be ASCII VTK mesh files.
   bool compareFiles ( const std::string& leftFileName,
                       const std::string& rightFileName ) const;

};

#endif // FASTCAULDRON_UNIT_TESTING__MESH_UNIT_TESTER__H
