// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS__FORMATION_H_
#define _GEOPHYSICS__FORMATION_H_


#include "database.h"

#include "Interface/Formation.h"
#include "Interface/Interface.h"

#include "CompoundLithology.h"
#include "CompoundLithologyArray.h"

#include "Local3DArray.h"
#include "Local2DArray.h"

#include "Polyfunction.h"//from CBM Generics


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}


namespace GeoPhysics {

   class Formation : virtual public DataAccess::Interface::Formation {

   public :

      Formation ( DataAccess::Interface::ProjectHandle* projectHandle,
                  database::Record*                          record );


      ~Formation ();

      virtual bool setLithologiesFromStratTable ();

      virtual void setAllochthonousLayer ( AllochthonousLithologyInterpolator* interpolator );

      virtual void setFaultLithologies ( bool& layerHasFaults,
                                         bool& error );

      bool getContainsFaults () const;

      const CompoundLithology* getCompoundLithology ( const unsigned int i, const unsigned int j ) const;

      // Temporary during transfer phase.
      CompoundLithologyArray& getCompoundLithologyArray ();

      // Temporary during transfer phase.
      const CompoundLithologyArray& getCompoundLithologyArray () const;

      virtual void switchLithologies ( const double age );

      bool getLithologyHasSwitched ( const unsigned int i, const unsigned int j ) const;

      virtual void determineMinMaxThickness ();

      virtual double getMinimumThickness () const;

      virtual double getMaximumThickness () const;

      // Would like to remove this function.
      virtual bool isCrust () const noexcept;

      // Would like to remove this function.
      virtual bool isMantle () const;

      virtual unsigned int setMaximumNumberOfElements ( const bool readSizeFromVolumeData );

      /// \brief Get the maximum number of elements in the thickness of the layer.
      ///
      /// This includes all elements that are active and not active.
      unsigned int getMaximumNumberOfElements () const;

      double getSolidThickness ( const unsigned int i,
                                 const unsigned int j,
                                 const unsigned int k,
                                 const double       age ) const;

      double getRealThickness ( const unsigned int i,
                                const unsigned int j,
                                const unsigned int k,
                                const double       age ) const;


      CBMGenerics::Polyfunction& getSolidThickness ( const unsigned int i,
                                                     const unsigned int j,
                                                     const unsigned int k );

      CBMGenerics::Polyfunction& getRealThickness ( const unsigned int i,
                                                    const unsigned int j,
                                                    const unsigned int k );

      const CBMGenerics::Polyfunction& getSolidThickness ( const unsigned int i,
                                                           const unsigned int j,
                                                           const unsigned int k ) const;

      const CBMGenerics::Polyfunction& getRealThickness ( const unsigned int i,
                                                          const unsigned int j,
                                                          const unsigned int k ) const;

      double getPresentDayErodedThickness ( const unsigned int i,
                                            const unsigned int j ) const;

      virtual void retrieveAllThicknessMaps ();

      virtual void restoreAllThicknessMaps ();

      /// \brief Get the refinement factor for the number of elements in the depth.
      int getDepthRefinementFactor () const;

      /// \brief Set the refinement factor for the number of elements in the depth.
      void setDepthRefinementFactor ( const int zRefinementFactor );

   protected :

      // I hope just temporary.
      friend class GeoPhysics::ProjectHandle;


      CompoundLithologyArray m_compoundLithologies;

      /// The minimum input thickness (after deposition).
      ///
      /// This is the minimum of all input maps, strat-depth map and mobile-layer thickness maps.
      double m_minimumDepositedThickness;

      /// The maximum input thickness (after deposition).
      ///
      /// This is the maximum of all input maps, strat-depth map and mobile-layer thickness maps.
      double m_maximumDepositedThickness;

      /// The amount of extra refinement required in the depth direction.
      int m_zRefinementFactor;

      /// The maximum number of elements in the depth direction.
      unsigned int m_maximumNumberOfElements;

