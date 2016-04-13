//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DERIVED_PROPERTIES__ABSTRACT_PROPERTY_MANAGER_H
#define DERIVED_PROPERTIES__ABSTRACT_PROPERTY_MANAGER_H

#include <vector>
#include <map>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractFormation.h"
#include "AbstractReservoir.h"

#include "SurfacePropertyCalculator.h"
#include "FormationMapPropertyCalculator.h"
#include "FormationPropertyCalculator.h"
#include "FormationSurfacePropertyCalculator.h"
#include "ReservoirPropertyCalculator.h"
#include "PropertySnapshotCalculatorMap.h"

// utilities library
#include "FormattingException.h"

namespace DerivedProperties {

   /// \brief Handles derived properties and their calculation.
   class AbstractPropertyManager {

   typedef formattingexception::GeneralException AbstractPropertyException;

   public :

      /// \brief Constructor.
      AbstractPropertyManager ();

      /// \brief Destructor.
      virtual ~AbstractPropertyManager ();


      /// \brief Get the property given the property-name.
      ///
      /// If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property.
      virtual const DataModel::AbstractProperty* getProperty ( const std::string& name ) const = 0;

      /// \brief Get the surface property values.
      ///
      /// If the surface property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] surface  The surface with which the values are associated.
      /// \pre property is not be null and is a valid property.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre surface is not null and is a  valid surface.
      /// \pre A calculator for this property exists.
      /// \post The result contains values of the required property at the required surface for the required snapshot age.
      /// If the propery is not computed then a null will be returned.
      virtual SurfacePropertyPtr getSurfaceProperty ( const DataModel::AbstractProperty* property,
                                                      const DataModel::AbstractSnapshot* snapshot,
                                                      const DataModel::AbstractSurface*  surface );

      /// \brief Get the formation property values.
      ///
      /// If the formation property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation with which the values are associated.
      /// If the propery is not computed then a null will be returned.
      virtual FormationMapPropertyPtr getFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                const DataModel::AbstractFormation* formation );

      /// \brief Get the formation property values.
      ///
      /// If the formation property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation with which the values are associated.
      /// If the propery is not computed then a null will be returned.
      virtual FormationPropertyPtr getFormationProperty ( const DataModel::AbstractProperty*  property,
                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                          const DataModel::AbstractFormation* formation );

