#ifndef _DEPTH_INTERPOLATOR_H_
#define _DEPTH_INTERPOLATOR_H_

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "LayerInterpolator.h"

/// An interpolator for a column (i,j) in the voxet grid.
/// It consists of a sequence of Layer-interpolators.
class DepthInterpolator {

   typedef std::vector<LayerInterpolator> LayerInterpolatorArray;
   typedef std::map<std::string, LayerInterpolator*> LayerInterpolatorMapping;

public :

   DepthInterpolator ();
   ~DepthInterpolator ();

   /// Define what the null value should be.
   void setNullValue ( const float nullValue );

   /// Set the interpolator to be null, i.e. if the (i,j) location is not part of the cauldron calculation domain.
   void setNullInterpolator ( const bool isNull );

   /// Determine whether or not this interoplator in null.
   bool isNullInterpolator () const;


   /// Set how many layers there are for this interpolator.
   void setNumberOfLayers ( const int numberOfLayers );

   /// Add a layer to this interpolator.
   LayerInterpolator& addLayer ( const std::string& layerName );

   /// Get the layer-interpolator, based in the formation-name.
   ///
   /// If the formation name has not been added to the depth interpolator this will result in an error condition.
   LayerInterpolator& operator ()( const std::string& layerName );

   // Return the top-most depth of the interpolator.
   float topOfInterval () const;

   // Return the bottom-most depth of the interpolator.
   float bottomOfInterval () const;

   /// \brief Return whether or not the depth is a part of the water-column.
   bool isPartOfWaterColumn ( const double depth ) const;

#if 0
   const LayerInterpolator* getInterpolator ( const float z ) const;

   const std::string& getLayerName ( const float z ) const;
#endif

   /// Evaluate the depth-interpolator for the particular depth.
   float operator ()( const float z ) const;

   /// What was the layer number of the last interpolation.
   int lastLayerNumber () const;

   /// Print the interplator in the stream.
   void print ( std::ostream& o ) const;

   /// Find the layer-number for the particular depth.
   int findLayer ( const float z ) const;

private :

   mutable int m_lastLayer;
   float m_nullValue;
   LayerInterpolatorArray m_interpolators;
   LayerInterpolatorMapping m_layerInterpolators;
   bool m_isNull;

};



inline int DepthInterpolator::lastLayerNumber () const {
   return m_lastLayer;
}


inline bool DepthInterpolator::isPartOfWaterColumn ( const double depth ) const {
   if (m_isNull) return false;
   return 0 <= depth && depth < topOfInterval ();
}


#endif // _DEPTH_INTERPOLATOR_H_
