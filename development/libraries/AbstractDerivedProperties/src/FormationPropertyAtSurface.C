//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationPropertyAtSurface.h"

#include <limits>

AbstractDerivedProperties::FormationPropertyAtSurface::FormationPropertyAtSurface ( const FormationPropertyPtr&       formationProperty,
                                                                                    const DataModel::AbstractSurface* surface ) :
   SurfaceProperty ( formationProperty->getProperty (), formationProperty->getSnapshot (), surface, formationProperty->getGrid ()),
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


double AbstractDerivedProperties::FormationPropertyAtSurface::get ( unsigned int i,
                                                            unsigned int j ) const {
   return m_formationProperty->get ( i, j, m_offset );
}

double AbstractDerivedProperties::FormationPropertyAtSurface::getUndefinedValue () const {
   return m_formationProperty->getUndefinedValue ();
}

bool AbstractDerivedProperties::FormationPropertyAtSurface::isRetrieved () const {
   return m_formationProperty->isRetrieved ();
}

void AbstractDerivedProperties::FormationPropertyAtSurface::retrieveData () const {
   m_formationProperty->retrieveData ();
}

void AbstractDerivedProperties::FormationPropertyAtSurface::restoreData () const {
   m_formationProperty->restoreData ();
}

bool AbstractDerivedProperties::FormationPropertyAtSurface::isPrimary () const {

   return m_formationProperty->isPrimary();
}
