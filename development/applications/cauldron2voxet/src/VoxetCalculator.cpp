//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VoxetCalculator.h"
#include <cmath>

#include "array.h"

// Data Access
#include "Interface/Formation.h"
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"
#include "Interface/Property.h"
#include "Interface/Grid.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"

#undef Min
#define Min(a,b)  ((a) < (b) ? (a) : (b))

#undef Max
#define Max(a,b)  ((a) > (b) ? (a) : (b))


const float VoxetCalculator::DefaultNullValue = 99999.0;


//------------------------------------------------------------//

VoxetCalculator::VoxetCalculator ( const GeoPhysics::ProjectHandle*           projectHandle,
                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                   const GridDescription&                     gridDescription,
                                   const std::map<std::string, double >&      propertyNullValueReplaceLookup ) :
   m_projectHandle ( projectHandle ),
   m_propertyManager ( propertyManager ),
   m_gridDescription ( gridDescription ),
   m_propertyNullValueReplaceLookup ( propertyNullValueReplaceLookup ),
   m_nodeIsDefined ( 0 ),
   m_interpolatorIsDefined ( 0 ),
   m_depthProperty ( 0 )
{
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

void VoxetCalculator::setDefinedNodes ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList, const bool verbose )
{
   int i;
   int j;
   unsigned int lastK;

   AbstractDerivedProperties::FormationPropertyList::const_iterator depthPropertyIter;
   AbstractDerivedProperties::FormationPropertyPtr depthPropertyValue;
   double undefinedValue = DefaultNullValue;

   m_nodeIsDefined = Array<bool>::create2d ( m_gridDescription.getCauldronNodeCount ( 0 ), m_gridDescription.getCauldronNodeCount ( 1 ), true );

   if ( depthPropertyValueList.size () > 0 ) {
      undefinedValue = depthPropertyValueList [ 0 ]->getUndefinedValue ();
   }

   for (depthPropertyIter = depthPropertyValueList.begin (); depthPropertyIter != depthPropertyValueList.end (); ++depthPropertyIter ) {
      depthPropertyValue = *depthPropertyIter;

      if ( depthPropertyValue->getFormation () == 0 || (!useBasement () && dynamic_cast<const Interface::Formation*>(depthPropertyValue->getFormation ())->kind () == Interface::BASEMENT_FORMATION )) {
         continue;
      }

      if (dynamic_cast<const Interface::Formation*>(depthPropertyValue->getFormation ())->kind () == Interface::BASEMENT_FORMATION){
         if(verbose){
	    cout << "  Using basement: " << depthPropertyValue->getFormation ()->getName () << endl;
         }
      }


      lastK = depthPropertyValue->lastK ();

      for ( i = 0; i < m_gridDescription.getCauldronNodeCount ( 0 ); ++i ) {

         for ( j = 0; j < m_gridDescription.getCauldronNodeCount ( 1 ); ++j ) {
            m_nodeIsDefined [ i ][ j ] = m_nodeIsDefined [ i ][ j ] && depthPropertyValue->get ( i, j, lastK ) != undefinedValue;
         }

      }

   }
}

//------------------------------------------------------------//

