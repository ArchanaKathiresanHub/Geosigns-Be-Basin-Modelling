//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SpeciesManager.h"
#include "Utilities.h"
#include <iostream>
#include <iomanip>
#include <string.h>

using namespace std;

Genex6::SpeciesManager::SpeciesManager () {

   int i;

   IatomC = -1;
   IatomH = -1;
   IatomO = -1;
   IatomN = -1;
   IatomS = -1;

   Lkerogen     = -1;
   Lpreasphalt  = -1;
   Lasphaltene  = -1;
   Lresin       = -1;
   Lprecoke     = -1;
   Lcoke1       = -1;
   Lcoke2       = -1;
   LC15plusAro  = -1;
   LC15plusSat  = -1;
   LC6to14Sat   = -1;
   LC6to14Aro   = -1;
   Lhetero1     = -1;
   LC5          = -1;
   LC4          = -1;
   LC3          = -1;
   LC2          = -1;
   LC1          = -1;
   LC15plusAroS = -1;
   LC15plusSatS = -1;
   LLSC         = -1;
   LC15plusAT   = -1; 
   LC6to14BT    = -1;
   LC6to14DBT   = -1;
   LC6to14BP    = -1;
   LC6to14AroS  = -1;
   LC6to14SatS  = -1;
   LcokeS       = -1;
   LSO4         = -1;
   LN2          = -1;
   LCOx         = -1;
   LCO3         = -1;
   LH2O         = -1;
   LOrgacid     = -1;

   LSulfur  = -1;
   LCarsul1 = -1;
   LCarsul2 = -2;

   for ( i = 0; i < numberOfSpecies + 1; ++i ) {
      s_mappingToPvtComponents [ i ] = ComponentId::UNKNOWN;
   }

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      s_mappingFromPvtComponents [ i ] = -1;
   }

   for ( i = 0; i < numberOfSpecies + 1; ++i ) {
      s_mappingToComponentManagerSpecies [ i ] = ComponentId::UNKNOWN;
   }

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      s_mappingFromComponentManagerSpecies [ i ] = -1;
   }

   s_idNames [ numberOfSpecies + 1 ] = "UNKNOWN";

}
   
   
int Genex6::SpeciesManager::GetElementIdByName ( const std::string& elementName ) const {
   int elem_id = -1;

   if( elementName == "H" ) elem_id = IatomH;
   else if( elementName == "O" ) elem_id = IatomO;
   else if( elementName == "C" ) elem_id = IatomC;
   else if( elementName == "N" ) elem_id = IatomN;
   else if( elementName == "S" ) elem_id = IatomS;
   return elem_id;
}

int Genex6::SpeciesManager::getSpeciesIdByName ( const std::string& speciesName ) const {
   int elem_id = -1;
   int i;

   // Species name in lower-case.
   string nameLc = Genex6::toLower ( speciesName );

   for ( i = 1; i <= numberOfSpecies; ++i ) {

      if ( nameLc == s_idNames [ i ]) {
         elem_id = i;
         break;
      }

   }

   return elem_id;
}

