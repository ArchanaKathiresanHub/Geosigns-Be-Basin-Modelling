// Copyright 2013, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#define NOMINMAX

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <time.h>

#include "EosPack.h"
#include "TrapperIoTableRec.h"
#include "PTDiagramCalculator.h"
#include "ObjectFactory.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MegaPaToPa;
using Utilities::Maths::CelciusToKelvin;
using Utilities::Maths::KelvinToCelcius;

#define PTDIAG_VERSION "3.6"

// Changes history:
//
// Version 3.6
// Added:
//   -pslice <Pressure val> [-pslice <Another pressure val>]
//   -tslice <Temperature val> [-tslice <Another temperature val>]
//   Creates file with 1D horisontal/vertical slices of PT phase diagram for density/viscosity values
//
// Version 3.5
// Updated:
//    algorithm for tracing countour lines. It makes search for starting point as recursive dividing P/T rectangle
//    on 4 equal parts (a la LGR) and using LGR near the bubble/dew line to allow more precise line reconstruction.
//    Intersection point of contour line with bubble/dew line reconstructed now by bisectioning line of last
//    contour line segment expanded till boundaries of P/T grid
//
//    tuning A/B parameter now does not reconstruct bubble/dew lines and contour lines other than 0.5 line, and much faster now
//
//    in code std::pair<double,double> and std::vector< std::pair<double, double> > were replaced with TPPoint and TPLine redefined types
//
// Version 3.4
// Added:
//  -diagonly parameter which removes from plot info about composition
//  -relcoef <value> parameter which could redefine relaxation coeff for Newton iterations in nonlinear solver in EosPack
//
// Version 3.3 
// Added:
// -prop dens,visc parameter which allows to calculate and save to .m file phase densities and viscosities values
// for each point on P/T grid. TotDensity, LiqDensity, VapDensity, LiqViscosity, VapViscosity matrices 
// will be wirtten to .m file. For undefined values 0.0 value is used.


/// Type of diagram: Mass, Mole of Volume
PTDiagramCalculator::DiagramType g_DiagType     = PTDiagramCalculator::MoleMassFractionDiagram;

bool g_DataOnly         = false;
bool g_DiagOnly         = false;
int  g_ColormapType     = 0;     // Draw colormap for liquid fraction in octave (0 - no colormap, 1 - linear colormap, 2 - logarithmic colormap)
bool g_IsBatch          = false;
bool g_LogCountourLines = false;
int  g_CountourLinesNum = 11;

bool g_DumpProperty = false;
bool g_PropList[2] = {false, false}; // density, viscosity

// Create 1D plot for horisontal/vertical line in PT diagram
std::vector<double>  g_valPSlice;
std::vector<double>  g_valTSlice;

std::string g_mFilePrefix;

// Drop spices from composition for octave outpu if their mass fraction less then given threshold
double g_MassThreshold = 0.0; 

/// EosPack parameters
double g_ABTerm   = 2;
bool   g_tuneAB   = false;

std::string g_abFileName; // file name to store found A/B value

double g_StopTol  = 1e-6;
int    g_MaxIters = 400;
double g_Tol      = 1e-4;
double g_NewtonRelCoef = 0.2;
bool   g_exportToPVTsim = false;


static void showUsage( const std::string & msg )
{
   std::cerr << endl;
   if ( msg.size() )
   {
      std::cerr << msg;
   }
   std::cerr << endl;

   std::cerr << "Usage: " << endl
      << "\t-help                      Shows this help message and exit.\n\n"
      << "\t-diag <mass | vol | mole*> Type of diagram, default is mass fraction diagram.\n"
      << "\t-dataonly                  Do not add plot commands to octave .m file.\n"
      << "\t-lines <linesNumber>       Set number of conotour lines to given number.\n"
      << "\t-log                       Contour lines will be in logarithmic scale - like (0.001, 0.01, 0.1). Given lines number value must be odd number\n"
      << "\t-diagonly                  Plot only diagram, do not show any info about composition\n"
      << "\t[-tol <smallValue> ]       Set small value which is used in bisection iterations for finding bubble/dew and coutour lines\n"
      << "\t[-dynamo]                  Create INC file to run with Dynamo\n\n"
      << "\t[-colormap]                Add liquid fraction values grid and colored countour lines to the plot\n"
      << "\t[-logcolormap]             The same as colormap but use logarithmic scale for colormap\n"
      << "\t[-abterm <val>]            Set parmateres A/B term for EosPack. This paramters has influence how PVT library labeling phases in 1 phase region\n" 
      << "\t[-prop <dens | visc>]      Save as matrix for each point P/T grid given property value for each phase\n"
      << "\t[-mfile <mFilePrefix>]     Use given name as file name for Matlab .m file.\n"
      << "                             Any negative value will set algorithm to the default behaviour (A/B doesn't be used)\n"
      << "\t[-tuneab [<filename>] ]    Do search for the value of A/B term in such way that single phases division line will go through the critical point\n"
      << "\t[-massthresh <val>]        Drop component if it mass fraction less then given value in percents\n"
      << "\t[-batch]                   Do not generate pause command in octave file to process bunch of compositions in one go\n"
      << "\t[-stoptol <val>]           Set stop tolerance for nonlinear solver of EosPack to the given value (default is 1e-6)\n"
      << "\t[-iters <val>]             Set max. iterations number for nonlinear solver of EosPack to the given value (default is 400)\n"
      << "\t[-relcoef <val>]           Set relaxation coefficient for Newton nonlinear solver of EosPack to the given value (default is 1.0, must be 0 < RelCoef <= 1.0)\n"
      << "\t[-pvtsim]                  Dump composition into csv file in order of components suitable for importing into PVTsim\n"
      << "\t[-pslice <val MPa>]        Create 1D horisontal (constant P) slice of PT diagram for given pressure value and densit/viscosity/liquid fraction properties\n"
      << "\t[-tslice <val K>]          Create 1D vertical (constant T) slice of PT diagram for given temperature and density/viscosity/liquide fraction  properties\n"
      << ""                            << "\n"
      << "\t-project <projectname>     Name of the project file\n"
      << "\t[-trap <trapname>]         Define trap for which diagram will be build\n"
      << "\t[-age <val>]               Define for which trap age diagram will be build\n"
      << "\t  or:\n"
      << "\t-compos filename           Use composition from given file. Format of this file should be the same as for EosPackDemo\n"
      << "\t  or:\n"
      << "\tno any parameter at all.   In this case diagram will be build to some default composition.\n";
}


void dumpBblDewLineToDynamoInc( TrapperIoTableRec & data, PTDiagramCalculator & diagBuilder );
PTDiagramCalculator * CreateDiagramAndSaveToMFile( TrapperIoTableRec & data, const std::string & cmdStr );
void dumpCompositionToCSVforPVTsim( TrapperIoTableRec & data );


