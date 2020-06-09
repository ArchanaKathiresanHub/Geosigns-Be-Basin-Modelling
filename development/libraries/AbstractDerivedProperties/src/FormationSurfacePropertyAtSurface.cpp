//
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationSurfacePropertyAtSurface.h"

#include <limits>

AbstractDerivedProperties::FormationSurfacePropertyAtSurface::FormationSurfacePropertyAtSurface ( const FormationPropertyPtr&       formationProperty,
                                                                                          const DataModel::AbstractSurface* surface ) :
   FormationSurfaceProperty ( formationProperty->getProperty (), formationProperty->getSnapshot (), formationProperty->getFormation (), surface, formationProperty->getGrid ()),
   m_formationProperty ( formationProperty )
{

   if ( formationProperty->getFormation ()->getTopSurfaceName () == surface->getName ()) {
      m_offset = formationProperty->lastK ();
   } else if ( formationProperty->getFormation ()->getBottomSurfaceName () == surface->getName ()) {
      m_offset = formationProperty->firstK ();
   } else {
      // Error.
      m_offset = std::numeric_limits<unsigned int>::max ();
   }

}


double AbstractDerivedProperties::FormationSurfacePropertyAtSurface::get ( unsigned int i,
                                                                   unsigned int j ) const {
   return m_formationProperty->get ( i, j, m_offset );
}

double AbstractDerivedProperties::FormationSurfacePropertyAtSurface::getUndefinedValue () const {
   return m_formationProperty->getUndefinedValue ();
}

bool AbstractDerivedProperties::FormationSurfacePropertyAtSurface::isRetrieved () const {
   return m_formationProperty->isRetrieved ();
}

void AbstractDerivedProperties::FormationSurfacePropertyAtSurface::retrieveData () const {
   m_formationProperty->retrieveData ();
}

void AbstractDerivedProperties::FormationSurfacePropertyAtSurface::restoreData () const {
   m_formationProperty->restoreData ();
}

bool AbstractDerivedProperties::FormationSurfacePropertyAtSurface::isPrimary () const {
   return m_formationProperty->isPrimary();
}
