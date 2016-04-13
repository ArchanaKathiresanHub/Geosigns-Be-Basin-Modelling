//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "FormationSurfacePropertyCalculator.h"
#include "AbstractPropertyManager.h"

// utilitites library
#include "LogHandler.h"

bool DerivedProperties::FormationSurfacePropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                           const DataModel::AbstractFormation* formation,
                                                                           const DataModel::AbstractSurface*   surface ) const {

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();
   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {
      const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

      if ( property == 0 ) {
         propertyIsComputable = false;
         std::string snapshotLog = " @ snapshot " + std::to_string( snapshot ? snapshot->getTime() : -9999 ) + "Ma.";
         std::string surfaceLog = "for surface " + (surface ? surface->getName() : "");
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "Propertie(s) " << getPropertyNames()
            << " (formation) cannot be computed because dependant abstract property " << dependentProperties[i] << " is not found "
            << (surface ? surfaceLog : "") << (snapshot ? snapshotLog : ".");
      } else {
         propertyIsComputable = propertyIsComputable and propManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, surface );
         if (!propertyIsComputable){
            std::string snapshotLog = " @ snapshot " + std::to_string( snapshot ? snapshot->getTime() : -9999 ) + "Ma.";
            std::string surfaceLog = "for surface " + (surface ? surface->getName() : "");
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "Propertie(s) " << getPropertyNames()
               << " (formation) cannot be computed because dependant abstract property " << dependentProperties[i] << " is not computable "
               << (surface ? surfaceLog : "") << (snapshot ? snapshotLog : ".");
         }
      }

   }

   return propertyIsComputable;

}
