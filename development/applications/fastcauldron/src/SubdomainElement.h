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

/// \brief A subdomain-element is an element that is part of a subdomain.
///
/// They have basic dof-, node- and element numbering for each subdomain.
typedef BaseElement<NullElement> SubdomainElement;


/// \typedef SubdomainElementArray
/// \brief A three-d array of subdomain-elements.
typedef PETSc_Local_3D_Array<SubdomainElement> SubdomainElementArray;


#endif // _FASTCAULDRON__SUBDOMAIN_ELEMENT__H_
