//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__CRUST_FORMATION_H
#define FASTCAULDRON__CRUST_FORMATION_H

// DataAccess library
#include "Interface/CrustFormation.h"
#include "Interface/GridMap.h"

// Geophysics library
#include "GeoPhysicsCrustFormation.h"

// Fastcauldron application
#include "layer.h"


class CrustFormation : virtual public LayerProps, virtual public GeoPhysics::GeoPhysicsCrustFormation {

public :
   CrustFormation ( Interface::ProjectHandle * projectHandle, database::Record * record );

   ~CrustFormation ();

   const string                                        & getName()                    const { return GeoPhysics::GeoPhysicsCrustFormation::getName(); }
   const DataAccess::Interface::LithoType              * getLithoType1()              const { return GeoPhysics::GeoPhysicsCrustFormation::getLithoType1(); }
   const DataAccess::Interface::GridMap                * getLithoType1PercentageMap() const { return GeoPhysics::GeoPhysicsCrustFormation::getLithoType1PercentageMap(); }
   const DataAccess::Interface::LithoType              * getLithoType2()              const { return GeoPhysics::GeoPhysicsCrustFormation::getLithoType2(); }
   const DataAccess::Interface::GridMap                * getLithoType2PercentageMap() const { return GeoPhysics::GeoPhysicsCrustFormation::getLithoType2PercentageMap(); }
   const DataAccess::Interface::LithoType              * getLithoType3()              const { return GeoPhysics::GeoPhysicsCrustFormation::getLithoType3(); }
   const DataAccess::Interface::GridMap                * getLithoType3PercentageMap() const { return GeoPhysics::GeoPhysicsCrustFormation::getLithoType3PercentageMap(); }
   DataAccess::Interface::ReservoirList                * getReservoirs()              const { return GeoPhysics::GeoPhysicsCrustFormation::getReservoirs(); }
   DataAccess::Interface::MobileLayerList              * getMobileLayers()            const { return GeoPhysics::GeoPhysicsCrustFormation::getMobileLayers(); }
   const DataAccess::Interface::AllochthonousLithology * getAllochthonousLithology()  const { return GeoPhysics::GeoPhysicsCrustFormation::getAllochthonousLithology(); }
   DataAccess::Interface::FaultCollectionList          * getFaultCollections()        const { return GeoPhysics::GeoPhysicsCrustFormation::getFaultCollections(); }
   bool                                                  isMobileLayer()              const { return GeoPhysics::GeoPhysicsCrustFormation::isMobileLayer(); }
   bool                                                  hasAllochthonousLithology()  const { return GeoPhysics::GeoPhysicsCrustFormation::hasAllochthonousLithology(); }
   bool                                                  hasConstrainedOverpressure() const { return GeoPhysics::GeoPhysicsCrustFormation::hasConstrainedOverpressure(); }
   bool                                                  hasChemicalCompaction()      const { return GeoPhysics::GeoPhysicsCrustFormation::hasChemicalCompaction(); }
   CBMGenerics::capillarySealStrength::MixModel          getMixModel()                const { return GeoPhysics::GeoPhysicsCrustFormation::getMixModel(); }
   float                                                 getLayeringIndex()           const { return GeoPhysics::GeoPhysicsCrustFormation::getLayeringIndex(); }
   const DataAccess::Interface::FluidType              * getFluidType()               const { return GeoPhysics::GeoPhysicsCrustFormation::getFluidType(); }
   const std::string                                   & getMixModelStr()             const { return GeoPhysics::GeoPhysicsCrustFormation::getMixModelStr(); }
   DataAccess::Interface::GridMap                      * loadThicknessMap()           const { return GeoPhysics::GeoPhysicsCrustFormation::loadThicknessMap(); }
   DataAccess::Interface::GridMap                      * computeThicknessMap()        const { return GeoPhysics::GeoPhysicsCrustFormation::computeThicknessMap(); }
   DataAccess::Interface::GridMap                      * computeFaultGridMap( const DataAccess::Interface::Grid * localGrid, const DataAccess::Interface::Snapshot * snapshot ) const {
      return GeoPhysics::GeoPhysicsCrustFormation::computeFaultGridMap( localGrid, snapshot );
   }
   const DataAccess::Interface::GridMap                * getInputThicknessMap()       const { return GeoPhysics::GeoPhysicsCrustFormation::getInputThicknessMap(); }
   int                                                   getDepositionSequence()      const { return GeoPhysics::GeoPhysicsCrustFormation::getDepositionSequence(); }
   void                                                  asString( string & str )     const { return GeoPhysics::GeoPhysicsCrustFormation::asString( str ); }

