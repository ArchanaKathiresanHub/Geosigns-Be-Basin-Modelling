//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__CRUST_FORMATION_H
#define FASTCAULDRON__CRUST_FORMATION_H

// DataAccess library
#include "Interface/CrustFormation.h"
#include "Interface/GridMap.h"

// Geophysics library
#include "GeoPhysicsCrustFormation.h"

// Fastcauldron application
#include "layer.h"


class CrustFormation : virtual public LayerProps, virtual public GeoPhysics::GeoPhysicsCrustFormation {

public :

   CrustFormation ( Interface::ProjectHandle * projectHandle, database::Record * record );

   ~CrustFormation ();

   void initialise ();

   /// \return False since this is a Crust formation
   bool isSourceRock () const noexcept;

   /// \return True if the crust is oceanic (only possible in ALC)
   ///    False if the crust is continental
   bool isBasalt() const noexcept;

   /// \return The heat production at the given (i,j) node
   double getHeatProduction ( const unsigned int i,
                              const unsigned int j ) const;

   /// \return Sets the crust lithologies
   /// \details If the thermal model is ALC, then creates a ALC basalt lithology array
   bool setLithologiesFromStratTable ();

   void allocateBasementVecs( );
   void reInitialiseBasementVecs();
   void initialiseBasementVecs();
   void setBasementVectorList();

   /// @todo How can the lithology change between differnt ij positions?
   const CompoundLithology* getBasaltLithology(const int iPosition, const int jPosition) const;

   using LayerProps::getLithology;

   const CompoundLithology* getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset );

   const CompoundLithology* getLithology(const int iPosition, const int jPosition, const int kPosition ) const;

   /// \return Sets the node lithology (i,j,k) of the formation to basalt
   /// \details This is mentioned as lithoswitching in Cauldron documentation
   void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition );
   /// \return True if the previous crust (in time) was basalt at the given (i,j,k) node, false otherwise
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

   /// @todo This does not need to be an array
   CompoundLithologyArray m_basaltLithology;

   bool isBasaltLayer; ///< True if the crust is oceanic (only possible in ALC), false if the crust is continental
};

inline bool CrustFormation::isSourceRock () const noexcept {
   return false;
}

inline bool CrustFormation::isBasalt() const noexcept {
   return isBasaltLayer;
}

inline double CrustFormation::getHeatProduction ( const unsigned int i,
                                                  const unsigned int j ) const {
   return m_heatProductionMap->getValue ( i, j );
}


inline void CrustFormation::setBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   if( !BasaltMap.isNull() ) {
      BasaltMap( iPosition, jPosition, kPosition ) = true;
   }
}

inline bool CrustFormation::getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ) {
   return previousBasaltMap( iPosition, jPosition, kPosition );
}




#endif // FASTCAULDRON__CRUST_FORMATION_H
