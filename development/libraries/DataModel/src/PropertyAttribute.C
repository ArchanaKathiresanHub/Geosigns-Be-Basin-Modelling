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
const std::string& DataModel::getPropertyOutputAttributeImage ( const PropertyOutputAttribute attr ) {

   static const std::string FastcauldronProperty  = "FASTCAULDRON_PROPERTY";
   static const std::string FastgenexProperty     = "FASTGENEX_PROPERTY";
   static const std::string FastmigProperty       = "FASTMIG_PROPERTY";
   static const std::string FastCtcProperty       = "FASTCTC_PROPERTY";
   static const std::string TrapsProperty         = "TRAPS_PROPERTY";
   static const std::string UnknownOutputProperty = "UNKNOWN_PROPERTY_OUTPUT_ATTRIBUTE";

   switch ( attr ) {
   case FASTCAULDRON_PROPERTY : return FastcauldronProperty;
   case FASTGENEX_PROPERTY    : return FastgenexProperty;
   case FASTMIG_PROPERTY      : return FastmigProperty;
   case FASTCTC_PROPERTY      : return FastCtcProperty;
   case TRAPS_PROPERTY        : return TrapsProperty;
   default                    : return UnknownOutputProperty;
   }

}
