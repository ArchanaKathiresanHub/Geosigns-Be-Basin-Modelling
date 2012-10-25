#ifndef _GEOPHYSICS__ALLOCHTHONOUS_LITHOLOGY_MANAGER_H_
#define _GEOPHYSICS__ALLOCHTHONOUS_LITHOLOGY_MANAGER_H_

#include <map>
#include <vector>

#include "database.h"
#include "cauldronschemafuncs.h"

#include "IntervalInterpolator.h"
#include "AllochthonousLithologyInterpolator.h"

// CauldronGridDescription
#include "CauldronGridDescription.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsProjectHandle.h"
#include "LithologyManager.h"

#include "hdf5.h"

namespace GeoPhysics {

   /// \brief Manager for the allochthonous modelling.
   ///
   /// Implemented as a singleton, it enables the setting of the interpolator of the allochthonous
   /// body in each of the layers of the model.
   class AllochthonousLithologyManager {

      /// \typedef AllochthonousLithologyInterpolatorMap
      /// \brief Mapping from formation-name to the allochthonous interpolator.
      typedef std::map <std::string, AllochthonousLithologyInterpolator*> AllochthonousLithologyInterpolatorMap;

      /// \typedef IntervalInterpolatorSequence
      /// \brief All of the interval-interpolators used in the model.
      typedef std::vector <Numerics::IntervalInterpolator*> IntervalInterpolatorSequence;

   public :

      /// \name Interpolation results data set names
      /// @{

      /// \var ScalingDataSetName
      /// Dataset name containing the scaling vector.
      static const std::string ScalingDataSetName;

      /// \var TranslationDataSetName
      /// Dataset name containing the translation vector.
      static const std::string TranslationDataSetName;

      /// \var PointsDataSetName
      /// Dataset name containing the interpolation point-set.
      static const std::string PointsDataSetName;

      /// \var CoefficientsDataSetName
      /// Dataset name containing the interpolation coefficients.
      static const std::string CoefficientsDataSetName;

      /// \var RHSDataSetName
      /// Dataset name containing the interpolation rhs vector.
      static const std::string RHSDataSetName;

      /// @}


      AllochthonousLithologyManager ( ProjectHandle* projectHandle );

      virtual ~AllochthonousLithologyManager ();

//       static AllochthonousLithologyManager& getInstance ();

//       static void destoryInstance ();


      /// Initialise the allochthonous lithology manager, returns false if an error has occurred.
      bool initialiseInterpolators ( database::Database* projectDatabase,
                                     const std::string&  directoryName );

      void setGridDescription ( const CauldronGridDescription& newDescription );

      /// Initialise all formations interpolators.
      void initialiseLayers ();

      /// Once all the initialisation has be performed then the data-structure must be frozen.
      /// 
      /// For each formation, the interval interpolators are sorted and the time steps 
      /// for the lithology switch is set.
      void freeze ( const int optimisationLevel );

      /// \brief Reset all time-dependant interpolators to initial values.
      void reset ();

   private :


      /// \brief Determine whether or not allochthonous modelling is required.
      bool allochthonousModellingRequired ( database::Database* projectDatabase ) const;

      /// \brief Determine whether or not allochthonous modelling is required for the formation.
      bool allochthonousModellingRequired ( database::Database* projectDatabase,
                                            const std::string&  formationName ) const;

      /// \brief Read in the interpolation data from the file.
      bool initialiseInterpolator ( hid_t&            fileId,
                                    database::Record* interpRecord );

      static AllochthonousLithologyManager* s_allochthonousLithologyManagerInstance;

      /// Set of all interval-interpolators that have been allocated for this project. It will
      /// be the job of the AllochthonousLithologyManager to delete all of these interpolators.
      IntervalInterpolatorSequence          intervalInterpolators;

      /// Set of all formation interpolators.
      AllochthonousLithologyInterpolatorMap interpolators;

      ProjectHandle* m_projectHandle;

   };

}


#endif // _GEOPHYSICS__ALLOCHTHONOUS_LITHOLOGY_MANAGER_H_
