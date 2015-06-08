#ifndef DATA_MODEL__PROPERTY_ATTRIBUTE__H
#define DATA_MODEL__PROPERTY_ATTRIBUTE__H

#include <string>

namespace DataModel {

   /// \enum PropertyAttribute
   /// \brief Attributes of properties to indicate if they are continuous across surface, discontinuous or otherwise.
   enum PropertyAttribute { 
      /// Indicates a property is calculated for the 3d grid and is continuous across a surface.
      ///
      /// For example Temperature, Pressure.
      /// It is possible to construct a 2d map from properties with this attribute.
      /// The 2d map will be associated with a surface only and not a formation.
      CONTINUOUS_3D_PROPERTY,

      /// Indicates a property is calculated for the 3d grid and is discontinuous across a surface.
      ///
      /// For example Porosity, MaxVes.
      /// It is possible to construct a 2d map from properties with this attribute.
      /// The 2d map will be associated with both a surface and a formation.
      DISCONTINUOUS_3D_PROPERTY,

      /// Indicates that the property is calculated for a 2d grid and is associated with a surface.
      ///
      /// E.g. ALCOrigLithMantleDepth.
      SURFACE_2D_PROPERTY,

      /// Indicates that the property is calculated for a 2d grid and is associated with a formation.
      ///
      /// E.g. Erosion factor, thickness
      FORMATION_2D_PROPERTY,

      /// Indicates that a property is associated with a trap.
      TRAP_PROPERTY,

      /// For unknown cases.
      UNKNOWN_PROPERTY_ATTRIBUTE };

   /// \brief Return the string representation of the PropertyAttribute enumeration type.
   const std::string& getPropertyAttributeImage ( const PropertyAttribute attr );

} // namespace DataModel

#endif // DATA_MODEL__PROPERTY_ATTRIBUTE__H
