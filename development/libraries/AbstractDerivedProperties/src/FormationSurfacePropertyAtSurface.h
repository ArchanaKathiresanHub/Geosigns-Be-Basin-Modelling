//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_AT_SURFACE_H
#define ABSTRACTDERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_AT_SURFACE_H

#include "AbstractSurface.h"

#include "FormationProperty.h"
#include "FormationSurfaceProperty.h"

namespace AbstractDerivedProperties {

   /// \brief Stores the values of the designated formation-property with an offset for the particular surface.
   class FormationSurfacePropertyAtSurface : public FormationSurfaceProperty {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] formationProperty  The property-values required.
      /// \param [in] surface            The surface to determine the offset.
      /// \pre formationProperty is not null.
      /// \pre surface is not null.
      /// \pre surface is attached to the formation associated to the formationProperty, i.e. must be either the top or bottom surface of the formation.
      FormationSurfacePropertyAtSurface ( const FormationPropertyPtr&       formationProperty,
                                          const DataModel::AbstractSurface* surface );

      /// \brief Get the value of the property at the position i,j.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Determine wether or not the data has been retrieved.
      virtual bool isRetrieved () const;

      /// \brief Retrieve the grid map
      virtual void retrieveData () const;

      /// \brief Restore the grid map.
      virtual void restoreData () const;

      /// \brief Return the primary property of the formation property
      virtual bool isPrimary() const; 

   private :

      const FormationPropertyPtr m_formationProperty;
      unsigned int               m_offset;

   }; 

} // namespace AbstractDerivedProperties

#endif // ABSTRACTDERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_AT_SURFACE_H
