#ifndef _SNAPSHOT_INTERPOLATOR_H_
#define _SNAPSHOT_INTERPOLATOR_H_

#include <string>
#include <vector>
#include <map>

#include "petscts.h"
#include "petscda.h"

#include "array.h"

#include "petscvector_readwrite.h"
#include "petscdimensions.h"
#include "h5_file_types.h"

#include "snapshotdata.h"
#include "PetscVectors.h"

#include "utils.h"
#include "globaldefs.h"

#include "FastcauldronSimulator.h"

#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/Interface.h"

using namespace DataAccess;


typedef std::vector < double > DoubleArray;

///
/// Compute a piecewise linear interpolant of the property. A contiguous chunk of memory
/// is used to store the coefficients. This memory is stored elsewhere (i.e. NOT IN THIS CLASS)
///
class PiecewiseLinearInterpolantCalculator {

public :

  PiecewiseLinearInterpolantCalculator ( const int panelCount );

  ~PiecewiseLinearInterpolantCalculator ();

  int maximumNumberOfPanels () const;

  /// Returns the number of doubles required for storage and computation of the interpolant.
  int numberOfCoefficientsRequired () const;

  void clearTimes ();

  /// Add a snapshot time that is to be interpolated.
  void addTime ( const double newTime );

  int numberOfSnapshotTimes () const;

  /// Compute the interpolant at the times stores in the snapshotTimes array (added beforehand)
  /// and store the values in the memory chunk.
  void computeInterpolant ( const DoubleArray& propertyValues,
                                  double*      memoryChunk );

  /// Find in which panel the current time is.
  void setCurrentTime ( const double newTime );

  /// Evaluate the interpolant (described by the values contained in the memory chunk) at the current time (set beforehand).
  double operator ()( const double* memoryChunk ) const;

private :

  const int m_maximumNumberOfPanels;

  DoubleArray snapshotTimes;
  double      currentTime;
  int         currentPanel;

};

//------------------------------------------------------------//

///
/// Compute a constant interpolant of the property. A contiguous chunk of memory
/// is used to store the coefficients. This memory is stored elsewhere (i.e. NOT IN THIS CLASS)
///
class ConstantInterpolantCalculator {

public :

  ConstantInterpolantCalculator ( const int panelCount );

  ~ConstantInterpolantCalculator ();

  int maximumNumberOfPanels () const;

  /// Returns the number of double required for storage and computation of the interpolant.
  int numberOfCoefficientsRequired () const;

  void clearTimes ();

  /// Add a snapshot time that is to be interpolated.
  void addTime ( const double newTime );

  int numberOfSnapshotTimes () const;

  /// Compute the interpolant at the times stored in the snapshotTimes array (added beforehand)
  /// and store the values in the memory chunk.
  void computeInterpolant ( const DoubleArray& propertyValues,
                                  double*      memoryChunk );

  /// Find in which panel the current time is.
  void setCurrentTime ( const double newTime );

  /// Evaluate the interpolant (described by the values contained in the memory chunk) at the current time (set beforehand).
  double operator ()( const double* memoryChunk ) const;

private :

  double snapshotTime;
  double currentTime;

};

//------------------------------------------------------------//


typedef double**** CoefficientArray;

typedef float***   SavedPropertyValuesArray;

typedef std::vector < SavedPropertyValuesArray > SavedPropertyValuesArrayVector;


class SnapshotInterpolator {

public :

  SnapshotInterpolator () {}

  virtual ~SnapshotInterpolator () {}


  virtual void setPropertyNames ( const std::string& newLayerName,
                                  const std::string& newPropertyName );

  virtual void setDA ( const DA  newDA ) = 0;

  virtual void setInterval ( const SnapshotInterval& interval,
                             const std::string&      directoryName,
                             const std::string&      formationName,
                             const std::string&      nameOfProperty ) = 0;

  virtual void setCurrentTime ( const double newTime ) = 0;


  virtual double operator ()( const int I, const int J, const int K ) const = 0;

protected :

  std::string layerName;
  std::string propertyName;

};

//------------------------------------------------------------//