int main( int argc, char ** argv )
{
   string projectFileName;
   string compFileName;
   std::string trapId;
   std::string trapAge;
   std::string cmdString;
   bool genDynamo = false;

   PTDiagramCalculator::DiagramType typeOfDiag = PTDiagramCalculator::MoleMassFractionDiagram;

   // save set of command line parameters for reference
   for ( int i = 1; i < argc; ++i )
   {  
      cmdString += i > 1 ? " " : "";
      cmdString += argv[i];
   }

   // Parsing parameters
   for ( int i = 1; i < argc; ++i )
   {  
      std::string prm( argv[i] );
      std::string val(  i < argc - 1 ? argv[i+1] : "" );
      std::string val2( i < argc - 2 ? argv[i+2] : "" );

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
      else if ( prm == "-age"         ) { trapAge            = std::string( "_" ) + val;    ++i; }
      else if ( prm == "-trap"        ) { trapId             = "_" + val + "_";             ++i; }
      else if ( prm == "-abterm"      ) { g_ABTerm           = atof( val.c_str() ); ++i; }
      else if ( prm == "-mfile"       ) { g_mFilePrefix      = val; ++i; }
      else if ( prm == "-tuneab"      )
      {
         g_tuneAB = true; 
         if ( val.size() && val[0] != '-' )
         {
            g_abFileName = val;
            ++i;
         }
      }
      else if ( prm == "-massthresh"  ) { g_MassThreshold    = atof( val.c_str() ); ++i; }
      else if ( prm == "-stoptol"     ) { g_StopTol          = atof( val.c_str() ); ++i; }
      else if ( prm == "-iters"       ) { g_MaxIters         = atol( val.c_str() ); ++i; }
      else if ( prm == "-tol"         ) { g_Tol              = atof( val.c_str() ); ++i; }
      else if ( prm == "-relcoef"     ) { g_NewtonRelCoef    = atof( val.c_str() ); ++i; }
      else if ( prm == "-pvtsim"      ) { g_exportToPVTsim   = true; }
      else if ( prm == "-pslice"      ) { g_valPSlice.push_back( atof( val.c_str() ) ); ++i; }
      else if ( prm == "-tslice"      ) { g_valTSlice.push_back( atof( val.c_str() ) ); ++i; }
      else if ( prm == "-batch"       ) { g_IsBatch          = true; }
      else if ( prm == "-dynamo"      ) { genDynamo          = true; }
      else if ( prm == "-dataonly"    ) { g_DataOnly         = true; }
      else if ( prm == "-diagonly"    ) { g_DiagOnly         = true; }
      else if ( prm == "-colormap"    ) { g_ColormapType     = 1; }
      else if ( prm == "-logcolormap" ) { g_ColormapType     = 2; }
      else if ( prm == "-lines"       ) { g_CountourLinesNum = atol( val.c_str() ) + 2; ++i; }
      else if ( prm == "-log"         ) { g_LogCountourLines = true; }
      else if ( prm == "-prop"        )
      {
         g_DumpProperty = true; 
         if ( val == "dens" ) { g_PropList[0] = true; ++i; }
         if ( val == "visc" ) { g_PropList[1] = true; ++i; }
      }
      else if ( prm == "-diag" )
      {
         if (      val == "mole" ) { g_DiagType = PTDiagramCalculator::MoleMassFractionDiagram; ++i; }
         else if ( val == "vol"  ) { g_DiagType = PTDiagramCalculator::VolumeFractionDiagram;   ++i; }
         else if ( val == "mass" ) { g_DiagType = PTDiagramCalculator::MassFractionDiagram;     ++i; }
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

   std::vector<TrapperIoTableRec> data;


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

      curRec.setCompositionComp( 3, ComponentId::C1,               7780993477.8834   ); // kg
      curRec.setCompositionComp( 3, ComponentId::C2,               7470802663.62787  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C3,               6369301385.92029  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C4,               6268552951.99726  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C5,               6208020563.40107  ); // kg
      curRec.setCompositionComp( 3, ComponentId::N2,               2659345319.13799  ); // kg
      curRec.setCompositionComp( 3, ComponentId::COX,              0.0               ); // kg
      curRec.setCompositionComp( 3, ComponentId::H2S,              541810267.726579  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14ARO,   70134580791.6047  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14SAT ,  193473629375.084  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C15_PLUS_ARO,     75361672187.7905  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C15_PLUS_SAT,     160933558529.36   ); // kg
      curRec.setCompositionComp( 3, ComponentId::LSC,              8168527698.32282  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C15_PLUS_AT,      26824731512.8973  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C15_PLUS_ARO_S,   0.0               ); // kg
      curRec.setCompositionComp( 3, ComponentId::C15_PLUS_SAT_S,   0.0               ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14BT,    4826027990.49316  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14DBT,   56599766.62811    ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14BP,    21763290.0290721  ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14SAT_S, 0.0               ); // kg
      curRec.setCompositionComp( 3, ComponentId::C6_MINUS_14ARO_S, 0.0               ); // kg
      curRec.setCompositionComp( 3, ComponentId::RESIN,            376228852155.787  ); // kg
      curRec.setCompositionComp( 3, ComponentId::ASPHALTENE,       46393676972.0415  ); // kg

      data.push_back( curRec );
   }
   else if ( !projectFileName.empty() )
   {
	  DataAccess::Interface::ObjectFactory* factory = new DataAccess::Interface::ObjectFactory;
	  DataAccess::Interface::ProjectHandle * ph = DataAccess::Interface::OpenCauldronProject( projectFileName, "r", factory );
      if ( !ph )
      {
         std::cerr << "Failing to load project: " << projectFileName << "\n";
         return -1;
      }
      std::cout << "Project loaded successfully ...\n";

      database::Table * trapperIoTbl = ph->getTable( "TrapperIoTbl" );
      if ( !trapperIoTbl )
      {
         std::cerr << "Can't load TrapperIoTable from project \n";
      }

      TrapperIoTableRec   curRec( trapperIoTbl );

      for ( database::Table::iterator tit = trapperIoTbl->begin(); tit != trapperIoTbl->end(); ++tit )
      {
         curRec.loadRec( tit );
         std::string recName = curRec.name();

         if ( !trapAge.empty() && !std::equal( trapAge.rbegin(), trapAge.rend(), recName.rbegin() ) ) continue;
         if ( !trapId.empty()  && recName.find( trapId  ) == std::string::npos                      ) continue;

         if ( curRec.totMass() > 0.0 )
         {
            data.push_back( curRec );
         }
      }

      DataAccess::Interface::CloseCauldronProject( ph );
	  delete factory;
   }
   else if ( !compFileName.empty() )
   {
      CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
      const int NUM_COMP     = ComponentId::NUMBER_OF_SPECIES_TO_FLASH;
      const int NUM_COMP_TOT = ComponentId::NUMBER_OF_SPECIES;

      double pressure;             //[Pa]               
      double temperature;          //[K]=[Celsius+273.15]
      double compMasses[NUM_COMP_TOT];
      
      std::ifstream ifs( compFileName.c_str(), ios_base::in );

      ifs >> pressure;    if ( !ifs.good() ) { std::cerr << "Can't read pressure from "    << compFileName << "\n"; return -1; }
      ifs >> temperature; if ( !ifs.good() ) { std::cerr << "Can't read temperature from " << compFileName << "\n"; return -1; }

      for ( int i = NUM_COMP_TOT - 1; i >= 0; --i )
      {
         ifs >> compMasses[i];
         if ( !ifs.good() )
         { 
            std::cerr << "Can't read component fraction from " << compFileName << " for " << theComponentManager.getSpeciesName( i ) << "\n"; 
            return -1;
         }
      }
      ifs.close();

      TrapperIoTableRec  curRec;
      
      // Fill composition:
      curRec.setAge( 0 );
      curRec.setTrapID( 0 );
      curRec.setReservoirName( compFileName.c_str() );
      curRec.setPressure( pressure * PaToMegaPa ); 
      curRec.setTemperature( temperature + KelvinToCelcius ); // stock tank temperature in Celsius 

      for ( int i = 0; i < NUM_COMP_TOT; ++i )
      {
         curRec.setCompositionComp( 0, i, compMasses[i] );
      }
      data.push_back( curRec );
   }

   for ( size_t it = 0; it < data.size(); ++it )
   {
      if ( g_exportToPVTsim )
      {
         dumpCompositionToCSVforPVTsim( data[it] );
      }
      // can't build diagram for empty composition
      if ( data[it].totMass() < 1.e-5 ) 
      {
         std::cout << "Empty composition ignorred, total mass: " << data[it].totMass() << "\n";
      }
      clock_t cStart = clock();
      std::unique_ptr<PTDiagramCalculator> diagBuilder( CreateDiagramAndSaveToMFile( data[it], cmdString ) );
      clock_t cEnd = clock();

      if ( diagBuilder.get() )
      {
         if ( genDynamo )
         {
            dumpBblDewLineToDynamoInc( data[it], *(diagBuilder.get()) );
         }
         std::cout << "Diagram creation time: " << (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC) << "\n";
         std::cout << "   Number of points along P axis - " << diagBuilder->getSizeGridP() << "\n";
         std::cout << "   Number of points along T axis - " << diagBuilder->getSizeGridT() << "\n";
         std::cout << "   Number of PVT library calls for bubble/dew points search - " << diagBuilder->getBubbleDewSearchIterationsNumber() << "\n";
         std::cout << "   Number of PVT library calls for building isolines - "        << diagBuilder->getContourLinesSearchIterationsNumber() << "\n";
         std::cout << "   Number of PVT library calls for tunning A/B - "              << diagBuilder->getTuneABIterationsNumber() << "\n";

      }
   }
   return 0;
}

// Set of auxillary functions to print in m file data from PTDiagramCalculator
static void dumpPTGrids( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder );
static void dumpCompositionInfo( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, std::vector<double> masses );
static void dumpLiquidFractionArray( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, TrapperIoTableRec & data );
static void dumpPropertiesListArrays( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, const std::vector<double> & masses );
static void dumpPropertySlice( std::unique_ptr<PTDiagramCalculator> & diagBuilder, const std::vector<double> & composition );
static void createListValuesForIsolinesCalculation( std::ofstream & ofs, std::vector<double> & vals, std::vector<size_t> & colors );
static void generateLiquidVaporSeparationLine( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder );
static void dumpSpecialPoints( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, TrapperIoTableRec & data );
static void generatePlotDescription( std::ofstream & ofs, const std::vector<double> & vals, const std::string& diagTypeStr );
static void plotSpecialPoints( std::ofstream & ofs );
static void plotPieChartForComposition( std::ofstream & ofs );
static void plotCompositionAsTable( std::ofstream & ofs );

PTDiagramCalculator * CreateDiagramAndSaveToMFile( TrapperIoTableRec & data, const std::string & cmdStr )
{
   std::string str = data.toString();
   std::cout << str << "\n";

   double massTotal = data.totMass();

   const int iNc = ComponentId::NUMBER_OF_SPECIES;

   std::vector<double> masses( iNc, 0.0 );
   
   for ( int comp = 0; comp < iNc; ++comp )
   {
      double cMass = data.compMass( comp );
      masses[comp] = (cMass / massTotal * 100) < g_MassThreshold ? 0.0 : cMass;
   }

   // Mesure time for finding bubble/dew line and 0.5 isoline
   clock_t cStart = clock();

   std::unique_ptr<PTDiagramCalculator> diagBuilder( new PTDiagramCalculator( g_DiagType, masses ) );
   
   if ( g_ABTerm > 0 ) // negative value means ignore AoverB term
   {
      diagBuilder->setAoverBTerm( g_ABTerm );
   }
   diagBuilder->setNonLinSolverConvPrms( g_StopTol, g_MaxIters, g_NewtonRelCoef );
   diagBuilder->setTolValue( g_Tol );
   
   if ( !g_tuneAB ) diagBuilder->findBubbleDewLines( data.temperature() + CelciusToKelvin, data.pressure() * MegaPaToPa, std::vector<double>() );
   
   clock_t cEnd = clock();
   double bubleDewSearchTime = (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC);

   // If needed to tune AB do it here
   double tunnedAB = g_ABTerm;
   double tuneABTime = 0.0;

   if ( g_tuneAB )
   {
      cStart = clock();
      tunnedAB = diagBuilder->searchAoverBTerm();
      cEnd = clock();
            
      tuneABTime = (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC);
      if ( g_abFileName.size() )
      {
         std::ofstream ofs( g_abFileName.c_str(), ios_base::out | ios_base::trunc );
         ofs << g_ABTerm;
      }
   }

   std::string plotName = data.name() + "_PTDiag";

   std::string diagTypeStr;
   switch( g_DiagType )
   {
      case PTDiagramCalculator::MoleMassFractionDiagram:   diagTypeStr = "mole";   break;
      case PTDiagramCalculator::VolumeFractionDiagram:     diagTypeStr = "volume"; break;
      case PTDiagramCalculator::MassFractionDiagram:       diagTypeStr = "mass";   break;
      default:                                             assert( 0 );            break;
   }
   plotName += "_" + diagTypeStr;

   if ( !g_mFilePrefix.empty() ) plotName = g_mFilePrefix;

   std::ofstream ofs( (plotName + ".m").c_str(), ios_base::out | ios_base::trunc );

   ofs << "%Pressure-Temperature phase diagram for hydrocarbons composition genertated by PTDiag utility\n";
   ofs << "%Units are: P [MPa], T [K], masss [kg] or fraction\n";
   ofs << "eosptdiagVer = 'v" << PTDIAG_VERSION << "';\n";
   ofs << "cmdLine = '" << cmdStr << "';\n\n";

   // dump info about P/T grids
   if ( !g_tuneAB ) dumpPTGrids( ofs, diagBuilder );

   // dump data for composition
   dumpCompositionInfo( ofs, diagBuilder, masses );

   if ( g_valPSlice.size() || g_valTSlice.size() ) dumpPropertySlice( diagBuilder, masses );

   if ( !g_DataOnly && !g_tuneAB ) ofs << "grid on\n";
   if ( !g_DataOnly && !g_tuneAB ) ofs << "hold off\n\n";
   if ( !g_DataOnly && ! g_DiagOnly && !g_tuneAB ) ofs << "subplot( 3, 4, [ 1 2 3 5 6 7 9 10 11] );\n";

   // dump liquid fraction data for each P/T grid point and plot countour lines for this array
   if ( g_ColormapType ) dumpLiquidFractionArray( ofs, diagBuilder, data );

   // dump density and/or viscosity if needed
   if ( g_DumpProperty ) dumpPropertiesListArrays( ofs, diagBuilder, masses );

   // Create list of values to calculate isolines into PTDiagramCalculator
   std::vector<double> vals;
   std::vector<size_t>    colors;
   
   // generate set of values for calculating isolines in PTDiagramCalculator and colors 
   if ( !g_tuneAB ) createListValuesForIsolinesCalculation( ofs, vals, colors );

   // draw Liquid/Vapor single phases separation line
   if ( !g_tuneAB ) generateLiquidVaporSeparationLine( ofs, diagBuilder );

   cStart = clock();
   // Calculate and plot diagram isolines
   for ( int i = 0; !g_tuneAB && i < vals.size(); ++i )
   {
      const PTDiagramCalculator::TPLine & isoline = diagBuilder->calcContourLine( vals[i] );
      if ( !isoline.size() ) continue;

      ofs << "%Contourline for liquid fraction value: " << vals[i] << "\n";
      ofs << "data_" << i << " = [\n" << "  ";

      for ( size_t j = 0; j < isoline.size(); ++j )
      {
         ofs << isoline[j].first << " ";
      }
      ofs << "\n" << "  ";

      for ( size_t j = 0; j < isoline.size(); ++j )
      {
         ofs << isoline[j].second * PaToMegaPa << " ";
      }
      ofs << "\n" << "];\n\n";
      
      if ( !g_DataOnly )
      {
         ofs << (i == 0 ? "hold on" : "" )<< "\n";
   
         ofs << "ph(" << i+1 << ") = plot( data_" << i << "(1,:), data_" << i << "(2,:), ";
      
         if ( vals.size() <= 11 )
         {
            ofs << "'" << colors[i] << "', ";
         }
         else
         {
            ofs << "'color', colors( " << (vals[i] <= 0.5 ? i+1 : vals.size() - i) << ",:), ";
         }
         ofs << "'linewidth', 2 );\n\n";
      }
   }
   cEnd = clock();
   double isolineSearchTime = (cEnd - cStart)/static_cast<double>(CLOCKS_PER_SEC);

   // dump CriticalPoint, Trap/Surface condition, Criconderm Therm/Bar points data
   dumpSpecialPoints( ofs, diagBuilder, data );

   ofs << "%Diagram calculation statistic\n";
   ofs << "BubleDewLineSearchTime = "   << bubleDewSearchTime << ";\n";
   ofs << "IsoLinesSearchTime = "       << isolineSearchTime << ";\n";
   ofs << "ABTuneTime = "               << tuneABTime << ";\n";

   ofs << "TunnedAoverB = "             << tunnedAB <<  ";\n";
   ofs << "UsedAoverB = "               << (g_ABTerm > 0 ? g_ABTerm : 5.87736) <<  ";\n";

   ofs << "BubbleDewLineSearchPVTCalls = " << diagBuilder->getBubbleDewSearchIterationsNumber() <<  ";\n";
   ofs << "TuneAoverBPVTCalls = "          << diagBuilder->getTuneABIterationsNumber() << ";\n";
   ofs << "IsoLineSearchPVTCalls = "       << diagBuilder->getContourLinesSearchIterationsNumber() << ";\n";
   ofs << "\n";

   // if only data requested, stops here
   if ( g_DataOnly || g_tuneAB ) return diagBuilder.release();

   // Generate legend/colorbar, axis labels, plot title
   generatePlotDescription( ofs, vals, diagTypeStr );

   // plot CriticalPoint, Trap/Surface condition, Criconderm Therm/Bar points
   plotSpecialPoints( ofs );

   // plot composition info as pie chart & table
   if ( !g_DiagOnly )
   {
      ofs << "%Show composition info into the plot\n";

      ofs << "%Create a pie chart for nonzero components\n";
      ofs << "subplot( 3, 4, [ 4 ] );\n\n";

      plotPieChartForComposition( ofs );

      ofs << "%Print composition info in text form\n";
      ofs << "subplot( 3, 4, [8 12] );\n";
   
      plotCompositionAsTable( ofs );
   }

   ofs << "print -djpg '-S2400,1800' " << plotName << ".jpg;\n"; 
   if ( !g_IsBatch ) { ofs << "pause;\n"; }

   return diagBuilder.release();
}

void dumpBblDewLineToDynamoInc( TrapperIoTableRec & data, PTDiagramCalculator & diagBuilder )
{
   // dump composition first
   std::string plotName = data.name() + "_PTDiag";
   switch( g_DiagType )
   {
      case PTDiagramCalculator::MoleMassFractionDiagram:   plotName += "_mole";   break;
      case PTDiagramCalculator::VolumeFractionDiagram:     plotName += "_volume"; break;
      case PTDiagramCalculator::MassFractionDiagram:       plotName += "_mass";   break;
      default:                                             assert( 0 );     break;
   }

   const char ** compNames = CBMGenerics::ComponentManager::getInstance().getSpeciesNameInputList();

   std::ofstream ofs( (plotName + ".INC").c_str(), ios_base::out | ios_base::trunc );
   ofs << "   ! create input table for PTFLASH\n";
   ofs << "   TABLE ( MASSFRAC H2O, \\\n";
   for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES_TO_FLASH-1; ++i )
   {
      ofs << "   MASSFRAC " << compNames[i] << ", \\\n";
   }
   ofs << "   MASSFRAC " << compNames[ComponentId::NUMBER_OF_SPECIES_TO_FLASH - 1] << " ) FlashIn;\n";

   double lumpedMasses[ComponentId::NUMBER_OF_SPECIES_TO_FLASH];
   double masses[ComponentId::NUMBER_OF_SPECIES];
   double fractions[ComponentId::NUMBER_OF_SPECIES];

   for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) 
   {
      masses[i] = data.compMass( i );
   }
   
   pvtFlash::EosPack::getInstance().lumpComponents( masses, lumpedMasses, fractions );
   ofs << "\n" << "   FlashIn H2O [1] = 0.0\n";

   for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES_TO_FLASH; ++i )
   {
      ofs << "   FlashIn " << compNames[i] << "[1] = " << lumpedMasses[i] / data.totMass() << "\n";
   }

   for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES_TO_FLASH; ++i )
   {
      ofs << "   HFEED[" << i + 1 << "] = " << lumpedMasses[i] / data.totMass() << "\n";
   }


   ofs << "\n";
   ofs << "  ! create output table. See ApplMoReSManual for the other outputs to PTFLASH.\n";
   ofs << "  TABLE (PRESSURE P, TEMP T, MASS MASS_GAS, MASS MASS_OIL, VOLUME VOL_GAS, \\\n";
   ofs << "      VOLUME VOL_OIL, DENSITY DENS_GAS, DENSITY DENS_OIL, \\\n";
   ofs << "      VISCOSITY VISC_GAS, VISCOSITY VISC_OIL, REAL VMF) FlashOut;\n";
   ofs << "\n";

   const PTDiagramCalculator::TPLine & isoline = diagBuilder.calcContourLine( 1.0 );
   for ( size_t j = 0; j < isoline.size(); ++j )
   {
      ofs << "   PTFLASH ( pvtmodel, " << isoline[j].second << "*PA, " << isoline[j].first << "*K, FlashIn, FlashOut );\n";
   }
   
   ofs << "\n";
   for ( size_t j = 0; j < isoline.size(); ++j )
   {
      if ( isoline[j].first < 273.15  || isoline[j].first > 773.15 ) continue;

      ofs << "   pvtmodel TEMPERATURE = CELSIUS(" << isoline[j].first-273.15 << ")\n";
      ofs << "   BUBBLEDEW ( pvtmodel, HFEED, SatPres, HFEEDO, HFEEDG )\n";
      ofs << "   PRINT( \"" << isoline[j].first << " \", PRESSURE, SatPres, \"\\n\" )\n";
   }
   ofs << "\n";
   
   const PTDiagramCalculator::TPLine & isoline2 = diagBuilder.calcContourLine( 0.0 );
   for ( size_t j = 0; j < isoline2.size(); ++j )
   {
      ofs << "   PTFLASH ( pvtmodel, " << isoline2[j].second << "*PA, " << isoline2[j].first << "*K, FlashIn, FlashOut );\n";
   }

   ofs << "\n";
   for ( size_t j = 0; j < isoline2.size(); ++j )
   {
      if ( isoline2[j].first < 273.15  || isoline2[j].first > 773.15 ) continue;

      ofs << "   pvtmodel TEMPERATURE = CELSIUS(" << isoline2[j].first-273.15 << ")\n";
      ofs << "   BUBBLEDEW ( pvtmodel, HFEED, SatPres, HFEEDO, HFEEDG )\n";
      ofs << "   PRINT( \"" << isoline2[j].first << " \", PRESSURE, SatPres, \"\\n\" )\n";
   }
   ofs << "\n";

}


