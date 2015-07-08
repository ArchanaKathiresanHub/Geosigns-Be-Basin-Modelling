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
#include "CmdPlotMC.h"
#include "MatlabExporter.h"

#include "casaAPI.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

CmdPlotMC::CmdPlotMC( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   // set default values
   m_mFileName    = "mcResultsPlot.m";
  
   if ( m_prms.size() > 0 )
   {
      m_mFileName = m_prms[0];
   }

   if ( m_prms.size() > 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown parameter of PlotMC command: " << m_prms[1];
   }
}

void CmdPlotMC::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Generating script to plot MC/MCMC results...";
   }

   MatlabExporter ofs( m_mFileName );

   ofs << "%CASA Monte Carlo plotting script\n";
   ofs << "clear all\n";
   ofs << "hold off\n";
   ofs << "close\n";

   ofs << "\ncolors = [\n   'b'\n   'g'\n   'r'\n   'c'\n   'm'\n   'y'\n   'k'\n   'w'\n   ];\n";
   ofs << "subPlotSize = 600;\n";
   ofs << "markerSize = 10;\n";

   ofs.exportParametersInfo( *sa.get() );

   ofs.exportMCResults( *sa.get() );
   
   ofs << "plotID = 1;\n";
   ofs << "prmsNumber = size( MCSamplingPrmsVal, 2 ) - 1;\n";
   ofs << "\n";
   ofs << "%generate red-green colormap and associate [rgb] color triplet to each RMSE value\n";
   ofs << "minRMSE = min( MCSamplingRMSE );\n";
   ofs << "maxRMSE = max( MCSamplingRMSE );\n";
   ofs << "\n";
   ofs << "g = [1:-0.01 : 0];\n";
   ofs << "cc(1:length(g), 1) = 1-g';\n";
   ofs << "cc(1:length(g), 2) = g';\n";
   ofs << "cc(1:length(g), 3) = 0;\n";
   ofs << "colormap(cc);\n";
   ofs << "colInd = 1 + floor((MCSamplingRMSE - minRMSE)./(maxRMSE - minRMSE)*(size(cc, 1) - 1));\n";
   ofs << "\n";
   ofs << "mSize(1:length(MCSamplingRMSE)) = markerSize;\n";
   ofs << "\n";
   ofs << "for i = 1:prmsNumber\n";
   ofs << "\n";
   ofs << "   for j = 1:prmsNumber\n";
   ofs << "      display(sprintf( 'processign plot: %dx%d', i, j) );\n";
   ofs << "\n";
   ofs << "      subplot( prmsNumber, 2 * prmsNumber + 1, [plotID plotID+1]);\n";
   ofs << "\n";
   ofs << "      if ( i == j )\n";
   ofs << "         %Plot histogram for each parameter\n";
   ofs << "         hist( MCSamplingPrmsVal(:,j+1), 20, 'facecolor', 'b', 'edgecolor', 'k' );\n";
   ofs << "      else \n";
   ofs << "         %Scatter plot for each parameters pair\n";
   ofs << "         scatter(MCSamplingPrmsVal(:, j + 1), MCSamplingPrmsVal(:, i + 1), mSize, cc(colInd, :), 'filled' );\n";
   ofs << "         % Mark best match position\n";
   ofs << "         hold on;\n";
   ofs << "         plot( MCSamplingPrmsVal(1, j + 1), MCSamplingPrmsVal(1, i + 1), 'ko', 'markersize', markerSize*0.8, 'markerfacecolor', 'k' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      if ( i == 1 )\n";
   ofs << "         title( SubParametersName{ j }, 'fontweight', 'bold' );\n";
   ofs << "      elseif ( i == prmsNumber )\n";
   ofs << "         xlabel( SubParametersName{ j }, 'fontweight', 'bold' );\n";
   ofs << "      end\n";
   ofs << "      if ( j == 1 )\n";
   ofs << "         ylabel( SubParametersName{ i }, 'fontweight', 'bold' );\n";
   ofs << "      end\n";
   ofs << "\n";
   ofs << "      %get axes handle to use for setting properties\n";
   ofs << "      h=get (gcf, 'currentaxes');\n";
   ofs << "      set( h, 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "      plotID = plotID + 2;\n";
   ofs << "      %eval( cmd );\n";
   ofs << "   end\n";
   ofs << "   plotID = plotID + 1;\n";
   ofs << "end\n";
   ofs << "\n";
   ofs << "subplot( prmsNumber, 2*prmsNumber + 1, [2*prmsNumber+1:2*prmsNumber+1:(prmsNumber * (2*prmsNumber+1))] );\n";
   ofs << "axis off\n";
   ofs << "\n";
   ofs << "if ( MCGOF(1) > 0 )\n";
   ofs << "c = floor( length( cc ) * (1-MCGOF(1)/100) );\n";
   ofs << "   bar( MCGOF(1), 'facecolor', cc(c,:) );\n";
   ofs << "   axis( [0 2 0 100] );\n";
   ofs << "   text( 0.2, MCGOF(1), sprintf('  GOF\\n%g%%', MCGOF(1) ), 'fontweight', 'bold' );\n";
   ofs << "end\n\n";
   ofs << "h = colorbar;\n";
   ofs << "ylabel(h, 'RMSE', 'fontweight', 'bold' );\n";
   ofs << "dc = (maxRMSE-minRMSE) / 5;\n";
   ofs << "\n";
   ofs << "for i = 0:5\n";
   ofs << "   cbTckLabels{i+1} = sprintf( '%g', minRMSE + dc * i );\n";
   ofs << "end\n";
   ofs << "set( h, 'ytick', [0, 0.2, 0.4, 0.6, 0.8, 1.0], 'yticklabel', cbTckLabels, 'fontweight', 'bold' );\n";
   ofs << "\n";
   ofs << "cmd = sprintf( 'print %s.jpg -S%d,%d', mfilename(), subPlotSize * prmsNumber, subPlotSize * prmsNumber );\n";
   ofs << "eval( cmd );\n";

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "\nSucceeded." << std::endl;
   }
}

void CmdPlotMC::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <scriptName>\n";
   std::cout << "   - create Matlab/Octave .m script file which could be used to create plot with results of MC/MCMC simulation\n";
   std::cout << "     \n";
   std::cout << "     Example:\"" << cmdName << "\" command:\n";
   std::cout << "         " << cmdName << " \"mcmcmplot.m\"\n";
}


