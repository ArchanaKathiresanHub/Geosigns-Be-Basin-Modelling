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
#include "ObsGridPropertyWell.h"
#include "RunCase.h"

// Stadard C lib
#include <cstdlib>
#include <cmath>

// STL
#include <iostream>
#include <memory>
#include <algorithm>


CmdPlotRSProxyQC::CmdPlotRSProxyQC( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   m_proxyName            = m_prms.size() > 0 ? m_prms[0] : "";
   std::string proxyCases = m_prms.size() > 1 ? m_prms[1] : "";
   std::string testCases  = m_prms.size() > 2 ? m_prms[2] : "";
   m_mFileName            = m_prms.size() > 3 ? m_prms[3] : "rsProxyQCPlot.m";
   std::string obsList    = m_prms.size() > 4 ? m_prms[4] : "";


   if ( m_proxyName.empty()  ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No rs proxy name was given to PlotRSProxyQC command"; }
   if ( proxyCases.empty()   ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Empty cases list for proxy plot QC command"; }
 
   // convert list of DoEs or data files like: "Tornado,BoxBenken" into array of names
   m_caseList = CfgFileParser::list2array( proxyCases, ',' );
   if ( m_caseList.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No any DoE or data file name was given for making proxy QC plot";

   if ( !obsList.empty() )
   {
      const std::vector<std::string> & givenTargetNames = CfgFileParser::list2array( obsList, ',' );
      m_targetNames.insert( givenTargetNames.begin(), givenTargetNames.end() );
   }

   if ( !testCases.empty() ) m_testCaseList = CfgFileParser::list2array( testCases, ',' );
}

void CmdPlotRSProxyQC::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   BOOST_LOG_TRIVIAL( info ) << "Generating script to plot QC for " << m_proxyName << "rs proxy...";

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

   ofs << "\nScenarioID    = '" << sa->scenarioID() << "';\n";

   ofs << "plotMarkerSize = 6;\n\n";

   ofs << "colors = [\n";
   ofs << "   'k'\n";
   ofs << "   'r'\n";
   ofs << "   'g'\n";
   ofs << "   'b'\n";
   ofs << "   'c'\n";
   ofs << "   'm'\n";
   ofs << "   'y'\n";
   ofs << "   ];\n";

   ofs << "markers = [\n";
   ofs << "   '+'\n";
   ofs << "   'x'\n";
   ofs << "   's'\n";
   ofs << "   '*'\n";
   ofs << "   'd'\n";
   ofs << "   'v'\n";
   ofs << "   '^'\n";
   ofs << "   '<'\n";
   ofs << "   '>'\n";
   ofs << "   'p'\n";
   ofs << "   'h'\n";
   ofs << "];\n";

   bool hasWellObs = false;
   // now go over simulations and proxy evaluation and for each observable
   // export data
   // 1st create observables list:
   // TODO implement for well trajectory
   for ( size_t i = 0; i < sa->obsSpace().size(); ++i )
   {
      const casa::Observable * obsObj = sa->obsSpace().observable( i );
      if ( !obsObj ) continue;

      const casa::ObsGridPropertyWell * wellObs = dynamic_cast<const casa::ObsGridPropertyWell *>( obsObj );

      const std::vector<std::string> & obsNamesList = obsObj->name();

      if ( wellObs && wellObs->dimension() > 1  )
      {
         ofs << obsWellData( i, sa, proxyCaseSet, testCaseSet );
         hasWellObs = true;
      }
      
      for ( size_t o = 0; o < obsObj->dimension(); ++o )
      {
         const std::string & obsName = obsNamesList[o];

         bool found = false;
         for ( std::set<std::string>::const_iterator it = m_targetNames.begin(); !found && it != m_targetNames.end(); ++it )
         {
            if ( obsName.find( *it ) != std::string::npos ) found = true; 
         }
         if ( !m_targetNames.empty() && !found ) continue;

         ofs << "\n";
         ofs << "ProxyQC( end+1 ).obsName = '" << obsName << "';\n";
         ofs << "ProxyQC( end ).obsNameFN = '" << MatlabExporter::correctName( obsName ) << "';\n";

         ofs << "\nProxyQC( end ).obsRefAndDevValues = [";
         const casa::ObsValue * refVal =  obsObj->referenceValue(); 
         if ( refVal && refVal->isDouble() )
         {
            const std::vector<double> & vals = refVal->asDoubleArray();
            ofs << vals[o] << " " << obsObj->stdDeviationForRefValue();
         } 
         ofs << "];\n";

         // go over DoE cases which were used to build proxy
         size_t csPos = 0;
         for ( size_t e = 0; e < m_caseList.size(); ++e )
         {         

            sa->doeCaseSet().filterByExperimentName( m_caseList[e] );
            assert( sa->doeCaseSet().size() == proxyCaseSetSize[e] );
         
            if ( !proxyCaseSetSize[e] ) continue; // skip empty experiments

            ofs << "\nProxyQC( end ).expNameProxyBld{" << e+1 << "} = '" << m_caseList[e] << "';\n";
         
            // collect observable values for e-th experiment
            std::vector<double> simCases;
            for ( size_t c = 0; c < sa->doeCaseSet().size(); ++c )
            {
               const casa::ObsValue * obv = sa->doeCaseSet().runCase( c )->obsValue( i );
               
               if ( obv->isDouble() ) { simCases.push_back( obv->asDoubleArray()[o] ); }
            }

            std::vector<double> prxCases;
            for ( size_t c = 0; c < proxyCaseSetSize[e]; ++c )
            {
               const casa::ObsValue * obv = proxyCaseSet[csPos]->obsValue( i );
               if ( obv->isDouble() ) { prxCases.push_back( obv->asDoubleArray()[o] ); }
               ++csPos;
            }
   
            assert( prxCases.size() == simCases.size() );
            // save experiment data for e-th experiment
            ofs << "\nProxyQC( end ).proxyBldData{" << e+1 << "} = [\n  ";
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

            ofs << "\nProxyQC( end ).expNameProxyTst{" << e+1 << "} = '" << m_testCaseList[e] << "';\n";
         
            // collect observable values for e-th experiment
            std::vector<double> simCases;
            for ( size_t c = 0; c < sa->doeCaseSet().size(); ++c )
            {
               const casa::ObsValue * obv = sa->doeCaseSet().runCase( c )->obsValue( i );
            
               if ( obv->isDouble() ) { simCases.push_back( obv->asDoubleArray()[o] ); }
            }

            std::vector<double> prxCases;
            for ( size_t c = 0; c < testCaseSetSize[e]; ++c )
            {
               const casa::ObsValue * obv = testCaseSet[csPos]->obsValue( i );
               if ( obv->isDouble() ) { prxCases.push_back( obv->asDoubleArray()[o] ); }
               ++csPos;
            }
            assert( prxCases.size() == simCases.size() );
            // save experiment data for e-th experiment
            ofs << "\nProxyQC( end ).proxyTstData{" << e+1 << "} = [\n  ";
         
            for ( size_t c = 0; c < simCases.size(); ++c ) { ofs << " " << simCases[c]; }
            ofs << "\n  ";
            for ( size_t c = 0; c < prxCases.size(); ++c ) { ofs << " " << prxCases[c]; }
            ofs << "\n];\n";
         }
      }
   }
   if ( hasWellObs )
   {
      // plot QC plot for build proxy cases wells type targets
      ofs << "\nhold off\n";
      ofs << "for w = 1 : length( WellsObs )\n";
      ofs << "   display( ['  processign QC plot for well: ' WellsObs(w).name] );\n";
      ofs << "\n";
      ofs << "  mr = markers( mod(w-1,length(markers))+1,:);\n";
      ofs << "  cl = colors( mod(w,length(colors))+1,:);\n";
      ofs << "\n";
      ofs << "  h(w) = plot(  WellsObs(w).r2build, WellsObs(w).depth,[cl mr], 'LineWidth', 4 );\n";
      ofs << "  hold on\n";
      ofs << "  legName{w} = WellsObs(w).name;\n";
      ofs << "\n";
      ofs << "end\n";
      ofs << "axis('ij' );\n";
      ofs << "\n";
      ofs << "legend( h, legName, 'location', 'northwest' );\n";
      ofs << "grid on;\n";
      ofs << "\n";
      ofs << "set( findobj( gcf(), 'type', 'axes', 'Tag', 'legend'), 'fontweight', 'bold' );\n";
      ofs << "\n";
      ofs << "ah=get (gcf, 'currentaxes');\n";
      ofs << "set( ah, 'fontweight', 'bold' );\n";
      ofs << "title( 'QC plot: R^2 (Build) vs depth for well type targets', 'fontweight', 'bold' );\n";
      ofs << "\n";
      ofs << "xlabel( 'R^2 (Build proxy Cases) []' );\n";
      ofs << "ylabel( 'Depth [m]' );\n";
      ofs << "print " << MatlabExporter::correctName( m_proxyName ) << "_" << "_proxyQC_wells_build.jpg -S1000,1000;\n\n";

      ofs << "close\n";
      // plot QC plot for test cases wells type targets
      ofs << "\nhold off\n";
      ofs << "for w = 1 : length( WellsObs )\n";
      ofs << "   display( ['  processign QC plot for well: ' WellsObs(w).name] );\n";
      ofs << "\n";
      ofs << "  mr = markers( mod(w-1,length(markers))+1,:);\n";
      ofs << "  cl = colors( mod(w,length(colors))+1,:);\n";
      ofs << "\n";
      ofs << "  h(w) = plot(  WellsObs(w).r2test, WellsObs(w).depth,[cl mr], 'LineWidth', 4 );\n";
      ofs << "  hold on\n";
      ofs << "  legName{w} = WellsObs(w).name;\n";
      ofs << "\n";
      ofs << "end\n";
      ofs << "axis('ij' );\n";
      ofs << "\n";
      ofs << "legend( h, legName, 'location', 'northwest' );\n";
      ofs << "grid on;\n";
      ofs << "\n";
      ofs << "set( findobj( gcf(), 'type', 'axes', 'Tag', 'legend'), 'fontweight', 'bold' );\n";
      ofs << "\n";
      ofs << "ah=get (gcf, 'currentaxes');\n";
      ofs << "set( ah, 'fontweight', 'bold' );\n";
      ofs << "title( 'QC plot: R^2 (Test) vs depth for well type targets', 'fontweight', 'bold' );\n";
      ofs << "\n";
      ofs << "xlabel( 'R^2 (Test proxy Cases) []' );\n";
      ofs << "ylabel( 'Depth [m]' );\n";
      ofs << "print " << MatlabExporter::correctName( m_proxyName ) << "_" << "_proxyQC_wells_test.jpg -S1000,1000;\n\n";
   }

   // plot QC plot per observable
   ofs << "close\n clear h legName\naxis( 'xy' )\n";
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
   ofs << "   % Get reference value and standard deviation\n";
   ofs << "   if ( length( ProxyQC(i).obsRefAndDevValues ) == 2 )\n";
   ofs << "      ObsRefVal    = ProxyQC(i).obsRefAndDevValues( 1 );\n";
   ofs << "      ObsRefStdDev = ProxyQC(i).obsRefAndDevValues( 2 );\n";
   ofs << "\n";
   ofs << "      minV = min( [minV ObsRefVal-ObsRefStdDev ] );\n";
   ofs << "      maxV = max( [maxV ObsRefVal+ObsRefStdDev ] );\n";
   ofs << "\n";
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
   ofs << "   axis( [minV maxV minV maxV] );\n";
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
   ofs << "   if ( length( ProxyQC(i).obsRefAndDevValues ) > 0 )\n";
   ofs << "      plot( [ ObsRefVal ], [ ObsRefVal ], 'or', 'markerfacecolor', 'w', 'markersize', plotMarkerSize, 'linewidth', 3 );\n";
   ofs << "      hrf = plot( [ ObsRefVal ], [ ObsRefVal], 'or', 'markersize', plotMarkerSize, 'linewidth', 3 );\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   % plot again experiment which were used to build proxy\n";
   ofs << "   for e = 1:length( ProxyQC(i).expNameProxyBld )\n";
   ofs << "      cl = colors( mod(e-1,length(colors))+1,:);\n";
   ofs << "      h(e) = plot( ProxyQC(i).proxyBldData{e}(1,:), ProxyQC(i).proxyBldData{e}(2,:), [cl 'o'], 'markerfacecolor', cl );\n";
   ofs << "      legName{e} = sprintf( '%s - %d cases', ProxyQC(i).expNameProxyBld{e}, length( ProxyQC(i).proxyBldData{e}(2,:) ) );\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   for e = 1:length( ProxyQC(i).proxyTstData )\n";
   ofs << "      mr = markers( mod(e-1,length(markers))+1,:);\n";
   ofs << "      cl = colors( mod(e,length(colors))+1,:);\n";
   ofs << "      h(end+1) = plot( ProxyQC(i).proxyTstData{e}(1,:), ProxyQC(i).proxyTstData{e}(2,:), [cl mr], 'markerfacecolor', cl, 'linewidth', 3 );\n";
   ofs << "      legName{end+1} = sprintf( '%s - %d cases', ProxyQC(i).expNameProxyTst{e}, length( ProxyQC(i).proxyTstData{e}(2,:) ) );\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   if ( exist( 'hrf' ) )\n";
   ofs << "       h = [h hrf];\n";
   ofs << "       legName = [legName, 'Reference value'];\n";
   ofs << "       clear hrf;\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   legend( h, legName, 'location', 'southeast' );\n";
   ofs << "   grid on;\n";
   ofs << "\n";
   ofs << "   set( findobj( gcf(), 'type', 'axes', 'Tag', 'legend'), 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   ah=get (gcf, 'currentaxes');\n";
   ofs << "   set( ah, 'fontweight', 'bold' );\n";
   ofs << "   title( ['" << m_proxyName << " QC plot: ' ProxyQC(i).obsNameFN], 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   xlabel( 'Simulated value' );\n";
   ofs << "   ylabel( 'Proxy value' );\n";
   ofs << "\n";
   ofs << "   clear h;\n";
   ofs << "   clear legName;\n";
   ofs << "   eval( sprintf( 'print QC_" << MatlabExporter::correctName( m_proxyName ) << "_" << "%s.jpg -S1000,1000', ProxyQC(i).obsNameFN ) );\n";
   ofs << "end\n";

   BOOST_LOG_TRIVIAL( info ) << "Script generation succeded";
}



std::string CmdPlotRSProxyQC::obsWellData( size_t                                  obsID
                                         , std::auto_ptr<casa::ScenarioAnalysis> & sa
                                         , std::vector<casa::RunCase *>          & proxyCaseSet
                                         , std::vector<casa::RunCase *>          & testCaseSet
                                         )
{
   std::ostringstream oss;

   const casa::ObsGridPropertyWell * wellObs = dynamic_cast<const casa::ObsGridPropertyWell *>( sa->obsSpace().observable( obsID ) );
   
   if ( !wellObs ) { return ""; }

   // construct obs name from the name of the first point
   const std::vector<std::string> & obsNames = wellObs->name();
   std::string name = obsNames[0];

   bool found = false;
   for ( std::set<std::string>::const_iterator it = m_targetNames.begin(); !found && it != m_targetNames.end(); ++it )
   {
      if ( name.find( *it ) != std::string::npos ) found = true; 
   }
   if ( !found ) return "";

   std::string::size_type pos = name.rfind( "_1" ); // automatically named well point
   if ( pos != std::string::npos ) { name = name.substr( 0, name.size() - 2 ); }

   std::vector< std::vector<double> > simVals( wellObs->dimension() );
   std::vector< std::vector<double> > prxVals( wellObs->dimension() );

   const std::vector<double> & depth = wellObs->depth();
   std::vector<double> R2build( wellObs->dimension(), 0.0 );
   std::vector<double> R2test( wellObs->dimension(), 0.0 );

   // go over DoEs which were used to construct proxy at first
   size_t csPos = 0;

   for ( size_t e = 0; e < m_caseList.size(); ++e )
   {
      // select next experiment
      sa->doeCaseSet().filterByExperimentName( m_caseList[e] );

      if ( !sa->doeCaseSet().size() ) continue; // skip empty experiments

      // collect observable values for e-th experiment
      for ( size_t c = 0; c < sa->doeCaseSet().size(); ++c )
      {
         const casa::ObsValue * obv = sa->doeCaseSet().runCase( c )->obsValue( obsID );
         const std::vector<double> & vals = obv->asDoubleArray();
         assert( vals.size() == simVals.size() );
         for ( size_t i = 0; i < vals.size(); ++i ) { simVals[i].push_back( vals[i] ); }

         obv = proxyCaseSet[csPos]->obsValue( obsID );
         const std::vector<double> & pvals = obv->asDoubleArray();
         assert( pvals.size() == prxVals.size() );
         for ( size_t i = 0; i < pvals.size(); ++i ) { prxVals[i].push_back( pvals[i] ); }
         ++csPos;
      }
   }

   // calculate average for simulated value
   std::vector<double> avrVals( simVals.size(), 0.0 );
   for ( size_t i = 0; i < simVals.size(); ++i )
   {
      for ( size_t j = 0; j < simVals[i].size(); ++j )
      {
         avrVals[i] += simVals[i][j];
      }
      if ( simVals[i].size() > 0 ) avrVals[i] = avrVals[i] / simVals[i].size();
   }

   // calculate R^2 with depth
   for ( size_t i = 0; i < wellObs->dimension(); ++i )
   {
      double sum1 = 0.0;
      double sum2 = 0.0;
      for ( size_t j = 0; j < simVals[i].size(); ++j )
      {
         sum1 += ( simVals[i][j] - prxVals[i][j] ) * ( simVals[i][j] - prxVals[i][j] );
         sum2 += ( avrVals[i]    - prxVals[i][j] ) * ( avrVals[i]    - prxVals[i][j] );

      }
      R2build[i] = 1.0e0 - (sum2 > 0.0 ? sum1 / sum2 : 1.0);
   }

   // go over DoE which were given for testing proxy
   csPos = 0;
   avrVals.clear();
   avrVals.assign( simVals.size(), 0.0 );
   for ( size_t c = 0; c < simVals.size(); ++c )
   {
      simVals[c].clear();
      prxVals[c].clear();
   }

   for ( size_t e = 0; e < m_testCaseList.size(); ++e )
   {
      sa->doeCaseSet().filterByExperimentName( m_testCaseList[e] );

      if ( !sa->doeCaseSet().size() ) continue; // skip empty experiments

      // collect observable values for e-th experiment
      for ( size_t c = 0; c < sa->doeCaseSet().size(); ++c )
      {
         const casa::ObsValue * obv = sa->doeCaseSet().runCase( c )->obsValue( obsID );
         const std::vector<double> & vals = obv->asDoubleArray();
         assert( vals.size() == simVals.size() );
         for ( size_t i = 0; i < vals.size(); ++i ) { simVals[i].push_back( vals[i] ); }

         obv = testCaseSet[csPos]->obsValue( obsID );
         const std::vector<double> & pvals = obv->asDoubleArray();
         assert( pvals.size() == prxVals.size() );
         for ( size_t i = 0; i < pvals.size(); ++i ) { prxVals[i].push_back( pvals[i] ); }
         ++csPos;
      }
   }

   // calculate average for simulated value
   for ( size_t i = 0; i < simVals.size(); ++i )
   {
      for ( size_t j = 0; j < simVals[i].size(); ++j )
      {
         avrVals[i] += simVals[i][j];
      }
      if ( simVals[i].size() > 0 ) avrVals[i] = avrVals[i] / simVals[i].size();
   }

   // calculate R^2 with depth
   for ( size_t i = 0; i < wellObs->dimension(); ++i )
   {
      double sum1 = 0.0;
      double sum2 = 0.0;
      for ( size_t j = 0; j < simVals[i].size(); ++j )
      {
         sum1 += ( simVals[i][j] - prxVals[i][j] ) * ( simVals[i][j] - prxVals[i][j] );
         sum2 += ( avrVals[i]    - prxVals[i][j] ) * ( avrVals[i]    - prxVals[i][j] );

      }
      R2test[i] = 1.0e0 - (sum2 > 0.0 ? sum1 / sum2 : 1.0);
   }

   // dump as matlab
   oss << "\nWellsObs( end+1 ).name = '" << MatlabExporter::correctName( name ) << "';\n";
   oss << "WellsObs( end ).depth = [ ";

   for ( size_t i = 0; i < depth.size(); ++i )
   {
      oss << depth[i] << " ";
   }
   oss << "];\n";

   oss << "WellsObs( end ).r2build = [ ";
   for ( size_t i = 0; i < R2build.size(); ++i )
   {
      oss << R2build[i] << " ";
   }
   oss << "];\n";

   oss << "WellsObs( end ).r2test = [ ";
   for ( size_t i = 0; i < R2test.size(); ++i )
   {
      oss << R2test[i] << " ";
   }
   oss << "];\n";


   return oss.str();
}

