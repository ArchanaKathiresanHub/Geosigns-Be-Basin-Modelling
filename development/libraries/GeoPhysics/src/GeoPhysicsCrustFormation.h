#ifndef _GEOPHYSICS__CRUST_FORMATION_H_
#define _GEOPHYSICS__CRUST_FORMATION_H_

#include "Interface/CrustFormation.h"

#include "GeoPhysicsFormation.h"

#include "Polyfunction.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class GridMap;
   }
}

namespace GeoPhysics {

   class GeoPhysicsCrustFormation : virtual public GeoPhysics::Formation,
      virtual public DataAccess::Interface::CrustFormation {

   public:

      GeoPhysicsCrustFormation( DataAccess::Interface::ProjectHandle* projectHandle,
                                database::Record*                          record );


      ~GeoPhysicsCrustFormation();

      bool isCrust() const;

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
         return DataAccess::Interface::BasementFormation::computeFaultGridMap( localGrid, snapshot ); }
      unsigned int                                          getElementRefinement() const { return DataAccess::Interface::BasementFormation::getElementRefinement(); }

      // Inherited from CrustFormation
      const DataAccess::Interface::GridMap                * getInputThicknessMap()       const { return DataAccess::Interface::CrustFormation::getInputThicknessMap(); }
      int                                                   getDepositionSequence()      const { return DataAccess::Interface::CrustFormation::getDepositionSequence(); }
      void                                                  asString( string & str )     const { return DataAccess::Interface::CrustFormation::asString( str ); }


      // Since this function is almost identical to the one in the GeoPhysicsMantleFormation,
      // would it be better to introduce a GeoPhysicsBasementFormation (derived from
      // the DAL::BasementFormation)?
     virtual bool setLithologiesFromStratTable ();

      void determineMinMaxThickness ();

      unsigned int setMaximumNumberOfElements ( const bool readSizeFromVolumeData );

      /// Determine the crust-thinning ratio.
      ///
      /// If the crust has a negative thickness, this function will return false.
      bool determineCrustThinningRatio ();

      double getCrustMaximumThicknessHistory ( const double age ) const;

      double getCrustThinningRatio () const;

      double getCrustMaximumThickness ( const unsigned int i,
                                        const unsigned int j ) const;


      void retrieveAllThicknessMaps ();

      void restoreAllThicknessMaps ();

   protected :


      CBMGenerics::Polyfunction m_crustThickessHistory;
      double                    m_crustThinningRatio;

      DataAccess::Interface::GridMap*  m_crustMaximumThickness;

   };
}

//------------------------------------------------------------//
//  Inline functions.
//------------------------------------------------------------//

inline double GeoPhysics::GeoPhysicsCrustFormation::getCrustThinningRatio () const {
   return m_crustThinningRatio;
}

inline bool GeoPhysics::GeoPhysicsCrustFormation::isCrust () const {
   return true;
}

#endif // _GEOPHYSICS__CRUST_FORMATION_H_
