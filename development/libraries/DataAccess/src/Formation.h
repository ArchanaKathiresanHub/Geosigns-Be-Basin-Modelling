//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _INTERFACE_FORMATION_H_
#define _INTERFACE_FORMATION_H_

#include "AbstractFormation.h"
#include "AbstractCompoundLithologyArray.h"

#include "DAObject.h"
#include "Interface.h"

#include "capillarySealStrength.h"
#include "FormattingException.h"

typedef formattingexception::GeneralException DataAccessInterfaceCompoundLithologyException;

namespace DataAccess
{
   namespace Interface
   {
      /// A Formation object in the Cauldron model describes a deposition or an erosion or a hiatus.
      /// As such it has a top Surface and a bottom Surface.
      /// It also has a number (>=1 && <= 3) of LithoType objects, together with percentage maps.
      class Formation : public DataModel::AbstractFormation, public DAObject
      {
    public:
       /// constructor
       Formation (ProjectHandle& projectHandle, database::Record * record);
       /// destructor
       virtual ~Formation (void);

       /// Return the Surface at the top of this Formation
       virtual const string & getName (void) const;

       /// Return the mangled name of this Formation
       virtual const string & getMangledName (void) const;

       /// Return the name of the Surface at the top of this Formation
       virtual const string & getTopSurfaceName (void) const;
       /// Return the name of the Surface at the bottom of this Formation
       virtual const string & getBottomSurfaceName (void) const;

       /// Return the Surface at the top of this Formation
       virtual const Surface * getTopSurface (void) const;
       /// Return the Surface at the bottom of this Formation
       virtual const Surface * getBottomSurface (void) const;

       /// Return the present day, user-supplied Formation thickness GridMap.
       /// If a value in the GridMap equals zero or is negative, the value specifies
       /// a hiatus or an erosion.
       virtual const GridMap * getInputThicknessMap (void) const;

       /// Return the first lithotype of this Formation
       virtual const LithoType * getLithoType1 (void) const;
       /// Return the percentage map of the first lithotype
       virtual const GridMap * getLithoType1PercentageMap (void) const;

       /// Return the second lithotype of this Formation
       virtual const LithoType * getLithoType2 (void) const;
       /// Return the percentage map of the second lithotype
       virtual const GridMap * getLithoType2PercentageMap (void) const;

       /// Return the third lithotype of this Formation
       virtual const LithoType * getLithoType3 (void) const;
       /// Return the percentage map of the third lithotype
       virtual const GridMap * getLithoType3PercentageMap (void) const;

       /// return the list of reservoirs in this formation.
       virtual ReservoirList * getReservoirs (void) const;

       /// return the list of MobileLayers of this formation.
       virtual MobileLayerList * getMobileLayers (void) const;

       /// return the AllochthonousLithology for this formation.
       virtual const AllochthonousLithology * getAllochthonousLithology (void) const;

       /// return the list of fault collections specified for this formation.
       virtual FaultCollectionList * getFaultCollections (void) const;

       /// tell whether this Formation is a mobile layer
       virtual bool isMobileLayer (void) const;
       /// tell whether this Formation has an allochthonous lithology.
       virtual bool hasAllochthonousLithology (void) const;
       /// tell whether this Formation is a source rock
       virtual bool isSourceRock (void) const;
       /// Return the SourceRock1 of this Formation
       virtual const SourceRock * getSourceRock1 (void) const;
       /// Return the SourceRock2 of this Formation
       virtual const SourceRock * getSourceRock2 (void) const;

       /// Return the SourceRock1 of this Formation
       virtual void setSourceRock1 ( const SourceRock * sr1 );
       /// Return the SourceRock2 of this Formation
       virtual void setSourceRock2 ( const SourceRock * sr2 );

       /// Return the SourceRock1 of this Formation
       virtual const std::string& getSourceRockType1Name (void) const;
       /// Return the SourceRock2 of this Formation
       virtual const std::string& getSourceRockType2Name (void) const;

       /// Return the Mixing Source Rock mode
       virtual bool getEnableSourceRockMixing(void) const;

       /// Return the Mixing Source Rock mode value
       virtual double getSourceRockMixingHI(void) const;

       /// Return the name of Mixing HI grid
       virtual const string & getSourceRockMixingHIGridName(void) const;

       /// Return the Mixing Source Rock mode value
       virtual double getSourceRockMixingHC(void) const;

       /// Return the name of Mixing HC grid
       virtual const string & getSourceRockMixingHCGridName(void) const;

       /// \brief Indicate whether or not the layer is an igneous intrusion.
       virtual bool getIsIgneousIntrusion () const;

       /// \brief The age at which the igneous intrusion event occurs.
       virtual double getIgneousIntrusionAge () const;