void dumpCompositionToCSVforPVTsim( TrapperIoTableRec & data )
{
   std::ofstream ofs( (data.name()+".csv").c_str(), ios_base::trunc );

   ofs << data.compMass( ComponentId::C1               ) << ",";
   ofs << data.compMass( ComponentId::C2               ) << ",";
   ofs << data.compMass( ComponentId::C3               ) << ",";
   ofs << data.compMass( ComponentId::C4               ) << ",";
   ofs << data.compMass( ComponentId::C5               ) << ",";
   ofs << data.compMass( ComponentId::N2               ) << ",";
   ofs << data.compMass( ComponentId::COX              ) << ",";
   ofs << data.compMass( ComponentId::H2S              ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14ARO   ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14SAT   ) << ",";
   ofs << data.compMass( ComponentId::C15_PLUS_ARO     ) << ",";
   ofs << data.compMass( ComponentId::C15_PLUS_SAT     ) << ",";
   ofs << data.compMass( ComponentId::LSC              ) << ",";
   ofs << data.compMass( ComponentId::C15_PLUS_AT      ) << ",";
   ofs << data.compMass( ComponentId::C15_PLUS_ARO_S   ) << ",";
   ofs << data.compMass( ComponentId::C15_PLUS_SAT_S   ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14BT    ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14DBT   ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14BP    ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14SAT_S ) << ",";
   ofs << data.compMass( ComponentId::C6_MINUS_14ARO_S ) << ",";
   ofs << data.compMass( ComponentId::RESIN            ) << ",";
   ofs << data.compMass( ComponentId::ASPHALTENE       ) << "\n";
}

