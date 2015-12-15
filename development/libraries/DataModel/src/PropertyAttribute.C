#include "PropertyAttribute.h"

const std::string& DataModel::getPropertyAttributeImage ( const PropertyAttribute attr ) {

   static const std::string ContinuousProperty    = "CONTINUOUS_3D_PROPERTY";
   static const std::string DiscontinuousProperty = "DISCONTINUOUS_3D_PROPERTY";
   static const std::string Surface2DProperty     = "SURFACE_2D_PROPERTY";
   static const std::string Formation2DProperty   = "FORMATION_2D_PROPERTY";
   static const std::string TrapProperty          = "TRAP_PROPERTY";
   static const std::string UnknownProperty       = "UNKNOWN_PROPERTY_ATTRIBUTE";

   switch ( attr ) {
      case CONTINUOUS_3D_PROPERTY    : return ContinuousProperty;
      case DISCONTINUOUS_3D_PROPERTY : return DiscontinuousProperty;
      case SURFACE_2D_PROPERTY       : return Surface2DProperty;
      case FORMATION_2D_PROPERTY     : return Formation2DProperty;
      case TRAP_PROPERTY             : return TrapProperty;
      default                        : return UnknownProperty;
   }

}
