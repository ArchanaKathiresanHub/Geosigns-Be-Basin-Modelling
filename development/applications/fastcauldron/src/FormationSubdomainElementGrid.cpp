#include "FormationSubdomainElementGrid.h"

#include "layer.h"

//------------------------------------------------------------//

FormationSubdomainElementGrid::FormationSubdomainElementGrid ( LayerProps& layer ) :
   FormationElementGrid<SubdomainElement>( layer )
{
}

//------------------------------------------------------------//

FormationSubdomainElementGrid::~FormationSubdomainElementGrid () {
}

//------------------------------------------------------------//

void FormationSubdomainElementGrid::createVolumeGrid ( const int numberOfDofs ) {
   m_formation.createVolumeGrid ( numberOfDofs );
}

//------------------------------------------------------------//

ElementVolumeGrid& FormationSubdomainElementGrid::getVolumeGrid ( const int numberOfDofs ) {
   return m_formation.getVolumeGrid ( numberOfDofs );
}

//------------------------------------------------------------//

const ElementVolumeGrid& FormationSubdomainElementGrid::getVolumeGrid ( const int numberOfDofs ) const {
   return m_formation.getVolumeGrid ( numberOfDofs );
}

//------------------------------------------------------------//