// Set of auxillary functions to print in m file data from PTDiagramCalculator
//
// Save grids along Pressure/Temperature axiss for PT diagram which were used to reconstruct diagram
static void dumpPTGrids( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder )
{
   const std::vector<double> & gridT = diagBuilder->getGridT();
   const std::vector<double> & gridP = diagBuilder->getGridP();

   ofs << "%Grid along T axis (X)\n";
   ofs << "gridT = [ ";
   for ( size_t i = 0; i < gridT.size(); ++i )
   {
      ofs << gridT[i] << " ";
   }
   ofs << "];\n\n";

   ofs << "%Grid along P axis (Y)\n";
   ofs << "gridP = [ ";
   for ( size_t i = 0; i < gridP.size(); ++i )
   {
      ofs << gridP[i] * PaToMegaPa << " ";
   }
   ofs << "];\n\n";
}

// Dump info about composition
static void dumpCompositionInfo( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, std::vector<double> masses )
{
   ofs << "%Hydrocarbons composition masses\n";
   ofs << "Composition = [\n";
   int iNc = ComponentId::NUMBER_OF_SPECIES;

   double mss[ComponentId::NUMBER_OF_SPECIES];
   double massTotal = 0.0;
   for ( int i = 0; i < iNc; ++i )
   {
      mss[i] = masses[i];
      massTotal += masses[i];
   }

   double gorm = pvtFlash::EosPack::getInstance().gorm( mss );

   for ( int i = 0; i < iNc; ++i )
   {
      ofs << "  " << masses[i] << "\n";
      // also calulate mole fraction
      masses[i] /= massTotal * pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm ); 
   }
   ofs << "];\n\n";

   massTotal = std::accumulate( masses.begin(), masses.end(), 0.0 ); 
   for ( size_t i = 0; i < iNc; ++i )
   {
      masses[i] /= massTotal;
   }
   ofs << "%Composition mole fraction\n";
   ofs << "CompMoleFrac = [\n";
   for ( size_t i = 0; i < iNc; ++i )
   {
      ofs << "  " << masses[i] << "\n";
   }
   ofs << "];\n\n";

   ofs << "%Hydrocarbons components names\n";
   ofs << "CompNames = [\n";

   for ( int i = 0; i < iNc; ++i )
   {
      ofs << "'" << std::setw( 11 ) << std::left << CBMGenerics::ComponentManager::getInstance().getSpeciesName( i ) << "'\n";
   }
   ofs << "];\n\n";
}


