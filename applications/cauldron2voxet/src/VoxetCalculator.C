#include "VoxetCalculator.h"
#include <cmath>

#include "array.h"

// Data Access
#include "Formation.h"

#undef Min
#define Min(a,b)  ((a) < (b) ? (a) : (b))

#undef Max
#define Max(a,b)  ((a) > (b) ? (a) : (b))


const float VoxetCalculator::DefaultNullValue = 99999.0;


//------------------------------------------------------------//

VoxetCalculator::VoxetCalculator ( const ProjectHandle*   projectHandle,
                                   const GridDescription& gridDescription ) : m_gridDescription ( gridDescription ) {

   m_depthProperty = 0;
   m_projectHandle = projectHandle;
}

//------------------------------------------------------------//

VoxetCalculator::~VoxetCalculator () {
   Array<bool>::delete2d ( m_nodeIsDefined );
   Array<bool>::delete2d ( m_interpolatorIsDefined );
}

//------------------------------------------------------------//

void VoxetCalculator::setDepthProperty ( const Property* depth ) {
   m_depthProperty = depth;
}

//------------------------------------------------------------//

void VoxetCalculator::setDefinedNodes ( const PropertyValueList* depthPropertyValueList)
{
   int i;
   int j;

   PropertyValueList::const_iterator depthPropertyIter;
   const PropertyValue* depthPropertyValue;
   const GridMap*       depthGridMap;

   m_nodeIsDefined = Array<bool>::create2d ( m_gridDescription.getCauldronNodeCount ( 0 ), m_gridDescription.getCauldronNodeCount ( 1 ), true );

   for (depthPropertyIter = depthPropertyValueList->begin (); depthPropertyIter != depthPropertyValueList->end (); ++depthPropertyIter ) {
      depthPropertyValue = *depthPropertyIter;

      if ( depthPropertyValue->getFormation () != 0 && depthPropertyValue->getFormation ()->kind () == Interface::BASEMENT_FORMATION ) {
         continue;
      }

      depthGridMap = depthPropertyValue->getGridMap ();
      assert ( depthGridMap != 0);

      for ( i = 0; i < m_gridDescription.getCauldronNodeCount ( 0 ); ++i ) {

         for ( j = 0; j < m_gridDescription.getCauldronNodeCount ( 1 ); ++j ) {
            m_nodeIsDefined [ i ][ j ] = m_nodeIsDefined [ i ][ j ] && depthGridMap->valueIsDefined ( i, j );
         }

      } 

   }
}

//------------------------------------------------------------//

void VoxetCalculator::addProperty ( const Property* property ) {

   PropertyInterpolator* newProperty2 = new PropertyInterpolator ( m_gridDescription.getVoxetNodeCount ( 0 ), m_gridDescription.getVoxetNodeCount ( 1 ), property );
   m_propertyInterpolators [ property ] = newProperty2;
}

//------------------------------------------------------------//

void VoxetCalculator::deleteProperty ( const Property* property ) {

   PropertyInterpolatorMap::iterator propertyIter = m_propertyInterpolators.find ( property );

   if ( propertyIter != m_propertyInterpolators.end ()) { 
      m_propertyInterpolators.erase ( propertyIter );
      delete (*propertyIter).second;
   }

}

//------------------------------------------------------------//

int VoxetCalculator::computeInterpolators ( const Snapshot * snapshot,
                                            const bool   verbose ) {

   m_interpolatorIsDefined = Array<bool>::create2d ( m_gridDescription.getVoxetNodeCount ( 0 ), m_gridDescription.getVoxetNodeCount ( 1 ), true );

   PropertyValueList* depthPropertyValueList = m_projectHandle->getPropertyValues (FORMATION, m_depthProperty, snapshot, 0, 0, 0);

   if ( depthPropertyValueList->size () == 0 ) {
      cerr << "Could not find the Depth property results in the project file " << endl
            << "Are you sure the project file contains output data?" << endl;
      return -1;
   }

   setDefinedNodes ( depthPropertyValueList );
   initialiseInterpolators ( depthPropertyValueList, snapshot, verbose );
   calculatorInterpolatorValues ( depthPropertyValueList, verbose );

   delete depthPropertyValueList;
   return 0;
}

//------------------------------------------------------------//

