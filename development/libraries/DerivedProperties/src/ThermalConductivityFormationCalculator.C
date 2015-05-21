#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

#include "Interface/Interface.h"


#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "ThermalConductivityFormationCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::ThermalConductivityFormationCalculator::ThermalConductivityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "ThCondVec2" );
}

void DerivedProperties::ThermalConductivityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                            const DataModel::AbstractSnapshot*          snapshot,
                                                                            const DataModel::AbstractFormation*         formation,
                                                                                  FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* thermalConductivityProperty = propertyManager.getProperty ( "ThCondVec2" );

   const DataModel::AbstractProperty* temperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* porosityProperty     = propertyManager.getProperty ( "Porosity" );

   const FormationPropertyPtr temperature  = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
   const FormationPropertyPtr porosity     = propertyManager.getFormationProperty ( porosityProperty,     snapshot, formation );

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   if ( temperature != 0 and porePressure != 0 and porosity != 0 and geoFormation != 0 ) {
      const double age = snapshot->getTime ();
      bool alcMode = ( geoFormation->kind() == DataAccess::Interface::BASEMENT_FORMATION ) and m_projectHandle->isALC ();

      DerivedFormationPropertyPtr thermalConductivity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( thermalConductivityProperty,
                                                                                                                                        snapshot,
                                                                                                                                        formation,
                                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                                        geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever porePressureRetriever ( porePressure );
      PropertyRetriever porosityRetriever ( porosity );

      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(geoFormation->getFluidType ());

      // We could use any of the formation-properties here to get the undefined value.
      double undefinedValue = thermalConductivity->getUndefinedValue ();
      double thermalConductivityNormal;
      double thermalConductivityPlane;

      for ( unsigned int i = thermalConductivity->firstI ( true ); i <= thermalConductivity->lastI ( true ); ++i ) {
            
         for ( unsigned int j = thermalConductivity->firstJ ( true ); j <= thermalConductivity->lastJ ( true ); ++j ) {
               
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               const GeoPhysics::CompoundLithology* lithology = (*lithologies)( i, j );

               for ( unsigned int k = thermalConductivity->firstK (); k <= thermalConductivity->lastK (); ++k ) {

                  if ( alcMode ) {
                     lithology->calcBulkThermCondNPBasement ( fluid,
                                                              0.01 * porosity->get ( i, j, k ),
                                                              temperature->get ( i, j, k ),
                                                              porePressure->get ( i, j, k ),
                                                              thermalConductivityNormal,
                                                              thermalConductivityPlane );
                  } else {
                     lithology->calcBulkThermCondNP ( fluid,
                                                      0.01 * porosity->get ( i, j, k ),
                                                      temperature->get ( i, j, k ),
                                                      porePressure->get ( i, j, k ),
                                                      thermalConductivityNormal,
                                                      thermalConductivityPlane );
                  }

                  thermalConductivity->set ( i, j, k, thermalConductivityNormal );
               }

            } else {

               for ( unsigned int k = thermalConductivity->firstK (); k <= thermalConductivity->lastK (); ++k ) {
                  thermalConductivity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( thermalConductivity );
   }

}