// Save Liquid fraction 2D array for each point on P/T grid and plot isolines
static void dumpLiquidFractionArray( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, TrapperIoTableRec & data )
{
   const std::vector<double> & gridT = diagBuilder->getGridT();
   const std::vector<double> & gridP = diagBuilder->getGridP();

   ofs << "LiqFraction = [\n";
   for ( size_t i = 0; i < gridP.size(); ++i )
   {
      for ( size_t j = 0; j < gridT.size(); ++j )
      {
         double val = diagBuilder->getLiquidFraction( static_cast<int>(i), static_cast<int>(j) );
         if ( val == 0.0 && g_ColormapType > 1 ) // can't calculate log10( 0 )
         {
            val = 1e-30;
         }
         ofs << val << " ";
      }
      ofs << "\n";
   }
   ofs << "];\n\n";

   if ( !g_DataOnly )
   {
      double maxP = ceil(  std::max( gridP.back()  * PaToMegaPa, data.pressure() ) );
      double minP = floor( std::min( gridP.front() * PaToMegaPa, data.pressure() ) );

      ofs << "for i = size( LiqFraction, 2 )-3:-1:1\n";
      ofs << "   if ( length( find( LiqFraction(:,i) != LiqFraction(1,i))) > 0 )\n";
      ofs << "      axis( [ gridT(1) gridT(i+3) " << minP << " " << maxP << " ] );\n";
      ofs << "      break;\n";
      ofs << "   end\n";
      ofs << "end\n\n";

      ofs << "colormap( cool(64) );\n";
      if ( g_ColormapType == 1 )
      {
         ofs << "contourf( gridT, gridP, LiqFraction, [0:0.1:1] );\n";
         ofs << "colorbar( 'SouthOutside' );\n";
      }
      else
      {
         ofs << "contourf( gridT, gridP, log10( LiqFraction ), [-8, -7, -6, -5, -4, -3, -2, -1, 0] );\n";
         ofs << "colorbar( 'SouthOutside' );\n";
         ofs << "cbh = findobj( gcf(), 'tag', 'colorbar' );\n";
         ofs << "set( cbh, 'xtick', [-8, -7, -6, -5, -4, -3, -2, -1, 0], 'xticklabel', {'1e-8', '1e-7', '1e-6', '1e-5', '1e-4', '1e-3', '1e-2', '1e-1', '1'} );\n";
      }
   }
}

static void dumpPropertiesListArrays( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, const std::vector<double> & composition )
{
   const std::vector<double> & gridT = diagBuilder->getGridT();
   const std::vector<double> & gridP = diagBuilder->getGridP();

   std::vector<double> viscLiq( gridT.size() * gridP.size(), 0.0 );
   std::vector<double> viscVap( gridT.size() * gridP.size(), 0.0 );

   std::vector<double> densTot( gridT.size() * gridP.size(), 0.0 );
   std::vector<double> densVap( gridT.size() * gridP.size(), 0.0 );
   std::vector<double> densLiq( gridT.size() * gridP.size(), 0.0 );


   // change the default behaviour for labeling phases for high temperature span
   if ( g_ABTerm > 0 ) { pvtFlash::EosPack::getInstance().setCritAoverBterm( g_ABTerm ); }

   // increase precision for nonlinear solver
   pvtFlash::EosPack::getInstance().setNonLinearSolverConvParameters( g_MaxIters, g_StopTol, g_NewtonRelCoef ); 

   // Do flashing once more for each point of the grid to collect density/viscosity values
   for ( size_t i = 0; i < gridP.size(); ++i )
   {
      for ( size_t j = 0; j < gridT.size(); ++j )
      {
         const int iNc     = ComponentId::NUMBER_OF_SPECIES;
         const int iNp     = PhaseId::NUMBER_OF_PHASES;
         const int iLiquid = PhaseId::LIQUID;
         const int iVapour = PhaseId::VAPOUR;
   
         // arrays for passing to flasher
         double masses[iNc];
         double phaseMasses[iNp][iNc];
         double phaseDens[iNp]   = {0.0, 0.0};
         double phaseVisc[iNp] = {0.0, 0.0};

         assert( composition.size() == iNc );
         for ( int ic = 0; ic < iNc; ++ic )
         {
            masses[ic] = composition[ic];
            for( int ip = 0; ip < iNp; ++ip )
            {
               phaseMasses[ip][ic] = 0.0;
            }
         }

         // Call flasher to get compositions for phases
         bool res = pvtFlash::EosPack::getInstance().computeWithLumping( gridT[j], gridP[i], masses, phaseMasses, phaseDens, phaseVisc );

         if ( res )
         {
            double massFrac[2] = {0.0, 0.0};
            for ( int ic = 0; ic < iNc; ++ic )
            {
               massFrac[0] += phaseMasses[0][ic];
               massFrac[1] += phaseMasses[1][ic];
            }
            double vol[2];
            vol[0] = massFrac[0] / phaseDens[0];
            vol[1] = massFrac[1] / phaseDens[1];

            densTot[i * gridT.size() + j] = (massFrac[0] + massFrac[1]) / (vol[0] + vol[1]);
            densVap[i * gridT.size() + j] = phaseDens[0] == 1000 ? 0.0 : phaseDens[0];
            densLiq[i * gridT.size() + j] = phaseDens[1] == 1000 ? 0.0 : phaseDens[1];
            viscVap[i * gridT.size() + j] = phaseVisc[0] == 1000 ? 0.0 : phaseVisc[0];
            viscLiq[i * gridT.size() + j] = phaseVisc[1] == 1000 ? 0.0 : phaseVisc[1];
        }
      }
   }
   // revert back flasher settings
   if ( g_ABTerm > 0 ) { pvtFlash::EosPack::getInstance().resetToDefaultCritAoverBterm(); }
   pvtFlash::EosPack::getInstance().setNonLinearSolverConvParameters(); // reset to default

   // Dump density values for phases
   if ( g_PropList[0] )
   {
      ofs << "\nTotDensity = [\n";
      for ( size_t i = 0; i < gridP.size(); ++i )
      {
         for ( size_t j = 0; j < gridT.size(); ++j )
         {
            ofs << densTot[i * gridT.size() + j] << " ";
         }
         ofs << "\n";
      }

       ofs << "];\n\nVapDensity = [\n";
      for ( size_t i = 0; i < gridP.size(); ++i )
      {
         for ( size_t j = 0; j < gridT.size(); ++j )
         {
            ofs << densVap[i * gridT.size() + j] << " ";
         }
         ofs << "\n";
      }

      ofs << "];\n\nLiqDensity = [\n";
      for ( size_t i = 0; i < gridP.size(); ++i )
      {
         for ( size_t j = 0; j < gridT.size(); ++j )
         {
            ofs << densLiq[i * gridT.size() + j] << " ";
         }
         ofs << "\n";
      }
      ofs << "];\n\n";
   }

   // Dump viscosity values for phases
   if ( g_PropList[1] )
   {
      ofs << "\nVapViscosity = [\n";
      for ( size_t i = 0; i < gridP.size(); ++i )
      {
         for ( size_t j = 0; j < gridT.size(); ++j )
         {
            ofs << viscVap[i * gridT.size() + j] << " ";
         }
         ofs << "\n";
      }
   
      ofs << "];\n\nLiqViscosity = [\n";
      for ( size_t i = 0; i < gridP.size(); ++i )
      {
         for ( size_t j = 0; j < gridT.size(); ++j )
         {
            ofs << viscLiq[i * gridT.size() + j] << " ";
         }
         ofs << "\n";
      }
      ofs << "];\n\n";
   }
}