bool VoxetCalculator::validCauldronElement ( const int i, const int j ) const {

   if ( i < 0 || j < 0 || i >= m_gridDescription.getCauldronNodeCount ( 0 ) || j >= m_gridDescription.getCauldronNodeCount ( 1 )) {
      return false;
   } else {

      int cauldronI = Min ( i, m_gridDescription.getCauldronNodeCount ( 0 ) - 2 );
      int cauldronJ = Min ( j, m_gridDescription.getCauldronNodeCount ( 1 ) - 2 );

      return m_nodeIsDefined [ cauldronI     ][ cauldronJ     ] &&
             m_nodeIsDefined [ cauldronI + 1 ][ cauldronJ     ] &&
             m_nodeIsDefined [ cauldronI + 1 ][ cauldronJ + 1 ] &&
             m_nodeIsDefined [ cauldronI     ][ cauldronJ + 1 ];
   }

}

//------------------------------------------------------------//

void VoxetCalculator::initialiseInterpolators ( const PropertyValueList* depthPropertyValueList,
                                                const Snapshot*          snapshot,
                                                const bool               verbose )
{
   int sedimentCount = 0;
   int numberOfLayerDepthNodes;
   int i;
   int j;
   int k;
   int interp;
   unsigned int cauldronI;
   unsigned int cauldronJ;

   float xValue;
   float yValue;

   PropertyInterpolatorMap::iterator propertyIter;

   const PropertyValue *depthPropertyValue;
   const GridMap *depthGridMap;

   PropertyValueList::const_iterator depthPropertyIter;

   // Get the property-values lists for each property-interpolator.
   for (propertyIter = m_propertyInterpolators.begin (); propertyIter != m_propertyInterpolators.end (); ++propertyIter)
   {
      if (verbose)
      {
         cout << "Initialising cauldron property " << propertyIter->first->getName () << endl;
      }
      propertyIter->second->setSnapshot (m_projectHandle, snapshot);
      propertyIter->second->getInterpolator ().setNullValue (propertyIter->second->getPropertyValue (0)->getGridMap ()->getUndefinedValue ());
   }


   // Count the number of sediments.
   for (depthPropertyIter = depthPropertyValueList->begin (); depthPropertyIter != depthPropertyValueList->end (); ++depthPropertyIter)
   {
      depthPropertyValue = *depthPropertyIter;

      if (depthPropertyValue->getFormation () != 0 && depthPropertyValue->getFormation ()->kind () == Interface::BASEMENT_FORMATION)
      {
         continue;
      }

      ++sedimentCount;
   }

   // Tell all interpolators the number of sediments or invalidate the itnerpolator.
   for (propertyIter = m_propertyInterpolators.begin (); propertyIter != m_propertyInterpolators.end (); ++propertyIter)
   {
      VoxetDomainInterpolator & interpolator = propertyIter->second->getInterpolator ();

      if (verbose)
      {
         cout << " initialising interpolator for property: " << propertyIter->first->getName () << endl << flush;
      }

      for (i = 0, xValue = m_gridDescription.getVoxetGridOrigin (0);
           i < m_gridDescription.getVoxetNodeCount (0); ++i, xValue += m_gridDescription.getVoxetGridDelta (0))
      {

         if (xValue < m_gridDescription.getCauldronGridOrigin (0, true) || xValue > m_gridDescription.getCauldronGridMaximum (0, true))
         {

            // x value lies outside of the cauldron domain so all interpolators (i,XX) are not valid interpolators.
            for (j = 0; j < m_gridDescription.getVoxetNodeCount (1); ++j, yValue += m_gridDescription.getVoxetGridDelta (1))
            {
               interpolator (i, j).setNullInterpolator (true);
               interpolator (i, j).setNullValue (DefaultNullValue);
               m_interpolatorIsDefined[i][j] = false;
            }

         }
         else
         {
            for (j = 0, yValue = m_gridDescription.getVoxetGridOrigin (1);
                 j < m_gridDescription.getVoxetNodeCount (1); ++j, yValue += m_gridDescription.getVoxetGridDelta (1))
            {

               if (yValue < m_gridDescription.getCauldronGridOrigin (1, true) || yValue > m_gridDescription.getCauldronGridMaximum (1, true))
               {
                  // y value lies outside of the cauldron domain so the interpolator (i,j) is not valid.
                  interpolator (i, j).setNullInterpolator (true);
                  interpolator (i, j).setNullValue (DefaultNullValue);

                  m_interpolatorIsDefined[i][j] = false;
               }
               else
               {
                  if (!m_gridDescription.getCauldronGrid ()->getGridPoint (xValue, yValue, cauldronI, cauldronJ) ||
			!validCauldronElement (cauldronI, cauldronJ))
                  {
                     // The (x,y) value lies outside of the valid cauldron calculation domain so the interpolator (i,j) is not valid.
                     interpolator (i, j).setNullInterpolator (true);
                     interpolator (i, j).setNullValue (DefaultNullValue);
                     m_interpolatorIsDefined[i][j] = false;
                  }
                  else
                  {
                     interpolator (i, j).setNullInterpolator (false);
                     m_interpolatorIsDefined[i][j] = true;
                     interpolator (i, j).setNumberOfLayers (sedimentCount);
                  }

               }

            }

         }

      }

   }

   // For each interpolator set the number of layers.
   for (depthPropertyIter = depthPropertyValueList->begin (); depthPropertyIter != depthPropertyValueList->end (); ++depthPropertyIter)
   {
      depthPropertyValue = *depthPropertyIter;

      if (depthPropertyValue->getFormation () != 0 && depthPropertyValue->getFormation ()->kind () == Interface::BASEMENT_FORMATION)
      {
         continue;
      }

      depthGridMap = depthPropertyValue->getGridMap ();
      assert (depthGridMap != 0);

      numberOfLayerDepthNodes = depthGridMap->getDepth ();

      for (propertyIter = m_propertyInterpolators.begin (); propertyIter != m_propertyInterpolators.end (); ++propertyIter)
      {
         VoxetDomainInterpolator & interpolator = propertyIter->second->getInterpolator ();

         for (i = 0; i < m_gridDescription.getVoxetNodeCount (0); ++i)
         {

            for (j = 0; j < m_gridDescription.getVoxetNodeCount (1); ++j)
            {

               if (!interpolator (i, j).isNullInterpolator ())
               {
                  LayerInterpolator & layerInterp = interpolator (i, j).addLayer (depthPropertyValue->getFormation ()->getName ());
                  layerInterp.setMaximumNumberOfSamples (numberOfLayerDepthNodes);
               }

            }

         }

      }

   }

   if (verbose)
   {
      cout << endl;
   }

}

