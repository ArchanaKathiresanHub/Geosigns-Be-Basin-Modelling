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

void CmdPlotTornado::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
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

   LogHandler( LogHandler::INFO_SEVERITY ) << "Generating script to plot Tornado diagram...";

   casa::SensitivityCalculator & sCalc = sa->sensitivityCalculator();

   std::vector< casa::TornadoSensitivityInfo > data = sCalc.calculateTornado( sa->doeCaseSet(), m_doeNames );
   if ( data.empty() ) { throw ErrorHandler::Exception( sCalc.errorCode() ) << sCalc.errorMessage(); }

   MatlabExporter ofs( m_mFileName );

   ofs << "%CASA Tornado diagram plotting script\n";

   ofs << "clear all\n";
   ofs << "hold off\n";
   ofs << "close\n";

   ofs << "minMinColor = [1 0 0];\n";
   ofs << "maxMaxColor = [0 0 1];\n";
   ofs << "defColor    = [0 0.8 0];\n";
   ofs << "undefColor  = [0.5 0.5 0.5];\n";
   ofs << "minColor    = [1.0 0.5 0.5];\n";
   ofs << "maxColor    = [0.5 0.5 1.0];\n";
   ofs << "lww         = 0.8;  %Space devoted for one parameter <1 \n";
   ofs << "hlw         = lww/2;\n";

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
      ofs << "TornadoSens.obsUndefValue( " << obsNum << ") = " << data[i].undefinedValue() << ";\n";

      const std::vector<std::pair<const casa::VarParameter *, int> > & varPrmList = data[i].varPrmList();
      const casa::TornadoSensitivityInfo::SensitivityData            & sens       = data[i].sensitivities();
      const casa::TornadoSensitivityInfo::SensitivityData            & relSens    = data[i].relSensitivities();
      const casa::TornadoSensitivityInfo::SensitivityData            & maxSens    = data[i].maxSensitivities();      
      const casa::TornadoSensitivityInfo::SensitivityData            & maxRelSens = data[i].maxRelSensitivities();      
      const casa::TornadoSensitivityInfo::SensitivityData            & maxSensPrm = data[i].prmValsForMaxSensitivities();

      for ( size_t j = 0; j < varPrmList.size(); ++j )
      {
         const std::vector<std::string> & prmNames = varPrmList[j].first->name();
         const std::string & prmName = prmNames[varPrmList[j].second];

         ofs << "TornadoSens.prmNames{ " << obsNum << ", " << j+1 << "} = '" << prmName << "';\n";
         ofs << "TornadoSens.prmRange{ " << obsNum << ", " << j+1 << "} = [" << data[i].minVarParameterRangeValue( j ) << " "
                                                                             << data[i].maxVarParameterRangeValue( j ) << "];\n";
         double absSensMin = sens[j].front() == UndefinedDoubleValue ? sens[j].front() : data[i].refObsValue() + sens[j].front();
         double absSensMax = sens[j].back()  == UndefinedDoubleValue ? sens[j].back()  : data[i].refObsValue() + sens[j].back();

         if ( sens[j].size() == 1 ) // categorical
         {
            ofs << "TornadoSens.absSensMin( "    << obsNum << ", " << j+1 << ") = " << data[i].refObsValue() << ";\n";
            ofs << "TornadoSens.absSensMax( "    << obsNum << ", " << j+1 << ") = " << absSensMin << ";\n";
            ofs << "TornadoSens.relSensMin( "    << obsNum << ", " << j+1 << ") = " << 101 << ";\n"; // sens will not be shown if pp > 100
            ofs << "TornadoSens.relSensMax( "    << obsNum << ", " << j+1 << ") = " << relSens[j][0] << ";\n";
            ofs << "TornadoSens.maxRelSensMin( " << obsNum << ", " << j+1 << ") = " << 101 << ";\n"; // sens will not be shown if pp > 100
            ofs << "TornadoSens.maxRelSensMax( " << obsNum << ", " << j+1 << ") = " << maxRelSens[j][0] << ";\n";
         }
         else
         {
            ofs << "TornadoSens.absSensMin( "    << obsNum << ", " << j+1 << ") = " << absSensMin << ";\n";
            ofs << "TornadoSens.absSensMax( "    << obsNum << ", " << j+1 << ") = " << absSensMax << ";\n";
            ofs << "TornadoSens.relSensMin( "    << obsNum << ", " << j+1 << ") = " << relSens[j][0] << ";\n";
            ofs << "TornadoSens.relSensMax( "    << obsNum << ", " << j+1 << ") = " << relSens[j][1] << ";\n";
            ofs << "TornadoSens.maxRelSensMin( " << obsNum << ", " << j+1 << ") = " << maxRelSens[j][0] << ";\n";
            ofs << "TornadoSens.maxRelSensMax( " << obsNum << ", " << j+1 << ") = " << maxRelSens[j][1] << ";\n";
         }
         
         absSensMin = maxSens[j].front() == UndefinedDoubleValue ? maxSens[j].front() : data[i].refObsValue() + maxSens[j].front();
         absSensMax = maxSens[j].back()  == UndefinedDoubleValue ? maxSens[j].back()  : data[i].refObsValue() + maxSens[j].back();

         ofs << "TornadoSens.maxSensMin( "    << obsNum << ", " << j+1 << ") = " << absSensMin << ";\n";
         ofs << "TornadoSens.maxSensMax( "    << obsNum << ", " << j+1 << ") = " << absSensMax << ";\n";
         ofs << "TornadoSens.maxSensMinPrm( " << obsNum << ", " << j+1 << ") = " << maxSensPrm[j].front() << ";\n";
         ofs << "TornadoSens.maxSensMaxPrm( " << obsNum << ", " << j+1 << ") = " << maxSensPrm[j].back()  << ";\n";
 
         ofs << "TornadoSens.validSensRngsPrmVal{ " << obsNum << ", " << j+1 << "} = [\n";
         const std::vector<double> & prmRngVals = data[i].validRngsPrmVals( j );
         for ( size_t k = 0; k < prmRngVals.size(); k += 2 )
         {
            ofs << "                           " << prmRngVals[k] << " " << prmRngVals[k+1] << ";\n";
         }
         ofs    << "                                                     ];\n";
 
         ofs << "TornadoSens.validSensRngsObsVal{ " << obsNum << ", " << j+1 << "} = [\n";
         const std::vector<double> & obsRngVals = data[i].validRngsObsVals( j );
         for ( size_t k = 0; k < obsRngVals.size(); k += 2 )
         {
            ofs << "                           " << obsRngVals[k] << " " << obsRngVals[k+1] << ";\n";
         }
         ofs    << "                                                     ];\n";
     }
   }

   ofs << "for i = 1:length( TornadoSens.obsRefVal )\n";
   ofs << "\n";
   ofs << "   display( ['Creating tornado diagram for observable: ' TornadoSens.obsName{i}] );\n";
   ofs << "\n";
   ofs << "   bv = TornadoSens.obsRefVal(i);\n";
   ofs << "   minAbs = TornadoSens.absSensMin(i,:);\n";
   ofs << "   maxAbs = TornadoSens.absSensMax(i,:);\n";
   ofs << "\n";
   ofs << "   minRel = TornadoSens.relSensMin(i,:);\n";
   ofs << "   maxRel = TornadoSens.relSensMax(i,:);\n";
   ofs << "   maxMinRel = TornadoSens.maxRelSensMin(i,:);\n";
   ofs << "   maxMaxRel = TornadoSens.maxRelSensMax(i,:);\n";
   ofs << "\n";
   ofs << "   maxMinAbs = TornadoSens.maxSensMin(i,:);\n";
   ofs << "   maxMaxAbs = TornadoSens.maxSensMax(i,:);\n";
   ofs << "\n";
   ofs << "   prmValidRngVals = TornadoSens.validSensRngsPrmVal(i,:);\n";
   ofs << "   obsValidRngVals = TornadoSens.validSensRngsObsVal(i,:);\n";
   ofs << "\n";
   ofs << "   % remove very small numbers\n";
   ofs << "   jj = find( abs(minRel) < 1.e-4);\n";
   ofs << "   minRel(jj) = 0;\n";
   ofs << "   jj = find( abs(maxRel) < 1.e-4);\n";
   ofs << "   maxRel(jj) = 0;\n";
   ofs << "\n";
   ofs << "   close;\n";
   ofs << "   hold off\n";
   ofs << "   %Exclude undefined values from sorting\n";
   ofs << "   vmin = maxMinAbs;\n";
   ofs << "   vmax = maxMaxAbs;\n";
   ofs << "   jj = find( vmin == TornadoSens.obsUndefValue(i) );\n";
   ofs << "   if length( jj ) > 0\n";
   ofs << "         vmin(jj) = bv;\n";
   ofs << "   end\n";
   ofs << "   jj = find( vmax == TornadoSens.obsUndefValue(i) );\n";
   ofs << "   if length( jj ) > 0\n";
   ofs << "         vmax(jj) = bv;\n";
   ofs << "   end\n";
   ofs << "   [v sind] = sort( max( abs(vmin - bv), abs(vmax - bv) ) );\n";
   ofs << "   \n";
   ofs << "   % exclude undefined values\n";
   ofs << "   AbsMax = max( v );\n";
   ofs << "\n";
   ofs << "   % Go over all parameters\n";
   ofs << "   for p = 1:length( sind )\n";
   ofs << "\n";
   ofs << "      % Default initialization\n";
   ofs << "      lw = lww / 4;\n";
   ofs << "      x = [ maxMinAbs(sind(p)) minAbs(sind(p)) maxMaxAbs(sind(p)) maxAbs(sind(p)) ];\n";
   ofs << "      y = [ 0 2*lw lw 3*lw ];\n";
   ofs << "\n";
   ofs << "      xcolor = [ minMinColor; minColor; maxMaxColor; maxColor ];\n";
   ofs << "\n";
   ofs << "      uind = find( x == TornadoSens.obsUndefValue(i) );\n";
   ofs << "\n";
   ofs << "      if ( length( uind ) == length(x) ) % all values undefined just draw 2 gray rectangles\n";
   ofs << "         x = [ bv-AbsMax bv-AbsMax bv+AbsMax bv+AbsMax ];\n";
   ofs << "         xcolor = [ undefColor; undefColor; undefColor; undefColor ];\n";
   ofs << "      else\n";
   ofs << "         if ( x(1) == TornadoSens.obsUndefValue(i) )\n";
   ofs << "            x(1) = bv-AbsMax;\n";
   ofs << "            xcolor( 1,:) = undefColor;\n";
   ofs << "         end\n";
   ofs << "\n";
   ofs << "         if ( x(2) == TornadoSens.obsUndefValue(i) )\n";
   ofs << "            x(2) = x(1);\n";
   ofs << "            xcolor( 2, : ) = undefColor;\n";
   ofs << "         end\n";
   ofs << "\n";
   ofs << "         if ( x(3) == TornadoSens.obsUndefValue(i) )\n";
   ofs << "            x(3) = bv+AbsMax;\n";
   ofs << "            xcolor( 3, : ) = undefColor;\n";
   ofs << "         end\n";
   ofs << "\n";
   ofs << "         if ( x(4) == TornadoSens.obsUndefValue(i) )\n";
   ofs << "            x(4) = x(3);\n";
   ofs << "            xcolor( 4, : ) = undefColor;\n";
   ofs << "         end\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      for r = 1:length( x )\n";
   ofs << "         xx = x( r );\n";
   ofs << "         %plot rectangle with correct color\n";
   ofs << "         rectangle( 'Position', [min(xx, bv), p-hlw+y(r), abs(bv - xx), lw ], 'FaceColor', xcolor(r,:), 'EdgeColor', xcolor(r,:)  );\n";
   ofs << "         hold on;\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      %plot min/max range values\n";
   ofs << "       prmRng = TornadoSens.prmRange{i,sind(p)};\n";
   ofs << "       if ( size( prmValidRngVals{sind(p)},1 ) > 0 )\n";
   ofs << "          rngs = prmValidRngVals{sind(p)};\n";
   ofs << "       elseif ( length( x ) == 4 ) % Defined over all range\n";
   ofs << "          rngs = prmRng;\n";
   ofs << "       else                         % undefined\n";
   ofs << "          rngs = [];\n";
   ofs << "       end\n";
   ofs << "\n";
   ofs << "       for r = 1 : size( rngs, 1 )\n";
   ofs << "          rng = rngs(r,:);\n";
   ofs << "          %scale range\n";
   ofs << "          rng(1) = (rng(1) - prmRng(1))/(prmRng(2) - prmRng(1));\n";
   ofs << "          rng(2) = (rng(2) - prmRng(1))/(prmRng(2) - prmRng(1));\n";
   ofs << "          rectangle( 'Position', [ bv - 2 * AbsMax, p - hlw + lww * rng(1), 0.1 * AbsMax, lww * (rng(2) - rng(1)) ], "
                                                              << "'FaceColor', defColor, 'EdgeColor', defColor );\n";
   ofs << "       end\n";
   ofs << "       rectangle( 'Position', [ bv - 2 * AbsMax, p-hlw, 0.1 * AbsMax, lww],'EdgeColor', 'k' );\n";
   ofs << "\n";
   ofs << "       text( bv - 2.1*AbsMax, p - hlw - 0.1 * hlw, sprintf( '%4.2g', prmRng(1) ), 'color', 'k', 'fontweight', 'bold' );\n";
   ofs << "       text( bv - 2.1*AbsMax, p + hlw + 0.1 * hlw, sprintf( '%4.2g', prmRng(2) ), 'color', 'k', 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "       %draw lines where min/max values along interval are\n";
   ofs << "       rng(1) = TornadoSens.maxSensMinPrm(i,sind(p));\n";
   ofs << "       rng(2) = TornadoSens.maxSensMaxPrm(i,sind(p));\n";
   ofs << "       %scale\n";
   ofs << "       rng(1) = (rng(1) - prmRng(1))/(prmRng(2) - prmRng(1));\n";
   ofs << "       rng(2) = (rng(2) - prmRng(1))/(prmRng(2) - prmRng(1));\n";
   ofs << "\n";
   ofs << "       plot( [bv-2.05*AbsMax, bv-1.95*AbsMax], [p - hlw, p - hlw ], 'color', xcolor(2,:), 'linewidth', 4 );\n";
   ofs << "       plot( [bv-2.05*AbsMax], [ p - hlw ], 'o', 'markersize', 6, 'markerfacecolor', xcolor(2,:), 'markeredgecolor', xcolor(2,:) );\n";
   ofs << "\n";
   ofs << "       plot( [bv-2.05*AbsMax, bv-1.95*AbsMax], [p + hlw, p + hlw], 'color', xcolor(4,:), 'linewidth', 4 );\n";
   ofs << "       plot( [bv-2.05*AbsMax],                 [p + hlw], 'o', 'markersize', 6, 'markerfacecolor', xcolor(4,:), 'markeredgecolor', xcolor(4,:) );\n";
   ofs << "\n";
   ofs << "       plot( [bv-1.95*AbsMax, bv-1.85*AbsMax], [ p - hlw + lww*rng(1), p - hlw + lww * rng(1)], 'color', xcolor(1,:), 'linewidth', 4 );\n";
   ofs << "       plot( [bv-1.85*AbsMax], [p-hlw+lww*rng(1)], 'o', 'markersize', 6, 'markerfacecolor', xcolor(1,:), 'markeredgecolor', xcolor(1,:) );\n";
   ofs << "\n";
   ofs << "       plot( [bv-1.95*AbsMax, bv - 1.85*AbsMax], [p-hlw+lww*rng(2), p-hlw+lww*rng(2)], 'color', xcolor(3,:), 'linewidth', 4 );\n";
   ofs << "       plot( [bv-1.85*AbsMax], [p-hlw+lww*rng(2)], 'o', 'markersize', 6, 'markerfacecolor', xcolor(3,:), 'markeredgecolor', xcolor(3,:) );\n";
   ofs << "\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   plot( [bv bv], [0,max(sind)+hlw], 'k', 'linewidth', 2 );\n";
   ofs << "   \n";
   ofs << "   av = axis();\n";
   ofs << "   if ( bv - av(1) < 2.5 * AbsMax )\n";
   ofs << "       av(1) = bv - 2.5 * AbsMax;\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   if ( av(2) - bv  < 1.5 * AbsMax )\n";
   ofs << "       av(2) = bv + 1.5 * AbsMax;\n";
   ofs << "   end\n";
   ofs << "   dx = (av(2)-av(1))/40;\n";
   ofs << "   axis( av );\n";
   ofs << "\n";
   ofs << "   for j = 1:length(minRel)\n";
   ofs << "\n";
   ofs << "      if ( maxRel(sind(j)) > 0 )\n";
   ofs << "         h = text( maxAbs(sind(j))+dx, j-hlw+3.5*lw , sprintf( '%4.2g%%', maxRel(sind(j)) ), 'color', maxColor, 'fontweight', 'bold' );\n";
   ofs << "      else\n";
   ofs << "         h = text( maxAbs(sind(j))-dx, j-hlw+3.5*lw, sprintf( '%4.2g%%', maxRel(sind(j)) ), 'color', maxColor, 'fontweight', 'bold'  );\n";
   ofs << "         set( h, 'HorizontalAlignment', 'right' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      if ( maxMaxRel(sind(j)) > 0 )\n";
   ofs << "         h = text( maxMaxAbs(sind(j))+dx, j-hlw+1.5*lw , sprintf( '%4.2g%%', maxMaxRel(sind(j)) ), 'color', maxMaxColor, 'fontweight', 'bold' );\n";
   ofs << "      else\n";
   ofs << "         h = text( maxMaxAbs(sind(j))-dx, j-hlw+1.5*lw, sprintf( '%4.2g%%', maxMaxRel(sind(j)) ), 'color', maxMaxColor, 'fontweight', 'bold'  );\n";
   ofs << "         set( h, 'HorizontalAlignment', 'right' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      if ( minRel(sind(j)) > 0 )\n";
   ofs << "         h = text( minAbs(sind(j))+dx, j-hlw+2.5*lw, sprintf( '%4.2g%%', minRel(sind(j)) ), 'color', minColor, 'fontweight', 'bold'  );\n";
   ofs << "      else\n";
   ofs << "         h = text( minAbs(sind(j))-dx, j-hlw+2.5*lw, sprintf( '%4.2g%%', minRel(sind(j)) ), 'color', minColor, 'fontweight', 'bold'  );\n";
   ofs << "         set( h, 'HorizontalAlignment', 'right' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      if ( maxMinRel(sind(j)) > 0 )\n";
   ofs << "         h = text( maxMinAbs(sind(j))+dx, j-hlw+0.5*lw, sprintf( '%4.2g%%', maxMinRel(sind(j)) ), 'color', minMinColor, 'fontweight', 'bold'  );\n";
   ofs << "      else\n";
   ofs << "         h = text( maxMinAbs(sind(j))-dx, j-hlw+0.5*lw, sprintf( '%4.2g%%', maxMinRel(sind(j)) ), 'color', minMinColor, 'fontweight', 'bold'  );\n";
   ofs << "         set( h, 'HorizontalAlignment', 'right' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      ylbl{j+1} = TornadoSens.prmNames(i, sind(j));\n";
   ofs << "\n";
   ofs << "      text( bv - AbsMax*2.4, j, TornadoSens.prmNames(i, sind(j)), 'fontweight', 'bold' );\n";
   ofs << "   end\n";
   ofs << "   set(gca, 'yticklabel', [] );\n";
   ofs << "\n";
   ofs << "   xlabel( TornadoSens.obsName{i}, 'fontweight', 'bold' );\n";
   ofs << "   ylabel( 'Variable parameter name', 'fontweight', 'bold' );\n";
   ofs << "   title( ['Variable parameters sensitivity tornado plot for ' TornadoSens.obsName{i}], 'fontweight', 'bold'  );\n";
   ofs << "\n";
   ofs << "   set( get( gcf, 'currentaxes' ), 'fontweight', 'bold' );\n";
   ofs << "   grid on\n";
   ofs << "\n";
   ofs << "   print( sprintf( 'tornado_%d.jpg', i ) );\n";
   ofs << "   eval( sprintf( 'print Tornado_%s.jpg -S1000,1000', TornadoSens.obsName{i} ) );\n";

   ofs << "end\n";

   LogHandler( LogHandler::INFO_SEVERITY ) << "Script generation succeeded";
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


