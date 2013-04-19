#include "ftparameters.h"

//#include "polyfunction.h"

#include <cmath>

using CBMGenerics::Polyfunction;
namespace Calibration
{

namespace FissionTracks
{

//get singleton
FtParameters&  FtParameters::getInstance()
{
   static FtParameters ftParameters;
   return ftParameters;
}

FtParameters::FtParameters()
{
   m_numBinClWeightPercent   = 30;
   m_stepBinClWeightPercent  = 0.1;
   m_startBinClWeightPercent = 0.0;

   m_numBinTrackLengths   = 40;
   m_stepBinTrackLengths  = 0.5;
   m_startBinTrackLengths = 0.0;

   m_numBinGrainAge   = 25;
   m_stepBinGrainAge  = 50.0;
   m_startBinGrainAge = 0.0;

   m_t0 = exp(-15.95); 
   m_t0Inv = 0.0005838;
   m_renormalizationFactor =  16.0/14.75;   // geotrack value

   m_binMergingFactor = 2;

   m_secsInMa = 3.15576e13; // number of s in a Million y
   m_kelvinAt0Celsius = 273.15;
   m_U238DecayConstant = 1.55125e-10;

   m_undefinedInt = -9999;
   m_undefinedDouble  = -9999.0;

/* #define FTFACTOR                    2              /\* display binnum =  */
/*                                                       FTBINNUMLENGTHDISTR/FTFACTOR *\/ */


   // Add four tie-points supplied by Geotrack (Cl wt%, L0)

   m_l0.AddPoint (.0, 17.0);
   m_l0.AddPoint (0.4265, 17.5);
   m_l0.AddPoint (0.874, 17.7);
   m_l0.AddPoint (5.33, 18.0);

   // Create polyfunction for c0 
   //Polyfunction<float> funcC0;
   CBMGenerics::Polyfunction funcC0;//Yiannis

   // Add four tie-points supplied by Geotrack (Cl wt%, c0)

   funcC0.AddPoint (.0, -3.786);
   funcC0.AddPoint (0.4265, -3.016);
   funcC0.AddPoint (0.874, -2.713);
   funcC0.AddPoint (5.33, -2.328);


   // retrieve interpolated c0 values and store in C0 array
   m_c0.resize( m_numBinClWeightPercent ) ;

   int i;
   double clWtPerc;
   for ( i = 0, clWtPerc = m_startBinClWeightPercent + 0.5 * m_stepBinClWeightPercent; 
         i < m_numBinClWeightPercent; 
         ++i, clWtPerc += m_stepBinClWeightPercent )
   {
      m_c0[i] = funcC0.F( clWtPerc );
   }

   // Create polyfunction for c1 
   //Polyfunction<float> funcC1;
   CBMGenerics::Polyfunction funcC1;//Yiannis

   // Add four tie-points supplied by Geotrack (Cl wt%, c1)

   funcC1.AddPoint (.0, 1.5756e-4);
   funcC1.AddPoint (0.4265, 1.1889e-4);
   funcC1.AddPoint (0.874, 1.0237e-4);
   funcC1.AddPoint (5.33, 0.8037e-4);

   // retrieve interpolated c1 values and store in C1 array
   m_c1.resize( m_numBinClWeightPercent ) ;

   for ( i = 0, clWtPerc = m_startBinClWeightPercent + 0.5 * m_stepBinClWeightPercent; 
         i < m_numBinClWeightPercent; 
         ++i, clWtPerc += m_stepBinClWeightPercent )
   {
      m_c1[i] = funcC1.F( clWtPerc );
   }
}

double FtParameters::getL0 ( int indexCl ) const   //!!! float? double? of base type might be significant for regression testing
{
   double percentCl = m_startBinClWeightPercent + m_stepBinClWeightPercent * (0.5 + (double) indexCl);

   return m_l0.F( percentCl );
}

int FtParameters::indexFromClWeightPercent (double clPerc) //!!! originally float
{
   return (int ( (clPerc - m_startBinClWeightPercent) / m_stepBinClWeightPercent ) );
}

double FtParameters::getUndefinedDouble() const
{
   return m_undefinedDouble;
}

int FtParameters::getUndefinedInt() const
{
   return m_undefinedInt;
}

double FtParameters::getU238DecayConstant()
{
   return m_U238DecayConstant;
}


}

}//Calibration namespace