//------------------------------------------------------------//

int VoxetCalculator::getMaximumNumberOfLayerNodes ( const PropertyValueList* depthPropertyValueList ) const {

   int maximumNumberOfNodes = 0;
   int layerNodeCount;

   PropertyValueList::const_iterator depthPropertyIter;
   const PropertyValue* depthPropertyValue;

   // Count the number of sediments.
   for (depthPropertyIter = depthPropertyValueList->begin (); depthPropertyIter != depthPropertyValueList->end (); ++depthPropertyIter ) {
      depthPropertyValue = *depthPropertyIter;

      if ( depthPropertyValue->getFormation () != 0 && depthPropertyValue->getFormation ()->kind () == Interface::BASEMENT_FORMATION ) {
         continue;
      }

      layerNodeCount = depthPropertyValue->getGridMap ()->getDepth ();

      if ( layerNodeCount > maximumNumberOfNodes ) {
         maximumNumberOfNodes = layerNodeCount;
      }

   }

   return maximumNumberOfNodes;
}

//------------------------------------------------------------//

void VoxetCalculator::calculatorInterpolatorValues ( const PropertyValueList* depthPropertyValueList,
                                                     const bool               verbose )
{
   int i;
   int j;
   int k;
   int l;
   int formationCount;

   double cauldronI;
   double cauldronJ;

   float xValue;
   float yValue;
   float property;
   float depth;

   string formationName;

   PropertyInterpolatorMap::iterator propertyIter;

   const int maximumNumberOfLayerNodes = getMaximumNumberOfLayerNodes (depthPropertyValueList);

   const PropertyValue *depthPropertyValue;
   const GridMap *depthGridMap;

   PropertyValueList::const_iterator depthPropertyIter;

   for (propertyIter = m_propertyInterpolators.begin (); propertyIter != m_propertyInterpolators.end (); ++propertyIter)
   {

      VoxetDomainInterpolator & interpolator = propertyIter->second->getInterpolator ();

      if (verbose)
      {
         cout << " computing interpolator for property: " << propertyIter->first->getName () << endl << flush;
      }

      for (i = 0, xValue = m_gridDescription.getVoxetGridOrigin (0);
           i < m_gridDescription.getVoxetNodeCount (0); ++i, xValue += m_gridDescription.getVoxetGridDelta (0))
      {
         for (j = 0, yValue = m_gridDescription.getVoxetGridOrigin (1);
              j < m_gridDescription.getVoxetNodeCount (1); ++j, yValue += m_gridDescription.getVoxetGridDelta (1))
         {

            // These numbers may be out of the range of the cauldron domain.
            m_gridDescription.getCauldronGrid ()->getGridPoint (xValue, yValue, cauldronI, cauldronJ);

            if (m_interpolatorIsDefined[i][j])
            {

               // Initialise the interpolator values.
               for (depthPropertyIter = depthPropertyValueList->begin (), formationCount = 0;
                    depthPropertyIter != depthPropertyValueList->end (); ++depthPropertyIter, ++formationCount)
               {

                  depthPropertyValue = *depthPropertyIter;

                  if (depthPropertyValue->getFormation () != 0 && depthPropertyValue->getFormation ()->kind () == Interface::BASEMENT_FORMATION)
                  {
                     continue;
                  }

                  const PropertyValue *propertyValue = propertyIter->second->getPropertyValue (formationCount);

                  if (!propertyValue)
                  {
                     if (verbose && i == 0 && j == 0)
                     {
                        cout << "    No data available for formation " << depthPropertyValue->getFormation ()->getName () << endl;
                     }
                     continue;
                  }

                  const GridMap *propertyGridMap = propertyValue->getGridMap ();

                  depthGridMap = depthPropertyValue->getGridMap ();
                  formationName = depthPropertyValue->getFormation ()->getName ();

                  // For each property-layer-interpolator set the layer top and bottom depths.
                  if (!interpolator (i, j).isNullInterpolator ())
                  {
                     LayerInterpolator & layerInterp = interpolator (i, j) (formationName);

                     layerInterp.clear ();

                     double topDepth = 1e9;
                     double bottomDepth = -1e9;

                     for (l = 0; l <= depthGridMap->getDepth () - 1; ++l)
                     {
                        depth = depthGridMap->getValue (cauldronI, cauldronJ, (double) l);
                        topDepth = Min (depth, topDepth);
                        bottomDepth = Max (depth, bottomDepth);

                        property = propertyGridMap->getValue (cauldronI, cauldronJ, (double) l);
                        layerInterp.addSample (depth, property);
                     }

                     layerInterp.setInterval (topDepth, bottomDepth);

                     layerInterp.freeze ();
                  }

               }

            }

         }

      }

   }

   if (verbose)
   {
      cout << endl;
   }

}

