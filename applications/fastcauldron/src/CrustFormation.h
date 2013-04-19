#ifndef _FASTCAULDRON__CRUST_FORMATION_H_
#define _FASTCAULDRON__CRUST_FORMATION_H_

#include "Interface/CrustFormation.h"
#include "Interface/GridMap.h"

#include "GeoPhysicsCrustFormation.h"
#include "layer.h"

class CrustFormation : virtual public LayerProps, virtual public GeoPhysics::GeoPhysicsCrustFormation {

public :

   CrustFormation ( Interface::ProjectHandle * projectHandle, database::Record * record );

   ~CrustFormation ();

   void initialise ();

   bool isSourceRock () const;

   double getHeatProduction ( const unsigned int i,
                              const unsigned int j ) const;

   bool setLithologiesFromStratTable ();
  
   bool isBasalt() const;
   
   void allocateBasementVecs( );
   void reInitialiseBasementVecs();
   void initialiseBasementVecs();
   void setBasementVectorList();
   
 
   const CompoundLithology* getBasaltLithology(const int iPosition, const int jPosition) const;

   using LayerProps::getLithology;
 
   const CompoundLithology* getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset );
   
   const CompoundLithology* getLithology(const int iPosition, const int jPosition, const int kPosition ) const;
   
   void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition );
   bool getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ); 
   
   void cleanVectors();
   
   Vec TopBasaltDepth;
   Vec BasaltThickness;
   Vec BottomBasaltDepth;
   Vec ThicknessBasaltALC;
   Vec ThicknessCCrustALC;
   Vec SmCCrustThickness;
   Vec SmTopBasaltDepth;
   Vec SmBottomBasaltDepth;
   PETSc_3D_Boolean_Array  BasaltMap;
   PETSc_3D_Boolean_Array  previousBasaltMap;

protected :
   
   const Interface::GridMap* m_heatProductionMap;

   
   CompoundLithologyArray m_basaltLithology;
   
   bool  isBasaltLayer;
};

inline bool CrustFormation::isSourceRock () const {
   return false;
}

inline double CrustFormation::getHeatProduction ( const unsigned int i,
                                                  const unsigned int j ) const {
   return m_heatProductionMap->getValue ( i, j );
}
inline bool CrustFormation::isBasalt() const {
   return isBasaltLayer;
}

inline void CrustFormation::setBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   if( !BasaltMap.isNull() ) {
      BasaltMap( iPosition, jPosition, kPosition ) = true;
   }
}

inline bool CrustFormation::getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   return previousBasaltMap( iPosition, jPosition, kPosition );
}




#endif // _FASTCAULDRON__CRUST_FORMATION_H_
