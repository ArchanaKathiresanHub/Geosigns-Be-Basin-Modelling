#include "FormationSubdomainElementGrid.h"

#include "layer.h"

//------------------------------------------------------------//

FormationSubdomainElementGrid::FormationSubdomainElementGrid ( LayerProps& layer ) : m_formation ( layer ) {

   m_elements.create ( m_formation.getVolumeGrid ( NumberOfPVTComponents ).getDa ());

   copyLayerElements ();
   linkElements ();

}

//------------------------------------------------------------//

FormationSubdomainElementGrid::~FormationSubdomainElementGrid () {
}

//------------------------------------------------------------//

void FormationSubdomainElementGrid::copyLayerElements () {

   const LayerElementArray& layerElements = m_formation.getLayerElements ();

   int i;
   int j;
   int k;

   for ( i = layerElements.firstI ( true ); i <= layerElements.lastI ( true ); ++i ) {

      for ( j = layerElements.firstJ ( true ); j <= layerElements.lastJ ( true ); ++j ) {

         for ( k = layerElements.firstK (); k <= layerElements.lastK (); ++k ) {
            m_elements ( i, j, k ).setLayerElement ( layerElements ( i, j, k ));
         }

      }

   }

}

//------------------------------------------------------------//

void FormationSubdomainElementGrid::linkElements () {

   const MapElementArray& mapElements = FastcauldronSimulator::getInstance ().getMapElementArray ();

   unsigned int i;
   unsigned int j;
   unsigned int k;

   for ( i = mapElements.firstI ( true ); i <= mapElements.lastI ( true ); ++i ) {

      for ( j = mapElements.firstJ ( true ); j <= mapElements.lastJ ( true ); ++j ) {

         const MapElement& mapElement = mapElements ( i, j );

         for ( k = 0; k < m_formation.getMaximumNumberOfElements (); ++k ) {
            m_elements ( i, j, k ).clearNeighbours ();

            if ( mapElement.isOnProcessor ()) {

               if ( not mapElement.isOnDomainBoundary ( MapElement::Front )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Front, &m_elements ( i, j - 1, k ));
               }

               if ( not mapElement.isOnDomainBoundary ( MapElement::Right )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Right, &m_elements ( i + 1, j, k ));
               }

               if ( not mapElement.isOnDomainBoundary ( MapElement::Back )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Back, &m_elements ( i, j + 1, k ));
               }

               if ( not mapElement.isOnDomainBoundary ( MapElement::Left )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Left, &m_elements ( i - 1, j, k ));
               }

               if ( k > 0 ) {
                  // Add element below to bottom face.
                  // The elments (in the thickness of the layer) are numbered 0 = deepest, n == shallowest.
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::DeepFace, &m_elements ( i, j, k - 1 ));
               }

               if ( k < m_formation.getMaximumNumberOfElements () - 1 ) {
                  // Add element above to top face.
                  // The elments (in the thickness of the layer) are numbered 0 = deepest, n == shallowest.
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::ShallowFace, &m_elements ( i, j, k + 1 ));
               }

            }

         }

      }

   }

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

bool FormationSubdomainElementGrid::isActive () const {
   return m_formation.isActive ();
}

//------------------------------------------------------------//

LayerProps& FormationSubdomainElementGrid::getFormation () {
   return m_formation;
}

//------------------------------------------------------------//

const LayerProps& FormationSubdomainElementGrid::getFormation () const {
   return m_formation;
}
//------------------------------------------------------------//
