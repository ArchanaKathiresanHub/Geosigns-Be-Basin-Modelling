///////////////////////////////////////////////////////////////////////////////
//                                VERSION:1.0                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_ftsample___
#define __rbyproject_ftsample___
///////////////////////////////////////////////////////////////////////////////
//                           FTSAMPLE definitions                            //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_rubyproject__
#include   "rubyproject.h"
#endif
class PropDoubleVector;  // Forward reference
class AftCalc;  // Forward reference
class Histogram;  // Forward reference
class FtGrainIoTbl;  // Forward reference
class HistogramTbl;  // Forward reference
class FtLengthIoTbl;  // Forward reference
class StratIoTbl;  // Forward reference


///////////////////////////////////////////////////////////////////////////////
//                            FTSAMPLE Structure                             //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_ftsample_class___
#define __rbyproject_ftsample_class___
typedef class FtSample
{
public:
   STRING               SAMPLECODE;
   float                DEPTH;
   float                DEPOAGE;
   int                  TOPNODEID;
   int                  BOTTOMNODEID;
   float                LENGTHFACTOR;
   BOOLEAN_t            SAMPLELOCATIONSET;
   PropDoubleVector *   TMPBINCNT;
   AftCalc *            AFTCALC;
   DaList_c             ObsTracklengths;
   DaList_c             PredTracklengths;
   Histogram *          GRAINFTAGEDIST;
   DaList_c             FtGrainAges;
// member functions
private:
public:
   void Add_FtGrainAges( Histogram * member );
   void Add_ObsTracklengths( Histogram * member );
   void Add_PredTracklengths( Histogram * member );
   void BinFtGrainAge( float ClPerc, float grainAge );
   void BinObsTracklengths( float tracklength, float ClPerc );
   void CalcFtLengthChi2( float* Chi2 );
   BOOLEAN_t CalcPredFtAge( float* PredFtAge );
   void CalcPredTracklengths( double timestep, double temperature, int ClIndex );
   static FtSample * Create( BOOLEAN_t alloc = FALSE );
   void Delete(void );
   static double DensityFromR( double r );
   void ForAll_FtGrainAges( ActionMethod Action, long* arg );
   void ForAll_ObsTracklengths( ActionMethod Action, long* arg );
   void ForAll_PredTracklengths( ActionMethod Action, long* arg );
   static BOOLEAN_t FtGrainAges_GtFunc( Histogram * histogram1, Histogram * histogram2 );
    FtSample( BOOLEAN_t alloc = FALSE );
   void GeotrackCorrPredTracklengthDists(void );
   float GetClContFraction( int ClBin, FtGrainIoTbl * FtGrainIoTbl );
   void GetFrom_FtGrainAges( Histogram ** member, long key );
   void GetFrom_ObsTracklengths( Histogram ** member, long key );
   void GetFrom_PredTracklengths( Histogram ** member, long key );
   void GetGrainAgeDist( int ChlorideIx, HistogramTbl * HistogramTbl );
   void GetMaxBinCount( double* MaxTracknum, double* MaxGrainnum );
   void GetObsTracklengthDist( int ChlorideIx, HistogramTbl * HistogramTbl );
   void GetPredTracklengthDist( int ChlorideIx, HistogramTbl * HistogramTbl );
   void GetSumGrainAgeDist( HistogramTbl * HistogramTbl, double* MaxBinCnt );
   void GetSumObsTracklengthDist( HistogramTbl * HistogramTbl, double* MaxBinCnt );
   void GetSumPredTracklengthDist( HistogramTbl * HistogramTbl, double* MaxBinCnt );
   BOOLEAN_t GetTemperature( double* temperature );
   static int IndexFromClPerc( float ClPerc );
   void Init(void );
   void Initialize( STRING sampleId, float depth, FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl, StratIoTbl * StratIoTbl );
   static FtSample * Make(void );
   void MakeDistributions( FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl );
   void MergeTracklengthDists(void );
   static BOOLEAN_t ObsTracklengths_GtFunc( Histogram * histogram1, Histogram * histogram2 );
   static BOOLEAN_t PredTracklengths_GtFunc( Histogram * histogram1, Histogram * histogram2 );
   void Print( long* arg );
   void PrintFtGrainAges(void );
   void PrintObsTracklengths(void );
   void PrintPredTracklengths(void );
   void Remove_FtGrainAges( Histogram * member );
   void Remove_ObsTracklengths( Histogram * member );
   void Remove_PredTracklengths( Histogram * member );
   void ScaleObsTracklengthDists( int tracknum );
   void ScalePredTracklenghtDists( int tracknum );
   void ScalePredTracklengthsToObserved(void );
   BOOLEAN_t SetFtSampleLocation( StratIoTbl * StratIoTbl );
   void SetIn_FtGrainAges( Histogram * member, long key );
   void SetIn_ObsTracklengths( Histogram * member, long key );
   void SetIn_PredTracklengths( Histogram * member, long key );
   void SumFtGrainAges( Histogram * sumHistogram );
   void SumObsTracklengthDists( Histogram * sumHistogram );
   void SumPredTracklengthDists( Histogram * sumHistogram );
   static double equivalentTime( double temperature, double h );
   static double hBinIndex( double h, int binnum, double L0, double c0, double c1 );
   static void hBinPrevious( int ix, int binnum, double L0, double c0, double c1, double temperature, double timestep, double* hPrev, int* ixPrev, double* tPrev );
   static double hValue( int ix, double L0, double c0, double c1 );
   static double prob( double z );
   virtual  ~FtSample(void );
} FtSample_t, *FtSample_c;
#endif // __rbyproject_ftsample_class___

