// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.


#include <cassert>
#include <iostream>
#include <string>
#include <time.h>

#include "EosPack.h"
#include "consts.h"
#include "TrapperIoTableRec.h"
#include "PTDiagCalculator.h"

#define PTDIAG_VERSION "1.0b"

static void showUsage( const std::string & msg )
{
   std::cerr << endl;
   if ( msg.size() )
   {
      std::cerr << msg;
   }
   std::cerr << endl;

   std::cerr << "Usage: " << endl
      << "\t-project projectname       Name of the project file" << std::endl
      << "\t-trap trapname             Define trap for which diagram will be build" << std::endl
      << "\t-age                       Define for which trap age diagram will be build" << std::endl
      << "\t-diag [mass | vol | mole]  Type of diagram" << std::endl
      << "\t-dynamo                    Create INC file to run with Dynamo" << std::endl
      << "\t-compos filename           Use composition from given file. Format of this file should be the same as for EosPackDemo (only no gor factor)" << std::endl
      << "\t[-help]                    Shows this help message and exit." << std::endl << std::endl;
}

void dumpBblDewLineToDynamoInc( TrapperIoTableRec & data, PTDiagramCalculator & diagBuilder, PTDiagramCalculator::DiagramType diagType );
PTDiagramCalculator * CreateDiagramAndSaveToMFile( TrapperIoTableRec & data, PTDiagramCalculator::DiagramType diagType );



