#ifndef FASTCAULDRON__SUBDOMAIN_ELEMENT__H
#define FASTCAULDRON__SUBDOMAIN_ELEMENT__H

#include "LayerElement.h"
#include "BoundaryId.h"

#include "BaseElement.h"

class NullElement {

public :
   NullElement () {}
   virtual ~NullElement () {}
};

typedef BaseElement<NullElement> SubdomainElement;


/// \typedef SubdomainElementArray
/// \brief A three-d array of subdomain-elements.
typedef PETSc_Local_3D_Array<SubdomainElement> SubdomainElementArray;


#endif // _FASTCAULDRON__SUBDOMAIN_ELEMENT__H_
