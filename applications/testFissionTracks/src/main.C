#include <iostream>
#include <iomanip>
#include "histogram.h"
#include "ftparameters.h"
#include "ftsampleprediction.h"
#include "ftsampleobservation.h"
#include "ftanalysis.h"
#include "ftgrain.h"
//#include "polyfunction.h"
#include <fstream>
#include <list>

int main(int argc, char** argv)
{
   using namespace std;

//    cout << "*******************************************************" << endl;
//    cout << "Fission Track Test Program" <<endl;
//    cout << "*******************************************************" << endl;
   
//    cout << endl;
//    cout << "-------------------------------------------------------" << endl;
//    cout << "Histogram test 1" << endl;
//    cout << "-------------------------------------------------------" << endl;
//    cout << endl;

//    int binNum = 36;

//    Histogram histogram1;
//    histogram1.Initialize(0.0, 0.5, binNum); //start-value, bin width, number of bins

//    Histogram histogram2 = histogram1;       
//    const double pi = 3.1415; 

//    for (int i = 0; i < histogram1.getBinNum(); ++i )
//    {
//       double f1 =  cos( i / double(binNum) * 3.0 * 2.0 * pi ); //three periods of cos function...
//       f1 = ( f1 > 0 ? f1 : 0.0 ) ;                               //...where only positive values are taken.
//       histogram1.SetBinCount( i, f1 );

//       double f2 =  cos( i / double(binNum) * 4.0 * 2.0 * pi ); //four periods of cos function...
//       f2 = ( f2 > 0 ? f2 : 0.0 ) ;                               //...where only positive values are taken.
//       histogram2.SetBinCount( i, f2 );
//    }   

//    cout << "+++ histogram1 +++" << endl;
//    histogram1.Print();
//    cout << "++++++++++++++++++" << endl << endl;

//    cout << "+++ histogram2 +++" << endl;
//    histogram2.Print();
//    cout << "++++++++++++++++++" << endl << endl;

//    float chi2 = CalcChi2(&histogram1, &histogram2);
   
//    cout.setf(ios::fixed,ios::floatfield);
//    cout << setw(20) << setprecision(10);
//    cout << "Chi2: " << chi2 << endl; 

//    cout << "-------------------------------------------------------" << endl;
//    cout << "FtParameters test 1" << endl;
//    cout << "-------------------------------------------------------" << endl;
   
//    cout << "numBinWeightPercent " << FtParameters::getInstance().getNumBinClWeightPercent() << endl;

//    cout << "-------------------------------------------------------" << endl;
//    cout << "FtSample test 1" << endl;
//    cout << "-------------------------------------------------------" << endl;
//    {
//       int timestep = 1;
//       double dt = 3.06898e+11;
//       double temperature = 298.15;
//       int indexCl = 0;
   
//       FtSamplePrediction ftSampleTest1;
//       ftSampleTest1.calcTrackLengths (dt, temperature, 0);
//       Histogram* predLengths0 = ftSampleTest1.getTrackLengths(0);

//       cout << "After timestep  " << timestep << ", dt = " << dt <<  ", T = " 
//            << temperature << " : Histogram for Cl-Index "  << indexCl << endl;

//       predLengths0->Print();
//    }
//    cout << "-------------------------------------------------------" << endl;
//    cout << "FtSample test 2" << endl;
//    cout << "-------------------------------------------------------" << endl;
//    {
//       int timestep;
//       double dt;
//       double temperature;
//       int indexCl;

//       FtSamplePrediction ftSampleTest2;
//       indexCl = 0;
//       ftSampleTest2.addClIndexIfNotYetExists( indexCl );

//       ifstream ifs("fttest2.in");

//       ifs >> timestep >> dt >> temperature;

//       while ( ifs )
//       {
//          ftSampleTest2.calcTrackLengths (dt, temperature, indexCl);

//          Histogram* predLengths0 = ftSampleTest2.getTrackLengths(indexCl);

//          cout.setf(ios::scientific,ios::floatfield);
//          cout << setw(25) << setprecision(18);

//          cout << "After timestep  " << timestep << ", dt = " << dt <<  ", T = " 
//               << temperature << " : Histogram for Cl-Index "  << indexCl << endl;
   
//          predLengths0->Print();
         
//          ifs >> timestep >> dt >> temperature;
//       }
//    }
//    cout << "-------------------------------------------------------" << endl;
//    cout << "FtSample test 3" << endl;
//    cout << "-------------------------------------------------------" << endl;
//    {
//       int timestep;
//       double dt;
//       double temperature;
//       int indexCl;
//       FtParameters& params = FtParameters::getInstance(); 
      
//       FtSamplePrediction ftSampleTest3("sample1");
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.149 ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.108 ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0     ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.01  ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.071 ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.091 ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.303 ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0     ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.226 ) );
//       ftSampleTest3.addClIndexIfNotYetExists( params.indexFromClWeightPercent( 0.141 ) );