int main( int argc, char ** argv )
{
   string projectFileName;
   string compFileName;
   std::string trapId;
   std::string trapAge;
   bool genDynamo = false;

   PTDiagramCalculator::DiagramType typeOfDiag = PTDiagramCalculator::MoleMassFractionDiagram;

   // Parsing parameters
   for ( int i = 1; i < argc-1; ++i )
   {  
      std::string prm( argv[i] );
      std::string val( argv[i+1] );

      if ( prm == "-project" )
      {
         if ( !projectFileName.empty() )
         {
            showUsage( "Multiple project specified" );
            return -1;
         }
         projectFileName = val;
         i++;
      }
      else if ( prm == "-compos" )
      {
         if ( !compFileName.empty() )
         {
            showUsage( "Multiple composition file specified" );
            return -1;
         }
         compFileName = val;
         i++;
      }
      else if ( prm == "-help" )
      {
         showUsage( argv[0] );
         return 0;
      }
      else if ( prm == "-age" )
      {
         trapAge = std::string( "_" ) + val;
         ++i;
      }
      else if ( prm == "-trap" )
      {
         trapId = val + "_";
         ++i;
      }
      else if ( prm == "-dynamo" )
      {
         genDynamo = true;
      }
      else if ( prm == "-diag" )
      {
         if ( val == "mole" )
         {
            typeOfDiag = PTDiagramCalculator::MoleMassFractionDiagram;
            ++i;
         }
         else if ( val == "vol" )
         {
            typeOfDiag = PTDiagramCalculator::VolumeFractionDiagram;
            ++i;
         }
         else if ( val == "mass" )
         {
            typeOfDiag = PTDiagramCalculator::MassFractionDiagram;
            ++i;
         }
         else
         {
            showUsage( "Uknown type of diagram" );
            return -1;
         }
      }
      else
      {
         showUsage( std::string( "Unkown argument was specified: ") + prm );
         return -1;
      }
   }

   if ( projectFileName.empty() && compFileName.empty() )
   {
      if (!trapId.empty() || !trapAge.empty() )
      {
         showUsage( "No project file specified" );
         return -1;
      }

      TrapperIoTableRec  curRec;
      
      // Fill dummy composition:
      curRec.setAge( 0 );
      curRec.setTrapID( 0 );
      curRec.setReservoirName( "Unknown" );
      curRec.setPressure( 101325.0 * 1e-6 * 125 ); 
      curRec.setTemperature( 15 + 300 ); // stock tank temperature in Celsius 

      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C1, 7780993477.8834 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C2, 7470802663.62787 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C3, 6369301385.92029 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C4, 6268552951.99726 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C5, 6208020563.40107  ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::N2, 2659345319.13799 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::COx, 0.0  ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::H2S, 541810267.726579 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14Aro, 70134580791.6047 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14Sat , 193473629375.084 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C15PlusAro, 75361672187.7905 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C15PlusSat, 160933558529.36 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::LSC, 8168527698.32282 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C15PlusAT, 26824731512.8973 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C15PlusAroS, 0.0  ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C15PlusSatS, 0.0 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14BT, 4826027990.49316 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14DBT, 56599766.62811 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14BP, 21763290.0290721 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14SatS, 0.0 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::C6Minus14AroS, 0.0  ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::resin, 376228852155.787 ); // kg
      curRec.setCompositionComp( 3, CBMGenerics::ComponentManager::asphaltene, 46393676972.0415 ); // kg

      clock_t cStart = clock();
      std::auto_ptr<PTDiagramCalculator> diagBuilder( CreateDiagramAndSaveToMFile( curRec, typeOfDiag ) );
      clock_t cEnd = clock();
      if ( diagBuilder.get() )
      {
         if ( genDynamo )
         {
            dumpBblDewLineToDynamoInc( curRec, *(diagBuilder.get()), typeOfDiag );
         }

         std::cout << "Calculation time: " << (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC) << std::endl;
         std::cout << "   Number of points along P axis - " << diagBuilder->getSizeGridP() << std::endl;
         std::cout << "   Number of points along T axis - " << diagBuilder->getSizeGridT() << std::endl;
         std::cout << "   Number of PVT library calls for bubble/dew points search - " << diagBuilder->getBubbleDewSearchIterationsNumber() << std::endl;
         std::cout << "   Number of PVT library calls for building isolines - " << diagBuilder->getContourLinesSearchIterationsNumber() << std::endl;
      }
   }
   else if ( !projectFileName.empty() )
   {
      DataAccess::Interface::ProjectHandle * ph = DataAccess::Interface::OpenCauldronProject( projectFileName, "r" );
      if ( !ph )
      {
         std::cerr << "Failing to load project: " << projectFileName << std::endl;
         return -1;
      }
      std::cout << "Project loaded successfully ..." << std::endl;

      database::Table * trapperIoTbl = ph->getTable( "TrapperIoTbl" );
      if ( !trapperIoTbl )
      {
         std::cerr << "Can't load TrapperIoTable from project " << std::endl;
      }
      std::cout << "Table TrapperIoTable successfully loaded ..." << std::endl;

      TrapperIoTableRec   curRec( trapperIoTbl );

      for ( database::Table::iterator tit = trapperIoTbl->begin(); tit != trapperIoTbl->end(); ++tit )
      {
         curRec.loadRec( tit );
         std::string recName = curRec.name();

         if ( !trapAge.empty() && recName.find( trapAge ) == std::string::npos ) continue;
         if ( !trapId.empty() && recName.find( trapId )== std::string::npos ) continue;

         if ( curRec.totMass() > 0.0 )
         {
            clock_t cStart = clock();
            std::auto_ptr<PTDiagramCalculator> diagBuilder( CreateDiagramAndSaveToMFile( curRec, typeOfDiag ) );
            clock_t cEnd = clock();
            if ( diagBuilder.get() )
            {
               if ( genDynamo )
               {
                  dumpBblDewLineToDynamoInc( curRec, *(diagBuilder.get()), typeOfDiag );
               }

               std::cout << "Calculation time: " << (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC) << std::endl;
               std::cout << "   Number of points along P axis - " << diagBuilder->getSizeGridP() << std::endl;
               std::cout << "   Number of points along T axis - " << diagBuilder->getSizeGridT() << std::endl;
               std::cout << "   Number of PVT library calls for bubble/dew points search - " << diagBuilder->getBubbleDewSearchIterationsNumber() << std::endl;
               std::cout << "   Number of PVT library calls for building isolines - " << diagBuilder->getContourLinesSearchIterationsNumber() << std::endl;
            }
         }
      }

      DataAccess::Interface::CloseCauldronProject( ph );
      std::cout << "Project closed. Exiting ..." << std::endl;
   }
   else if ( !compFileName.empty() )
   {
      CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
      const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
      const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;

      double pressure;             //[Pa]               
      double temperature;          //[K]=[Celsius+273.15]
      double compMasses[NUM_COMP_TOT];
      
      std::ifstream ifs( compFileName.c_str(), ios_base::in );

      ifs >> pressure;    if ( !ifs.good() ) { std::cerr << "Can't read pressure from "    << compFileName << std::endl; return -1; }
      ifs >> temperature; if ( !ifs.good() ) { std::cerr << "Can't read temperature from " << compFileName << std::endl; return -1; }

      int i;
      for ( i = NUM_COMP_TOT - 1; i >= 0; --i )
      {
         ifs >> compMasses[i];
         if ( !ifs.good() )
         { 
            std::cerr << "Can't read component fraction from " << compFileName << " for " << theComponentManager.GetSpeciesName( i ) << std::endl; 
            return -1;
         }
      }
      ifs.close();

      TrapperIoTableRec  curRec;
      
      // Fill composition:
      curRec.setAge( 0 );
      curRec.setTrapID( 0 );
      curRec.setReservoirName( compFileName.c_str() );
      curRec.setPressure( pressure * CBMGenerics::Pa2MPa ); 
      curRec.setTemperature( temperature + CBMGenerics::K2C ); // stock tank temperature in Celsius 

      for ( i = 0; i < NUM_COMP_TOT; ++i )
      {
         curRec.setCompositionComp( 0, i, compMasses[i] );
      }

      clock_t cStart = clock();
      std::auto_ptr<PTDiagramCalculator> diagBuilder( CreateDiagramAndSaveToMFile( curRec, typeOfDiag ) );
      clock_t cEnd = clock();
      if ( diagBuilder.get() )
      {
         if ( genDynamo )
         {
            dumpBblDewLineToDynamoInc( curRec, *(diagBuilder.get()), typeOfDiag );
         }

         std::cout << "Calculation time: " << (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC) << std::endl;
         std::cout << "   Number of points along P axis - " << diagBuilder->getSizeGridP() << std::endl;
         std::cout << "   Number of points along T axis - " << diagBuilder->getSizeGridT() << std::endl;
         std::cout << "   Number of PVT library calls for bubble/dew points search - " << diagBuilder->getBubbleDewSearchIterationsNumber() << std::endl;
         std::cout << "   Number of PVT library calls for building isolines - " << diagBuilder->getContourLinesSearchIterationsNumber() << std::endl;
      }
   }
   return 0;
}


