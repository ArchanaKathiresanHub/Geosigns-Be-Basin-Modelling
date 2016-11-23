#ifndef _GEOPHYSICS__MANTLE_FORMATION_H_
#define _GEOPHYSICS__MANTLE_FORMATION_H_

#include "Interface/MantleFormation.h"

#include "GeoPhysicsFormation.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}



namespace GeoPhysics {

   class GeoPhysicsMantleFormation : virtual public GeoPhysics::Formation,
                                     virtual public DataAccess::Interface::MantleFormation {

   public :

      GeoPhysicsMantleFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                  database::Record*                          record );

      // redefine set of methods which are differently were defined in base classes
      // Inherited from BasementFormation
      const string                                        & getName()                    const { return DataAccess::Interface::BasementFormation::getName(); }
      const DataAccess::Interface::LithoType              * getLithoType1()              const { return DataAccess::Interface::BasementFormation::getLithoType1(); }
      const DataAccess::Interface::GridMap                * getLithoType1PercentageMap() const { return DataAccess::Interface::BasementFormation::getLithoType1PercentageMap(); }
      const DataAccess::Interface::LithoType              * getLithoType2()              const { return DataAccess::Interface::BasementFormation::getLithoType2(); }
      const DataAccess::Interface::GridMap                * getLithoType2PercentageMap() const { return DataAccess::Interface::BasementFormation::getLithoType2PercentageMap(); }
      const DataAccess::Interface::LithoType              * getLithoType3()              const { return DataAccess::Interface::BasementFormation::getLithoType3(); }
      const DataAccess::Interface::GridMap                * getLithoType3PercentageMap() const { return DataAccess::Interface::BasementFormation::getLithoType3PercentageMap(); }
      DataAccess::Interface::ReservoirList                * getReservoirs()              const { return DataAccess::Interface::BasementFormation::getReservoirs(); }
      DataAccess::Interface::MobileLayerList              * getMobileLayers()            const { return DataAccess::Interface::BasementFormation::getMobileLayers(); }
      const DataAccess::Interface::AllochthonousLithology * getAllochthonousLithology()  const { return DataAccess::Interface::BasementFormation::getAllochthonousLithology(); }
      DataAccess::Interface::FaultCollectionList          * getFaultCollections()        const { return DataAccess::Interface::BasementFormation::getFaultCollections(); }
      bool                                                  isMobileLayer()              const { return DataAccess::Interface::BasementFormation::isMobileLayer(); }
      bool                                                  hasAllochthonousLithology()  const { return DataAccess::Interface::BasementFormation::hasAllochthonousLithology(); }
      bool                                                  isSourceRock()               const { return DataAccess::Interface::BasementFormation::isSourceRock(); }
      bool                                                  hasConstrainedOverpressure() const { return DataAccess::Interface::BasementFormation::hasConstrainedOverpressure(); }
      bool                                                  hasChemicalCompaction()      const { return DataAccess::Interface::BasementFormation::hasChemicalCompaction(); }
      CBMGenerics::capillarySealStrength::MixModel          getMixModel()                const { return DataAccess::Interface::BasementFormation::getMixModel(); }
      float                                                 getLayeringIndex()           const { return DataAccess::Interface::BasementFormation::getLayeringIndex(); }
      const DataAccess::Interface::FluidType              * getFluidType()               const { return DataAccess::Interface::BasementFormation::getFluidType(); }
      const std::string                                   & getMixModelStr()             const { return DataAccess::Interface::BasementFormation::getMixModelStr(); }
      DataAccess::Interface::GridMap                      * loadThicknessMap()           const { return DataAccess::Interface::BasementFormation::loadThicknessMap(); }
      DataAccess::Interface::GridMap                      * computeThicknessMap()        const { return DataAccess::Interface::BasementFormation::computeThicknessMap(); }
      DataAccess::Interface::GridMap                      * computeFaultGridMap( const DataAccess::Interface::Grid * localGrid, const DataAccess::Interface::Snapshot * snapshot ) const {
         return DataAccess::Interface::BasementFormation::computeFaultGridMap( localGrid, snapshot );
      }
      // Inherited from MantleFormation
      const DataAccess::Interface::GridMap                * getInputThicknessMap()       const { return DataAccess::Interface::MantleFormation::getInputThicknessMap(); }
      int                                                   getDepositionSequence()      const { return DataAccess::Interface::MantleFormation::getDepositionSequence(); }
      void                                                  asString( string & str )     const { return DataAccess::Interface::MantleFormation::asString( str ); }

      bool isMantle () const;

      // Since this function is almost identical to the one in the CrustFormation,
      // would it be better to introduce a GeoPhysicsBasementFormation (derived from
      // the DAL::BasementFormation)?
      virtual bool setLithologiesFromStratTable ();

      void determineMinMaxThickness ();

      unsigned int setMaximumNumberOfElements ( const bool readSizeFromVolumeData );

      void retrieveAllThicknessMaps ();

      void restoreAllThicknessMaps ();

      double m_mantleElementHeight0;
    };
}

inline bool GeoPhysics::GeoPhysicsMantleFormation::isMantle () const {
   return true;
}


#endif // _GEOPHYSICS__MANTLE_FORMATION_H_
