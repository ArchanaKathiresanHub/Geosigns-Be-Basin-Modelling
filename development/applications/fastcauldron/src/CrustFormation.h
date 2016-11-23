#ifndef _FASTCAULDRON__CRUST_FORMATION_H_
#define _FASTCAULDRON__CRUST_FORMATION_H_

#include "Interface/CrustFormation.h"
#include "Interface/GridMap.h"

#include "GeoPhysicsCrustFormation.h"
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

   void determineMinMaxThickness()                                                          { return GeoPhysics::GeoPhysicsCrustFormation::determineMinMaxThickness(); }
   bool isCrust()                                                                     const { return GeoPhysics::GeoPhysicsCrustFormation::isCrust(); }
   unsigned int setMaximumNumberOfElements( const bool readSizeFromVolumeData )             { return GeoPhysics::GeoPhysicsCrustFormation::setMaximumNumberOfElements( readSizeFromVolumeData ); }
   void retrieveAllThicknessMaps()                                                          { return GeoPhysics::GeoPhysicsCrustFormation::retrieveAllThicknessMaps(); }
   void restoreAllThicknessMaps()                                                           { return GeoPhysics::GeoPhysicsCrustFormation::restoreAllThicknessMaps(); }
   
   // Inherited via LayerProperties
   void switchLithologies( const double age )                                               { return LayerProps::LayerProps::switchLithologies( age ); }

   void initialise ();

   bool isSourceRock () const;

   double getHeatProduction ( const unsigned int i,
                              const unsigned int j ) const;

   bool setLithologiesFromStratTable ();
  
   bool isBasalt() const;
   
   void allocateBasementVecs( );
   void reInitialiseBasementVecs();
   void initialiseBasementVecs();
   void setBasementVectorList();
   
 
   const CompoundLithology* getBasaltLithology(const int iPosition, const int jPosition) const;

   using LayerProps::getLithology;
 
   const CompoundLithology* getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset );
   
   const CompoundLithology* getLithology(const int iPosition, const int jPosition, const int kPosition ) const;
   
   void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition );
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

   
   CompoundLithologyArray m_basaltLithology;
   
   bool  isBasaltLayer;
};

inline bool CrustFormation::isSourceRock () const {
   return false;
}

inline double CrustFormation::getHeatProduction ( const unsigned int i,
                                                  const unsigned int j ) const {
   return m_heatProductionMap->getValue ( i, j );
}
inline bool CrustFormation::isBasalt() const {
   return isBasaltLayer;
}

inline void CrustFormation::setBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   if( !BasaltMap.isNull() ) {
      BasaltMap( iPosition, jPosition, kPosition ) = true;
   }
}

inline bool CrustFormation::getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   return previousBasaltMap( iPosition, jPosition, kPosition );
}




#endif // _FASTCAULDRON__CRUST_FORMATION_H_
