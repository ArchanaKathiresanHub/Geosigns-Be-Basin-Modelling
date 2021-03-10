#ifndef _MAP_INTERPOLATOR_H_
#define _MAP_INTERPOLATOR_H_


#include "petscdmda.h"
#include "petscvec.h"

#include "CauldronGridDescription.h"
#include "PetscVectors.h"

using namespace GeoPhysics;

class MapInterpolator {

public :

  MapInterpolator ();

  ~MapInterpolator ();


  void operator ()( DM    fromDA,
                    Vec   fromVec,
                    const CauldronGridDescription& fromDescription,
                    DM    toDA,
                    Vec   toVec,
                    const CauldronGridDescription& toDescription ) const;

  void interpolateFromHighResolutionMap ( DM    highResDA,
                                          Vec   highResVec,
                                          const CauldronGridDescription& highResDescription,
                                          DM    lowResDA,
                                          Vec   lowResVec,
                                          const CauldronGridDescription& lowResDescription ) const;

  void interpolateFromLowResolutionMap ( DM    lowResDA,
                                         Vec   lowResVec,
                                         const CauldronGridDescription& lowResDescription,
                                         DM    highResDA,
                                         Vec   highResVec,
                                         const CauldronGridDescription& highResDescription ) const;

private :

  #ifdef INTERPOLATION_INDEXING_FIXED
  double getReferenceValue ( const int     highResIndex,
                             const int     highResStartIncrement,
                             const int     subsamplingRatio,
                             const int     maxLowResIndex,
                             const double* referenceValues ) const;
  #endif

  void setReferenceArray ( const int      resolutionDelta,
                                 double*& referenceValues ) const;

  /// Interpolation functions defined on a reference element [-1,1]x[-1,1]
  void setBasis ( const double  x,
                  const double  y,
                        double* basis ) const;

  /// 
  void getProperty ( const PETSC_3D_Array& propertyVec,
                     const int             startI,
                     const int             startJ,
                     const int             K,
                           double*         property ) const;

  bool containsNullValue ( const double* property ) const;

  double innerProduct ( const double* bases, 
                        const double* property ) const;

};


#endif // _MAP_INTERPOLATOR_H_
