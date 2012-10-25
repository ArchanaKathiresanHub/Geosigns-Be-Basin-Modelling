///////////////////////////////////////////////////////////////////////////////
//                                VERSION:1.0                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_aftcalc___
#define __rbyproject_aftcalc___
///////////////////////////////////////////////////////////////////////////////
//                            AFTCALC definitions                            //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_rubyproject__
#include   "rubyproject.h"
#endif
class CalcEngine;  // Forward reference
class FtSample;  // Forward reference
class PropDoubleVector;  // Forward reference
class Polyfunction;
class FtGrainIoTbl;
class FtSampleIoTbl;
class HistogramTbl;
class FtLengthIoTbl;
class StratIoTbl;


///////////////////////////////////////////////////////////////////////////////
//                             AFTCALC Structure                             //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbyproject_aftcalc_class___
#define __rbyproject_aftcalc_class___
typedef class AftCalc
{
public:
   static AftCalc *		Current;
   CalcEngine *         CALCENGINE;
   List_c               FtSamples;
   Polyfunction *       L0;
   PropDoubleVector *   C0;
   PropDoubleVector *   C1;
   typedef enum { LENGTHDISTSCALINGMODE_SCALETOOBSERVED = 1, LENGTHDISTSCALINGMODE_SCALETOTRACKNUMDIST, LENGTHDISTSCALINGMODE_NOSCALING } LENGTHDISTSCALINGMODESwitch;
   LENGTHDISTSCALINGMODESwitch LENGTHDISTSCALINGMODE;
   int                  TRACKNUMDIST;
// member functions
private:
public:
   void Add_FtSamples( FtSample * member );
    AftCalc( BOOLEAN_t alloc = FALSE );
   void BinFtGrainAges( FtGrainIoTbl * FtGrainIoTbl );
   void BinFtLengths( FtSampleIoTbl * FtSampleIoTbl, FtLengthIoTbl * FtLengthIoTbl, FtGrainIoTbl * FtGrainIoTbl );
   static float CalcFtAge( double NsDivNi, double zeta, double UstglTrDens );
   static void CalcFtAgeChi2( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int totNs, int totNi, float* Chi2 );
   static void CalcFtCentralAge( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int grainNum, int totNs, int totNi, double UstglTrDens, double zeta, float* CentralAge, float* error );
   static void CalcFtCorrCoeff( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int totNs, int totNi, int grainNum, float* CorrCoeff );
   void CalcFtGrainAges( FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl );
   void CalcFtLengthChi2( FtSampleIoTbl * FtSampleIoTbl );
   static void CalcFtMeanAge( double zeta, double zetaErr, double UstglTrDens, float MeanRatio, float MeanRatioErr, float* MeanAge, float* error );
   static void CalcFtMeanRatio( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int grainNum, float* MeanRatio, float* error );
   static void CalcFtNsDivNi( int totNs, int totNi, float* NsDivNi, float* error );
   static void CalcFtP_Chi2( int dof, float Chi2, float* P_Chi2 );
   static void CalcFtPooledAge( double zeta, double zetaErr, double UstglTrDens, int totNs, int totNi, float* PooledAge, float* error );
   void CalcFtSampleData( FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl );
   static void CalcFtVarianceSqrtNiNs( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int grainNum, float* VarianceSqrtNi, float* VarianceSqrtNs );
   void CalcPredFtAge( FtSampleIoTbl * FtSampleIoTbl );
   void CalcPredTracklengths( double time, double timestep );
   static AftCalc * Create( BOOLEAN_t alloc = FALSE );
   void Delete(void );
   void DeleteModel(void );
   void Finalize(void );
   FtSample * FindFtSample( STRING sampleId );
   void ForAll_FtSamples( ActionMethod Action, long* arg );
   static BOOLEAN_t FtSamples_GtFunc( FtSample * ftsample1, FtSample * ftsample2 );
   static float Gauss( float x );
   void GeotrackCorrPredTracklengthDists(void );
   double GetC0( int ix );
   double GetC1( int ix );
   static AftCalc * GetCurrent (void);
   BOOLEAN_t GetFtHistogramPlots( STRING sampleId, int ChlorideIx, HistogramTbl * ObsDist, HistogramTbl * PredDist, int* MaxTrackCount, HistogramTbl * AgeDist, int* MaxGrainCount );
   static void GetFtSampleCounts( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId,  int*  grainNum,  int*  totNs,  int*  totNi );
   double GetL0( int ClIndex );
   void Init(void );
   void Initialize(void );
   static AftCalc * Make(void );
   void MakeFtSamples( FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl, StratIoTbl * StratIoTbl );
   void MergeTracklengthDists(void );
   void Print( long* arg );
   void Remove_FtSamples( FtSample * member );
   void ScaleObsTracklengthDists( int tracknum );
   void ScalePredTracklenghtDists( int tracknum );
   void ScalePredTracklengthsToObserved(void );
   void ScaleTracklengthDists(void );
   void SetFtIoTbls( FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl, StratIoTbl * StratIoTbl );
   virtual  ~AftCalc(void );
} AftCalc_t, *AftCalc_c;
#endif // __rbyproject_aftcalc_class___