void Genex6::SpeciesManager::setSpeciesNum ( const std::string& speciesName, int id )
{
   int START = 0;

   s_idNames [ id - START ] = Genex6::toLower ( speciesName );
   if( Genex6::toLower ( speciesName ) == "kerogen" ) {
      Lkerogen = id;
   } else if( Genex6::toLower ( speciesName ) == "preasphalt" ) {
      Lpreasphalt = id;
   } else if( Genex6::toLower ( speciesName ) == "asphaltenes" ) {
      Lasphaltene = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::ASPHALTENE;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::ASPHALTENE;
      s_mappingFromPvtComponents [ ComponentId::ASPHALTENE ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::ASPHALTENE ] = id;
   } else if( Genex6::toLower ( speciesName ) == "resins" ) {
      Lresin = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::RESIN;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::RESIN;
      s_mappingFromPvtComponents [ ComponentId::RESIN ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::RESIN ] = id;
   } else if( Genex6::toLower ( speciesName ) == "precoke" ) {
      Lprecoke = id;
   } else if( Genex6::toLower ( speciesName ) == "coke1" ) {
      Lcoke1 = id;
   } else if( Genex6::toLower ( speciesName ) == "coke2" ) {
      Lcoke2 = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+aro" ) {
      LC15plusAro = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C15_PLUS_ARO;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C15_PLUS_ARO;
      s_mappingFromPvtComponents [ ComponentId::C15_PLUS_ARO ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C15_PLUS_ARO ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+sat" ) {
      LC15plusSat = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C15_PLUS_SAT;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C15_PLUS_SAT;
      s_mappingFromPvtComponents [ ComponentId::C15_PLUS_SAT ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C15_PLUS_SAT ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14sat" ) {
      LC6to14Sat = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14SAT;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14SAT;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14SAT ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14SAT ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14aro" ) {
      LC6to14Aro = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14ARO;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14ARO;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14ARO ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14ARO ] = id;
   } else if( Genex6::toLower ( speciesName ) == "hetero1" ) {
      Lhetero1 = id;
   } else if( Genex6::toLower ( speciesName ) == "hetero2" ) {
      Lhetero2 = id;
   } else if( Genex6::toLower ( speciesName ) == "c5" ) {
      LC5 = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C5;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C5;
      s_mappingFromPvtComponents [ ComponentId::C5 ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C5 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c4" ) {
      LC4 = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C4;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C4;
      s_mappingFromPvtComponents [ ComponentId::C4 ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C4 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c3" ) {
      LC3 = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C3;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C3;
      s_mappingFromPvtComponents [ ComponentId::C3 ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C3 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c2" ) {
      LC2 = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C2;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C2;
      s_mappingFromPvtComponents [ ComponentId::C2 ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C2 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c1" ) {
      LC1 = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C1;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C1;
      s_mappingFromPvtComponents [ ComponentId::C1 ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C1 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "cox" ) {
      LCOx = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::COX;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::COX;
      s_mappingFromPvtComponents [ ComponentId::COX ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::COX ] = id;
   } else if( Genex6::toLower ( speciesName ) == "n2" ) {
      LN2 = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::N2;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::N2;
      s_mappingFromPvtComponents [ ComponentId::N2 ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::N2 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "lsc" ) {
      LLSC = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::LSC;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::LSC;
      s_mappingFromPvtComponents [ ComponentId::LSC ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::LSC ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+aros" ) {
      LC15plusAroS = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C15_PLUS_ARO_S;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C15_PLUS_ARO_S;
      s_mappingFromPvtComponents [ ComponentId::C15_PLUS_ARO_S ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C15_PLUS_ARO_S] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+sats" ) {
      LC15plusSatS = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C15_PLUS_SAT_S;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C15_PLUS_SAT_S;
      s_mappingFromPvtComponents [ ComponentId::C15_PLUS_SAT_S ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C15_PLUS_SAT_S] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+at" ) {
      LC15plusAT = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C15_PLUS_AT;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C15_PLUS_AT;
      s_mappingFromPvtComponents [ ComponentId::C15_PLUS_AT ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C15_PLUS_AT] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14bt" ) {
      LC6to14BT = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14BT;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14BT;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14BT ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14BT] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14dbt" ) {
      LC6to14DBT = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14DBT;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14DBT;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14DBT ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14DBT] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14bp" ) {
      LC6to14BP = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14BP;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14BP;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14BP ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14BP] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14aros" ) {
      LC6to14AroS = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14ARO_S;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14ARO_S;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14ARO_S ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14ARO_S] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14sats" ) {
      LC6to14SatS = id;
      s_mappingToPvtComponents [ id - START ] = ComponentId::C6_MINUS_14SAT_S;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::C6_MINUS_14SAT_S;
      s_mappingFromPvtComponents [ ComponentId::C6_MINUS_14SAT_S ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::C6_MINUS_14SAT_S] = id;
   }else if( Genex6::toLower ( speciesName ) == "h2s" ) {
      s_mappingToPvtComponents [ id - START ] = ComponentId::H2S;
      s_mappingToComponentManagerSpecies [ id - START ] = ComponentId::H2S;
      s_mappingFromPvtComponents [ ComponentId::H2S ] = id;
      s_mappingFromComponentManagerSpecies [ ComponentId::H2S ] = id;
   } else if( Genex6::toLower ( speciesName ) == "cokes" ) {
      LcokeS = id;
   } else if( Genex6::toLower ( speciesName ) == "h2o" ) {
      LH2O = id;
   } else if( Genex6::toLower ( speciesName ) == "[so4]" ) {
      LSO4 = id;
   } else if( Genex6::toLower ( speciesName ) == "sulfur" ) {
      LSulfur = id;
   } else if( Genex6::toLower ( speciesName ) == "carsul1" ) {
      LCarsul1 = id;
   } else if( Genex6::toLower ( speciesName ) == "carsul2" ) {
      LCarsul2 = id;
   } else if( Genex6::toLower ( speciesName ) == "[co3]" ) {
      LCO3 = id;
   } else if( Genex6::toLower ( speciesName ) == "orgacid" ) {
      LOrgacid = id;
   }

}

void Genex6::SpeciesManager::setElementNum( const std::string& elementName, int id )
{
   if( elementName == "H" ) IatomH = id;
   else if( elementName == "O" ) IatomO = id;
   else if( elementName == "C" ) IatomC = id;
   else if( elementName == "N" ) IatomN = id;
   else if( elementName == "S" ) IatomS = id;
   else {
      std::cerr << "Error: Elements " << elementName << " is not supported!" << std::endl;
   }

}
bool Genex6::SpeciesManager::isSulphur() const
{
   return !(LcokeS < 0);
}

void Genex6::SpeciesManager::print ( std::ostream& stream ) const {

   int i;

   stream << " ALL species names and IDs: " << endl;

   for ( i = 1; i <= getNumberOfSpecies (); ++i ) {
      stream << i << " = " << s_idNames [ i ] << endl;
   }

   stream << "--------------------------------" << endl;

   stream << " PVT species names and IDs: " << endl;

   for ( i = 1; i <= getNumberOfSpecies (); ++i ) {

      if ( s_mappingToPvtComponents [ i ] != ComponentId::UNKNOWN ) {
         stream << std::setw (  4 ) << i << " = "
              << std::setw ( 20 ) << s_idNames [ i ] << "  " 
              << std::setw ( 20 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesName( int ( s_mappingToPvtComponents [ i ]) ) << "  "
              << std::setw ( 4 ) << s_mappingFromPvtComponents [ int ( s_mappingToPvtComponents [ i ])] << " "
              << std::setw ( 4 ) << int ( s_mappingToPvtComponents [ i ])
              << endl;
      } else {
         stream << std::setw ( 4 ) << i << " = " << std::setw ( 20 ) << s_idNames [ i ] << std::setw ( 20 ) << "  UNKNOWN " << endl;
      }

   }

   stream << "--------------------------------" << endl;


   stream << " Component manager species names and IDs: " << endl;

   for ( i = 1; i <= getNumberOfSpecies (); ++i ) {

      if ( s_mappingToComponentManagerSpecies [ i ] != ComponentId::UNKNOWN ) {
         stream << setw ( 4 ) << i << " = " << setw ( 20 ) << s_idNames [ i ] << "  " 
              << setw ( 20 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesName ( int ( s_mappingToComponentManagerSpecies [ i ])) << "  "
              << setw ( 4 ) << s_mappingFromComponentManagerSpecies [ int ( s_mappingToComponentManagerSpecies [ i ])] << " "
              << endl;
      } else {
         stream << i << " = " << s_idNames [ i ] << "  UNKNOWN " << endl;
      }

   }

}