PTDiagramCalculator * CreateDiagramAndSaveToMFile( TrapperIoTableRec & data, PTDiagramCalculator::DiagramType diagType )
{
   std::string str = data.toString();
   std::cout << str << std::endl;

   double massTotal = data.totMass();
   if ( massTotal < 1.0 ) return 0; // can't build diagram for empty composition

   const int iNc = CBMGenerics::ComponentManager::NumberOfSpecies;

   std::vector<double> masses( iNc, 0.0 );

   for ( size_t comp = 0; comp < iNc; ++comp )
   {
      masses[comp] = data.compMass( comp );
   }

   std::auto_ptr<PTDiagramCalculator> diagBuilder( new PTDiagramCalculator( diagType, masses ) );
   diagBuilder->findBubbleDewLines( data.temperature() + CBMGenerics::C2K, data.pressure() * CBMGenerics::MPa2Pa, std::vector<double>() );

   double vals[]   = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
   int    colors[] = { 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };
   
   std::string plotName = data.name() + "_PTDiag";
   switch( diagType )
   {
      case PTDiagramCalculator::MoleMassFractionDiagram:   plotName += "_mole";   break;
      case PTDiagramCalculator::VolumeFractionDiagram:     plotName += "_volume"; break;
      case PTDiagramCalculator::MassFractionDiagram:       plotName += "_mass";   break;
      default:                                             assert( 0 );     break;
   }
   std::ofstream ofs( (plotName + ".m").c_str(), ios_base::out | ios_base::trunc );
 
   const std::vector<double> & gridT = diagBuilder->getGridT();
   const std::vector<double> & gridP = diagBuilder->getGridP();

   ofs << "#Pressure-Temperature phase diagram for hydrocarbons composition genertated by PTDiag utility v." << PTDIAG_VERSION << std::endl;
   ofs << "#Units are: P [MPa], T [K], masss [kg] or fraction" << std::endl;
   ofs << "#Grid along T axis (X)" << std::endl;
   ofs << "gridT = [ ";
   for ( size_t i = 0; i < gridT.size(); ++i )
   {
      ofs << gridT[i] << ( i + 1 == gridT.size() ? "" : ", " );
   }
   ofs << "];" << std::endl << std::endl;

   ofs << "#Grid along P axis (Y)" << std::endl;
   ofs << "gridP = [ ";
   for ( size_t i = 0; i < gridP.size(); ++i )
   {
      ofs << gridP[i] * CBMGenerics::Pa2MPa << ( i + 1 == gridP.size() ? "" : ", " );
   }
   ofs << "];" << std::endl << std::endl;

   ofs << "#Hydrocarbons composition masses" << std::endl;
   ofs << "Composition = [" << std::endl;
   for ( size_t i = 0; i < iNc; ++i )
   {
      ofs << "  " << data.compMass( i ) << std::endl;
   }
   ofs << "];" << std::endl << std::endl;

   ofs << "#Hydrocarbons components names" << std::endl;
   ofs << "CompNames = [" << std::endl;
   for ( size_t i = 0; i < iNc; ++i )
   {
      ofs << "'" << CBMGenerics::ComponentManager::getInstance().GetSpeciesName( i ) << "'" << std::endl;
   }
   ofs << "];" << std::endl << std::endl;

   ofs << "grid on" << std::endl << "hold off" << std::endl << std::endl;

   for ( int i = 0; i < sizeof(vals)/sizeof(double); ++i )
   {
      const std::vector< std::pair<double,double> > & isoline = diagBuilder->calcContourLine( vals[i] );
      if ( !isoline.size() ) continue;

      ofs << "#Contourline for liquid fraction value: " << vals[i] << std::endl;
      ofs << "data_" << i << " = [" << std::endl << "  ";

      for ( size_t j = 0; j < isoline.size(); ++j )
      {
         ofs << isoline[j].first << (j + 1 == isoline.size() ? "" : ", ");
      }
      ofs << std::endl << "  ";

      for ( size_t j = 0; j < isoline.size(); ++j )
      {
         ofs << isoline[j].second * CBMGenerics::Pa2MPa << (j + 1 == isoline.size() ? "" : ", ");
      }
      ofs << std::endl << "];" << std::endl << std::endl;
      
      ofs << (i == 0 ? "hold on" : "" )<< std::endl;
      ofs << "plot( data_" << i << "(1,:), data_" << i << "(2,:), '" << colors[i] ;
/*
      switch( i )
      {
         case 0:                               ofs << "-x"; break;
         //case sizeof(vals)/sizeof(double) - 1: ofs << "-+"; break;
      }
*/
      ofs << "' )" << std::endl << std::endl;
   }

   ofs << "#Axiss names" << std::endl;
   ofs << "xlabel( '" << "Temperature, K" << "')" << std::endl;
   ofs << "ylabel( '" << "Pressure, MPa" << "')" << std::endl;
   ofs << std::endl;
   ofs << "#Plot legend" << std::endl;
   ofs << "legend( ";
   for ( size_t j = 0; j < (sizeof(vals)/sizeof(double) / 2 + 1); ++j )
   {
      ofs << "'" << "Vap/Liq ";
      switch( diagType )
      {
         case PTDiagramCalculator::MoleMassFractionDiagram:   ofs << "mole";   break;
         case PTDiagramCalculator::VolumeFractionDiagram:     ofs << "volume"; break;
         case PTDiagramCalculator::MassFractionDiagram:       ofs << "mass";   break;
         default:                                             assert( 0 );     break;
      }
      ofs << " frac ";
      if ( vals[j] == 0.0 ) ofs << "0.0";
      else                  ofs << vals[j];
      ofs << "00'" << ((j < sizeof(vals)/sizeof(double) / 2) ? ", " : "");
   }
   ofs << ", 'location', 'northeast' );" << std::endl;
   ofs << std::endl;

   ofs << "#Plot title" << std::endl;
   ofs << "title( 'Phase envelop PVT library. Cauldron-BPA 2013.1' );" << std::endl;
   ofs << std::endl;

   double dP = (gridP[1] - gridP[0]) * CBMGenerics::Pa2MPa * 1.5;
   double dT = (gridT[1] - gridT[0]);

   ofs << "#Point for surface conditions" << std::endl;
   double P = 101325.0 * CBMGenerics::Pa2MPa; // stock tank pressure
   double T = 273.15 + 15;                    // stock tank temp
   ofs << "plot( [" << T << "], [" << P << "], 'ro' );" << std::endl;
   ofs << "text( " << T + dT << ", " << P + dP << ", 'Tsrf = " << T << " K'  );" << std::endl;
   ofs << "text( " << T + dT << ", " << P + 3 * dP << ", 'Psrf = " << P << " MPa');" << std::endl;
   ofs << std::endl;

   ofs << "#Point for trap conditions" << std::endl;
   P = data.pressure();
   T = data.temperature() + CBMGenerics::C2K;
   ofs << "plot( [" << T << "], [" << P << "], 'ko' );" << std::endl;
   ofs << "text( " << T + dT << ", " << P - dP << ", 'Tcm = " << T << " K');" << std::endl;
   ofs << "text( " << T + dT << ", " << P + dP << ", 'Pcm = " << P << " MPa');" << std::endl;
   ofs << std::endl;

   P = diagBuilder->getCriticalP() * CBMGenerics::Pa2MPa;
   T = diagBuilder->getCriticalT();
   if ( P > 0.0 && T > 0.0 )
   {
      ofs << "#Critical point" << std::endl;
      ofs << "plot( [" << T << "], [" << P << "], 'ro' );" << std::endl;
      ofs << "text( " << T + dT << ", " << P - dP << ", 'Tcr = " << T << " K'  );" << std::endl;
      ofs << "text( " << T + dT << ", " << P + dP << ", 'Pcr = " << P << " MPa');" << std::endl;
      ofs << std::endl;
   }

   T = data.temperature() + CBMGenerics::C2K;
   if ( diagBuilder->getBubblePressure( T, P ) )
   {
      ofs << "#Bubble/dew point for temperature in trap" << std::endl;
      P *= CBMGenerics::Pa2MPa;
      ofs << "plot( [" << T << "], [" << P << "], 'go' );" << std::endl;
      ofs << "text( " << T + dT << ", " << P - dP << ", 'Tcm = " << T << " K'  );" << std::endl;
      ofs << "text( " << T + dT << ", " << P + dP << ", 'Pbb = " << P << " MPa');" << std::endl;
      ofs << std::endl;
   }

   ofs << "#Show composition info into the plot" << std::endl;
   ofs << "ax = axis;" << std::endl;
   ofs << "dP = (ax(4) - ax(3)) * 0.5 / " << iNc << " * 0.9;" << std::endl;
   ofs << "dT = (ax(2) - ax(1)) / " << gridT.size() << ";" << std::endl;
   ofs << "for i = 1 : length( Composition )" << std::endl;
   ofs << "   str = sprintf( '%s - %g', CompNames(i,:), Composition(i)/sum( Composition(:) ));" << std::endl;
   ofs << "text( dT*3, ax(4) - dP - dP * i, str );" << std::endl;
   ofs << "end;" << std::endl;

   ofs << "pause;" << std::endl; 
   //ofs << "print('" << plotName << ".jpg" << "');" << std::endl; 

   return diagBuilder.release();
}