///////////////////////////////////////////////////////////////////////////////
//                     FTSAMPLE Public Attribute Macros                      //
///////////////////////////////////////////////////////////////////////////////

#define FtSample_SAMPLECODE(obj)			((obj)->SAMPLECODE)
#define FtSample_DEPTH(obj)			((obj)->DEPTH)
#define FtSample_DEPOAGE(obj)			((obj)->DEPOAGE)
#define FtSample_TOPNODEID(obj)			((obj)->TOPNODEID)
#define FtSample_BOTTOMNODEID(obj)			((obj)->BOTTOMNODEID)
#define FtSample_LENGTHFACTOR(obj)			((obj)->LENGTHFACTOR)
#define FtSample_SAMPLELOCATIONSET(obj)			((obj)->SAMPLELOCATIONSET)
#define FtSample_TMPBINCNT(obj)			((obj)->TMPBINCNT)
#define FtSample_AFTCALC(obj)			((obj)->AFTCALC)
#define FtSample_ObsTracklengths(obj)			((obj)->ObsTracklengths)
#define FtSample_IteratorInit_ObsTracklengths(obj)	(DaListInitNextItem(FtSample_ObsTracklengths(obj)))
#define FtSample_NbItem_ObsTracklengths(obj)		(DALISTNbItem(FtSample_ObsTracklengths(obj)))
#define FtSample_Empty_ObsTracklengths(obj)		(DALISTEmpty(FtSample_ObsTracklengths(obj)))
#define FtSample_Increment_ObsTracklengths(obj)		(DALISTIncr(FtSample_ObsTracklengths(obj)))
#define FtSample_First_ObsTracklengths(obj)		((Histogram *)DaListFirstItem(FtSample_ObsTracklengths(obj)))
#define FtSample_Last_ObsTracklengths(obj)		((Histogram *)DaListLastItem(FtSample_ObsTracklengths(obj)))
#define FtSample_DestroyContents_ObsTracklengths(obj,f)	((Histogram *)DaListDestroyContents(FtSample_ObsTracklengths(obj),f))
#define FtSample_IteratorClear_ObsTracklengths(it)	DaListIteratorClear(&it)
#define FtSample_ForEach_ObsTracklengths(obj,method)	\
  {						\
  ITERATOR_t it = FtSample_IteratorInit_ObsTracklengths(obj);	\
  Histogram * histogram;					\
  while ( histogram = (Histogram *) Next(it) ) {method;}	\
  FtSample_IteratorClear_ObsTracklengths(it);				\
  }