//------------------------------------------------------------//


float VoxetCalculator::interpolatedProperty ( const Grid*    grid,
                                              const GridMap* property,
                                              const unsigned int computedI,
                                              const unsigned int computedJ,
                                              const unsigned int k,
                                              const float    x,
                                              const float    y ) const {

   const unsigned int i = Min ( computedI, m_gridDescription.getCauldronNodeCount ( 0 ) - 2 );
   const unsigned int j = Min ( computedJ, m_gridDescription.getCauldronNodeCount ( 1 ) - 2 );


   float props [ 4 ];
   float weights [ 4 ];

   double x1;
   double x2;
   double y1;
   double y2;

   int l;
   float value = 0.0;
   float nullValue = property->getUndefinedValue ();

   if ( property->valueIsDefined ( i, j )) {
      props [ 0 ] = property->getValue ( i, j, k );
   } else {
      return property->getUndefinedValue ();
   }

   if ( property->valueIsDefined ( i + 1, j )) {
      props [ 1 ] = property->getValue ( i + 1, j, k );
   } else {
      return property->getUndefinedValue ();
   }

   if ( property->valueIsDefined ( i + 1, j + 1 )) {
      props [ 2 ] = property->getValue ( i + 1, j + 1, k );
   } else {
      return property->getUndefinedValue ();
   }

   if ( property->valueIsDefined ( i, j + 1 )) {
      props [ 3 ] = property->getValue ( i, j + 1, k );
   } else {
      return property->getUndefinedValue ();
   }

   grid->getPosition ( i, j, x1, y1 );
   grid->getPosition ( i + 1, j + 1, x2, y2 );

   if ( x1 == nullValue || x2 == nullValue || y1 == nullValue || y2 == nullValue ) {
      return nullValue;
   }

   weights [ 0 ] = ( x2 - x ) / ( x2 - x1 ) * ( y2 - y ) / ( y2 - y1 );
   weights [ 1 ] = ( x - x1 ) / ( x2 - x1 ) * ( y2 - y ) / ( y2 - y1 );
   weights [ 2 ] = ( x - x1 ) / ( x2 - x1 ) * ( y - y1 ) / ( y2 - y1 );
   weights [ 3 ] = ( x2 - x ) / ( x2 - x1 ) * ( y - y1 ) / ( y2 - y1 );

   for ( l = 0; l < 4; ++l ) {
      value += weights [ l ] * props [ l ];
   }

   return value;
}

