//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// CASADemo
#include "CasaCommander.h"
#include "CmdEvaluateResponse.h"
#include "CmdPlotRSProxyQC.h"
#include "CfgFileParser.h"
#include "MatlabExporter.h"

// CASA
#include "casaAPI.h"
#include "RunCase.h"

// Stadard C lib
#include <cstdlib>
#include <cmath>

// STL
#include <iostream>
#include <memory>

CmdPlotRSProxyQC::CmdPlotRSProxyQC( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_proxyName            = m_prms.size() > 0 ? m_prms[0] : "";
   std::string proxyCases = m_prms.size() > 1 ? m_prms[1] : "";
   std::string testCases  = m_prms.size() > 2 ? m_prms[2] : "";
   m_mFileName            = m_prms.size() > 3 ? m_prms[3] : "rsProxyQCPlot.m";

   if ( m_proxyName.empty()  ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No rs proxy name was given to PlotRSProxyQC command"; }
   if ( proxyCases.empty()   ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Empty cases list for proxy plot QC command"; }
 
   // convert list of DoEs or data files like: "Tornado,BoxBenken" into array of names
   m_caseList = CfgFileParser::list2array( proxyCases, ',' );
   if ( m_caseList.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No any DoE or data file name was given for making proxy QC plot";

   if ( !testCases.empty() ) m_testCaseList = CfgFileParser::list2array( testCases, ',' );
}

void CmdPlotRSProxyQC::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Generating script to plot QC for " << m_proxyName << "rs proxy...\n";
   }

   std::vector<casa::RunCase *> proxyCaseSet;
   std::vector<casa::RunCase *> testCaseSet;
   std::vector<size_t>          proxyCaseSetSize;
   std::vector<size_t>          testCaseSetSize;

   // generate run cases sets
   CmdEvaluateResponse::createRunCasesSet( sa, proxyCaseSet, m_caseList, proxyCaseSetSize );
   if ( !m_testCaseList.empty() ) CmdEvaluateResponse::createRunCasesSet( sa, testCaseSet,  m_testCaseList, testCaseSetSize );

    // Search for given proxy name in the set of calculated proxies
   casa::RSProxy * proxy = sa->rsProxySet().rsProxy( m_proxyName.c_str() );
  
   if ( !proxy ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown proxy name:" << m_proxyName; }

   // call response evaluation for both cases set
   for ( size_t i = 0; i < proxyCaseSet.size(); ++i )
   {
      if ( ErrorHandler::NoError != proxy->evaluateRSProxy( *proxyCaseSet[i] ) )
      {
         throw ErrorHandler::Exception( proxy->errorCode() ) << proxy->errorMessage();
      }
   }

   for ( size_t i = 0; i < testCaseSet.size(); ++i )
   {
      if ( ErrorHandler::NoError != proxy->evaluateRSProxy( *testCaseSet[i] ) )
      {
         throw ErrorHandler::Exception( proxy->errorCode() ) << proxy->errorMessage();
      }
   }

   MatlabExporter ofs( m_mFileName );

   ofs << "%CASA " << m_proxyName << " rs proxy QC plots\n";
   ofs << "clear all\n\n";

   ofs << "plotMarkerSize = 6;\n\n";

   ofs << "colors = [\n";
   ofs << "   'k'\n";
   ofs << "   'b'\n";
   ofs << "   'g'\n";
   ofs << "   'r'\n";
   ofs << "   'c'\n";
   ofs << "   'm'\n";
   ofs << "   'y'\n";
   ofs << "   'w'\n";
   ofs << "   ];\n";

   ofs << "markers = [\n";
   ofs << "   '+'\n";
   ofs << "   '*'\n";
   ofs << "   'x'\n";
   ofs << "   's'\n";
   ofs << "   'd'\n";
   ofs << "   'v'\n";
   ofs << "   '^'\n";
   ofs << "   '<'\n";
   ofs << "   '>'\n";
   ofs << "   'p'\n";
   ofs << "   'h'\n";
   ofs << "];\n";

   ofs.exportObservablesInfo( *sa.get() );

   // now go over simulations and proxy evaluation and for each observable
   // export data
   // 1st create observables list:
   // TODO implement for well trajectory
   for ( size_t i = 0; i < sa->obsSpace().size(); ++i )
   {
      ofs << "\nProxyQC(" << i+1 << ").obsName = '" << sa->obsSpace().observable( i )->name()[0] << "';\n";

      // go over DoE cases which were used to build proxy
      size_t csPos = 0;
      for ( size_t e = 0; e < m_caseList.size(); ++e )
      {         

         sa->doeCaseSet().filterByExperimentName( m_caseList[e] );
         assert( sa->doeCaseSet().size() == proxyCaseSetSize[e] );
         
         if ( !proxyCaseSetSize[e] ) continue; // skip empty experiments

         ofs << "\nProxyQC(" << i+1 << ").expNameProxyBld{" << e+1 << "} = '" << m_caseList[e] << "';\n";
         
         // collect observable values for e-th experiment
         std::vector<double> simCases;
         for ( size_t c = 0; c < sa->doeCaseSet().size(); ++c )
         {
            const casa::ObsValue * obv = sa->doeCaseSet().runCase( c )->obsValue( i );
            
            if ( obv->isDouble() )
            {
               const std::vector<double> & vec = obv->asDoubleArray();
               simCases.insert( simCases.end(), vec.begin(), vec.end() );
            }
         }

         std::vector<double> prxCases;
         for ( size_t c = 0; c < proxyCaseSetSize[e]; ++c )
         {
            const casa::ObsValue * obv = proxyCaseSet[csPos]->obsValue( i );
            if ( obv->isDouble() )
            {
               const std::vector<double> & vec = obv->asDoubleArray();
               prxCases.insert( prxCases.end(), vec.begin(), vec.end() );
            }
            ++csPos;
         }
         assert( prxCases.size() == simCases.size() );
         // save experiment data for e-th experiment
         ofs << "\nProxyQC(" << i+1 << ").proxyBldData{" << e+1 << "} = [\n  ";
         for ( size_t c = 0; c < simCases.size(); ++c ) { ofs << " " << simCases[c]; }
         ofs << "\n  ";
         for ( size_t c = 0; c < simCases.size(); ++c ) { ofs << " " << prxCases[c]; }
         ofs << "\n];\n";
      }

      // go over DoE cases which were given for testing proxy
      csPos = 0;
      for ( size_t e = 0; e < m_testCaseList.size(); ++e )
      {         

         sa->doeCaseSet().filterByExperimentName( m_testCaseList[e] );
         assert( sa->doeCaseSet().size() == testCaseSetSize[e] );
         
         if ( !testCaseSetSize[e] ) continue; // skip empty experiments

         ofs << "\nProxyQC(" << i+1 << ").expNameProxyTst{" << e+1 << "} = '" << m_testCaseList[e] << "';\n";
         
         // collect observable values for e-th experiment
         std::vector<double> simCases;
         for ( size_t c = 0; c < sa->doeCaseSet().size(); ++c )
         {
            const casa::ObsValue * obv = sa->doeCaseSet().runCase( c )->obsValue( i );
            
            if ( obv->isDouble() )
            {
               const std::vector<double> & vec = obv->asDoubleArray();
               simCases.insert( simCases.end(), vec.begin(), vec.end() );
            }
         }

         std::vector<double> prxCases;
         for ( size_t c = 0; c < testCaseSetSize[e]; ++c )
         {
            const casa::ObsValue * obv = testCaseSet[csPos]->obsValue( i );
            if ( obv->isDouble() )
            {
               const std::vector<double> & vec = obv->asDoubleArray();
               prxCases.insert( prxCases.end(), vec.begin(), vec.end() );
            }
            ++csPos;
         }
         assert( prxCases.size() == simCases.size() );
         // save experiment data for e-th experiment
         ofs << "\nProxyQC(" << i+1 << ").proxyTstData{" << e+1 << "} = [\n  ";
         for ( size_t c = 0; c < simCases.size(); ++c ) { ofs << " " << simCases[c]; }
         ofs << "\n  ";
         for ( size_t c = 0; c < simCases.size(); ++c ) { ofs << " " << prxCases[c]; }
         ofs << "\n];\n";
      }
   }

   ofs << "for i = 1:length( ProxyQC )\n";
   ofs << "   hold off\n";
   ofs << "   display( ['  processign plot for observable: ' ProxyQC(i).obsName] );\n";
   ofs << "\n\n";
   ofs << "   % plot all experiments which were used to build proxy, get scale\n";
   ofs << "   for e = 1:length( ProxyQC(i).expNameProxyBld )\n";
   ofs << "      plot( ProxyQC(i).proxyBldData{e}(1,:), ProxyQC(i).proxyBldData{e}(2,:), 'ko', 'markerfacecolor', 'k' );\n";
   ofs << "      hold on\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   for e = 1:length( ProxyQC(i).proxyTstData )\n";
   ofs << "      plot( ProxyQC(i).proxyTstData{e}(1,:), ProxyQC(i).proxyTstData{e}(2,:), 'ko', 'markerfacecolor', 'k' );\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   minV = min( axis() );\n";
   ofs << "   maxV = max( axis() );\n";
   ofs << "   hold off\n";
   ofs << "   close\n";
   ofs << "\n";
   ofs << "   axis( [minV maxV minV maxV] );\n";
   ofs << "   % Get reference value and standard deviation\n";
   ofs << "   if ( length( ObservablesRefValue{ i } ) > 0 )\n";
   ofs << "      ObsRefVal    = ObservablesRefValue{ i, 1 };\n";
   ofs << "      ObsRefStdDev = ObservablesRefValue{ i, 2 };\n";
   ofs << "      % Plot standard deviation as a stripe\n";
   ofs << "      rectangle( 'Position', [ObsRefVal - ObsRefStdDev, minV, 2 * ObsRefStdDev, maxV - minV ], 'FaceColor', [0.8 1 0.8], 'EdgeColor', 'g', 'linewidth', 3 );\n";
   ofs << "      hold on;\n";
   ofs << "\n";
   ofs << "      % and a rectangle\n";
   ofs << "      rectangle( 'Position', [ObsRefVal - ObsRefStdDev, ObsRefVal - ObsRefStdDev, 2 * ObsRefStdDev, 2 * ObsRefStdDev ], 'FaceColor', [0.5 1 0.5], 'EdgeColor', 'g', 'linewidth', 3 );\n";
   ofs << "   else\n";
   ofs << "      hold on;\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   % plot 90%-100%-110% lines\n";
   ofs << "   pbndsX = [ minV maxV; minV     maxV;     minV     maxV];\n";
   ofs << "   pbndsY = [ minV maxV; minV*0.9 maxV*0.9; minV*1.1 maxV*1.1];\n";
   ofs << "\n";
   ofs << "   plot( pbndsX', pbndsY', '-r', 'linewidth', 2 );\n";
   ofs << "\n";
   ofs << "   % plot 90%-100%-110% legends\n";
   ofs << "   dx = (maxV-minV)*0.13;\n";
   ofs << "   text( maxV-dx,     maxV-1.1*dx,          '100%', 'fontweight', 'bold' );\n";
   ofs << "   text( maxV-dx,     (maxV-dx)*0.9-0.1*dx, '90%',  'fontweight', 'bold' );\n";
   ofs << "   text( maxV-1.4*dx, (maxV-dx)*1.1-0.6*dx, '110%', 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   if ( length( ObservablesRefValue{ i } ) > 0 )\n";
   ofs << "      plot( [ ObsRefVal ], [ ObsRefVal ], 'or', 'linewidth', 3, 'markerfacecolor', 'w', 'markersize', plotMarkerSize, 'linewidth', 3 );\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   % plot again experiment which were used to build proxy\n";
   ofs << "   for e = 1:length( ProxyQC(i).expNameProxyBld )\n";
   ofs << "      cl = colors( mod(e-1,length(colors))+1,:);\n";
   ofs << "      h(e) = plot( ProxyQC(i).proxyBldData{e}(1,:), ProxyQC(i).proxyBldData{e}(2,:), [cl 'o'], 'markerfacecolor', cl );\n";
   ofs << "      legName{e} = ProxyQC(i).expNameProxyBld{e};\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   for e = 1:length( ProxyQC(i).proxyTstData )\n";
   ofs << "      mr = markers( mod(e-1,length(markers))+1,:);\n";
   ofs << "      h(end+1) = plot( ProxyQC(i).proxyTstData{e}(1,:), ProxyQC(i).proxyTstData{e}(2,:), ['r' mr], 'markerfacecolor', 'r' );\n";
   ofs << "      legName{end+1} = ProxyQC(i).expNameProxyTst{e};\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   legend( h, legName, 'location', 'northeastoutside' );\n";
   ofs << "\n";
   ofs << "   set( findobj( gcf(), 'type', 'axes', 'Tag', 'legend'), 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   ah=get (gcf, 'currentaxes');\n";
   ofs << "   set( ah, 'fontweight', 'bold' );\n";
   ofs << "   title( ['" << m_proxyName << " QC plot: ' ProxyQC(i).obsName], 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   clear h;\n";
   ofs << "   clear legName;\n";
   ofs << "   eval( sprintf( 'print "<< m_proxyName << "_proxyQC_Obs_%d.jpg -S1200x800', i ) )\n";
   ofs << "end\n";

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Succeded...\n";
   }
}