static void dumpPropertySlice( std::unique_ptr<PTDiagramCalculator> & diagBuilder, const std::vector<double> & composition )
{
   const std::vector<double> & gridT = diagBuilder->getGridT();
   const std::vector<double> & gridP = diagBuilder->getGridP();

   // Allocate arrays to keep flasher results alon slice
   std::vector<double> viscLiq( std::max( gridT.size(), gridP.size() ), 0.0 );
   std::vector<double> viscVap( std::max( gridT.size(), gridP.size() ), 0.0 );

   std::vector<double> densTot( std::max( gridT.size(), gridP.size() ), 0.0 );
   std::vector<double> densVap( std::max( gridT.size(), gridP.size() ), 0.0 );
   std::vector<double> densLiq( std::max( gridT.size(), gridP.size() ), 0.0 );
   std::vector<double> liqFrac( std::max( gridT.size(), gridP.size() ), 0.0 );

   const int iNc     = ComponentId::NUMBER_OF_SPECIES;
   const int iNp     = PhaseId::NUMBER_OF_PHASES;
   const int iLiquid = PhaseId::LIQUID;
   const int iVapour = PhaseId::VAPOUR;

   assert( composition.size() == iNc );

   // change the default behaviour for labeling phases for high temperature span
   if ( g_ABTerm > 0 ) { pvtFlash::EosPack::getInstance().setCritAoverBterm( g_ABTerm ); }

   // increase precision for nonlinear solver
   pvtFlash::EosPack::getInstance().setNonLinearSolverConvParameters( g_MaxIters, g_StopTol, g_NewtonRelCoef ); 

   std::ofstream ofs;
   std::ostringstream oss;
   if ( !g_mFilePrefix.empty() ) oss << g_mFilePrefix << "_"; 
   oss << "PTSlicesSet.m";

   // run over given values for P slices and generate 1D plots
   for ( std::vector<double>::const_iterator pit = g_valPSlice.begin(); pit != g_valPSlice.end(); ++pit )
   {
      if ( !ofs.is_open() ) ofs.open( oss.str().c_str(), ios_base::out | ios_base::trunc );

      for ( size_t j = 0; j < gridT.size(); ++j )
      {
         // arrays for passing to flasher
         double masses[iNc];
         double phaseMasses[iNp][iNc];
         double phaseDens[iNp] = {1.0e-15, 1.0e-15};
         double phaseVisc[iNp] = {0.0, 0.0};

         for ( int ic = 0; ic < iNc; ++ic )
         {
            masses[ic] = composition[ic];
            for( int ip = 0; ip < iNp; ++ip )
            {
               phaseMasses[ip][ic] = 0.0;
            }
         }
         // Call flasher to get compositions for phases
         pvtFlash::EosPack::getInstance().computeWithLumping( gridT[j], (*pit) * 1e6, masses, phaseMasses, phaseDens, phaseVisc );
         
         double massLiquid = std::accumulate( phaseMasses[iLiquid], phaseMasses[iLiquid] + iNc, 1.0e-15 );
         double massVapour = std::accumulate( phaseMasses[iVapour], phaseMasses[iVapour] + iNc, 1.0e-15 );

         double volLiquid = massLiquid / phaseDens[iLiquid];
         double volVapour = massVapour / phaseDens[iVapour];

         densTot[j] = (massVapour + massLiquid) / (volVapour + volLiquid);
         densVap[j] = phaseDens[iVapour] == 1000.0 ? 0.0 : phaseDens[iVapour];
         densLiq[j] = phaseDens[iLiquid] == 1000.0 ? 0.0 : phaseDens[iLiquid];
         viscVap[j] = phaseVisc[iVapour] == 1000.0 ? 0.0 : phaseVisc[iVapour];
         viscLiq[j] = phaseVisc[iLiquid] == 1000.0 ? 0.0 : phaseVisc[iLiquid];
         liqFrac[j] = massLiquid / ( massLiquid + massVapour );
      }

      ofs << "\n%1D slice for P = " <<  *pit << " [MPa]\n";
      ofs << "P_" << pit - g_valPSlice.begin() << "_val = " << (*pit) << ";\n\n";

      ofs << "%Grid along T\n";
      ofs << "sliceP_T_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << gridT[j] << " "; ofs << "];\n\n";

      ofs << "\n%Vapour density\n";
      ofs << "sliceP_vapDens_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << densVap[j] << " "; ofs << "];\n\n";

      ofs << "\n%Liquid density\n";
      ofs << "sliceP_liqDens_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << densLiq[j] << " "; ofs << "];\n\n";

      ofs << "\n%Total density\n";
      ofs << "sliceP_totDens_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << densTot[j] << " "; ofs << "];\n\n";

      ofs << "\n%Vapour viscosity\n";
      ofs << "sliceP_vapVisc_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << viscVap[j] << " "; ofs << "];\n\n";

      ofs << "\n%Liquid viscosity\n";
      ofs << "sliceP_liqVisc_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << viscLiq[j] << " "; ofs << "];\n\n";

      ofs << "\n%Liquid fraction\n";
      ofs << "sliceP_liqFrac_" << pit - g_valPSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridT.size(); ++j ) ofs << liqFrac[j] << " "; ofs << "];\n\n";
   }
   
   // Do flashing once more for each point of the T grid to collect density/viscosity values
   for ( std::vector<double>::const_iterator pit = g_valTSlice.begin(); pit != g_valTSlice.end(); ++pit )
   {
      if ( !ofs.is_open() ) ofs.open( oss.str().c_str(), ios_base::out | ios_base::trunc );

      for ( size_t j = 0; j < gridP.size(); ++j )
      {
         // arrays for passing to flasher
         double masses[iNc];
         double phaseMasses[iNp][iNc];
         double phaseDens[iNp] = {1.0e-15, 1.0e-15};
         double phaseVisc[iNp] = {0.0, 0.0};

         for ( int ic = 0; ic < iNc; ++ic )
         {
            masses[ic] = composition[ic];
            for( int ip = 0; ip < iNp; ++ip )
            {
               phaseMasses[ip][ic] = 0.0;
            }
         }
         // Call flasher to get compositions for phases
         pvtFlash::EosPack::getInstance().computeWithLumping( (*pit), gridP[j], masses, phaseMasses, phaseDens, phaseVisc );
         double massLiquid = std::accumulate( phaseMasses[iLiquid], phaseMasses[iLiquid] + iNc, 1.0e-15 );
         double massVapour = std::accumulate( phaseMasses[iVapour], phaseMasses[iVapour] + iNc, 1.0e-15 );

         double volLiquid = massLiquid / phaseDens[iLiquid];
         double volVapour = massVapour / phaseDens[iVapour];

         densTot[j] = (massVapour + massLiquid) / (volVapour + volLiquid);
         densVap[j] = phaseDens[iVapour] == 1000.0 ? 0.0 : phaseDens[iVapour];
         densLiq[j] = phaseDens[iLiquid] == 1000.0 ? 0.0 : phaseDens[iLiquid];
         viscVap[j] = phaseVisc[iVapour] == 1000.0 ? 0.0 : phaseVisc[iVapour];
         viscLiq[j] = phaseVisc[iLiquid] == 1000.0 ? 0.0 : phaseVisc[iLiquid];
         liqFrac[j] = massLiquid / ( massLiquid + massVapour );
      }

      ofs << "\n%1D slice for T = " <<  *pit << " [K]\n";
      ofs << "T_" << pit - g_valTSlice.begin() << "_val = " << (*pit) << ";\n\n";

      ofs << "%Grid along P\n";
      ofs << "sliceT_P_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << gridP[j] << " "; ofs << "];\n\n";

      ofs << "\n%Vapour density\n";
      ofs << "sliceT_vapDens_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << densVap[j] << " "; ofs << "];\n\n";

      ofs << "\n%Liquid density\n";
      ofs << "sliceT_liqDens_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << densLiq[j] << " "; ofs << "];\n\n";

      ofs << "\n%Total density\n";
      ofs << "sliceT_totDens_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << densTot[j] << " "; ofs << "];\n\n";

      ofs << "\n%Vapour viscosity\n";
      ofs << "sliceT_vapVisc_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << viscVap[j] << " "; ofs << "];\n\n";

      ofs << "\n%Liquid viscosity\n";
      ofs << "sliceT_liqVisc_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << viscLiq[j] << " "; ofs << "];\n\n";

      ofs << "\n%Liquid fraction\n";
      ofs << "sliceT_liqFrac_" << pit - g_valTSlice.begin() << " = [ ";
      for ( size_t j = 0; j < gridP.size(); ++j ) ofs << liqFrac[j] << " "; ofs << "];\n\n";
   }

   // revert back flasher settings
   if ( g_ABTerm > 0 ) { pvtFlash::EosPack::getInstance().resetToDefaultCritAoverBterm(); }
   pvtFlash::EosPack::getInstance().setNonLinearSolverConvParameters(); // reset to default

   if ( ofs.is_open() ) ofs.close();
}

