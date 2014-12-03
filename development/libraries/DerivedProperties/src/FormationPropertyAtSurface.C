#include "FormationPropertyAtSurface.h"

#include <limits>

DerivedProperties::FormationPropertyAtSurface::FormationPropertyAtSurface ( const FormationPropertyPtr&       formationProperty,
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


double DerivedProperties::FormationPropertyAtSurface::get ( unsigned int i,
                                                            unsigned int j ) const {
   return m_formationProperty->get ( i, j, m_offset );
}

double DerivedProperties::FormationPropertyAtSurface::getUndefinedValue () const {
   return m_formationProperty->getUndefinedValue ();
}

bool DerivedProperties::FormationPropertyAtSurface::isRetrieved () const {
   return m_formationProperty->isRetrieved ();
}

void DerivedProperties::FormationPropertyAtSurface::retrieveData () const {
   m_formationProperty->retrieveData ();
}

void DerivedProperties::FormationPropertyAtSurface::restoreData () const {
   m_formationProperty->restoreData ();
}
