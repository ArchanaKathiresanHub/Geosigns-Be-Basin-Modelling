///////////////////////////////////////////////////////////////////////////////
//                                VERSION:1.0                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_histogram___
#define __rbyproject_histogram___
///////////////////////////////////////////////////////////////////////////////
//                           HISTOGRAM definitions                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_rubyproject__
#include   "rubyproject.h"
#endif
///////////////////////////////////////////////////////////////////////////////
//                    Public object specific information                     //
///////////////////////////////////////////////////////////////////////////////

  
  #include "histogramtbl.h"

//class PropDoubleVector;  // Forward reference


///////////////////////////////////////////////////////////////////////////////
//                            HISTOGRAM Structure                            //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_histogram_class___
#define __rbyproject_histogram_class___
typedef class Histogram
{
public:
   float                BINSTART;
   float                BINWIDTH;
   int                  BINNUM;
   PropDoubleVector_c   BIN;
// member functions
private:
public:
   void AddBinFractions( Histogram_c source, float factor );
   void AddBins( Histogram_c source );
   double BinCount( int binIx );
   static float CalcChi2( Histogram_c predHist, Histogram_c obsHist );
   Histogram * CopyYourself(void);
   void CopyBin( double* newbin );
   void CopyContentsTo (Histogram_c dest);
   static Histogram_c Create( BOOLEAN_t alloc = FALSE );
   void Delete(void );
   void FillHistogramTbl( HistogramTbl_c HistogramTbl );
   BOOLEAN_t FindAndIncrementBin( float binVal );
   BOOLEAN_t FindBin( float binVal, int* binIx );
   double GetMaxBinCount(void );
   double GetMean(void );
    Histogram( BOOLEAN_t alloc = FALSE );
   void IncrementBin( int binIx );
   void Init(void );
   void Initialize( float binstart, float binwidth, int binnum );
   static BOOLEAN_t JoinBins( Histogram_c master, Histogram_c slave, Histogram_c masterRes, Histogram_c slaveRes );
   static Histogram_c Make(void );
   void MergeBins( int numbin );
   void MulBinCount( int binIx, double scalefactor );
   void MulBins( double scalefactor );
   void Print( long* arg );
   void PrintBinHeader(void );
   void PrintBins(void );
   void Scale( int totBinCnt );
   void SetBinCount( int binIx, double binVal );
   void SumBinCount( int binIx, double sumValue );
   double SumBins( int startIx, int endIx );
   virtual  ~Histogram(void );
} Histogram_t, *Histogram_c;
#endif // __rbyproject_histogram_class___

///////////////////////////////////////////////////////////////////////////////
//                     HISTOGRAM Public Attribute Macros                     //
///////////////////////////////////////////////////////////////////////////////

#define Histogram_BINSTART(obj)			((obj)->BINSTART)
#define Histogram_BINWIDTH(obj)			((obj)->BINWIDTH)
#define Histogram_BINNUM(obj)			((obj)->BINNUM)
#define Histogram_BIN(obj)			((obj)->BIN)



///////////////////////////////////////////////////////////////////////////////
//                         Access For Public Methods                         //
///////////////////////////////////////////////////////////////////////////////

 void Histogram_AddBinFractions(Histogram_c histogram , Histogram_c source, float factor);
 void Histogram_AddBins(Histogram_c histogram , Histogram_c source);
 double Histogram_BinCount(Histogram_c histogram , int binIx);
 float CalcChi2( Histogram_c predHist, Histogram_c obsHist);
 void Histogram_CopyBin(Histogram_c histogram , double* newbin);
 Histogram_c Histogram_Create( BOOLEAN_t alloc = FALSE);
 void Histogram_Delete(Histogram_c histogram );
 void Histogram_FillHistogramTbl(Histogram_c histogram , HistogramTbl_c HistogramTbl);
 BOOLEAN_t Histogram_FindAndIncrementBin(Histogram_c histogram , float binVal);
 BOOLEAN_t Histogram_FindBin(Histogram_c histogram , float binVal, int* binIx);
 double Histogram_GetMaxBinCount(Histogram_c histogram );
 double Histogram_GetMean(Histogram_c histogram );
 void Histogram_IncrementBin(Histogram_c histogram , int binIx);
 void Histogram_Init(Histogram_c histogram );
 void Histogram_Initialize(Histogram_c histogram , float binstart, float binwidth, int binnum);
 BOOLEAN_t JoinBins( Histogram_c master, Histogram_c slave, Histogram_c masterRes, Histogram_c slaveRes);
 Histogram_c Histogram_Make(void);
 void Histogram_MergeBins(Histogram_c histogram , int numbin);
 void Histogram_MulBinCount(Histogram_c histogram , int binIx, double scalefactor);
 void Histogram_MulBins(Histogram_c histogram , double scalefactor);
 void Histogram_Print(Histogram_c histogram , long* arg);
 void Histogram_PrintBinHeader(Histogram_c histogram );
 void Histogram_PrintBins(Histogram_c histogram );
 void Histogram_Scale(Histogram_c histogram , int totBinCnt);
 void Histogram_SetBinCount(Histogram_c histogram , int binIx, double binVal);
 void Histogram_SumBinCount(Histogram_c histogram , int binIx, double sumValue);
 double Histogram_SumBins(Histogram_c histogram , int startIx, int endIx);

#endif // __rbyproject_histogram__