void VoxetCalculator::addProperty ( const Property* property ) {

   PropertyInterpolator* newProperty2 = new PropertyInterpolator ( m_gridDescription.getVoxetNodeCount ( 0 ), m_gridDescription.getVoxetNodeCount ( 1 ), property );
   m_propertyInterpolators [ property ] = newProperty2;

   cout<< "  Property added: "<<property->getName()<<endl;
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
   AbstractDerivedProperties::FormationPropertyList depthDerivedPropertyValueList = m_propertyManager.getFormationProperties ( m_depthProperty, snapshot, useBasement ());

   if ( depthDerivedPropertyValueList.size () == 0 ) {
      cerr << "Could not find the Depth property results in the project file " << endl
            << "Are you sure the project file contains output data?" << endl;
      return -1;
   }

   setDefinedNodes ( depthDerivedPropertyValueList,verbose );
   initialiseInterpolators ( depthDerivedPropertyValueList, snapshot, verbose );
   calculatorInterpolatorValues ( depthDerivedPropertyValueList, verbose );

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

void VoxetCalculator::initialiseInterpolators ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList,
                                                const Snapshot*                                         snapshot,
                                                const bool                                              verbose )
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
   AbstractDerivedProperties::FormationPropertyPtr depthPropertyValue;
   AbstractDerivedProperties::FormationPropertyList::const_iterator depthPropertyIter;

   // Get the property-values lists for each property-interpolator.
   for (propertyIter = m_propertyInterpolators.begin (); propertyIter != m_propertyInterpolators.end (); ++propertyIter)
   {
      if (verbose)
      {
         cout << "Initialising cauldron property " << propertyIter->first->getName () << endl;
      }

      propertyIter->second->setSnapshot (m_projectHandle, m_propertyManager, snapshot, useBasement ());
      propertyIter->second->getInterpolator ().setNullValue (propertyIter->second->getDerivedProperty (0)->getUndefinedValue ());
   }


   // Count the number of sediments.
   for (depthPropertyIter = depthPropertyValueList.begin (); depthPropertyIter != depthPropertyValueList.end (); ++depthPropertyIter)
   {
      depthPropertyValue = *depthPropertyIter;

      if ( depthPropertyValue->getFormation () == 0 || (!useBasement () && dynamic_cast<const Interface::Formation*>(depthPropertyValue->getFormation ())->kind () == Interface::BASEMENT_FORMATION )) {
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
   for (depthPropertyIter = depthPropertyValueList.begin (); depthPropertyIter != depthPropertyValueList.end (); ++depthPropertyIter)
   {
      depthPropertyValue = *depthPropertyIter;

      if ( depthPropertyValue->getFormation () == 0 || (!useBasement () && dynamic_cast<const Interface::Formation*>(depthPropertyValue->getFormation ())->kind () == Interface::BASEMENT_FORMATION )) {
         continue;
      }

      numberOfLayerDepthNodes = depthPropertyValue->lengthK ();

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

int VoxetCalculator::getMaximumNumberOfLayerNodes ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList ) const {

   int maximumNumberOfNodes = 0;
   int layerNodeCount;

   AbstractDerivedProperties::FormationPropertyList::const_iterator depthPropertyIter;
   AbstractDerivedProperties::FormationPropertyPtr depthPropertyValue;

   // Count the number of sediments.
   for (depthPropertyIter = depthPropertyValueList.begin (); depthPropertyIter != depthPropertyValueList.end (); ++depthPropertyIter ) {
      depthPropertyValue = *depthPropertyIter;

      if ( depthPropertyValue->getFormation () == 0 || (!useBasement () && dynamic_cast<const Interface::Formation*>(depthPropertyValue->getFormation ())->kind () == Interface::BASEMENT_FORMATION )) {
         continue;
      }

      layerNodeCount = depthPropertyValue->lengthK ();

      if ( layerNodeCount > maximumNumberOfNodes ) {
         maximumNumberOfNodes = layerNodeCount;
      }

   }

   return maximumNumberOfNodes;
}

//------------------------------------------------------------//

void VoxetCalculator::calculatorInterpolatorValues ( const AbstractDerivedProperties::FormationPropertyList& depthPropertyValueList,
                                                     const bool                                              verbose )
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

   AbstractDerivedProperties::FormationPropertyList::const_iterator depthPropertyIter;
   AbstractDerivedProperties::FormationPropertyPtr depthPropertyValue;

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
               for (depthPropertyIter = depthPropertyValueList.begin (), formationCount = 0;
                    depthPropertyIter != depthPropertyValueList.end (); ++depthPropertyIter, ++formationCount)
               {

		  if (verbose && i == 0 && j == 0)
		  {
		     cout << "formation count: " << formationCount << endl << flush;
		  }

                  depthPropertyValue = *depthPropertyIter;

		  if ( depthPropertyValue->getFormation () == 0 || (!useBasement () && dynamic_cast<const Interface::Formation*>(depthPropertyValue->getFormation ())->kind () == Interface::BASEMENT_FORMATION )) {
		     continue;
		  }

		  if (verbose && i == 0 && j == 0)
		  {
		     cout << " computing formation: " << depthPropertyValue->getFormation ()->getName () << endl << flush;
		  }

                  const AbstractDerivedProperties::FormationPropertyPtr propertyValue = propertyIter->second->getDerivedProperty (formationCount);

                  if (!propertyValue)
                  {
                     if (verbose && i == 0 && j == 0)
                     {
                        cout << "    No data available for formation " << depthPropertyValue->getFormation ()->getName () << endl;
                     }
                     continue;
                  }

		  if (verbose && i == 0 && j == 0)
		  {
		     cout << " found property value for property " << propertyValue->getProperty()->getName() <<
			" for formation: " << propertyValue->getFormation ()->getName () << endl << flush;
		  }

                  formationName = depthPropertyValue->getFormation ()->getName ();

                  // For each property-layer-interpolator set the layer top and bottom depths.
                  if (!interpolator (i, j).isNullInterpolator ())
                  {
                     LayerInterpolator & layerInterp = interpolator (i, j) (formationName);

                     layerInterp.clear ();

                     double topDepth = 1e9;
                     double bottomDepth = -1e9;

                     for (l = depthPropertyValue->lengthK () - 1; l >= 0; -- l)
                     {
                        depth = depthPropertyValue->interpolate (cauldronI, cauldronJ, (double) l);
                        topDepth = Min (depth, topDepth);
                        bottomDepth = Max (depth, bottomDepth);


                        property = propertyValue->interpolate (cauldronI, cauldronJ, (double) l);
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

      bool nullValueReplace=false;
      double nullValueReplaceValue=0.0;
      if( m_propertyNullValueReplaceLookup.find( propertyIter->first->getName () ) != m_propertyNullValueReplaceLookup.end() ){
         nullValueReplace=true;
         nullValueReplaceValue = m_propertyNullValueReplaceLookup[propertyIter->first->getName ()];
         cout<< "   replacing null value ( "<<this->getNullValue()<<" ) by : "<<nullValueReplaceValue<<endl;
      }

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
                  if(nullValueReplace && NumericFunctions::isEqual(propertyValues ( i, j, k ), this->getNullValue(), std::numeric_limits<float>::epsilon()) ){
                     propertyValues ( i, j, k )=nullValueReplaceValue;
                  }

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
      nullValue = interpolators->second->getDerivedProperty (0)->getUndefinedValue ();
   }
   else
   {
      nullValue = DefaultNullValue;
   }


   return nullValue;
}

//------------------------------------------------------------//

bool & VoxetCalculator::useBasement ()
{
   return m_useBasement;
}

bool VoxetCalculator::useBasement () const
{
   return m_useBasement;
}

//------------------------------------------------------------//

float VoxetCalculator::getNullValue ( const Property* property ) const {

   float nullValue;

   if ( property != 0 ) {
      PropertyInterpolatorMap::const_iterator interpolators = m_propertyInterpolators.find ( property );

      if ( interpolators != m_propertyInterpolators.end ()) {
         nullValue = interpolators->second->getDerivedProperty ( 0 )->getUndefinedValue ();
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
}

//------------------------------------------------------------//

VoxetCalculator::PropertyInterpolator::~PropertyInterpolator () {
}

//------------------------------------------------------------//

void VoxetCalculator::PropertyInterpolator::setSnapshot ( const GeoPhysics::ProjectHandle*           projectHandle,
                                                          DerivedProperties::DerivedPropertyManager& propertyManager,
                                                          const Snapshot*                            snapshot,
                                                          const bool                                 useBasement ) {
   m_snapshot = snapshot;
   m_derivedPropertyValues = propertyManager.getFormationProperties ( m_property, m_snapshot, useBasement );
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

AbstractDerivedProperties::FormationPropertyPtr VoxetCalculator::PropertyInterpolator::getDerivedProperty ( const unsigned int position ) const {

   if (position >= m_derivedPropertyValues.size()) {
      return AbstractDerivedProperties::FormationPropertyPtr ();
   }

   return m_derivedPropertyValues [ position ];
}

//------------------------------------------------------------//

VoxetDomainInterpolator& VoxetCalculator::PropertyInterpolator::getInterpolator () {
   return m_interpolators;
}

//------------------------------------------------------------//

const GridDescription& VoxetCalculator::getGridDescription () const {
   return m_gridDescription;
}
