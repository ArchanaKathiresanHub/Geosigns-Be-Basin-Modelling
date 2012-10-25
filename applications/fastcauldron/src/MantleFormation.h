#ifndef _FASTCAULDRON_MANTLE_FORMATION_H_
#define _FASTCAULDRON_MANTLE_FORMATION_H_

#include "Interface/MantleFormation.h"
#include "layer.h"

#include "GeoPhysicsMantleFormation.h"


class MantleFormation : virtual public LayerProps, virtual public GeoPhysics::GeoPhysicsMantleFormation {

public :

   MantleFormation ( Interface::ProjectHandle * projectHandle,
                     database::Record *              record );

   ~MantleFormation ();

   void initialise ();


   double presentDayThickness ( const int i, const int j ) const;

   bool isSourceRock () const;

   void setMaximumThicknessValue ( const double newThickness );

   double initialDayThickness ( const int i, const int j ) const;

   bool isBasalt() const;

   void allocateBasementVecs();
   void reInitialiseBasementVecs();
   void initialiseBasementVecs();
   void setBasementVectorList();
   void cleanVectors();
private :

   const Interface::GridMap* m_presentDayThickness;
   bool isBasaltLayer;

public :
   Vec UpliftedOrigMantleDepth;
   Vec LithosphereThicknessMod;

   PETSc_3D_Boolean_Array  BasaltMap;
   PETSc_3D_Boolean_Array  previousBasaltMap;

   CompoundLithologyArray m_basaltLithology;

   bool setLithologiesFromStratTable ();

   const CompoundLithology* getBasaltLithology(const int iPosition, const int jPosition) const;

   using LayerProps::getLithology;
   
   const CompoundLithology* getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset );

   const CompoundLithology* getLithology(const int iPosition, const int jPosition, const int kPosition ) const;

   void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition );
   bool getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ); 

   using GeoPhysics::GeoPhysicsMantleFormation::m_mantleElementHeight0;
};

inline bool MantleFormation::isSourceRock () const {
   return false;
}
inline bool MantleFormation::isBasalt() const {
   return isBasaltLayer;
}

inline void MantleFormation::setBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   if( !BasaltMap.isNull() ) {
      BasaltMap( iPosition, jPosition, kPosition ) = true;
   }
}
inline bool MantleFormation::getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   return previousBasaltMap( iPosition, jPosition, kPosition );
}



#endif // _FASTCAULDRON_MANTLE_FORMATION_H_