static void createListValuesForIsolinesCalculation( std::ofstream & ofs, std::vector<double> & vals, std::vector<size_t> & colors )
{
   if ( g_LogCountourLines ) // if countour lines should have logarithmic scale build sequence like this [0 0.001 0.01 0.1 0.5 0.9 0.99 0.999 1]
   {
      if ( !(g_CountourLinesNum % 2) )
      {
         g_CountourLinesNum++; 
      }

      vals.push_back( 0.0 );
      for ( int i = 1; i < g_CountourLinesNum / 2; ++i )
      {
         vals.push_back( std::pow( 10.0, -(g_CountourLinesNum/2 - i) ) );
      }

      vals.push_back( 0.5 );

      for ( int i = g_CountourLinesNum / 2; i < g_CountourLinesNum - 2; ++i )
      {
         vals.push_back( 1.0 - std::pow( 10.0, -i + (g_CountourLinesNum/2 - 1) ) );
      }
      vals.push_back( 1.0 );

      for ( int i = 0; i < vals.size(); ++i )
         std::cout << vals[i] << " ";
   }
   else // just linear spacing like [0 0.2 0.4 0.6 0.8 1]
   {
      for ( int i = 0; i < g_CountourLinesNum; ++i )
      {
         vals.push_back( i * (1.0 / (g_CountourLinesNum - 1) ) );
      }
   }

   // if number of lines rather small use legend and predefined plot colors
   if ( vals.size() <= 11 ) 
   {
      for ( int i = 0; i < vals.size() / 2 + vals.size() % 2; ++i )
      {
         colors.push_back( i );
      }
      for ( size_t i = vals.size() / 2 +vals.size() % 2; i < vals.size(); ++i )
      {
         colors.push_back( vals.size() - 1 - i );
      }
   }
   else // use colormap to define plot colors
   {
      if ( !g_DataOnly ) ofs << "colors = jet( " << vals.size()/2+1 << ");\n";
   }

   ofs << "%Contour lines for\n";
   ofs << "clVals = [ ";
   for ( size_t i = 0; i < vals.size(); ++i )
   {
      ofs << vals[i] << " ";
   }
   ofs << "];\n\n";
}


// Generate Liquid/Fraction separation line
static void generateLiquidVaporSeparationLine( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder )
{
   const PTDiagramCalculator::TPLine & spsline = diagBuilder->getSinglePhaseSeparationLine();
   if ( spsline.size() )
   {
      ofs << "%Vapor/Liquid single phase separation line\n";
      ofs << "spsline = [\n" << "  ";

      for ( size_t j = 0; j < spsline.size(); ++j )
      {
         ofs << spsline[j].first << " ";
      }
      ofs << "\n" << "  ";

      for ( size_t j = 0; j < spsline.size(); ++j )
      {
         ofs << spsline[j].second * PaToMegaPa << " ";
      }
      ofs << "\n" << "];\n\n";
      
      if ( !g_DataOnly )
      {
         ofs << "hold on\n";
         ofs << "plot( spsline(1,:), spsline(2,:), 'k', 'linewidth', 3 )\n\n";
         ofs << "plot( spsline(1,:), spsline(2,:), 'r', 'linewidth', 1 )\n\n";
      }
   }
}


// Dump data for special points on diagram like Trap Condition, Bubble point for trap temperature, 
//  Surface conditions, Critical point, Criconden therm/bar points
static void dumpSpecialPoints( std::ofstream & ofs, std::unique_ptr<PTDiagramCalculator> & diagBuilder, TrapperIoTableRec & data )
{
   // Point with trap condition could call axiss ajustmenst that is why we should draw it first
   ofs << "%Point for trap conditions\n"; 
   ofs << "TrapCond = [" << (data.temperature() + CelciusToKelvin) << ", " << data.pressure() << "];\n";
   ofs << "\n";

   ofs << "%Point for surface conditions\n";
   ofs << "SurfCond = [" << 273.15 + 15 << ", " << 101325.0 * PaToMegaPa << "];\n";
   ofs << "\n";

   ofs << "%Critical point\n";
   ofs << "CritPoint = [" << diagBuilder->getCriticalPoint().first << ", " << 
                             diagBuilder->getCriticalPoint().second * PaToMegaPa << "];\n";
   ofs << "\n";

   ofs << "%Bubble/dew point for temperature in trap\n";
   double T = data.temperature() + CelciusToKelvin;
   double P;
   if ( diagBuilder->getBubblePressure( T, &P ) )
   {
      ofs << "BubblePoint = [" << T << ", " << P * PaToMegaPa << "];\n";
   }
   else 
   {
      ofs << "BubblePoint = [" << T << ", " << 0.0 << "];\n";
   }
   ofs << "\n";

   const PTDiagramCalculator::TPPoint & cct = diagBuilder->getCricondenthermPoint();
   ofs << "%Cricondentherm point\n";
   ofs << "CricondthermPoint = [" << cct.first << ", " << cct.second * PaToMegaPa << "];\n";
   ofs << "\n";

   const PTDiagramCalculator::TPPoint & ccp = diagBuilder->getCricondenbarPoint();
   ofs << "%Cricondenbar point\n";
   ofs << "CricondenbarPoint = [" << ccp.first << ", " << ccp.second * PaToMegaPa << "];\n";
   ofs << "\n";
}


static void generatePlotDescription( std::ofstream & ofs, const std::vector<double> & vals, const std::string& diagTypeStr )
{
   ofs << "%Axiss names\n";
   ofs << "xlabel( '" << "Temperature, K" << "')\n";
   ofs << "ylabel( '" << "Pressure, MPa" << "')\n";
   ofs << "\n";
   ofs << "%Plot legend\n";
   // Generate legend/colorbar
   if ( g_CountourLinesNum <= 11 )
   {  // simplest case - legend and colorbar separated
      if ( g_ColormapType )
      {
         ofs << "colormap( cool( 64 ) );\n";
      
         if ( g_ColormapType == 1 )
         {
            ofs << "contourf( gridT, gridP, LiqFraction, [0:0.1:1] );\n";
            ofs << "colorbar( 'SouthOutside' );\n";
         }
         else
         {
            ofs << "contourf( gridT, gridP, log10( LiqFraction ), [-8, -7, -6, -5, -4, -3, -2, -1, 0] );\n";
            ofs << "colorbar( 'SouthOutside' );\n";
            ofs << "cbh = findobj( gcf(), 'tag', 'colorbar' );\n";
            ofs << "set( cbh, 'xtick', [-8, -7, -6, -5, -4, -3, -2, -1, 0], ";
            ofs <<           "'xticklabel', {'1e-8', '1e-7', '1e-6', '1e-5', '1e-4', '1e-3', '1e-2', '1e-1', '1'} );\n";
         }
      }
      // generate legend
      ofs << "legend( ph, ";
      if ( g_ColormapType ) ofs << "'',"; // for some reason octave shift legend by one line if contourf was called

      for ( size_t j = 0; j < vals.size() / 2 + 1; ++j )
      {
         ofs << "'" << "Vap/Liq " << diagTypeStr << " frac ";
         if ( vals[j] == 0.0 ) ofs << "0.0";
         else                  ofs << vals[j];
         ofs << "'" << ((j < vals.size() / 2) ? ", " : "");
      }
      ofs << ", 'location', 'northeast' );\n";
   }
   else
   {
      if ( g_ColormapType )
      {  // we should join 2 coloromaps in one colorbar         
         ofs << "colormap( [cool(64); [1 1 1; 1 1 1; 1 1 1]; colors ] );\n";
         ofs << "isolNum = size(colors,1);\n";
         ofs << "if isolNum/64 > 1\n";
         ofs << "   ticksNum = 10;\n";
         ofs << "elseif isolNum/64 > 0.5\n";
         ofs << "   ticksNum = 5;\n";
         ofs << "elseif isolNum/64 > 0.2\n";
         ofs << "   ticksNum = 3;\n";
         ofs << "else\n";
         ofs << "   ticksNum = 2;\n";
         ofs << "end\n";
         ofs << "tickStep = isolNum/64/ticksNum;\n";
         ofs << "isoTicks = [1+(3/64) : tickStep : 1+(3+isolNum)/64 ];\n";
         ofs << "caxis( [0 (1 + (3+isolNum)/64.0)] );\n";
         ofs << "colorbar( 'SouthOutside' );\n";
         ofs << "cbh = findobj( gcf(), 'tag', 'colorbar' );\n";
         ofs << "tickStep = 0.5/(length(isoTicks)-1);\n";
         ofs << "isoTicksLabel = [0, tickStep:tickStep:(0.5-tickStep), 0.5];\n";
         ofs << "set(cbh, 'xtick', [0,0.2,0.4,0.6,0.8,1.0,[isoTicks]], ";
         ofs << "'xticklabel', [{'0'; '0.2'; '0.4'; '0.6'; '0.8'; '1.0'}; cellstr(num2str(isoTicksLabel')) ] );\n";
         ofs << "set( cbh, 'xlabel', 'Liquid " << diagTypeStr << " fraction (Left). Vap/Liq " << diagTypeStr << " fraction (Right)' );";
      }
      else
      {
         // if there is no contourf use simple colorbar
         ofs << "colorbar( 'SouthOutside' );\n";
         ofs << "cbh = findobj( gcf(), 'tag', 'colorbar' );\n";
         ofs << "caxis( [0 1] );\n";
         ofs << "set( cbh, 'xlabel', 'Vap/Liq " << diagTypeStr << " fraction', 'xtick', ";
         if ( !g_LogCountourLines )
         {
            ofs << "[ 0 0.2 0.4 0.6 0.8 1 ], 'xticklabel', {'0', '0.1', '0.2', '0.3', '0.4', '0.5'} );\n";
         }
         else
         {
            int minV = static_cast<int>( std::floor( (std::log10( vals[1] ) + 0.5 ) ) );
            ofs<< "[ 0 ";
            for ( int i = minV; i <= 0; ++i ) ofs << (minV-i)/minV << " ";
            ofs << "], 'xticklabel', {";
            for ( int i = minV-1; i < 0; ++i ) ofs << "'1e" << i << "', ";
            ofs << "'0.5'} )\n";
         }
      }
   }

   ofs << "\n";

   ofs << "%Plot title\n";
   ofs << "title( 'Phase envelop PVT library. Cauldron-BPA 2013.1' );\n";
   ofs << "grid on\n";
   ofs << "\n\n";
}