///////////////////////////////////////////////////////////////////////////////
//                      AFTCALC Public Attribute Macros                      //
///////////////////////////////////////////////////////////////////////////////

#define AftCalc_CALCENGINE(obj)			((obj)->CALCENGINE)
#define AftCalc_FtSamples(obj)			((obj)->FtSamples)
#define AftCalc_IteratorInit_FtSamples(obj)	(ListInitNextItem(AftCalc_FtSamples(obj)))
#define AftCalc_NbItem_FtSamples(obj)		(LISTNbItem(AftCalc_FtSamples(obj)))
#define AftCalc_Empty_FtSamples(obj)		(LISTEmpty(AftCalc_FtSamples(obj)))
#define AftCalc_Increment_FtSamples(obj)		(LISTIncr(AftCalc_FtSamples(obj)))
#define AftCalc_First_FtSamples(obj)		((FtSample *)ListFirstItem(AftCalc_FtSamples(obj)))
#define AftCalc_Last_FtSamples(obj)		((FtSample *)ListLastItem(AftCalc_FtSamples(obj)))
#define AftCalc_DestroyContents_FtSamples(obj,f)	((FtSample *)ListDestroyContents(AftCalc_FtSamples(obj),f))
#define AftCalc_IteratorClear_FtSamples(it)	ListIteratorClear(&it)
#define AftCalc_ForEach_FtSamples(obj,method)	\
  {						\
  ITERATOR_t it = AftCalc_IteratorInit_FtSamples(obj);	\
  FtSample * ftsample;					\
  while ( ftsample = (FtSample *) Next(it) ) {method;}	\
  AftCalc_IteratorClear_FtSamples(it);				\
  }
#define AftCalc_L0(obj)			((obj)->L0)
#define AftCalc_C0(obj)			((obj)->C0)
#define AftCalc_C1(obj)			((obj)->C1)
#define AftCalc_LENGTHDISTSCALINGMODE(obj)			((obj)->LENGTHDISTSCALINGMODE)
#define AftCalc_LENGTHDISTSCALINGMODE_Set_Mode_SCALETOOBSERVED(obj)		((obj)->LENGTHDISTSCALINGMODE = AftCalc::LENGTHDISTSCALINGMODE_SCALETOOBSERVED)
#define If_AftCalc_LENGTHDISTSCALINGMODE_SCALETOOBSERVED(obj)			((obj)->LENGTHDISTSCALINGMODE == AftCalc::LENGTHDISTSCALINGMODE_SCALETOOBSERVED)
#define AftCalc_LENGTHDISTSCALINGMODE_Set_Mode_SCALETOTRACKNUMDIST(obj)		((obj)->LENGTHDISTSCALINGMODE = AftCalc::LENGTHDISTSCALINGMODE_SCALETOTRACKNUMDIST)
#define If_AftCalc_LENGTHDISTSCALINGMODE_SCALETOTRACKNUMDIST(obj)			((obj)->LENGTHDISTSCALINGMODE == AftCalc::LENGTHDISTSCALINGMODE_SCALETOTRACKNUMDIST)
#define AftCalc_LENGTHDISTSCALINGMODE_Set_Mode_NOSCALING(obj)		((obj)->LENGTHDISTSCALINGMODE = AftCalc::LENGTHDISTSCALINGMODE_NOSCALING)
#define If_AftCalc_LENGTHDISTSCALINGMODE_NOSCALING(obj)			((obj)->LENGTHDISTSCALINGMODE == AftCalc::LENGTHDISTSCALINGMODE_NOSCALING)
#define AftCalc_TRACKNUMDIST(obj)			((obj)->TRACKNUMDIST)