       /// \brief Set the igneous-intrusion event.
       ///
       ///  This is only so that there is no lookup when getting the event object.
       virtual void setIgneousIntrusionEvent ( IgneousIntrusionEvent* igneousIntrusion );

       /// \brief Return a pointer to the intrusion event.
       ///
       /// Return null if the formation is not an intrusion.
       virtual const IgneousIntrusionEvent* getIgneousIntrusionEvent () const;

       /// \brief Return the refinement value in depth direction.
       virtual unsigned int getElementRefinement () const;

       /// \brief Indicate whether or not the formation should be included in the Darcy sub-domain.
       virtual bool getIncludeInDarcy () const;

       /// Return the Hc
       /// Return whether or not the formation has constrained overpressure.
       virtual bool hasConstrainedOverpressure () const;

       virtual ConstrainedOverpressureIntervalList* getConstrainedOverpressureIntervalList () const;

       /// Return whether or not the formation has chemical compaction.
       virtual bool hasChemicalCompaction () const;

       virtual CBMGenerics::capillarySealStrength::MixModel getMixModel (void) const;

       /// Return the mix model (HOMOGENEOUS or LAYERED)
       ///
       /// If wrong model return UNDEFINED
       virtual const std::string& getMixModelStr(void) const;

       /// Return the layering index between 0 and 1 for LAYERED
       ///
       /// Return NoDataValue (-9999) for HOMOGENEOUS
       virtual float getLayeringIndex(void) const;

       virtual const FluidType * getFluidType (void) const;

       /// Return the position in the formation deposition sequence.
       virtual int getDepositionSequence () const;

       virtual GridMap * computeFaultGridMap (const Grid * localGrid, const Snapshot * snapshot) const;

       /// Set the top surface
       /// Internal function
       void setTopSurface (Surface * surface);
       /// Set the bottom surface
       /// Internal function
       void setBottomSurface (Surface * surface);

       virtual FormationKind kind () const;

       /// Load thickness map associated with this formation
       /// Internal function
       virtual GridMap * loadThicknessMap (void) const;
       /// Compute the thickness map in case it could not be loaded
       /// Internal function
       virtual GridMap * computeThicknessMap (void) const;

       virtual GridMap * loadMixingHIMap (void) const;
       virtual const GridMap * getMixingHIMap (void) const;

       virtual GridMap * loadMixingHCMap (void) const;
       virtual const GridMap * getMixingHCMap (void) const;

       virtual void printOn (ostream &) const;

       /// return a string description of this class
       virtual void asString (string &) const;

       /// return the Compound lithology array.
       virtual const DataModel::AbstractCompoundLithologyArray& getCompoundLithologyArray () const override;

       /// return the maximum number of elements
       virtual unsigned int getMaximumNumberOfElements () const override;

    protected :

       string m_mangledName;

       static const unsigned int ThicknessMap = 0;
       static const unsigned int LithoType1Map = 1;
       static const unsigned int LithoType2Map = 2;
       static const unsigned int LithoType3Map = 3;
       static const unsigned int MixingHIMap = 4;
       static const unsigned int MixingHCMap = 5;

       Surface * m_top;
       Surface * m_bottom;

       mutable AllochthonousLithology const * m_allochthonousLithology;
       mutable SourceRock const * m_sourceRock1;
       mutable SourceRock const * m_sourceRock2;
       mutable LithoType const * m_lithoType1;
       mutable LithoType const * m_lithoType2;
       mutable LithoType const * m_lithoType3;

       mutable const FluidType* m_fluidType;

       FormationKind m_kind;

       const IgneousIntrusionEvent* m_igneousIntrusion;

      };

      /// \brief Functor class for ordering formations.
      class FormationLessThan {

      public :

      bool operator ()( const Formation* f1,
                        const Formation* f2 ) const;

      };


   }
}

inline unsigned int DataAccess::Interface::Formation::getMaximumNumberOfElements ( ) const {
    //This function should only be used in GeoPhysics::GeoPhysicsFormation class. Needs to be defined in the AbstractFormation class for testing!
    throw DataAccessInterfaceCompoundLithologyException() <<"DataAccess::Interface::Formation::getMaximumNumberOfElements is called but not implemented. Should only be called from GeoPhysics::GeoPhysicsFormation class!";
}

inline const DataModel::AbstractCompoundLithologyArray& DataAccess::Interface::Formation::getCompoundLithologyArray () const {
    //This function should only be used in GeoPhysics::GeoPhysicsFormation class. Needs to be defined in the AbstractFormation class for testing!
    throw DataAccessInterfaceCompoundLithologyException() <<"DataAccess::Interface::Formation::getMaximumNumberOfElements is called but not implemented. Should only be called from GeoPhysics::GeoPhysicsFormation class!";
}
#endif // _INTERFACE_FORMATION_H_