//------------------------------------------------------------//

void VoxetCalculator::print ( std::ostream& o ) const {

   PropertyInterpolatorMap::const_iterator propertyIter;
   int count;
   int i;
   int j;
   int k;

   for ( propertyIter = m_propertyInterpolators.begin (), count = 0; propertyIter != m_propertyInterpolators.end (); ++propertyIter, ++count ) {

      const VoxetDomainInterpolator& interpolator = propertyIter->second->getInterpolator ();

      o << " Interpolator " << count << "  " << propertyIter->first->getName () << std::endl;

      for ( j = 0; j < m_gridDescription.getVoxetNodeCount ( 0 ); ++j ) {

         for ( k = 0; k < m_gridDescription.getVoxetNodeCount ( 1 ); ++k ) {
            o << " Depth Interpolator " << j << "  " << k << std::endl;
            interpolator ( j, k ).print ( o );
         }

      } 

   }


}


//------------------------------------------------------------//

void VoxetCalculator::computeProperty ( const CauldronProperty*  property,
                                              VoxetPropertyGrid& propertyValues,
                                        const bool               verbose ) {

   int i;
   int j;
   int k;

   int outputDotNumber = m_gridDescription.getVoxetNodeCount () / 20;
   int count;

   float x;
   float y;
   float z;
   const float conversionFactor = property->getConversionFactor ();

   PropertyInterpolatorMap::iterator propertyIter = m_propertyInterpolators.find ( property->getProperty ());

   if ( propertyIter != m_propertyInterpolators.end ()) {

      if ( verbose ) {
         cout << " interpolating voxet for property : " << propertyIter->first->getName () << "  ";
      }

      const VoxetDomainInterpolator& propertyInterpolator = propertyIter->second->getInterpolator ();

      for ( i = 0, count = 0; i < m_gridDescription.getVoxetNodeCount ( 0 ); ++i ) {

         for ( j = 0; j < m_gridDescription.getVoxetNodeCount ( 1 ); ++j ) {

            const DepthInterpolator& interpolator = propertyInterpolator ( i, j );

            for ( k = 0, z = m_gridDescription.getVoxetGridOrigin ( 2 );
                  k < m_gridDescription.getVoxetNodeCount ( 2 );
                  ++k, z += m_gridDescription.getVoxetGridDelta ( 2 ), ++count ) {
               propertyValues ( i, j, k ) = conversionFactor * interpolator ( z );

               if ( verbose && count == outputDotNumber ) {
                  cout << "." << flush;
                  count = 0;
               }

            }

         }

      }

      if ( verbose ) {
         cout << " done." << endl;
      }

   } else {

      for ( i = 0; i < m_gridDescription.getVoxetNodeCount ( 0 ); ++i ) {

         for ( j = 0; j < m_gridDescription.getVoxetNodeCount ( 1 ); ++j ) {

            for ( k = 0, z = m_gridDescription.getVoxetGridOrigin ( 2 );
                  k < m_gridDescription.getVoxetNodeCount ( 2 );
                  ++k, z += m_gridDescription.getVoxetGridDelta ( 2 )) {
               propertyValues ( i, j, k ) = DefaultNullValue;
            }

         }

      }

   }

}

//------------------------------------------------------------//

