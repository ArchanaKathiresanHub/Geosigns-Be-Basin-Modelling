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

   Lkerogen = -1;
   Lpreasphalt = -1;
   Lasphaltenes = -1;
   Lresins = -1;
   Lprecoke = -1;
   Lcoke1 = -1;
   Lcoke2 = -1;
   LC15plusAro = -1;
   LC15plusSat = -1;
   LC6to14Sat = -1;
   LC6to14Aro = -1;
   Lhetero1 = -1;
   LC5 = -1;
   LC4 = -1;
   LC3 = -1;
   LC2 = -1;
   LC1 = -1;
   LC15plusAroS = -1;
   LC15plusSatS = -1;
   LLSC = -1;
   LC15plusAT = -1; 
   LC6to14BT = -1;
   LC6to14DBT = -1;
   LC6to14BP = -1;
   LC6to14AroS = -1;
   LC6to14SatS = -1;
   LcokeS = -1;
   LSO4 = -1;
   LN2 = -1;
   LCOx = -1;
   LCO3 = -1;

   LSulfur  = -1;
   LCarsul1 = -1;
   LCarsul2 = -2;

   for ( i = 0; i < numberOfSpecies + 1; ++i ) {
      s_mappingToPvtComponents [ i ] = pvtFlash::UNKNOWN;
   }

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      s_mappingFromPvtComponents [ i ] = -1;
   }

   for ( i = 0; i < numberOfSpecies + 1; ++i ) {
      s_mappingToComponentManagerSpecies [ i ] = CBMGenerics::ComponentManager::UNKNOWN;
   }

   for ( i = 0; i < CBMGenerics::ComponentManager::NumberOfSpecies; ++i ) {
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
      Lasphaltenes = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::ASPHALTENES;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::asphaltene;
      s_mappingFromPvtComponents [ pvtFlash::ASPHALTENES ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::asphaltene ] = id;
   } else if( Genex6::toLower ( speciesName ) == "resins" ) {
      Lresins = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::RESINS;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::resin;
      s_mappingFromPvtComponents [ pvtFlash::RESINS ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::resin ] = id;
   } else if( Genex6::toLower ( speciesName ) == "precoke" ) {
      Lprecoke = id;
   } else if( Genex6::toLower ( speciesName ) == "coke1" ) {
      Lcoke1 = id;
   } else if( Genex6::toLower ( speciesName ) == "coke2" ) {
      Lcoke2 = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+aro" ) {
      LC15plusAro = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C15_ARO;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C15PlusAro;
      s_mappingFromPvtComponents [ pvtFlash::C15_ARO ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C15PlusAro ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+sat" ) {
      LC15plusSat = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C15_SAT;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C15PlusSat;
      s_mappingFromPvtComponents [ pvtFlash::C15_SAT ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C15PlusSat ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14sat" ) {
      LC6to14Sat = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14SAT;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14Sat;
      s_mappingFromPvtComponents [ pvtFlash::C6_14SAT ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14Sat ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14aro" ) {
      LC6to14Aro = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14ARO;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14Aro;
      s_mappingFromPvtComponents [ pvtFlash::C6_14ARO ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14Aro ] = id;
   } else if( Genex6::toLower ( speciesName ) == "hetero1" ) {
      Lhetero1 = id;
   } else if( Genex6::toLower ( speciesName ) == "c5" ) {
      LC5 = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C5;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C5;
      s_mappingFromPvtComponents [ pvtFlash::C5 ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C5 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c4" ) {
      LC4 = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C4;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C4;
      s_mappingFromPvtComponents [ pvtFlash::C4 ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C4 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c3" ) {
      LC3 = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C3;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C3;
      s_mappingFromPvtComponents [ pvtFlash::C3 ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C3 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c2" ) {
      LC2 = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C2;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C2;
      s_mappingFromPvtComponents [ pvtFlash::C2 ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C2 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c1" ) {
      LC1 = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C1;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C1;
      s_mappingFromPvtComponents [ pvtFlash::C1 ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C1 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "cox" ) {
      LCOx = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::COX;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::COx;
      s_mappingFromPvtComponents [ pvtFlash::COX ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::COx ] = id;
   } else if( Genex6::toLower ( speciesName ) == "n2" ) {
      LN2 = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::N2;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::N2;
      s_mappingFromPvtComponents [ pvtFlash::N2 ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::N2 ] = id;
   } else if( Genex6::toLower ( speciesName ) == "lsc" ) {
      LLSC = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::LSC;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::LSC;
      s_mappingFromPvtComponents [ pvtFlash::LSC ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::LSC ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+aros" ) {
      LC15plusAroS = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C15_AROS;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C15PlusAroS;
      s_mappingFromPvtComponents [ pvtFlash::C15_AROS ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C15PlusAroS ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+sats" ) {
      LC15plusSatS = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C15_SATS;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C15PlusSatS;
      s_mappingFromPvtComponents [ pvtFlash::C15_SATS ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C15PlusSatS ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c15+at" ) {
      LC15plusAT = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C15_AT;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C15PlusAT;
      s_mappingFromPvtComponents [ pvtFlash::C15_AT ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C15PlusAT ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14bt" ) {
      LC6to14BT = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14BT;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14BT;
      s_mappingFromPvtComponents [ pvtFlash::C6_14BT ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14BT ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14dbt" ) {
      LC6to14DBT = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14DBT;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14DBT;
      s_mappingFromPvtComponents [ pvtFlash::C6_14DBT ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14DBT ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14bp" ) {
      LC6to14BP = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14BP;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14BP;
      s_mappingFromPvtComponents [ pvtFlash::C6_14BP ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14BP ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14aros" ) {
      LC6to14AroS = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14AROS;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14AroS;
      s_mappingFromPvtComponents [ pvtFlash::C6_14AROS ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14AroS ] = id;
   } else if( Genex6::toLower ( speciesName ) == "c6-14sats" ) {
      LC6to14SatS = id;
      s_mappingToPvtComponents [ id - START ] = pvtFlash::C6_14SATS;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::C6Minus14SatS;
      s_mappingFromPvtComponents [ pvtFlash::C6_14SATS ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::C6Minus14SatS ] = id;
   }else if( Genex6::toLower ( speciesName ) == "h2s" ) {
      s_mappingToPvtComponents [ id - START ] = pvtFlash::H2S;
      s_mappingToComponentManagerSpecies [ id - START ] = CBMGenerics::ComponentManager::H2S;
      s_mappingFromPvtComponents [ pvtFlash::H2S ] = id;
      s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::H2S ] = id;
   } else if( Genex6::toLower ( speciesName ) == "cokes" ) {
      LcokeS = id;
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

      if ( s_mappingToPvtComponents [ i ] != pvtFlash::UNKNOWN ) {
         stream << std::setw (  4 ) << i << " = "
              << std::setw ( 20 ) << s_idNames [ i ] << "  " 
              << std::setw ( 20 ) << pvtFlash::ComponentIdNames[ int ( s_mappingToPvtComponents [ i ])] << "  " 
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

      if ( s_mappingToComponentManagerSpecies [ i ] != CBMGenerics::ComponentManager::UNKNOWN ) {
         stream << setw ( 4 ) << i << " = " << setw ( 20 ) << s_idNames [ i ] << "  " 
              << setw ( 20 ) << CBMGenerics::ComponentManager::getInstance ().GetSpeciesName ( int ( s_mappingToComponentManagerSpecies [ i ])) << "  " 
              << setw ( 4 ) << s_mappingFromComponentManagerSpecies [ int ( s_mappingToComponentManagerSpecies [ i ])] << " "
              << endl;
      } else {
         stream << i << " = " << s_idNames [ i ] << "  UNKNOWN " << endl;
      }

   }

}