//       for ( indexCl = 0 ; indexCl < params.getNumBinClWeightPercent(); ++indexCl )
//       { 
//          if ( ftSampleTest3.getTrackLengths(indexCl) )
//          {
//             cout << "active bin " << indexCl << endl; 
//          }         
//       }

//       ifstream ifs("fttest2.in");

//       ifs >> timestep >> dt >> temperature;

//       while ( ifs )
//       {
         
//          for ( indexCl = 0 ; indexCl < params.getNumBinClWeightPercent(); ++indexCl )
//          { 
//             if ( ftSampleTest3.getTrackLengths(indexCl) )
//             {
//                ftSampleTest3.calcTrackLengths (dt, temperature, indexCl);
//             }         
//          }
//          indexCl = 2;
//          Histogram* predLengths0 = ftSampleTest3.getTrackLengths(indexCl);

//          cout.setf(ios::scientific,ios::floatfield);
//          cout << setw(25) << setprecision(18);

//          cout << "After timestep  " << timestep << ", dt = " << dt <<  ", T = " 
//               << temperature << " : Histogram for Cl-Index "  << indexCl << endl;
   
//          predLengths0->Print();
         
//          ifs >> timestep >> dt >> temperature;
//       }
//    }
//    cout << "-------------------------------------------------------" << endl;
//    cout << "FtSample test 4" << endl;
//    cout << "-------------------------------------------------------" << endl;
   {
      //using namespace FissionTracks;
       using Calibration::FissionTracks::FtParameters;
       using Calibration::FissionTracks::FtAnalysis;
       using Calibration::FissionTracks::FtGrain;
       using Calibration::FissionTracks::FtSamplePrediction;
       using Calibration::FissionTracks::Histogram;
       using Calibration::FissionTracks::FtSampleObservation;

     
      FtParameters& params = FtParameters::getInstance(); 
      
      FtAnalysis ftAnalysis;

      string str, sampleId;
      int nSamples;
      double zeta, UStdGlassTrackDensity;
      cin >>  str >> nSamples;
      for (int i = 0; i < nSamples; ++i)
      {
         int nGrains;
         cin >> str >> sampleId >> str >> nGrains>> str >> zeta >> str >> UStdGlassTrackDensity;
         ftAnalysis.addSample(sampleId, zeta, UStdGlassTrackDensity);
         
         for (int j = 0; j < nGrains; ++j)
         {
            int grainId, nLengths, nSpontTracks, nInducedTracks;
            double ClWeightPerc;
            cin >> str >> grainId >> str >> sampleId 
                >> str >> nLengths >> str >> nSpontTracks >> str >> nInducedTracks >> str >> ClWeightPerc;
            
            ftAnalysis.addGrain(sampleId, grainId, nSpontTracks, nInducedTracks, ClWeightPerc);
            for (int k = 0; k < nLengths; ++k)
            {
               double length;
               cin >> str >> str >> str >> str >> str >> length;
               ftAnalysis.addTrackLength(sampleId, grainId, length);
            }
         }
      }

      //procss measured data
      ftAnalysis.initialize();
      list<FtGrain*>& grains = ftAnalysis.getGrains();
      
      list<FtGrain*>::const_iterator iGrain;
      for ( iGrain = grains.begin(); iGrain != grains.end() ; ++iGrain)
      {
         
         cout.setf(ios::scientific,ios::floatfield);
         cout << setw(10) << setprecision(4);
         cout << (*iGrain)->getSampleId() << " " <<  (*iGrain)->getGrainId() << " "  
              << (*iGrain)->getGrainAge() << " " 
              << (*iGrain)->getGrainAgeErr() << endl; 
      }

      for (int i = 0; i < nSamples; ++i)
      {
         int nTimesteps;
         cin >> str >> nTimesteps >> str >> sampleId;
         FtSamplePrediction* ftPredSample =   ftAnalysis.getPredSample(sampleId);  

         for (int j = 0; j < nTimesteps; ++j)
         {
            double timestepSize, temperature;
            cin >> str >> timestepSize >> str >> temperature >> str >> sampleId;

            ftPredSample->advanceState (timestepSize, temperature);
         }
         
         for ( int indexCl = 0 ; indexCl < params.getNumBinClWeightPercent(); ++indexCl )
         {
            Histogram* predLengths = ftPredSample->getTrackLengths(indexCl);
            if (predLengths)
            {
               cout.setf(ios::scientific,ios::floatfield);
               cout << setw(25) << setprecision(18);
               cout << "predicted track lengths of sample " << sampleId << ", indexCl " << indexCl << endl;
               predLengths->Print();
               cout << endl;
            }
         }
      }
      
      ftAnalysis.finalize();

      sampleId = "695-10";

      FtSampleObservation* ftObsSample  = ftAnalysis.getObsSample(sampleId);
      FtSamplePrediction*  ftPredSample = ftAnalysis.getPredSample(sampleId);
      cout << "Sample-Data "
           << ftObsSample  -> getSampleCode()  << " "
           << ftObsSample  -> getZeta() << " "
           << ftObsSample  -> getUStdGlassTrackDensity() << " "
           << ftPredSample -> getPredAge() << " "
           << ftObsSample  -> getPooledAge() << " "
           << ftObsSample  -> getPooledAgeErr() << " "
           << ftObsSample  -> getAgeChi2() << " "
           << ftObsSample  -> getDegreesOfFreedom() << " "
           << ftObsSample  -> getPAgeChi2() << " "
           << ftObsSample  -> getCorrCoeff() << " "
           << ftObsSample  -> getVarSqrtNs() << " "
           << ftObsSample  -> getVarSqrtNi() << " "
           << ftObsSample  -> getNsDivNi() << " "
           << ftObsSample  -> getNsDivNiErr() << " "
           << ftObsSample  -> getMeanRatioNsNi() << " "
           << ftObsSample  -> getMeanRatioNsNiErr() << " "
           << ftObsSample  -> getCentralAge() << " "
           << ftObsSample  -> getCentralAgeErr() << " "
           << ftObsSample  -> getMeanAge() << " "
           << ftObsSample  -> getMeanAgeErr() << " "
           << ftAnalysis.getLengthChi2( ftObsSample->getSampleCode() ) << " "
           << ftObsSample  -> getApatiteYield() << " "
           << endl;


      sampleId = "614-6";

      ftObsSample  = ftAnalysis.getObsSample(sampleId);
      ftPredSample = ftAnalysis.getPredSample(sampleId);
      cout << "Sample-Data "
           << ftObsSample  -> getSampleCode()  << " "
           << ftObsSample  -> getZeta() << " "
           << ftObsSample  -> getUStdGlassTrackDensity() << " "
           << ftPredSample -> getPredAge() << " "
           << ftObsSample  -> getPooledAge() << " "
           << ftObsSample  -> getPooledAgeErr() << " "
           << ftObsSample  -> getAgeChi2() << " "
           << ftObsSample  -> getDegreesOfFreedom() << " "
           << ftObsSample  -> getPAgeChi2() << " "
           << ftObsSample  -> getCorrCoeff() << " "
           << ftObsSample  -> getVarSqrtNs() << " "
           << ftObsSample  -> getVarSqrtNi() << " "
           << ftObsSample  -> getNsDivNi() << " "
           << ftObsSample  -> getNsDivNiErr() << " "
           << ftObsSample  -> getMeanRatioNsNi() << " "
           << ftObsSample  -> getMeanRatioNsNiErr() << " "
           << ftObsSample  -> getCentralAge() << " "
           << ftObsSample  -> getCentralAgeErr() << " "
           << ftObsSample  -> getMeanAge() << " "
           << ftObsSample  -> getMeanAgeErr() << " "
           << ftAnalysis.getLengthChi2( ftObsSample->getSampleCode() ) << " "
           << ftObsSample  -> getApatiteYield() << " "
           << endl;

     
   }   

   return 0;
}

