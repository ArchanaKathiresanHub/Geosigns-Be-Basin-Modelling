#ifndef _GEOPHYSICS__OBJECT_FACTORY_H_
#define _GEOPHYSICS__OBJECT_FACTORY_H_

#include "database.h"
#include "Interface/ObjectFactory.h"

#include "PiecewiseInterpolator.h"

#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFluidType.h"
#include "SimpleLithology.h"
#include "CompoundLithology.h"
#include "FracturePressureCalculator.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class FluidType;
      class LithoType;
      class Formation;
      class CrustFormation;
      class MantleFormation;
   }
}

namespace GeoPhysics
{
   class ProjectHandle;
}

namespace GeoPhysics {

   class ObjectFactory : public DataAccess::Interface::ObjectFactory {

   public :

      /// Returns the project-handle.
      DataAccess::Interface::ProjectHandle* produceProjectHandle ( database::ProjectFileHandlerPtr pfh,
                                                                   const string &       name,
                                                                   const string &       accessMode ) const;

      /// Allocate a GeoPhysics fluid-type.
      DataAccess::Interface::FluidType* produceFluidType ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                           database::Record*                     record ) const;

      /// Allocate a GeoPhysics formation.
      DataAccess::Interface::Formation* produceFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                           database::Record*                     record ) const;

      /// Allocate a GeoPhysics crust-formation.
      DataAccess::Interface::CrustFormation* produceCrustFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                     database::Record*                     record ) const;

      /// Allocate a GeoPhysics mantle-formation.
      DataAccess::Interface::MantleFormation* produceMantleFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                       database::Record*                     record ) const;


      /// Allocate a GeoPhysics source-rock.
      DataAccess::Interface::SourceRock* produceSourceRock ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                             database::Record*                     record ) const;

      /// Allocate a GeoPhysics litho-type (simple-lithology).
      DataAccess::Interface::LithoType* produceLithoType ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                           database::Record *                    record ) const;

      /// Allocate a GeoPhysics litho-type (simple-lithology).
      // Look at removing this function
      DataAccess::Interface::LithoType* produceLithoType ( const SimpleLithology* litho,
                                                           const std::string&     newName ) const;


      /// Allocate a GeoPhysics litho-type (simple-lithology).
      // Look at removing this function
      DataAccess::Interface::LithoType* produceLithoType ( const SimpleLithology*            litho,
                                                           const std::string&                newName,
                                                           const double                      permeabilityAnisotropy,
                                                           const std::vector<double> &  porosityPercentageSamples,
                                                           const std::vector<double> & permeabilitySamples ) const;

      /// New classes in the GeoPhysics namespace.
      ///
      /// 1. CompoundLithology.


      /// Allocate a compound lithology.
      // Would be better to construct the compound-lithology from the compound-lithology-composition.
      ///
      /// Notice here that the project-handle is from the geophysics library.
      virtual GeoPhysics::CompoundLithology* produceCompoundLithology ( GeoPhysics::ProjectHandle* projectHandle ) const;

      virtual GeoPhysics::FracturePressureCalculator* produceFracturePressureCalculator ( GeoPhysics::ProjectHandle* projectHandle ) const;


   };

}


#endif // _GEOPHYSICS__OBJECT_FACTORY_H_