///////////////////////////////////////////////////////////////////////////////
//                         Access For Public Methods                         //
///////////////////////////////////////////////////////////////////////////////

 void AftCalc_Add_FtSamples(AftCalc * aftcalc , FtSample * member);
 void AftCalc_BinFtGrainAges(AftCalc * aftcalc , FtGrainIoTbl * FtGrainIoTbl);
 void AftCalc_BinFtLengths(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl, FtLengthIoTbl * FtLengthIoTbl, FtGrainIoTbl * FtGrainIoTbl);
 float CalcFtAge( double NsDivNi, double zeta, double UstglTrDens);
 void CalcFtAgeChi2( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int totNs, int totNi, float* Chi2);
 void CalcFtCentralAge( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int grainNum, int totNs, int totNi, double UstglTrDens, double zeta, float* CentralAge, float* error);
 void CalcFtCorrCoeff( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int totNs, int totNi, int grainNum, float* CorrCoeff);
 void AftCalc_CalcFtGrainAges(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl);
 void AftCalc_CalcFtLengthChi2(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl);
 void CalcFtMeanAge( double zeta, double zetaErr, double UstglTrDens, float MeanRatio, float MeanRatioErr, float* MeanAge, float* error);
 void CalcFtMeanRatio( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int grainNum, float* MeanRatio, float* error);
 void CalcFtNsDivNi( int totNs, int totNi, float* NsDivNi, float* error);
 void CalcFtP_Chi2( int dof, float Chi2, float* P_Chi2);
 void CalcFtPooledAge( double zeta, double zetaErr, double UstglTrDens, int totNs, int totNi, float* PooledAge, float* error);
 void AftCalc_CalcFtSampleData(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl);
 void CalcFtVarianceSqrtNiNs( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId, int grainNum, float* VarianceSqrtNi, float* VarianceSqrtNs);
 void AftCalc_CalcPredFtAge(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl);
 void AftCalc_CalcPredTracklengths(AftCalc * aftcalc , double time, double timestep);
 AftCalc * AftCalc_Create( BOOLEAN_t alloc = FALSE);
 void AftCalc_Delete(AftCalc * aftcalc );
 void AftCalc_DeleteModel(AftCalc * aftcalc );
 void AftCalc_Finalize(AftCalc * aftcalc );
 FtSample * AftCalc_FindFtSample(AftCalc * aftcalc , STRING sampleId);
 void AftCalc_ForAll_FtSamples(AftCalc * aftcalc , ActionMethod Action, long* arg);
 BOOLEAN_t AftCalc_FtSamples_GtFunc( FtSample * ftsample1, FtSample * ftsample2);
 float Gauss( float x);
 void AftCalc_GeotrackCorrPredTracklengthDists(AftCalc * aftcalc );
 double AftCalc_GetC0(AftCalc * aftcalc , int ix);
 double AftCalc_GetC1(AftCalc * aftcalc , int ix);
 BOOLEAN_t AftCalc_GetFtHistogramPlots(AftCalc * aftcalc , STRING sampleId, int ChlorideIx, HistogramTbl * ObsDist, HistogramTbl * PredDist, int* MaxTrackCount, HistogramTbl * AgeDist, int* MaxGrainCount);
 void GetFtSampleCounts( FtGrainIoTbl * FtGrainIoTbl, STRING sampleId,  int*  grainNum,  int*  totNs,  int*  totNi);
 double AftCalc_GetL0(AftCalc * aftcalc , int ClIndex);
 void AftCalc_Init(AftCalc * aftcalc );
 void AftCalc_Initialize(AftCalc * aftcalc );
 AftCalc * AftCalc_Make(void);
 void AftCalc_MakeFtSamples(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl, StratIoTbl * StratIoTbl);
 void AftCalc_MergeTracklengthDists(AftCalc * aftcalc );
 void AftCalc_Print(AftCalc * aftcalc , long* arg);
 void AftCalc_Remove_FtSamples(AftCalc * aftcalc , FtSample * member);
 void AftCalc_ScaleObsTracklengthDists(AftCalc * aftcalc , int tracknum);
 void AftCalc_ScalePredTracklenghtDists(AftCalc * aftcalc , int tracknum);
 void AftCalc_ScalePredTracklengthsToObserved(AftCalc * aftcalc );
 void AftCalc_ScaleTracklengthDists(AftCalc * aftcalc );
 void AftCalc_SetFtIoTbls(AftCalc * aftcalc , FtSampleIoTbl * FtSampleIoTbl, FtGrainIoTbl * FtGrainIoTbl, FtLengthIoTbl * FtLengthIoTbl, StratIoTbl * StratIoTbl);

#endif // __rbyproject_aftcalc__