// Plot special points on diagram
static void plotSpecialPoints( std::ofstream & ofs )
{
   // Point with trap condition could call axiss ajustmenst that is why we should draw it first
   ofs << "%Plot trap conditions\n"; 
   ofs << "plot( [ TrapCond(1) ], [ TrapCond(2) ], 'ko', 'linewidth', 2 );\n";

   ofs << "%Check and shift axiss if trap condition point outside of the axis range\n";   
   ofs << "ax = axis;\n";
   ofs << "if ( ax(4) < TrapCond(2) )\n";
   ofs << "   axis( [ ax(1) ax(2) ax(3) TrapCond(2) ] );\n";
   ofs << "end;\n\n";

   ofs << "dT = (ax(2) - ax(1)) / 100;\n";
   ofs << "dP = (ax(4) - ax(3)) / 100;\n";

   ofs << "text( TrapCond(1) + dT, TrapCond(2) - dP, sprintf( 'Tcm = %g K',   TrapCond(1) ), 'fontweight', 'bold' );\n";
   ofs << "text( TrapCond(1) + dT, TrapCond(2) + dP, sprintf( 'Pcm = %g MPa', TrapCond(2) ), 'fontweight', 'bold' );\n";
   ofs << "\n";

   ofs << "%Point for surface conditions\n";
   ofs << "plot( [ SurfCond(1) ], [ SurfCond(2) ], 'ro', 'linewidth', 2 );\n";
   ofs << "text( SurfCond(1) + dT, SurfCond(2) + dP,     sprintf( 'Tsrf = %g K',   SurfCond(1) ), 'fontweight', 'bold' );\n";
   ofs << "text( SurfCond(1) + dT, SurfCond(2) + 3 * dP, sprintf( 'Psrf = %g MPa', SurfCond(2) ), 'fontweight', 'bold' );\n";
   ofs << "\n";

   ofs << "%Critical point\n";
   ofs << "if ( CritPoint(1) > 0 && CritPoint(2) > 0 )\n";
   ofs << "   plot( [ CritPoint(1) ], [ CritPoint(2) ], 'ro', 'linewidth', 2 );\n";
   ofs << "   text( CritPoint(1) + dT, CritPoint(2) - dP, sprintf( 'Tcr = %g K',   CritPoint(1) ), 'fontweight', 'bold' );\n";
   ofs << "   text( CritPoint(1) + dT, CritPoint(2) + dP, sprintf( 'Pcr = %g MPs', CritPoint(2) ), 'fontweight', 'bold' );\n";
   ofs << "end\n";
   ofs << "\n";

   ofs << "%Bubble/dew point for temperature in trap\n";
   ofs << "if( BubblePoint(2) > 0 )\n";
   ofs << "   plot( [ BubblePoint(1) ], [ BubblePoint(2) ], 'go', 'linewidth', 2 );\n";
   ofs << "   text( BubblePoint(1) + dT, BubblePoint(2) - dP, sprintf( 'Tcm = %g K',   BubblePoint(1) ), 'fontweight', 'bold' );\n";
   ofs << "   text( BubblePoint(1) + dT, BubblePoint(2) + dP, sprintf( 'Pbb = %g MPa', BubblePoint(2) ), 'fontweight', 'bold' );\n";
   ofs << "end\n";
   ofs << "\n";

   ofs << "%Cricondentherm point\n";
   ofs << "plot( [ CricondthermPoint(1) ], [ CricondthermPoint(2) ], 'bo', 'linewidth', 2 );\n";
   ofs << "text( CricondthermPoint(1) + dT, CricondthermPoint(2) - dP, sprintf( 'Tct = %g K',   CricondthermPoint(1) ), 'fontweight', 'bold' );\n";
   ofs << "text( CricondthermPoint(1) + dT, CricondthermPoint(2) + dP, sprintf( 'Pct = %g MPa', CricondthermPoint(2) ), 'fontweight', 'bold' );\n";
   ofs << "\n";

   ofs << "%Cricondenbar point\n";
   ofs << "plot( [ CricondenbarPoint(1) ], [ CricondenbarPoint(2) ], 'bo', 'linewidth', 2 );\n";
   ofs << "text( CricondenbarPoint(1) + dT, CricondenbarPoint(2) - dP, sprintf( 'Tcp = %g K',   CricondenbarPoint(1) ), 'fontweight', 'bold' );\n";
   ofs << "text( CricondenbarPoint(1) + dT, CricondenbarPoint(2) + dP, sprintf( 'Pcp = %g MPa', CricondenbarPoint(2) ), 'fontweight', 'bold' );\n";
   ofs << "\n";
}

// Plot compositioin as pie chart on diagram
static void plotPieChartForComposition( std::ofstream & ofs )
{
   ofs << "box off;\n";
   ofs << "axis off;\n\n";

   ofs << "CompositionNorm = " << (g_DiagType == PTDiagramCalculator::MoleMassFractionDiagram ? "CompMoleFrac;" : "Composition ./ sum( Composition );")
       << "\n";
   ofs << "nonZeroInd = find( CompositionNorm > 0 );\n";

   if ( g_ColormapType )
   {
      ofs << "nonZeroInd = flipud( nonZeroInd );\n";
   }

   ofs << "nnzCmp = CompositionNorm( nonZeroInd );\n";
   ofs << "nnzCmpNames = cellstr( CompNames( nonZeroInd, : ) );\n";
   ofs << "expl = zeros( length( nnzCmp ), 1 );\n";
   ofs << "expl( find( nnzCmp < 0.08 ) ) = 1;\n\n";

   ofs << "pie( CompositionNorm( nonZeroInd ), cellstr( CompNames( nonZeroInd, : ) ), expl );\n\n";
   ofs << "axis( [-1.2 1.2 -1.2 1.2] );\n";
   ofs << "title( 'Non zero components " << (g_DiagType == PTDiagramCalculator::MoleMassFractionDiagram ? "mole " : "") <<
          "mass" << " fraction' );\n";
}

// Print info about composition as a table
static void plotCompositionAsTable( std::ofstream & ofs )
{
   ofs << "box off;\n";
   
   ofs << "ax = axis;\n";
   ofs << "dy = (ax(4) - ax(3)) / " << ComponentId::NUMBER_OF_SPECIES << " * 0.8;\n";
   ofs << "axis off;\n\n";

   ofs << "for i = 1:length( Composition )\n";
   ofs << "   str = sprintf( '%s - %1.3e', CompNames(i,:), " << 
      ( PTDiagramCalculator::MoleMassFractionDiagram == g_DiagType ? "CompMoleFrac( i )" : "CompositionNorm( i )" ) << " );\n";
   ofs << "   text( 0.1, ax(4) - dy * (i+1), str );\n";
   ofs << "end\n";
}


