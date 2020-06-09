/*
 * \file ChemicalCompactionCalculator.cpp
 */

#include "ChemicalCompactionCalculator.h"
#include "WalderhaugCompactionCalculator.h"
#include "SchneiderCompactionCalculator.h"

#include <iostream>
#include <sstream>

namespace GeoPhysics {

ChemicalCompactionCalculator * ChemicalCompactionCalculator :: create( const std::string & algorithmName )
{
   if( algorithmName == "Walderhaug" )
   {
      return new WalderhaugCompactionCalculator ();
   }
   else if( algorithmName == "Schneider" )
   {
      return new SchneiderCompactionCalculator ();
   }
   else
   {
      std::cerr << "Error message: Invalid Chemical compaction algorithm name " << std::endl;
      return 0;
   }
}

};//end of namespace