///
/// The InterpolatorCalculator class is resp
///
template <class InterpolatorCalculator>
class GenericSnapshotInterpolator : public SnapshotInterpolator {

public :

  ///
  /// Default value is used if no data can be found in the file for the corresponding property.
  ///
  GenericSnapshotInterpolator ( const double newDefaultValue,
                                const int    maximumNumberOfPanels );

  ~GenericSnapshotInterpolator ();


  /// Sets the DA used throughout the interpolator and allocates the memory used for the interpolants.
  void setDA ( const DA  newDA );

  /// Read-in the properties for the times specified in the snapshot interval.
  void setInterval ( const SnapshotInterval& interval,
                     const std::string&      directoryName,
                     const std::string&      formationName,
                     const std::string&      nameOfProperty );

  void setCurrentTime ( const double newTime );


  /// Evaluate the interpolant at the position specified ( I, J, K )
  double operator ()( const int I, const int J, const int K ) const;


private :

  void computeInterpolant ();

  /// Retrieve the property values for all snapshot times, for a specific node from the read-in values.
  void getPropertyData ( const int          localI,
                         const int          localJ,
                         const int          localK,
                               DoubleArray& propertyData ) const;

  /// Read-in the property values from the file.
  bool readTimeStepData ( H5_ReadOnly_File&         inFile,
                          DA                        propertyDA,
                          SavedPropertyValuesArray& propertyValues );

#if 0
  void setTimeStepData ( const std::string&        fileName,
                         DA                        propertyDA,
                         SavedPropertyValuesArray& propertyValues );
#endif


  /// The property values read in from the file
  SavedPropertyValuesArrayVector savedProperties;

  /// Which dimension are we working in.
  Petsc_3D                       petscD;

  /// The coefficients of the interpolants.
  CoefficientArray               allCoefficients;

  /// If the property cannot be found in the file, which value should be used.
  double                         defaultValue;

  /// Object that computed the interpolant.
  InterpolatorCalculator         interpolant;

  DA                             layerDA;
  DALocalInfo                    dimensionInfo;

  float*                         localBuffer;
  int                            localBufferSize;

};

typedef GenericSnapshotInterpolator < PiecewiseLinearInterpolantCalculator > LinearSnapshotInterpolator;

typedef GenericSnapshotInterpolator < ConstantInterpolantCalculator >        ConstantSnapshotInterpolator;


typedef std::map < PropertyList, SnapshotInterpolator* > SnapshotIntervalPropertyInterpolatorMap;


//------------------------------------------------------------//


template <class InterpolatorCalculator>
GenericSnapshotInterpolator<InterpolatorCalculator>::GenericSnapshotInterpolator ( const double newDefaultValue,
                                                                                   const int    maximumNumberOfPanels ) : interpolant ( maximumNumberOfPanels ) {

  int I;

  defaultValue = newDefaultValue;

  savedProperties.reserve ( interpolant.maximumNumberOfPanels ());

  for ( I = 0; I < interpolant.maximumNumberOfPanels (); I++ ) {
    savedProperties.push_back ( 0 );
  }

  allCoefficients = 0;
  localBuffer = 0;
}

//------------------------------------------------------------//

template <class InterpolatorCalculator>
GenericSnapshotInterpolator<InterpolatorCalculator>::~GenericSnapshotInterpolator () {

  int I;

  if ( allCoefficients != 0 ) {
    Array<double>::delete4d ( allCoefficients );
  }

  for ( I = 0; I < interpolant.maximumNumberOfPanels (); I++ ) {

    if ( savedProperties [ I ] != 0 ) {
      Array<float>::delete3d ( savedProperties [ I ]);
    }

  }

  if ( localBuffer != 0 ) {
    delete [] localBuffer;
  }

  savedProperties.clear ();
}

//------------------------------------------------------------//

