//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CasaCommander.h"
#include "CmdPlotPareto.h"
#include "MatlabExporter.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

CmdPlotPareto::CmdPlotPareto( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "plotPareto command required proxy name as parameter at least" ;
   }
   m_proxyName = m_prms[0];

   // set default values
   m_mFileName    = "ParetoPlot.m";
  
   if ( m_prms.size() > 1 )
   {
      m_mFileName = m_prms[1];
   }

   if ( m_prms.size() > 2 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown parameter of PlotPareto command: " << m_prms[2];
   }
}

void CmdPlotPareto::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   // find proxy first
   // Search for given proxy name in the set of calculated proxies
   const casa::RSProxy * proxy = sa->rsProxySet().rsProxy( m_proxyName.c_str() );
   // call response evaluation
   if ( !proxy ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown proxy name:" << m_proxyName; }

   LogHandler( LogHandler::INFO ) << "Generating script to plot Pareto diagram ...";

   casa::SensitivityCalculator & sCalc = sa->sensitivityCalculator();

   casa::ParetoSensitivityInfo data;
   sCalc.calculatePareto( proxy, data );

   MatlabExporter ofs( m_mFileName );

   ofs << "%CASA Pareto diagram plotting script\n";

    // export cases parameters
   //ofs.exportParametersInfo( *sa.get() );

   // export cases observables
   //ofs.exportObservablesInfo( *sa.get() );

  
   ofs << "clear all\n";
   ofs << "hold off\n";
   ofs << "close\n";

   ofs << "\nScenarioID    = '" << sa->scenarioID() << "';\n";
   
   ofs << "ProxyName = '" << MatlabExporter::correctName( m_proxyName ) << "';\n";

   for ( size_t i = 0; i < data.m_vprmSens.size(); ++i )
   {
      ofs << " PrmParetoSensitivitySorted.prmName{ " << i+1 << "} = '" << 
         MatlabExporter::correctName( data.m_vprmPtr[i]->name()[data.m_vprmSubID[i]] ) << "';\n";
      ofs << " PrmParetoSensitivitySorted.prmSens( " << i+1 << ") = " << data.m_vprmSens[i] << ";\n";
   }

   ofs << "bar( PrmParetoSensitivitySorted.prmSens, 'facecolor', 'r');\n";
   ofs << "axis( [0 length( PrmParetoSensitivitySorted.prmSens)+1 0 110 ] );\n";
   ofs << "hold on\n";
   ofs << "plot( cumsum( PrmParetoSensitivitySorted.prmSens ), '-xb', 'linewidth', 3 );\n";
   ofs << "grid on\n";
   ofs << "legend( 'Sensitivity', 'Cumulative sensitivity' );\n";
   ofs << "\n";
   ofs << "for i = 1:length( PrmParetoSensitivitySorted.prmSens )\n";
   ofs << "   h = text( i, 5, PrmParetoSensitivitySorted.prmName{i}, 'fontweight', 'bold' );\n";
   ofs << "   set( h, 'rotation', 90 );\n";
   ofs << "end\n";
   ofs << "ylabel( 'Sensitivity [%]', 'fontweight', 'bold' );\n";
   ofs << "title ( ['Variable parameters Pareto sensitivity diagram for ' ProxyName], 'fontweight', 'bold' );\n";
   ofs << "xlabel( 'Variable parameter', 'fontweight', 'bold' );\n";
   ofs << "set( get( gcf, 'currentaxes' ), 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "print( ['Pareto_' ProxyName '.jpg'] );\n";

   LogHandler( LogHandler::INFO ) << "Script generation succeeded";
}

void CmdPlotPareto::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <proxyName> <scriptName> \n";
   std::cout << "   - create Matlab/Octave .m script file which could be used to create a plot with Pareto diagram to show the global parameters sensitivity\n";
   std::cout << "     \n";
   std::cout << "     Example:\"" << cmdName << "\" command:\n";
   std::cout << " #                                    Proxy name     Matlab/Octave script name\n";
   std::cout << "         " << cmdName << "\"FirstOrder\"    \"ParetoPlotTornado1stOrdRS.m\"\n";
}

