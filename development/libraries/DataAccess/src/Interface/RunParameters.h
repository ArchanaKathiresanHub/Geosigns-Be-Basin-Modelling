#ifndef _INTERFACE_RUNPARAMETERS_H_
#define _INTERFACE_RUNPARAMETERS_H_


#include "Interface/DAObject.h"
#include "Interface/Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// Parameters controlling:
      ///     - time-stepping for both temperature and pressure simulators;
      ///     - maximum element sizes in both sediments and basement layers;
      ///     - the various algorithms used in the model for vre and velocity calculation;
      ///     - ...
      class RunParameters : public DAObject
      {
      public :

         /// constructor.
         RunParameters (ProjectHandle * projectHandle, database::Record * record);

         /// destructor.
         virtual ~RunParameters ();

         /// \name Algorithms
         /// @{

         /// Return name of VRe algorithm.
         virtual const std::string& getVreAlgorithm () const;

         /// Return model of velocity algorithm.
		 virtual const SeismicVelocityModel getSeismicVelocityAlgorithm() const;

         /// Return name of chemical compaction algorithm.
         virtual const std::string& getChemicalCompactionAlgorithm () const;

         /// @}

         /// \name Dynamic time stepping parameters.
         /// @{

         /// Initial time step.
         ///
         /// The time step used at the start of every snapshot interval.
         virtual double getPrefReconstep () const;

         /// Optimal temperature difference between successive time-steps for the whole basin.
         virtual double getOptimalTotalTempDiff () const;

         /// Optimal temperature difference between successive time-steps for source rock layers.
         virtual double getOptimalSourceRockTempDiff () const;

         /// Optimal overpressure difference between successive time-steps for the whole basin.
         virtual double getOptimalTotalPresDiff () const;

         /// Maximum factor by which the time-step can be increased.
         virtual double getMaxTimeStepIncreaseFactor () const;

         /// Minimum factor by which the time-step can be decreased.
         virtual double getMinTimeStepDecreaseFactor () const;
         /// @}

         /// \name Geometry parameters.
         /// @{

         /// Maximum height of any element in the sediment layers.
         virtual double getBrickHeightSediment () const;

         /// Maximum height of any element in the crust layer.
         virtual double getBrickHeightCrust () const;

         /// Maximum height of any element in the mantle layer.
         virtual double getBrickHeightMantle () const;
         /// @}

         /// \name Overpressure parameters.
         /// @{

         /// Maximum number of iterations for the geometric loop overpressure calculation.
         virtual int getMaxNumberOfRunOverpressure  () const;

         /// Temperature gradient used in the overpressure calculation.
         ///
         /// This value is not used in the coupled calculation.
         virtual double getTemperatureGradient () const;

         /// Overpressure optimisation level.
         virtual int getOptimisationLevel () const;

         /// Overpressure optimisation level string.
         virtual const std::string& getOptimisationLevelStr () const;

         /// Indicates when ves constraints are permitted during the overpressure calculation.
         virtual bool getConstrainVES () const;
         /// @}

         virtual const std::string& getFracturePressureFunction () const;

         virtual const std::string& getFractureType () const;

         virtual int getFractureModel () const;

         /// \name Pressure and temperature parameters.

         /// Global switch on chemical compaction.
         virtual bool getChemicalCompaction () const;

         /// Indicates whether ir not the advection/convection term is required.
         virtual bool getConvectiveTerm () const;

         /// Indicates whether overpressure calculation is performed using non-geometric loop or not.
         virtual bool getNonGeometricLoop () const;

         /// Global switch on allochthonous modelling.
         virtual bool getAllochthonousModelling () const;

         /// \brief Indicate whether or not OTGC is required in the Darcy flow simulator.
         virtual bool getApplyOtgcToDarcy () const;
         /// @}

         /// Print the attributes of this MobileLayer
         // May not work if user application is compiled under IRIX with CC -lang:std
         virtual void printOn (ostream &) const;

         /// \brief Indicate whether or not burial-rate time-stepping is to be used.
         virtual bool useBurialRateTimeStepping () const;

         /// \brief Get the fraction of a segment that is to be buried per time-step.
         virtual double getSegmentFractionToBury () const;

         void asString ( std::string & str ) const;

	 /// Maximum time step for Darcy simulator
	 virtual double getDarcyMaxTimeStep() const;

      protected :

         int m_optimisationLevel;

      private:

		  SeismicVelocityModel m_seismicVelocityModel;

      };


   }
}
#endif // _INTERFACE_RUNPARAMETERS_H_