#define FtSample_PredTracklengths(obj)			((obj)->PredTracklengths)
#define FtSample_IteratorInit_PredTracklengths(obj)	(DaListInitNextItem(FtSample_PredTracklengths(obj)))
#define FtSample_NbItem_PredTracklengths(obj)		(DALISTNbItem(FtSample_PredTracklengths(obj)))
#define FtSample_Empty_PredTracklengths(obj)		(DALISTEmpty(FtSample_PredTracklengths(obj)))
#define FtSample_Increment_PredTracklengths(obj)		(DALISTIncr(FtSample_PredTracklengths(obj)))
#define FtSample_First_PredTracklengths(obj)		((Histogram *)DaListFirstItem(FtSample_PredTracklengths(obj)))
#define FtSample_Last_PredTracklengths(obj)		((Histogram *)DaListLastItem(FtSample_PredTracklengths(obj)))
#define FtSample_DestroyContents_PredTracklengths(obj,f)	((Histogram *)DaListDestroyContents(FtSample_PredTracklengths(obj),f))
#define FtSample_IteratorClear_PredTracklengths(it)	DaListIteratorClear(&it)
#define FtSample_ForEach_PredTracklengths(obj,method)	\
  {						\
  ITERATOR_t it = FtSample_IteratorInit_PredTracklengths(obj);	\
  Histogram * histogram;					\
  while ( histogram = (Histogram *) Next(it) ) {method;}	\
  FtSample_IteratorClear_PredTracklengths(it);				\
  }
#define FtSample_GRAINFTAGEDIST(obj)			((obj)->GRAINFTAGEDIST)
#define FtSample_FtGrainAges(obj)			((obj)->FtGrainAges)
#define FtSample_IteratorInit_FtGrainAges(obj)	(DaListInitNextItem(FtSample_FtGrainAges(obj)))
#define FtSample_NbItem_FtGrainAges(obj)		(DALISTNbItem(FtSample_FtGrainAges(obj)))
#define FtSample_Empty_FtGrainAges(obj)		(DALISTEmpty(FtSample_FtGrainAges(obj)))
#define FtSample_Increment_FtGrainAges(obj)		(DALISTIncr(FtSample_FtGrainAges(obj)))
#define FtSample_First_FtGrainAges(obj)		((Histogram *)DaListFirstItem(FtSample_FtGrainAges(obj)))
#define FtSample_Last_FtGrainAges(obj)		((Histogram *)DaListLastItem(FtSample_FtGrainAges(obj)))
#define FtSample_DestroyContents_FtGrainAges(obj,f)	((Histogram *)DaListDestroyContents(FtSample_FtGrainAges(obj),f))
#define FtSample_IteratorClear_FtGrainAges(it)	DaListIteratorClear(&it)
#define FtSample_ForEach_FtGrainAges(obj,method)	\
  {						\
  ITERATOR_t it = FtSample_IteratorInit_FtGrainAges(obj);	\
  Histogram * histogram;					\
  while ( histogram = (Histogram *) Next(it) ) {method;}	\
  FtSample_IteratorClear_FtGrainAges(it);				\
  }