float VoxetCalculator::getNullValue ()
{
   float nullValue;

   PropertyInterpolatorMap::const_iterator interpolators = m_propertyInterpolators.begin ();

   if (interpolators != m_propertyInterpolators.end ())
   {
      nullValue = interpolators->second->getPropertyValue (0)->getGridMap ()->getUndefinedValue ();
   }
   else
   {
      nullValue = DefaultNullValue;
   }

   return nullValue;
}

//------------------------------------------------------------//

float VoxetCalculator::getNullValue ( const Property* property ) const {

   float nullValue;

   if ( property != 0 ) {
      PropertyInterpolatorMap::const_iterator interpolators = m_propertyInterpolators.find ( property );

      if ( interpolators != m_propertyInterpolators.end ()) {
         nullValue = interpolators->second->getPropertyValue ( 0 )->getGridMap ()->getUndefinedValue ();
      } else {
         nullValue = DefaultNullValue;
      }

   } else {
      nullValue = DefaultNullValue;
   }

   return nullValue;
}

//------------------------------------------------------------//

VoxetDomainInterpolator& VoxetCalculator::getVoxetDomainInterpolator ( const Property* property )
{
   PropertyInterpolatorMap::iterator interpolator = m_propertyInterpolators.find (property);

   if (interpolator != m_propertyInterpolators.end ())
   {
      return interpolator->second->getInterpolator ();
   }
   else
   {
      // Error;
      assert (false);

      static VoxetDomainInterpolator nullResult ( 0, 0 );

      return nullResult;
   }
}

//------------------------------------------------------------//

VoxetDomainInterpolator& VoxetCalculator::getAnyVoxetDomainInterpolator ()
{
   PropertyInterpolatorMap::iterator interpolator = m_propertyInterpolators.begin ();

   if (interpolator != m_propertyInterpolators.end ())
   {
      return interpolator->second->getInterpolator ();
   }
   else
   {
      assert (false);

      static VoxetDomainInterpolator nullResult ( 0, 0 );

      return nullResult;
   }

}

//------------------------------------------------------------//

VoxetCalculator::PropertyInterpolator::PropertyInterpolator ( const unsigned int nodesX,
                                                              const unsigned int nodesY,
                                                              const Property*    property ) :
   m_property ( property ),
   m_interpolators ( nodesX, nodesY ) {

   m_propertyValues = 0;
}

//------------------------------------------------------------//

VoxetCalculator::PropertyInterpolator::~PropertyInterpolator () {

   if ( m_propertyValues != 0 ) {
      size_t i;

      for ( i = 0; i < m_propertyValues->size (); ++i ) {
         const PropertyValue *propertyValue = (*m_propertyValues)[ i ];

         if ( propertyValue == 0 ) {
            continue;
         }

         const GridMap *propertyGridMap = propertyValue->getGridMap ();

         if ( propertyGridMap != 0 ) {
            propertyGridMap->release ();
         }
         
      }

      delete m_propertyValues;
   }

}

//------------------------------------------------------------//

void VoxetCalculator::PropertyInterpolator::setSnapshot ( const ProjectHandle* projectHandle,
                                                          const Snapshot*      snapshot ) {
   m_snapshot = snapshot;
   m_propertyValues = projectHandle->getPropertyValues (FORMATION, m_property, m_snapshot, 0, 0, 0);
}

//------------------------------------------------------------//

const Snapshot* VoxetCalculator::PropertyInterpolator::getSnapshot () const {
   return m_snapshot;
}

//------------------------------------------------------------//

const Property* VoxetCalculator::PropertyInterpolator::getProperty () const {
   return m_property;
}

//------------------------------------------------------------//

const PropertyValueList& VoxetCalculator::PropertyInterpolator::getPropertyValues () const {
   return *m_propertyValues;
}

//------------------------------------------------------------//

const PropertyValue* VoxetCalculator::PropertyInterpolator::getPropertyValue ( const unsigned int position ) const {
   return (*m_propertyValues)[ position ];
}

//------------------------------------------------------------//

VoxetDomainInterpolator& VoxetCalculator::PropertyInterpolator::getInterpolator () {
   return m_interpolators;
}

//------------------------------------------------------------//

const GridDescription& VoxetCalculator::getGridDescription () const {
   return m_gridDescription;
}