template <class InterpolatorCalculator>
void GenericSnapshotInterpolator<InterpolatorCalculator>::setDA ( const DA  newDA ) {

  int I;

  layerDA = newDA;

  DAGetLocalInfo( layerDA , &dimensionInfo );

  allCoefficients = Array<double>::create4d ( dimensionInfo.xm,
                                              dimensionInfo.ym,
                                              dimensionInfo.zm,
                                              interpolant.numberOfCoefficientsRequired ());

  localBufferSize = dimensionInfo.xm * dimensionInfo.ym * dimensionInfo.zm;
  localBuffer = new float [ localBufferSize ];

  for ( I = 0; I < interpolant.maximumNumberOfPanels (); I++ ) {
    savedProperties [ I ] = Array<float>::create3d ( dimensionInfo.xm, dimensionInfo.ym, dimensionInfo.zm, CAULDRONIBSNULLVALUE );
  }

}

//------------------------------------------------------------//

template <class InterpolatorCalculator>
bool GenericSnapshotInterpolator<InterpolatorCalculator>::readTimeStepData ( H5_ReadOnly_File&         inFile,
                                                                             DA                        propertyDA,
                                                                             SavedPropertyValuesArray& propertyValues ) {


  bool status = false;

  hid_t groupID = inFile.openGroup ( propertyName.c_str ());

  if ( groupID > -1 ) {

    hid_t dataID = inFile.openDataset ( layerName.c_str (), groupID );

    if ( dataID >= 0 ) {
      PetscVector_ReadWrite<float>   propertyReader;
      int I;

      ///
      /// Is there a better way to read in the data from the hdf file?
      ///
      status = propertyReader.collectRawData ( &inFile, dataID, dimensionInfo, &localBuffer, localBufferSize, false );
      //      status = propertyReader.collectRawData ( &inFile, dataID, dimensionInfo, &propertyValues [ 0 ][ 0 ], size );

      if ( status ) {

        for ( I = 0; I < localBufferSize; I++ ) {
          propertyValues [ 0 ][ 0 ][ I ] = localBuffer [ I ];
        }

      }

    }

    inFile.closeDataset ( dataID );
  }

  inFile.closeGroup ( groupID );

  return status;
}

//------------------------------------------------------------//

#if 0
template <class InterpolatorCalculator>
void GenericSnapshotInterpolator<InterpolatorCalculator>::setTimeStepData ( const std::string&        fileName,
                                                                            DA                        propertyDA,
                                                                            SavedPropertyValuesArray& propertyValues ) {

  H5_ReadOnly_File         inFile;
  H5_Parallel_PropertyList pList;

  if ( ! File_Exists ( fileName )) {
    PetscPrintf ( PETSC_COMM_WORLD,
                  "MeSsAgE ERROR GenericSnapshotInterpolator<InterpolatorCalculator>::setInterval   fault file, %s, could not be found\n", 
                  fileName.c_str ());
  }

  inFile.open ( fileName.c_str (), &pList );


  if ( ! readTimeStepData ( inFile, layerDA, savedProperties [ T ])) {
    ///
    /// If data cannot be read from the file for whatever reason, then 
    /// fill the array with the default value.
    ///
    for ( J = 0; J < dimensionInfo.xm * dimensionInfo.ym * dimensionInfo.zm; J++ ) {
      savedProperties [ T ][ 0 ][ 0 ][ J ] = defaultValue;
    }
    
  }

  inFile.close ();
}
#endif

//------------------------------------------------------------//