void dumpBblDewLineToDynamoInc( TrapperIoTableRec & data, PTDiagramCalculator & diagBuilder, PTDiagramCalculator::DiagramType diagType )
{
   // dump composition first
   std::string plotName = data.name() + "_PTDiag";
   switch( diagType )
   {
      case PTDiagramCalculator::MoleMassFractionDiagram:   plotName += "_mole";   break;
      case PTDiagramCalculator::VolumeFractionDiagram:     plotName += "_volume"; break;
      case PTDiagramCalculator::MassFractionDiagram:       plotName += "_mass";   break;
      default:                                             assert( 0 );     break;
   }
   const char  * compNames[] = { "asphaltenes", 
                                 "resins",
                                 "C15_Aro",
                                 "C15_Sat", 
                                 "C6_14Aro",
                                 "C6_14Sat",
                                 "C5",
                                 "C4",
                                 "C3",
                                 "C2",
                                 "C1",
                                 "COx",
                                 "N2", 
                                 "H2S" };

   std::ofstream ofs( (plotName + ".INC").c_str(), ios_base::out | ios_base::trunc );
   ofs << "   ! create input table for PTFLASH" << std::endl;
   ofs << "   TABLE ( MASSFRAC H2O, \\" << std::endl;
   for ( int i = 0; i < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash-1; ++i )
   {
      ofs << "   MASSFRAC " << compNames[i] << ", \\" << std::endl;
   }
   ofs << "   MASSFRAC " << compNames[CBMGenerics::ComponentManager::NumberOfSpeciesToFlash - 1] << " ) FlashIn;" << std::endl;

   double lumpedMasses[CBMGenerics::ComponentManager::NumberOfSpeciesToFlash];
   double masses[CBMGenerics::ComponentManager::NumberOfSpecies];
   double fractions[CBMGenerics::ComponentManager::NumberOfSpecies];

   for ( int i = 0; i < CBMGenerics::ComponentManager::NumberOfSpecies; ++i ) 
   {
      masses[i] = data.compMass( i );
   }
   
   pvtFlash::EosPack::getInstance().lumpComponents( masses, lumpedMasses, fractions );
   ofs << std::endl << "   FlashIn H2O [1] = 0.0" << std::endl;

   for ( int i = 0; i < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++i )
   {
      ofs << "   FlashIn " << compNames[i] << "[1] = " << lumpedMasses[i] / data.totMass() << std::endl;
   }

   for ( int i = 0; i < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++i )
   {
      ofs << "   HFEED[" << i + 1 << "] = " << lumpedMasses[i] / data.totMass() << std::endl;
   }


   ofs << std::endl;
   ofs << "  ! create output table. See ApplMoReSManual for the other outputs to PTFLASH." << std::endl;
   ofs << "  TABLE (PRESSURE P, TEMP T, MASS MASS_GAS, MASS MASS_OIL, VOLUME VOL_GAS, \\" << std::endl;
   ofs << "      VOLUME VOL_OIL, DENSITY DENS_GAS, DENSITY DENS_OIL, \\" << std::endl;
   ofs << "      VISCOSITY VISC_GAS, VISCOSITY VISC_OIL, REAL VMF) FlashOut;" << std::endl;
   ofs << std::endl;

   const std::vector< std::pair<double,double> > & isoline = diagBuilder.calcContourLine( 1.0 );
   for ( size_t j = 0; j < isoline.size(); ++j )
   {
      ofs << "   PTFLASH ( pvtmodel, " << isoline[j].second << "*PA, " << isoline[j].first << "*K, FlashIn, FlashOut );" << std::endl;
   }
   
   ofs << std::endl;
   for ( size_t j = 0; j < isoline.size(); ++j )
   {
      if ( isoline[j].first < 273.15  || isoline[j].first > 773.15 ) continue;

      ofs << "   pvtmodel TEMPERATURE = CELSIUS(" << isoline[j].first-273.15 << ")" << std::endl;
      ofs << "   BUBBLEDEW ( pvtmodel, HFEED, SatPres, HFEEDO, HFEEDG )" << std::endl;
      ofs << "   PRINT( \"" << isoline[j].first << " \", PRESSURE, SatPres, \"\\n\" )" << std::endl;
   }
   ofs << std::endl;
   
   const std::vector< std::pair<double,double> > & isoline2 = diagBuilder.calcContourLine( 0.0 );
   for ( size_t j = 0; j < isoline2.size(); ++j )
   {
      ofs << "   PTFLASH ( pvtmodel, " << isoline2[j].second << "*PA, " << isoline2[j].first << "*K, FlashIn, FlashOut );" << std::endl;
   }

   ofs << std::endl;
   for ( size_t j = 0; j < isoline2.size(); ++j )
   {
      if ( isoline2[j].first < 273.15  || isoline2[j].first > 773.15 ) continue;

      ofs << "   pvtmodel TEMPERATURE = CELSIUS(" << isoline2[j].first-273.15 << ")" << std::endl;
      ofs << "   BUBBLEDEW ( pvtmodel, HFEED, SatPres, HFEEDO, HFEEDG )" << std::endl;
      ofs << "   PRINT( \"" << isoline2[j].first << " \", PRESSURE, SatPres, \"\\n\" )" << std::endl;
   }
   ofs << std::endl;

}