      /// The solid thickness derived from the input maps.
      ///
      /// This assumes a linear burial rate of solid material, used
      /// in geometric loop calculations.
      GeoPhysics::Local3DArray<CBMGenerics::Polyfunction> m_solidThickness;

      /// The real thickness, derived from the input maps.
      ///
      /// This assumes a linear burial rate of real thickness, used
      /// in non-geometric loop calculations.
      GeoPhysics::Local3DArray<CBMGenerics::Polyfunction> m_realThickness;

      GeoPhysics::Local2DArray<double> m_presentDayErodedThickness;

      /// Indicate whether or not the formations contains faults.
      bool m_containsFault;

   };

}

inline const GeoPhysics::CompoundLithology* GeoPhysics::Formation::getCompoundLithology ( const unsigned int i, const unsigned int j ) const {
   return m_compoundLithologies ( i, j );
}

inline GeoPhysics::CompoundLithologyArray& GeoPhysics::Formation::getCompoundLithologyArray () {
   return m_compoundLithologies;
}

inline const GeoPhysics::CompoundLithologyArray& GeoPhysics::Formation::getCompoundLithologyArray () const {
   return m_compoundLithologies;
}

inline bool GeoPhysics::Formation::getLithologyHasSwitched ( const unsigned int i, const unsigned int j ) const {
   return m_compoundLithologies.hasSwitched ( i, j );
}

inline double GeoPhysics::Formation::getMinimumThickness () const {
   return m_minimumDepositedThickness;
}

inline double GeoPhysics::Formation::getMaximumThickness () const {
   return m_maximumDepositedThickness;
}

inline bool GeoPhysics::Formation::isCrust () const noexcept{
   return false;
}

inline bool GeoPhysics::Formation::isMantle () const {
   return false;
}

inline unsigned int GeoPhysics::Formation::getMaximumNumberOfElements () const {
   return m_maximumNumberOfElements;
}

inline bool GeoPhysics::Formation::getContainsFaults () const {
   return m_containsFault;
}

inline CBMGenerics::Polyfunction& GeoPhysics::Formation::getSolidThickness ( const unsigned int i,
                                                                             const unsigned int j,
                                                                             const unsigned int k ) {
   return m_solidThickness ( i, j, k );
}

inline CBMGenerics::Polyfunction& GeoPhysics::Formation::getRealThickness ( const unsigned int i,
                                                                            const unsigned int j,
                                                                            const unsigned int k ) {
   return m_realThickness ( i, j, k );
}

inline const CBMGenerics::Polyfunction& GeoPhysics::Formation::getSolidThickness ( const unsigned int i,
                                                                                   const unsigned int j,
                                                                                   const unsigned int k ) const {
   return m_solidThickness ( i, j, k );
}

inline const CBMGenerics::Polyfunction& GeoPhysics::Formation::getRealThickness ( const unsigned int i,
                                                                                  const unsigned int j,
                                                                                  const unsigned int k ) const {
   return m_realThickness ( i, j, k );
}


inline double GeoPhysics::Formation::getSolidThickness ( const unsigned int i,
                                                         const unsigned int j,
                                                         const unsigned int k,
                                                         const double       age ) const {
   return m_solidThickness ( i, j, k ).F ( age );
}

inline double GeoPhysics::Formation::getRealThickness ( const unsigned int i,
                                                        const unsigned int j,
                                                        const unsigned int k,
                                                        const double       age ) const {
   return m_realThickness ( i, j, k ).F ( age );
}

inline double GeoPhysics::Formation::getPresentDayErodedThickness ( const unsigned int i,
                                                                    const unsigned int j ) const {
   return m_presentDayErodedThickness ( i, j );
}

inline int GeoPhysics::Formation::getDepthRefinementFactor () const {
   return m_zRefinementFactor;
}

#endif // _GEOPHYSICS__FORMATION_H_
