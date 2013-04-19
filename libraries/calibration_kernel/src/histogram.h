#ifndef CALIBRATION_HISTOGRAM_H
#define CALIBRATION_HISTOGRAM_H

#include <vector>

namespace Calibration
{

namespace FissionTracks
{

class Histogram
{
 private:
  //private data
  float            BINSTART;
  float            BINWIDTH;
  int              BINNUM;
  std::vector<double>   BIN;
   
 public:
   //public constants
   static const double IBSNULLVALUE = -9999;


public:
   //public functions
   void AddBinFractions( Histogram* source, float factor );
   
   void AddBins( Histogram* source );
   
   double BinCount( int binIx );
 
   void CopyBin( double* newbin );

   void CopyBin( const std::vector<double>& newbin );
   
   void CopyContentsTo (Histogram* dest);
   
   //void FillHistogramTbl( HistogramTbl_c HistogramTbl );
   
   bool FindAndIncrementBin( float binVal );
   
   bool FindBin( float binVal, int* binIx );
   
   float getBinStart();

   float getBinWidth();

   int getBinNum();

   double GetMaxBinCount();
   
   double GetMean();
   
   Histogram();
   
   void IncrementBin( int binIx );
   
   void Init();
   
   void Initialize( float binstart, float binwidth, int binnum );
   
   void MergeBins( int numbin );
   
   void MulBinCount( int binIx, double scalefactor );
   
   void MulBins( double scalefactor );
   
   void Print();
   
   void PrintBinHeader();
   
   void PrintBins();
   
   void Scale( int totBinCnt );
   
   void SetBinCount( int binIx, double binVal );

   void SumBinCount( int binIx, double sumValue );
   
   double SumBins( int startIx, int endIx );
   
   virtual  ~Histogram();

   //friends

   friend float CalcChi2( Histogram* predHist, Histogram* obsHist );

   friend bool JoinBins ( Histogram* master, Histogram* slave, Histogram* masterRes, Histogram* slaveRes );

};

}

}//namespace Calibration

#endif