      /// \brief Get the surface and formation property values.
      ///
      /// If the property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation with which the values are associated.
      /// \param [in] surface   The surface with which the values are associated.
      /// If the propery is not computed then a null will be returned.
      virtual FormationSurfacePropertyPtr getFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                        const DataModel::AbstractSurface*   surface );

      /// \brief Get the reservoir property values.
      ///
      /// If the reservoir property values have not been computed and there is an associated calculator
      /// then the values will be calculated as required. Additional properties may also be calculated.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] reservoir The reservoir to which the values are associated.
      /// If the propery is not computed then a null will be returned.
      virtual ReservoirPropertyPtr getReservoirProperty ( const DataModel::AbstractProperty*  property,
                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                          const DataModel::AbstractReservoir* reservoir );

      /// \brief Determine if the formation property is computable.
      ///
      /// If snapshot is null then this will determine if the property is computable at some undefined snapshot time.
      /// If formation is null then this will determine if the property is computable at some undefined formation in the domain.
      ///
      /// \param [in] property  The property we would like to know is calculatable.
      /// \param [in] snapshot  The snapshot at which we would like to know if the property is calculatable.
      /// \param [in] formation The formation for which we would like to know if the property is calculatable.
      /// \pre The property points to a valid property object.
      /// \pre The snapshot points to a valid snapshot object or is null.
      /// \pre The formation points to a valid formation object or is null.
      virtual bool formationPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                   const DataModel::AbstractSnapshot*  snapshot = 0,
                                                   const DataModel::AbstractFormation* formation = 0 ) const;

      /// \brief Determine if the formation-surface property is computable.
      ///
      /// If snapshot is null then this will determine if the property is computable at some undefined snapshot time.
      /// If formation is null then this will determine if the property is computable at some undefined formation in the domain.
      /// If surface is null then this will determine if the property is computable at some undefined surface in the domain.
      ///
      /// \param [in] property  The property we would like to know is calculatable.
      /// \param [in] snapshot  The snapshot at which we would like to know if the property is calculatable.
      /// \param [in] formation The formation for which we would like to know if the property is calculatable.
      /// \param [in] surface   The surface for which we would like to know if the property is calculatable.
      /// \pre The property points to a valid property object.
      /// \pre The snapshot points to a valid snapshot object or is null.
      /// \pre The formation points to a valid formation object or is null.
      /// \pre The surface points to a valid surface object or is null.
      virtual bool formationSurfacePropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                          const DataModel::AbstractSnapshot*  snapshot = 0,
                                                          const DataModel::AbstractFormation* formation = 0,
                                                          const DataModel::AbstractSurface*   surface = 0 ) const;

      /// \brief Determine if the surface property is computable.
      ///
      /// If snapshot is null then this will determine if the property is computable at some undefined snapshot time.
      /// If surface is null then this will determine if the property is computable at some undefined surface in the domain.
      ///
      /// \param [in] property The property we would like to know is calculatable.
      /// \param [in] snapshot The snapshot at which we would like to know if the property is calculatable.
      /// \param [in] surface  The surface for which we would like to know if the property is calculatable.
      /// \pre The property points to a valid property object.
      /// \pre The snapshot points to a valid snapshot object or is null.
      /// \pre The surface points to a valid surface object or is null.
      virtual bool surfacePropertyIsComputable ( const DataModel::AbstractProperty* property,
                                                 const DataModel::AbstractSnapshot* snapshot = 0,
                                                 const DataModel::AbstractSurface*  surface = 0 ) const;

      /// \brief Determine if the formation-map property is computable.
      ///
      /// If snapshot is null then this will determine if the property is computable at some undefined snapshot time.
      /// If formation is null then this will determine if the property is computable at some undefined formation in the domain.
      ///
      /// \param [in] property  The property we would like to know is calculatable.
      /// \param [in] snapshot  The snapshot at which we would like to know if the property is calculatable.
      /// \param [in] formation The formation for which we would like to know if the property is calculatable.
      /// \pre The property points to a valid property object.
      /// \pre The snapshot points to a valid snapshot object or is null.
      /// \pre The formation points to a valid formation object or is null.
      virtual bool formationMapPropertyIsComputable ( const DataModel::AbstractProperty* property,
                                                      const DataModel::AbstractSnapshot*  snapshot = 0,
                                                      const DataModel::AbstractFormation* formation = 0 ) const;


      /// \brief Determine if the reservoir property is computable.
      ///
      /// If snapshot is null then this will determine if the property is computable at some undefined snapshot time.
      /// If reservoir is null then this will determine if the property is computable at some undefined reservoir in the domain.
      ///
      /// \param [in] property  The property we would like to know is calculatable.
      /// \param [in] snapshot  The snapshot at which we would like to know if the property is calculatable.
      /// \param [in] reservoir The reservoir for which we would like to know if the property is calculatable.
      /// \pre The property points to a valid property object.
      /// \pre The snapshot points to a valid snapshot object or is null.
      /// \pre The reservoir points to a valid reservoir object or is null.
      virtual bool reservoirPropertyIsComputable ( const DataModel::AbstractProperty*  property,
                                                   const DataModel::AbstractSnapshot*  snapshot = 0,
                                                   const DataModel::AbstractReservoir* reservoir = 0 ) const;


      /// \brief Remove all properties associated with a particular snapshot.
      void removeProperties ( const DataModel::AbstractSnapshot* snapshot );

      /// \brief Get the grid for the map.
      virtual const DataModel::AbstractGrid* getMapGrid () const = 0;

      /// \brief Determine whether or not the node is valid.
      virtual bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const = 0;

   protected :

      /// \brief Add a calculator for a property or set of properties defined on a surface.
      ///
      /// \param [in] calculator  A calculator of surface properties.
      /// \param [in] snapshot    The snapshot for which the calculator is valid.
      ///
      /// If the snapshot is null value then this indicates that the calculator is for any snapshot age.
      void addSurfacePropertyCalculator ( const SurfacePropertyCalculatorPtr& calculator,
                                          const DataModel::AbstractSnapshot*  snapshot = 0 );

      /// \brief Add a calculator for a property or set of map properties defined on a formation.
      ///
      /// \param [in] calculator  A calculator of formation map properties.
      /// \param [in] snapshot    The snapshot for which the calculator is valid.
      ///
      /// If the snapshot is null value then this indicates that the calculator is for any snapshot age.
      void addFormationMapPropertyCalculator ( const FormationMapPropertyCalculatorPtr& calculator,
                                               const DataModel::AbstractSnapshot*       snapshot = 0 );

      /// \brief Add a calculator for a property or set of properties defined on a formation.
      ///
      /// \param [in] calculator  A calculator of formation properties.
      /// \param [in] snapshot    The snapshot for which the calculator is valid.
      ///
      /// If the snapshot is null value then this indicates that the calculator is for any snapshot age.
      void addFormationPropertyCalculator ( const FormationPropertyCalculatorPtr& calculator,
                                            const DataModel::AbstractSnapshot*    snapshot = 0,
                                            const bool                            debug = false );
 
      /// \brief Add a calculator for a property or set of properties defined on a surface and formation.
      ///
      /// \param [in] calculator  A calculator of formation-surface properties.
      /// \param [in] snapshot    The snapshot for which the calculator is valid.
      ///
      /// If the snapshot is null value then this indicates that the calculator is for any snapshot age.
      void addFormationSurfacePropertyCalculator ( const FormationSurfacePropertyCalculatorPtr& calculator,
                                                   const DataModel::AbstractSnapshot*           snapshot = 0 );

      /// \brief Add a calculator for a property or set of properties defined for a reservoir.
      ///
      /// \param [in] calculator  A calculator of reservoir properties.
      /// \param [in] snapshot    The snapshot for which the calculator is valid.
      ///
      /// If the snapshot is null value then this indicates that the calculator is for any snapshot age.
      void addReservoirPropertyCalculator ( const ReservoirPropertyCalculatorPtr& calculator,
                                            const DataModel::AbstractSnapshot*    snapshot = 0 );

      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] surfaceProperty  A set of property values associated with a surface.
      void addSurfaceProperty ( const SurfacePropertyPtr& surfaceProperty );
      
      /// \brief Add a set of property values to the available property values.
      ///
      /// \param [in] formationMapProperty  A set of map property values associated with a formation.
      void addFormationMapProperty ( const FormationMapPropertyPtr& formationMapProperty );
      
      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] formationProperty  A set of property values associated with a formation.
      void addFormationProperty ( const FormationPropertyPtr& formationProperty );

      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] formationSurfaceProperty  A set of property values associated with a formation and surface.
      void addFormationSurfaceProperty ( const FormationSurfacePropertyPtr& formationSurfaceProperty );

      /// \brief Add a set of property values to the availble property values.
      ///
      /// \param [in] reservoirProperty  A set of property values associated with a reservoir.
      void addReservoirProperty ( const ReservoirPropertyPtr& reservoirProperty );

      /// \brief Get the calculator for the property and snapshot.
      ///
      /// \param [in] property The property whose calulator is requested.
      /// \param [in] snapshot The associated snapshot for the calcualtor.
      ///
      /// If no calculator has been added then a null will be returned.
      SurfacePropertyCalculatorPtr getSurfaceCalculator ( const DataModel::AbstractProperty* property,
                                                          const DataModel::AbstractSnapshot* snapshot ) const;

      /// \brief Get the calculator for the property and snapshot.
      ///
      /// \param [in] property The property whose calulator is requested.
      /// \param [in] snapshot The associated snapshot for the calcualtor.
      ///
      /// If no calculator has been added then a null will be returned.
      FormationSurfacePropertyCalculatorPtr getFormationSurfaceCalculator ( const DataModel::AbstractProperty* property,
                                                                            const DataModel::AbstractSnapshot* snapshot ) const;

      /// \brief Get the calculator for the property and snapshot.
      ///
      /// \param [in] property The property whose calulator is requested.
      /// \param [in] snapshot The associated snapshot for the calcualtor.
      ///
      /// If no calculator has been added then a null will be returned.
      FormationMapPropertyCalculatorPtr getFormationMapCalculator ( const DataModel::AbstractProperty* property,
                                                                    const DataModel::AbstractSnapshot* snapshot ) const;

      /// \brief Get the calculator for the property and snapshot.
      ///
      /// \param [in] property The property whose calulator is requested.
      /// \param [in] snapshot The associated snapshot for the calcualtor.
      ///
      /// If no calculator has been added then a null will be returned.
      FormationPropertyCalculatorPtr getFormationCalculator ( const DataModel::AbstractProperty* property,
                                                              const DataModel::AbstractSnapshot* snapshot ) const;

      /// \brief Get the calculator for the property and snapshot.
      ///
      /// \param [in] property The property whose calulator is requested.
      /// \param [in] snapshot The associated snapshot for the calcualtor.
      ///
      /// If no calculator has been added then a null will be returned.
      ReservoirPropertyCalculatorPtr getReservoirCalculator ( const DataModel::AbstractProperty* property,
                                                              const DataModel::AbstractSnapshot* snapshot ) const;

      /// \brief Search the list of available surface property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property The property whose values are requested.
      /// \param [in] snapshot The snapshot time at which the values were calculated.
      /// \param [in] surface  The surface with which the values are associated.
      SurfacePropertyPtr findSurfacePropertyValues ( const DataModel::AbstractProperty* property,
                                                     const DataModel::AbstractSnapshot* snapshot,
                                                     const DataModel::AbstractSurface*  surface ) const;

      /// \brief Search the list of available formation map property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation with which the values are associated.
      FormationMapPropertyPtr findFormationMapPropertyValues ( const DataModel::AbstractProperty*  property,
                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                               const DataModel::AbstractFormation* formation ) const;

      /// \brief Search the list of available formation property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation with which the values are associated.
      FormationPropertyPtr findFormationPropertyValues ( const DataModel::AbstractProperty*  property,
                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                         const DataModel::AbstractFormation* formation ) const;

      /// \brief Search the list of available formation surface property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] formation The formation with which the values are associated.
      /// \param [in] surface   The surface with which the values are associated.
      FormationSurfacePropertyPtr findFormationSurfacePropertyValues ( const DataModel::AbstractProperty*  property,
                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                       const DataModel::AbstractFormation* formation,
                                                                       const DataModel::AbstractSurface*   surface ) const;      

      /// \brief Search the list of available reservoir property values for a specific set of values.
      ///
      /// If the values are not found then a null will be returned.
      /// \param [in] property  The property whose values are requested.
      /// \param [in] snapshot  The snapshot time at which the values were calculated.
      /// \param [in] reservoir The reservoir with which the values are associated.
      ReservoirPropertyPtr findReservoirPropertyValues ( const DataModel::AbstractProperty*  property,
                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                         const DataModel::AbstractReservoir* reservoir ) const;

   private :

      /// \brief Mapping from property and snapshot to the associated surface property calculator.
      typedef PropertySnapshotCalculatorMap<SurfacePropertyCalculatorPtr> SurfacePropertyCalculatorMap;

      /// \brief Mapping from property and snapshot to the associated formation map property calculator.
      typedef PropertySnapshotCalculatorMap<FormationMapPropertyCalculatorPtr> FormationMapPropertyCalculatorMap;

      /// \brief Mapping from property and snapshot to the associated formation property calculator.
      typedef PropertySnapshotCalculatorMap<FormationPropertyCalculatorPtr> FormationPropertyCalculatorMap;

      /// \brief Mapping from property and snapshot to the associated formation and surface property calculator.
      typedef PropertySnapshotCalculatorMap<FormationSurfacePropertyCalculatorPtr> FormationSurfacePropertyCalculatorMap;

      /// \brief Mapping from property and snapshot to the associated reservoir property calculator.
      typedef PropertySnapshotCalculatorMap<ReservoirPropertyCalculatorPtr> ReservoirPropertyCalculatorMap;



      /// \brief Map of property to surface-property calculator.
      SurfacePropertyCalculatorMap m_surfacePropertyCalculators;

      /// \brief List of all surface-properties that have been stored.
      SurfacePropertyList          m_surfaceProperties;

      /// \brief Map of property to formation-property calculator.
      FormationMapPropertyCalculatorMap m_formationMapPropertyCalculators;

      /// \brief List of all formation-properties that have been stored.
      FormationMapPropertyList         m_formationMapProperties;

      /// \brief Map of property to formation and surface property calculator.
      FormationSurfacePropertyCalculatorMap m_formationSurfacePropertyCalculators;

      /// \brief List of all formation and surface properties that have been stored.
      FormationSurfacePropertyList    m_formationSurfaceProperties;

      /// \brief Map of property to formation-property calculator.
      FormationPropertyCalculatorMap  m_formationPropertyCalculators;

      /// \brief List of all formation-properties that have been stored.
      FormationPropertyList           m_formationProperties;

      /// \brief Map of property to reservoir-property calculator.
      ReservoirPropertyCalculatorMap m_reservoirPropertyCalculators;

      /// \brief List of all reservoir-properties that have been stored.
      ReservoirPropertyList          m_reservoirProperties;

   };

} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__ABSTRACT_PROPERTY_MANAGER_H