   void determineMinMaxThickness()                                                    final { return GeoPhysics::GeoPhysicsCrustFormation::determineMinMaxThickness(); }
   bool isCrust()                                                      const noexcept final { return GeoPhysics::GeoPhysicsCrustFormation::isCrust(); }
   unsigned int setMaximumNumberOfElements( const bool readSizeFromVolumeData )             { return GeoPhysics::GeoPhysicsCrustFormation::setMaximumNumberOfElements( readSizeFromVolumeData ); }
   void retrieveAllThicknessMaps()                                                          { return GeoPhysics::GeoPhysicsCrustFormation::retrieveAllThicknessMaps(); }
   void restoreAllThicknessMaps()                                                           { return GeoPhysics::GeoPhysicsCrustFormation::restoreAllThicknessMaps(); }
   unsigned int  getElementRefinement()                                               const { return GeoPhysics::GeoPhysicsCrustFormation::getElementRefinement(); }

   // Inherited via LayerProperties
   void switchLithologies( const double age )                                               { return LayerProps::LayerProps::switchLithologies( age ); }

   void initialise ();

   /// \return False since this is a Crust formation
   bool isSourceRock () const noexcept;

   /// \return True if the crust is oceanic (only possible in ALC)
   ///    False if the crust is continental
   bool isBasalt() const noexcept;

   /// \return The heat production at the given (i,j) node
   double getHeatProduction ( const unsigned int i,
                              const unsigned int j ) const;

   /// \return Sets the crust lithologies
   /// \details If the thermal model is ALC, then creates a ALC basalt lithology array
   bool setLithologiesFromStratTable ();

   void allocateBasementVecs( );
   void reInitialiseBasementVecs();
   void initialiseBasementVecs();
   void setBasementVectorList();

   /// @todo How can the lithology change between differnt ij positions?
   const CompoundLithology* getBasaltLithology(const int iPosition, const int jPosition) const;

   using LayerProps::getLithology;

   const CompoundLithology* getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset );

   const CompoundLithology* getLithology(const int iPosition, const int jPosition, const int kPosition ) const;

   /// \return Sets the node lithology (i,j,k) of the formation to basalt
   /// \details This is mentioned as lithoswitching in Cauldron documentation
   void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition );
   /// \return True if the previous crust (in time) was basalt at the given (i,j,k) node, false otherwise
   bool getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ); 

   void cleanVectors();

   Vec TopBasaltDepth;
   Vec BasaltThickness;
   Vec BottomBasaltDepth;
   Vec ThicknessBasaltALC;
   Vec ThicknessCCrustALC;
   Vec SmCCrustThickness;
   Vec SmTopBasaltDepth;
   Vec SmBottomBasaltDepth;
   PETSc_3D_Boolean_Array  BasaltMap;
   PETSc_3D_Boolean_Array  previousBasaltMap;

protected :

   const Interface::GridMap* m_heatProductionMap;

   /// @todo This does not need to be an array
   CompoundLithologyArray m_basaltLithology;

   bool isBasaltLayer; ///< True if the crust is oceanic (only possible in ALC), false if the crust is continental
};

inline bool CrustFormation::isSourceRock () const noexcept {
   return false;
}

inline bool CrustFormation::isBasalt() const noexcept {
   return isBasaltLayer;
}

inline double CrustFormation::getHeatProduction ( const unsigned int i,
                                                  const unsigned int j ) const {
   return m_heatProductionMap->getValue ( i, j );
}


inline void CrustFormation::setBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   if( !BasaltMap.isNull() ) {
      BasaltMap( iPosition, jPosition, kPosition ) = true;
   }
}

inline bool CrustFormation::getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   return previousBasaltMap( iPosition, jPosition, kPosition );
}




#endif // FASTCAULDRON__CRUST_FORMATION_H
