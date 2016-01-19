#include "FormationSurfacePropertyAtSurface.h"

#include <limits>

DerivedProperties::FormationSurfacePropertyAtSurface::FormationSurfacePropertyAtSurface ( const FormationPropertyPtr&       formationProperty,
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


double DerivedProperties::FormationSurfacePropertyAtSurface::get ( unsigned int i,
                                                                   unsigned int j ) const {
   return m_formationProperty->get ( i, j, m_offset );
}

double DerivedProperties::FormationSurfacePropertyAtSurface::getUndefinedValue () const {
   return m_formationProperty->getUndefinedValue ();
}

bool DerivedProperties::FormationSurfacePropertyAtSurface::isRetrieved () const {
   return m_formationProperty->isRetrieved ();
}

void DerivedProperties::FormationSurfacePropertyAtSurface::retrieveData () const {
   m_formationProperty->retrieveData ();
}

void DerivedProperties::FormationSurfacePropertyAtSurface::restoreData () const {
   m_formationProperty->restoreData ();
}

bool DerivedProperties::FormationSurfacePropertyAtSurface::isPrimary () const {
   return m_formationProperty->isPrimary();
}
