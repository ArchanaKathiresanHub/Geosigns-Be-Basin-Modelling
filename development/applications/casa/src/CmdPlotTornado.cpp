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
#include "CmdPlotTornado.h"
#include "MatlabExporter.h"
#include "CfgFileParser.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

CmdPlotTornado::CmdPlotTornado( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "plotTornado command required DoEs list as parameter at least" ;
   }
   
   // convert list of DoEs like: "Tornado,BoxBenken" into array of DoE names
   m_doeNames = CfgFileParser::list2array( m_prms[0], ',' );

   // set default values
   m_mFileName    = "TornadoPlot.m";
  
   if ( m_prms.size() > 1 )
   {
      m_mFileName = m_prms[1];
   }

   if ( m_prms.size() > 2 )
   {
      const std::vector<std::string> & givenTargetNames = CfgFileParser::list2array( m_prms[2], ',' );
      m_targetNames.insert( givenTargetNames.begin(), givenTargetNames.end() );
   }

   if ( m_prms.size() > 3 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown parameter of PlotTornado command: " << m_prms[2];
   }
}

void CmdPlotTornado::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   // check DoEs name first
   for ( size_t i = 0; i < m_doeNames.size(); ++i )
   {
      sa->doeCaseSet().filterByExperimentName( m_doeNames[i] );
      if ( !sa->doeCaseSet().size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Non existing DoE for plotTornado command: " << m_doeNames[i];
      }
   }
   sa->doeCaseSet().filterByExperimentName( "" );

   LogHandler( LogHandler::INFO ) << "Generating script to plot Tornado diagram...";

   casa::SensitivityCalculator & sCalc = sa->sensitivityCalculator();

   std::vector< casa::TornadoSensitivityInfo > data = sCalc.calculateTornado( sa->doeCaseSet(), m_doeNames );
   if ( data.empty() ) { throw ErrorHandler::Exception( sCalc.errorCode() ) << sCalc.errorMessage(); }

   MatlabExporter ofs( m_mFileName );

   ofs << "%CASA Tornado diagram plotting script\n";

   ofs << "clear all\n";
   ofs << "hold off\n";
   ofs << "close\n";

   ofs << "\nScenarioID    = '" << sa->scenarioID() << "';\n";

   //ofs << "ProxyName = '" << m_proxyName << "';\n";

   for ( size_t i = 0, obsNum = 0; i < data.size(); ++i )
   {
      // get name of the observable
      const casa::Observable * obsObj = data[i].observable();
      if ( !obsObj ) continue;
      const std::vector<std::string> & obsNamesList = obsObj->name();
      const std::string & obsName = obsNamesList[data[i].observableSubID()];

      bool found = false;
      for ( std::set<std::string>::const_iterator it = m_targetNames.begin(); !found && it != m_targetNames.end(); ++it )
      {
         if ( obsName.find( *it ) != std::string::npos ) found = true; 
      }
      if ( !m_targetNames.empty() && !found ) continue;

      ++obsNum;

      ofs << "TornadoSens.obsName{ " << obsNum << "} = '" << MatlabExporter::correctName( obsName ) << "';\n";
      ofs << "TornadoSens.obsRefVal( " << obsNum << ") = " << data[i].refObsValue() << ";\n";

      const std::vector<std::pair<const casa::VarParameter *, int> > & varPrmList = data[i].varPrmList();
      const casa::TornadoSensitivityInfo::SensitivityData            & sens       = data[i].sensitivities();
      const casa::TornadoSensitivityInfo::SensitivityData            & relSens    = data[i].relSensitivities();
      
      for ( size_t j = 0; j < varPrmList.size(); ++j )
      {
         const std::vector<std::string> & prmNames = varPrmList[j].first->name();
         const std::string & prmName = prmNames[varPrmList[j].second];

         ofs << "TornadoSens.prmNames{ " << obsNum << ", " << j+1 << "} = '" << prmName << "';\n";

         double minVal; 
         double maxVal;
         double minRelVal;
         double maxRelVal;
         
         if ( sens[j].size() == 1 ) // categorical
         {
            minVal = data[i].refObsValue();
            maxVal = minVal + sens[j][0];
            minRelVal = 101; // sens will not be shown if pp > 100
            maxRelVal = relSens[j][0];
         }
         else
         {
            minVal = data[i].refObsValue() + sens[j][0];
            maxVal = data[i].refObsValue() + sens[j][1];
            minRelVal = relSens[j][0];
            maxRelVal = relSens[j][1];
         }
         ofs << "TornadoSens.absSensMin( " << obsNum << ", " << j+1 << ") = " << minVal << ";\n";
         ofs << "TornadoSens.absSensMax( " << obsNum << ", " << j+1 << ") = " << maxVal << ";\n";
         ofs << "TornadoSens.relSensMin( " << obsNum << ", " << j+1 << ") = " << minRelVal << ";\n";
         ofs << "TornadoSens.relSensMax( " << obsNum << ", " << j+1 << ") = " << maxRelVal << ";\n";
      }
   }

   ofs << "for i = 1:length( TornadoSens.obsRefVal )\n";
   ofs << "\n";
   ofs << "   display( ['Creating tornado diagram for observable: ' TornadoSens.obsName{i}] );\n";
   ofs << "\n";
   ofs << "   bv = TornadoSens.obsRefVal(i);\n";
   ofs << "   minAbs = TornadoSens.absSensMin(i,:);\n";
   ofs << "   maxAbs = TornadoSens.absSensMax(i,:);\n";
   ofs << "   \n";
   ofs << "   minRel = TornadoSens.relSensMin(i,:);\n";
   ofs << "   maxRel = TornadoSens.relSensMax(i,:);\n";
   ofs << "   \n";
   ofs << "   % remove very small numbers\n";
   ofs << "   jj = find( abs(minRel) < 1.e-4);\n";
   ofs << "   minRel(jj) = 0;\n";
   ofs << "   jj = find( abs(maxRel) < 1.e-4);\n";
   ofs << "   maxRel(jj) = 0;\n";
   ofs << "\n";
   ofs << "   close;\n";
   ofs << "   hold off\n";
   ofs << "   [v sind] = sort( max( abs(maxAbs - bv), abs(bv - minAbs) ) );\n";
   ofs << "   \n";
   ofs << "   bgmn = barh( minAbs(sind), 'facecolor', 'r' );\n";
   ofs << "   set( bgmn(1), 'basevalue', bv );\n";
   ofs << "   hold on\n";
   ofs << "   bgmx = barh( maxAbs(sind), 'facecolor', 'b' );\n";
   ofs << "   set( bgmx(1), 'basevalue', bv);\n";
   ofs << "   \n";
   ofs << "   av = axis();\n";
   ofs << "   if ( bv - av(1) < 2 * v(end) )\n";
   ofs << "       av(1) = bv - 2.5 * v(end);\n";
   ofs << "   end\n";
   ofs << "   \n";
   ofs << "   if ( av(2) - bv  < 1.5 * v(end) )\n";
   ofs << "       av(2) = bv + 1.5 * v(end);\n";
   ofs << "   end\n";
   ofs << "   dx = (av(2)-av(1))/40;\n";
   ofs << "   axis( av );\n";
   ofs << "  \n";
   ofs << "   for j = 1:length(minRel)\n";
   ofs << "      \n";
   ofs << "      if ( maxRel(sind(j)) > 0 )\n";
   ofs << "         h = text( maxAbs(sind(j))+dx, j, sprintf( '%4.2g%%', maxRel(sind(j)) ), 'color', 'b', 'fontweight', 'bold'  );\n";
   ofs << "      else\n";
   ofs << "         h = text( maxAbs(sind(j))-dx, j, sprintf( '%4.2g%%', maxRel(sind(j)) ), 'color', 'b', 'fontweight', 'bold'  );\n";
   ofs << "         set( h, 'HorizontalAlignment', 'right' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      if ( minRel(sind(j)) > 0 )\n";
   ofs << "         h = text( minAbs(sind(j))+dx, j, sprintf( '%4.2g%%', minRel(sind(j)) ), 'color', 'r', 'fontweight', 'bold'  );\n";
   ofs << "      else\n";
   ofs << "         h = text( minAbs(sind(j))-dx, j, sprintf( '%4.2g%%', minRel(sind(j)) ), 'color', 'r', 'fontweight', 'bold'  );\n";
   ofs << "         set( h, 'HorizontalAlignment', 'right' );\n";
   ofs << "      end\n";
   ofs << "      ylbl{j+1} = TornadoSens.prmNames(i, sind(j));\n";
   ofs << "\n";
   ofs << "      text( bv - v(end)*2.4, j, TornadoSens.prmNames(i, sind(j)), 'fontweight', 'bold' );\n";
   ofs << "   end\n";
   ofs << "   set(gca, 'yticklabel', [] );\n";
   ofs << "\n";
   ofs << "   xlabel( TornadoSens.obsName{i}, 'fontweight', 'bold' );\n";
   ofs << "   ylabel( 'Variable parameter name', 'fontweight', 'bold' );\n";
   ofs << "   title( ['Variable parameters sensitivity tornado plot for ' TornadoSens.obsName{i}], 'fontweight', 'bold'  );\n";
   ofs << "   \n";
   ofs << "   set( get( gcf, 'currentaxes' ), 'fontweight', 'bold' );\n";
   ofs << "   grid on\n";
   ofs << "   \n";
   ofs << "   print( sprintf( 'tornado_%d.jpg', i ) );\n";
   ofs << "   eval( sprintf( 'print Tornado_%s.jpg -S1000,1000', TornadoSens.obsName{i} ) );\n";

   ofs << "end\n";

   LogHandler( LogHandler::INFO ) << "Script generation succeeded";
}

void CmdPlotTornado::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <DoEsList> <scriptName> [targetsList]\n";
   std::cout << "   - create Matlab/Octave .m script file which could be used to create a plot with Tornado\n";
   std::cout << "     diagram per observable to show the local parameters sensitivity\n";
   std::cout << "     Where:\n";
   std::cout << "       DoEsList - list of DoE which will be used for tornado diagram construction. Based on this list,\n";
   std::cout << "                  the 1st order RS with Kriging will be created and used for parameters sensitivity calculation\n";
   std::cout << "       scriptName - name of the output matlab/octave file which will be generated. User should run this script to\n";
   std::cout << "                    create Tornado plots pictures\n";
   std::cout << "       targetsList - optional comma separated targets name list for which Tornado diagrams will be generated\n";
   std::cout << "\n";
   std::cout << "     Example:\"" << cmdName << "\" command:\n";
   std::cout << " #                                DoEs list     Matlab/Octave script name\n";
   std::cout << "         " << cmdName << "\"FullFactorial,Tornado\"    \"TornadoPlotTornadoFFDoEs.m\"   \"Target1,Target2\" \n";
}