///////////////////////////////////////////////////////////////////////////////
//                         Access For Public Methods                         //
///////////////////////////////////////////////////////////////////////////////

 void FtSample_Add_FtGrainAges(FtSample * ftsample , Histogram * member);
 void FtSample_Add_ObsTracklengths(FtSample * ftsample , Histogram * member);
 void FtSample_Add_PredTracklengths(FtSample * ftsample , Histogram * member);
 void FtSample_BinFtGrainAge(FtSample * ftsample , float ClPerc, float grainAge);
 void FtSample_BinObsTracklengths(FtSample * ftsample , float tracklength, float ClPerc);
 void FtSample_CalcFtLengthChi2(FtSample * ftsample , float* Chi2);
 BOOLEAN_t FtSample_CalcPredFtAge(FtSample * ftsample , float* PredFtAge);
 void FtSample_CalcPredTracklengths(FtSample * ftsample , double timestep, double temperature, int ClIndex);
 FtSample * FtSample_Create( BOOLEAN_t alloc = FALSE);
 void FtSample_Delete(FtSample * ftsample );
 double DensityFromR( double r);
 void FtSample_ForAll_FtGrainAges(FtSample * ftsample , ActionMethod Action, long* arg);
 void FtSample_ForAll_ObsTracklengths(FtSample * ftsample , ActionMethod Action, long* arg);
 void FtSample_ForAll_PredTracklengths(FtSample * ftsample , ActionMethod Action, long* arg);
 BOOLEAN_t FtSample_FtGrainAges_GtFunc( Histogram * histogram1, Histogram * histogram2);
 void FtSample_GeotrackCorrPredTracklengthDists(FtSample * ftsample );
 float FtSample_GetClContFraction(FtSample * ftsample , int ClBin, FtGrainIoTbl * FtGrainIoTbl);
 void FtSample_GetFrom_FtGrainAges(FtSample * ftsample , Histogram ** member, long key);
 void FtSample_GetFrom_ObsTracklengths(FtSample * ftsample , Histogram ** member, long key);
 void FtSample_GetFrom_PredTracklengths(FtSample * ftsample , Histogram ** member, long key);
 void FtSample_GetGrainAgeDist(FtSample * ftsample , int ChlorideIx, HistogramTbl * HistogramTbl);
 void FtSample_GetMaxBinCount(FtSample * ftsample , double* MaxTracknum, double* MaxGrainnum);
 void FtSample_GetObsTracklengthDist(FtSample * ftsample , int ChlorideIx, HistogramTbl * HistogramTbl);
 void FtSample_GetPredTracklengthDist(FtSample * ftsample , int ChlorideIx, HistogramTbl * HistogramTbl);
 void FtSample_GetSumGrainAgeDist(FtSample * ftsample , HistogramTbl * HistogramTbl, double* MaxBinCnt);
 void FtSample_GetSumObsTracklengthDist(FtSample * ftsample , HistogramTbl * HistogramTbl, double* MaxBinCnt);
 void FtSample_GetSumPredTracklengthDist(FtSample * ftsample , HistogramTbl * HistogramTbl, double* MaxBinCnt);
 BOOLEAN_t FtSample_GetTemperature(FtSample * ftsample , double* temperature);
 int IndexFromClPerc( float ClPerc);
 void FtSample_Init(FtSample * ftsample );
 void FtSample_Initialize(FtSample * ftsample , STRING sampleId, float depth, FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl, StratIoTbl * StratIoTbl);
 FtSample * FtSample_Make(void);
 void FtSample_MakeDistributions(FtSample * ftsample , FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl);
 void FtSample_MergeTracklengthDists(FtSample * ftsample );
 BOOLEAN_t FtSample_ObsTracklengths_GtFunc( Histogram * histogram1, Histogram * histogram2);
 BOOLEAN_t FtSample_PredTracklengths_GtFunc( Histogram * histogram1, Histogram * histogram2);
 void FtSample_Print(FtSample * ftsample , long* arg);
 void FtSample_PrintFtGrainAges(FtSample * ftsample );
 void FtSample_PrintObsTracklengths(FtSample * ftsample );
 void FtSample_PrintPredTracklengths(FtSample * ftsample );
 void FtSample_Remove_FtGrainAges(FtSample * ftsample , Histogram * member);
 void FtSample_Remove_ObsTracklengths(FtSample * ftsample , Histogram * member);
 void FtSample_Remove_PredTracklengths(FtSample * ftsample , Histogram * member);
 void FtSample_ScaleObsTracklengthDists(FtSample * ftsample , int tracknum);
 void FtSample_ScalePredTracklenghtDists(FtSample * ftsample , int tracknum);
 void FtSample_ScalePredTracklengthsToObserved(FtSample * ftsample );
 BOOLEAN_t FtSample_SetFtSampleLocation(FtSample * ftsample , StratIoTbl * StratIoTbl);
 void FtSample_SetIn_FtGrainAges(FtSample * ftsample , Histogram * member, long key);
 void FtSample_SetIn_ObsTracklengths(FtSample * ftsample , Histogram * member, long key);
 void FtSample_SetIn_PredTracklengths(FtSample * ftsample , Histogram * member, long key);
 void FtSample_SumFtGrainAges(FtSample * ftsample , Histogram * sumHistogram);
 void FtSample_SumObsTracklengthDists(FtSample * ftsample , Histogram * sumHistogram);
 void FtSample_SumPredTracklengthDists(FtSample * ftsample , Histogram * sumHistogram);
 double equivalentTime( double temperature, double h);
 double hBinIndex( double h, int binnum, double L0, double c0, double c1);
 void hBinPrevious( int ix, int binnum, double L0, double c0, double c1, double temperature, double timestep, double* hPrev, int* ixPrev, double* tPrev);
 double hValue( int ix, double L0, double c0, double c1);
 double prob( double z);

#endif // __rbyproject_ftsample__
