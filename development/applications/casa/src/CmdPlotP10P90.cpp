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
#include "CmdPlotP10P90.h"
#include "MatlabExporter.h"

#include "casaAPI.h"

#include "LogHandler.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

CmdPlotP10P90::CmdPlotP10P90( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   // set default values
   m_mFileName    = "mcP10P90Plot.m";
  
   if ( m_prms.size() > 0 )
   {
      m_mFileName = m_prms[0];
   }

   if ( m_prms.size() > 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown parameter of PlotP10P90 command: " << m_prms[1];
   }
}

void CmdPlotP10P90::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Generating script to plot P10-P90 diagrams from MC/MCMC results...";

   MatlabExporter ofs( m_mFileName );

   ofs << "%CASA P10-P90 diagram plotting script\n";
   ofs << "clear all\n";
   ofs << "hold off\n";
   ofs << "close\n";

   ofs << "\nScenarioID    = '" << sa->scenarioID() << "';\n";
   
   ofs.exportObservablesInfo( *sa.get() );
   ofs.exportMCResults(       *sa.get() );

   ofs << "%create histograms\n";
   ofs << "for i = 2 : size(MCSamplingObsVal,2)\n";
   ofs << "   display( ['Create histogram for ' ObservablesName{i-1, :}] );\n";
   ofs << "   [nn xx] = hist( MCSamplingObsVal(:,i), floor( sqrt( length( MCSamplingObsVal(:,i) ) ) ) );\n";
   ofs << "   bar( xx, nn, 'facecolor', 'b', 'edgecolor', 'b' );\n";
   ofs << "   title( ['Histogram for ' ObservablesName{i-1, :} ], 'fontweight', 'bold' );\n";
   ofs << "   ylabel( 'Number of cases', 'fontweight', 'bold' );\n";
   ofs << "   xlabel( ObservablesName{i-1, :} , 'fontweight', 'bold' );\n";
   ofs << "   set( get( gcf, 'currentaxes' ), 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   print (sprintf( 'mc_hist_obs_%d.jpg', i-1 ) );\n";
   ofs << "end\n";
   ofs << "\n";
   ofs << "%compute PDF & CDF\n";
   ofs << "for o = 2 : size(MCSamplingObsVal,2)\n";
   ofs << "   display( ['Create CDF plot for ' ObservablesName{o-1, :}] );\n";
   ofs << "\n";
   ofs << "   ObservationsVec = MCSamplingObsVal(:,o);\n";
   ofs << "\n";
   ofs << "   T = size( ObservationsVec, 1 );\n";
   ofs << "   curveLength = floor( sqrt(T) );\n";
   ofs << "\n";
   ofs << "   stdDev  = std(  ObservationsVec );\n";
   ofs << "   meanVal = mean( ObservationsVec );\n";
   ofs << "   \n";
   ofs << "   h = 1.06 * stdDev * T^(-1/5);\n";
   ofs << "\n";
   ofs << "   xMin = min( ObservationsVec );\n";
   ofs << "   xMax = max( ObservationsVec );\n";
   ofs << "   dX = (xMax-xMin) / (curveLength-1);\n";
   ofs << "   xvec = [xMin:dX:xMax];\n";
   ofs << "\n";
   ofs << "if ( length( xvec ) != curveLength )\n";
   ofs << "         continue;\n";
   ofs << "end\n";
   ofs << "\n";
   ofs << "   pdfVec=zeros( curveLength, 1 );\n";
   ofs << "   cdfVec=zeros( curveLength, 1 );\n";
   ofs << "\n";
   ofs << "   for i=1:curveLength,\n";
   ofs << "      x = xvec(i);\n";
   ofs << "      zvec = (x-ObservationsVec) / h;\n";
   ofs << "      Kvec=zvec*0;\n";
   ofs << "\n";
   ofs << "      for j = 1:T,\n";
   ofs << "         z = zvec(j);\n";
   ofs << "         if abs(z) <= 2.23606797749979 %sqrt(5)\n";
   ofs << "            Kvec(j) = 0.335410196624968 * (1-(1/5)*z*z); % 3/(4*sqrt(5))\n";
   ofs << "         else\n";
   ofs << "            Kvec(j)=0;\n";
   ofs << "         end\n";
   ofs << "      end\n";
   ofs << "      pdfVec( i )= sum( Kvec ) / ( h * T );\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   %calc CDF by integrating pdf\n";
   ofs << "   for i=1:curveLength-1,\n";
   ofs << "     cdfVec(i+1) = cdfVec(i) + 0.5 * (pdfVec(i) + pdfVec(i+1) ) * dX;\n";
   ofs << "   end\n";
   ofs << "\n";
   ofs << "   hold off\n";
   ofs << "   plot( xvec, cdfVec*100, '-r', 'linewidth', 3 );\n";
   ofs << "   hold on\n";
   ofs << "   plot( xvec, 100 - cdfVec*100, '-b', 'linewidth', 3 );\n";
   ofs << "\n";
   ofs << "   grid on\n";
   ofs << "\n";
   ofs << "   % calc observable values for 10-50-90 percents of CDF,\n";
   ofs << "   % because xvec and cdfVec monotonic increase we can call interp1\n";
   ofs << "   p1p9 = interp1( cdfVec, xvec, [0.1 0.5 0.9] );\n";
   ofs << "\n";
   ofs << "   plot( MC_P10P90_CDF(o-1,:), [10:10:90], 'rx', 'linewidth', 3 );\n";
   ofs << "   plot( MC_P10P90_CDF(o-1,:), 100 - [10:10:90], 'bx', 'linewidth', 3 );\n";
   ofs << "\n";
   ofs << "   plot( [p1p9(1) p1p9(1)], [0 100], ':g', 'linewidth', 2 );\n";
   ofs << "   plot( [p1p9(2) p1p9(2)], [0 100], ':b', 'linewidth', 2 );\n";
   ofs << "   plot( [p1p9(3) p1p9(3)], [0 100], ':k', 'linewidth', 2 );\n";
   ofs << "\n";
   ofs << "   legend( 'Ascending CDF', 'Descending CDF', 'Ascending CDF SUMlib', 'Descending CDF SUMlib',";
   ofs << "            sprintf( 'P10 - %g', p1p9(1) ),sprintf( 'P50 - %g', p1p9(2) ), sprintf( 'P90 - %g', p1p9(3) ) )\n";
   ofs << "\n";
   ofs << "   text( p1p9(1)+0.5 * dX, 50, '10%', 'color', 'g', 'fontweight', 'bold' );\n";
   ofs << "   text( p1p9(2)+0.5 * dX, 50, '50%', 'color', 'g', 'fontweight', 'bold' );\n";
   ofs << "   text( p1p9(3)+0.5 * dX, 50, '90%', 'color', 'g', 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   title( ['Ascending and Descending CDF for ' ObservablesName{o-1, :}], 'fontweight', 'bold' );\n";
   ofs << "   xlabel( ObservablesName{o-1, :} , 'fontweight', 'bold' );\n";
   ofs << "   ylabel( 'Cumulative probability [%]' , 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   av = axis();\n";
   ofs << "   axis( [av(1) av(2) 0 100] );\n";
   ofs << "   set( get( gcf, 'currentaxes' ), 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "   print ( sprintf( 'p10p90_cdf_obs_%d.jpg', o-1 ) );\n";
   ofs << "\n";
   ofs << "   % Plot probability density function\n";
   ofs << "   hold off\n";
   ofs << "   display( ['Create PDF plot for ' ObservablesName{o-1, :}] );\n";
   ofs << "   plot( xvec, pdfVec, '-ro', 'linewidth', 3 );\n";
   ofs << "   hold on\n";
   ofs << "   x0 = meanVal - 0.6745 * stdDev;\n";
   ofs << "   x1 = meanVal + 0.6745 * stdDev;\n";
   ofs << "   ax = axis;\n";
   ofs << "   y0 = ax(3);\n";
   ofs << "   y1 = ax(4);\n";
   ofs << "   plot( [x0 x0], [y0 y1], ':b', 'linewidth', 2 );\n";
   ofs << "   plot( [x1 x1], [y0 y1], ':g', 'linewidth', 2 );\n";
   ofs << "   text( meanVal, y0 + (y1-y0)/10, '50%', 'color', 'b', 'fontweight', 'bold' );\n";
   ofs << "   legend( 'PDF', sprintf( '%g', x0 ), sprintf( '%g', x1 ) );\n";
   ofs << "   \n";
   ofs << "   grid on\n";
   ofs << "   title( ['Normal PDF for ' ObservablesName{o-1, :}], 'fontweight', 'bold' );\n";
   ofs << "   xlabel( ObservablesName{o-1, :} , 'fontweight', 'bold' );\n";
   ofs << "   ylabel( 'Probability density' , 'fontweight', 'bold' );\n";
   ofs << "   set( get( gcf, 'currentaxes' ), 'fontweight', 'bold' );\n";
   ofs << "   print ( sprintf( 'p10p90_pdf_obs_%d.jpg', o-1 ) );\n";
   ofs << "end\n";

   LogHandler( LogHandler::INFO_SEVERITY ) << "Script generation succeeded";
}

void CmdPlotP10P90::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <scriptName>\n";
   std::cout << "   - create Matlab/Octave .m script file which could be used to create plot with P10-P90 CDF diagram for each observable \n";
   std::cout << "     using the results of MC/MCMC simulation\n";
   std::cout << "     \n";
   std::cout << "     Example:\"" << cmdName << "\" command:\n";
   std::cout << "         " << cmdName << " \"mcmcmplot.m\"\n";
}


