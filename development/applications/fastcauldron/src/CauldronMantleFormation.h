#ifndef _FASTCAULDRON_MANTLE_FORMATION_H_
#define _FASTCAULDRON_MANTLE_FORMATION_H_

#include "MantleFormation.h"
#include "layer.h"

#include "GeoPhysicsMantleFormation.h"


class CauldronMantleFormation : virtual public LayerProps, virtual public GeoPhysics::GeoPhysicsMantleFormation {

public :

   CauldronMantleFormation ( Interface::ProjectHandle * projectHandle,
                     database::Record *              record );

   ~CauldronMantleFormation ();

   const string                                        & getName()                    const { return GeoPhysics::GeoPhysicsMantleFormation::getName(); }
   const DataAccess::Interface::LithoType              * getLithoType1()              const { return GeoPhysics::GeoPhysicsMantleFormation::getLithoType1(); }
   const DataAccess::Interface::GridMap                * getLithoType1PercentageMap() const { return GeoPhysics::GeoPhysicsMantleFormation::getLithoType1PercentageMap(); }
   const DataAccess::Interface::LithoType              * getLithoType2()              const { return GeoPhysics::GeoPhysicsMantleFormation::getLithoType2(); }
   const DataAccess::Interface::GridMap                * getLithoType2PercentageMap() const { return GeoPhysics::GeoPhysicsMantleFormation::getLithoType2PercentageMap(); }
   const DataAccess::Interface::LithoType              * getLithoType3()              const { return GeoPhysics::GeoPhysicsMantleFormation::getLithoType3(); }
   const DataAccess::Interface::GridMap                * getLithoType3PercentageMap() const { return GeoPhysics::GeoPhysicsMantleFormation::getLithoType3PercentageMap(); }
   DataAccess::Interface::ReservoirList                * getReservoirs()              const { return GeoPhysics::GeoPhysicsMantleFormation::getReservoirs(); }
   DataAccess::Interface::MobileLayerList              * getMobileLayers()            const { return GeoPhysics::GeoPhysicsMantleFormation::getMobileLayers(); }
   const DataAccess::Interface::AllochthonousLithology * getAllochthonousLithology()  const { return GeoPhysics::GeoPhysicsMantleFormation::getAllochthonousLithology(); }
   DataAccess::Interface::FaultCollectionList          * getFaultCollections()        const { return GeoPhysics::GeoPhysicsMantleFormation::getFaultCollections(); }
   bool                                                  isMobileLayer()              const { return GeoPhysics::GeoPhysicsMantleFormation::isMobileLayer(); }
   bool                                                  hasAllochthonousLithology()  const { return GeoPhysics::GeoPhysicsMantleFormation::hasAllochthonousLithology(); }
   bool                                                  hasConstrainedOverpressure() const { return GeoPhysics::GeoPhysicsMantleFormation::hasConstrainedOverpressure(); }
   bool                                                  hasChemicalCompaction()      const { return GeoPhysics::GeoPhysicsMantleFormation::hasChemicalCompaction(); }
   CBMGenerics::capillarySealStrength::MixModel          getMixModel()                const { return GeoPhysics::GeoPhysicsMantleFormation::getMixModel(); }
   float                                                 getLayeringIndex()           const { return GeoPhysics::GeoPhysicsMantleFormation::getLayeringIndex(); }
   const DataAccess::Interface::FluidType              * getFluidType()               const { return GeoPhysics::GeoPhysicsMantleFormation::getFluidType(); }
   const std::string                                   & getMixModelStr()             const { return GeoPhysics::GeoPhysicsMantleFormation::getMixModelStr(); }
   DataAccess::Interface::GridMap                      * loadThicknessMap()           const { return GeoPhysics::GeoPhysicsMantleFormation::loadThicknessMap(); }
   DataAccess::Interface::GridMap                      * computeThicknessMap()        const { return GeoPhysics::GeoPhysicsMantleFormation::computeThicknessMap(); }
   DataAccess::Interface::GridMap                      * computeFaultGridMap( const DataAccess::Interface::Grid * localGrid, const DataAccess::Interface::Snapshot * snapshot ) const {
      return GeoPhysics::GeoPhysicsMantleFormation::computeFaultGridMap( localGrid, snapshot );
   }
   const DataAccess::Interface::GridMap                * getInputThicknessMap()       const { return GeoPhysics::GeoPhysicsMantleFormation::getInputThicknessMap(); }
   int                                                   getDepositionSequence()      const { return GeoPhysics::GeoPhysicsMantleFormation::getDepositionSequence(); }
   void                                                  asString( string & str )     const { return GeoPhysics::GeoPhysicsMantleFormation::asString( str ); }
   unsigned int                                          getElementRefinement()       const { return GeoPhysics::GeoPhysicsMantleFormation::getElementRefinement(); }

   void determineMinMaxThickness()                                                    final { return GeoPhysics::GeoPhysicsMantleFormation::determineMinMaxThickness(); }
   bool isCrust()                                                      const noexcept final { return GeoPhysics::GeoPhysicsMantleFormation::isCrust(); }
   unsigned int setMaximumNumberOfElements( const bool readSizeFromVolumeData )             { return GeoPhysics::GeoPhysicsMantleFormation::setMaximumNumberOfElements( readSizeFromVolumeData ); }
   void retrieveAllThicknessMaps()                                                          { return GeoPhysics::GeoPhysicsMantleFormation::retrieveAllThicknessMaps(); }
   void restoreAllThicknessMaps()                                                           { return GeoPhysics::GeoPhysicsMantleFormation::restoreAllThicknessMaps(); }
   bool isMantle() const                                                                    { return GeoPhysics::GeoPhysicsMantleFormation::isMantle(); }

   // Inherited via LayerProperties
   void switchLithologies( const double age ) { return LayerProps::LayerProps::switchLithologies( age ); }

   void initialise ();

   bool isSourceRock () const;

   bool isBasalt() const;

   void allocateBasementVecs();
   void reInitialiseBasementVecs();
   void initialiseBasementVecs();
   void setBasementVectorList();
   void cleanVectors();
private :

   const Interface::GridMap* m_presentDayThickness;
   bool isBasaltLayer;

public :
   Vec UpliftedOrigMantleDepth;
   Vec LithosphereThicknessMod;

   PETSc_3D_Boolean_Array  BasaltMap;
   PETSc_3D_Boolean_Array  previousBasaltMap;

   CompoundLithologyArray m_basaltLithology;

   bool setLithologiesFromStratTable ();

   const CompoundLithology* getBasaltLithology(const int iPosition, const int jPosition) const;

   using LayerProps::getLithology;

   const CompoundLithology* getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset );

   const CompoundLithology* getLithology(const int iPosition, const int jPosition, const int kPosition ) const;

   /// \brief Set the activity of the mantle layer elements.
   ///
   /// Both element and face activity is set.
   /// Since the solid/real-thickness histories are not set for the mantle the depths
   /// will be used, which must therefore be defined before calling this.
   virtual void setLayerElementActivity ( const double age );

   void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition );
   bool getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition );
};

inline bool CauldronMantleFormation::isSourceRock () const {
   return false;
}
inline bool CauldronMantleFormation::isBasalt() const {
   return isBasaltLayer;
}

inline void CauldronMantleFormation::setBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   if( !BasaltMap.isNull() ) {
      BasaltMap( iPosition, jPosition, kPosition ) = true;
   }
}
inline bool CauldronMantleFormation::getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   return previousBasaltMap( iPosition, jPosition, kPosition );
}



#endif // _FASTCAULDRON_MANTLE_FORMATION_H_