template <class InterpolatorCalculator>
void GenericSnapshotInterpolator<InterpolatorCalculator>::setInterval ( const SnapshotInterval& interval,
                                                                        const std::string&      directoryName,
                                                                        const std::string&      formationName,
                                                                        const std::string&      nameOfProperty ) {


  unsigned int i;
  unsigned int j;
  unsigned int k;
  unsigned int localI;
  unsigned int localJ;
  unsigned int localK;

  int              T;
  std::string      fileName;

  interpolant.clearTimes ();

  for ( T = 0; T < interval.numberOfSnapshots(); T++ ) {
    interpolant.addTime ( interval ( T ).time ());
  }

  for ( T = 0; T < interval.numberOfSnapshots(); T++ ) {

     const double snapshotAge = interval ( T ).time ();
     const Interface::Formation* formation = FastcauldronSimulator::getInstance ().findFormation ( formationName );
     const Interface::Snapshot*  snapshot  = FastcauldronSimulator::getInstance ().findSnapshot  ( snapshotAge, Interface::MAJOR | Interface::MINOR );
     const Interface::Property*  property  = FastcauldronSimulator::getInstance ().findProperty  ( nameOfProperty );

     Interface::PropertyValueList* properties = FastcauldronSimulator::getInstance ().getPropertyValues ( Interface::FORMATION,
                                                                                                          property,
                                                                                                          snapshot,
                                                                                                          0, formation, 0,
                                                                                                          Interface::VOLUME );

     if ( properties->size () != 0 ) {

        const Interface::PropertyValue* propertyValue = (*properties->begin ());
        const Interface::GridMap* propertyMap = propertyValue->getGridMap ();
        if ( propertyMap != 0 ) {
           propertyMap->retrieveData ();

           for ( k = propertyMap->firstK (), localK = 0; k <= propertyMap->lastK (); ++k, ++localK ) {

              for ( i = propertyMap->firstI (), localI = 0; i <= propertyMap->lastI (); ++i, ++localI ) {

                 for ( j = propertyMap->firstJ (), localJ = 0; j <= propertyMap->lastJ (); ++j, ++localJ ) {
                    savedProperties [ T ][ localI ][ localJ ][ localK ] = static_cast<float>(propertyMap->getValue ( i, j, k ));
                 }

              }

           }

           propertyMap->restoreData ( false );
        } else {

           for ( i = 0; i < (unsigned int)(dimensionInfo.xm * dimensionInfo.ym * dimensionInfo.zm); ++i ) {
              savedProperties [ T ][ 0 ][ 0 ][ i ] = static_cast<float>(defaultValue);
           }

        }

     } else {

        for ( i = 0; i < (unsigned int)(dimensionInfo.xm * dimensionInfo.ym * dimensionInfo.zm); ++i ) {
           savedProperties [ T ][ 0 ][ 0 ][ i ] = static_cast<float>(defaultValue);
        }

     }

  }

  computeInterpolant ();
}

//------------------------------------------------------------//

template <class InterpolatorCalculator>
void GenericSnapshotInterpolator<InterpolatorCalculator>::setCurrentTime ( const double newTime ) {
  interpolant.setCurrentTime ( newTime );
}

//------------------------------------------------------------//

template <class InterpolatorCalculator>
void GenericSnapshotInterpolator<InterpolatorCalculator>::getPropertyData ( const int          localI,
                                                                            const int          localJ,
                                                                            const int          localK,
                                                                                  DoubleArray& propertyData ) const {

  int T;

  propertyData.clear ();

  for ( T = 0; T < interpolant.numberOfSnapshotTimes (); T++ ) {
    propertyData.push_back ( savedProperties [ T ][ localI ][ localJ ][ localK ]);
  }

}

//------------------------------------------------------------//

template <class InterpolatorCalculator>
void GenericSnapshotInterpolator<InterpolatorCalculator>::computeInterpolant () {

  int I;
  int J;
  int K;

  DoubleArray propertyData ( interpolant.maximumNumberOfPanels ());

  for ( I = 0; I < dimensionInfo.xm; I++ ){

    for ( J = 0; J < dimensionInfo.ym; J++ ){

      for ( K = 0; K < dimensionInfo.zm; K++ ){
        getPropertyData ( I, J, K, propertyData );
        interpolant.computeInterpolant ( propertyData, allCoefficients [ I ][ J ][ K ] );
      }

    }

  }

  propertyData.clear ();
}


//------------------------------------------------------------//


template <class InterpolatorCalculator>
double GenericSnapshotInterpolator<InterpolatorCalculator>::operator ()( const int I, const int J, const int K ) const {

  return interpolant ( allCoefficients [ I - dimensionInfo.xs ][ J - dimensionInfo.ys ][ K - dimensionInfo.zs ]);

}

//------------------------------------------------------------//


#endif // _SNAPSHOT_INTERPOLATOR_H_
