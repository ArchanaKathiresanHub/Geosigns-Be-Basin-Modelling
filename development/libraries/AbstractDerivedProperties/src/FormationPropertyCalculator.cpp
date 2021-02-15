//
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationPropertyCalculator.h"
#include "AbstractPropertyManager.h"

#include "AbstractProperty.h"
#include <string>

bool AbstractDerivedProperties::FormationPropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                    const DataModel::AbstractFormation* formation ) const
{
   // Determine if the required properties are computable.
   for ( const std::string& dependentProperty : getDependentPropertyNames() )
   {
      const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperty );

      if ( property == 0 ||
           !propManager.formationPropertyIsComputable ( property, snapshot, formation ))
      {
         return false;
      }
   }

   return true;
}

void AbstractDerivedProperties::FormationPropertyCalculator::setUp2dEltMapping( AbstractPropertyManager& aPropManager,
                                                                        const FormationPropertyPtr aProperty,
                                                                        ElementList & mapElementList ) const
{


   int QuadIndices[4][2] = {{0,0},{1,0},{1,1},{0,1}};

   // DMDAGetInfo(*mapDA,PETSC_IGNORE,&xdim,&ydim,PETSC_IGNORE,PETSC_IGNORE,
   //             PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE);
   // DMDAGetCorners(*mapDA,&xs,&ys,PETSC_IGNORE,&xm,&ym,PETSC_IGNORE);

   unsigned int i, j;

   // for (j=ys; j<ys+ym; j++) {

   //    for (i=xs; i<xs+xm; i++) {

   for (j = aProperty->firstJ( true ); j < aProperty->lastJ( true ); ++ j ) {

      for (i = aProperty->firstI( true ); i < aProperty->lastI( true ); ++ i ) {

         //        if (i == xdim-1 || j == ydim-1 ) {
         if (i == aProperty->getGrid()->numIGlobal() || j == aProperty->getGrid()->numJGlobal() ) {
            continue;
         } else {
            Elt2dIndices EltIndices;
            int Inode;
            bool ElementExists = true;

            for (Inode = 0; Inode<4; Inode++) {
               int idxX = QuadIndices[Inode][0];
               int idxY = QuadIndices[Inode][1];

               if ( not aPropManager.getNodeIsValid ( i + idxX, j + idxY )) {
                  EltIndices.nodeDefined[Inode] = false;
                  ElementExists = false;
               } else {
                  EltIndices.nodeDefined[Inode] = true;
               }

               EltIndices.i[Inode] = i+idxX;
               EltIndices.j[Inode] = j+idxY;
               EltIndices.exists = ElementExists;
            }

            mapElementList.push_back(EltIndices);
         }

      }

   }

}

double AbstractDerivedProperties::FormationPropertyCalculator::calculateAtPosition(const GeoPhysics::GeoPhysicsFormation* /*formation*/,
                                                                                   const GeoPhysics::CompoundLithology* /*lithology*/, const std::map<std::string, double> &/*dependentProperties*/) const
{
  return -1;
}
