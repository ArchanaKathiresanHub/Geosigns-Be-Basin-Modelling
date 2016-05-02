#include "FormationPropertyCalculator.h"
#include "AbstractPropertyManager.h"

bool DerivedProperties::FormationPropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                    const DataModel::AbstractFormation* formation ) const {

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();
   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {
      const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

      if ( property == 0 ) {
         propertyIsComputable = false;
      } else {
         propertyIsComputable = propertyIsComputable and propManager.formationPropertyIsComputable ( property, snapshot, formation );
      }

   }

   return propertyIsComputable;
}

void DerivedProperties::FormationPropertyCalculator::setUp2dEltMapping( AbstractPropertyManager& aPropManager,
                                                                        const FormationPropertyPtr aProperty,
                                                                        ElementList & mapElementList ) const
{


   int QuadIndices[4][2] = {{0,0},{1,0},{1,1},{0,1}};

   int xs,ys,xm,ym,xdim,ydim;

   // DMDAGetInfo(*mapDA,PETSC_NULL,&xdim,&ydim,PETSC_NULL,PETSC_NULL,
   //             PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL);
   // DMDAGetCorners(*mapDA,&xs,&ys,PETSC_NULL,&xm,&ym,PETSC_NULL);

   int i, j;

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
