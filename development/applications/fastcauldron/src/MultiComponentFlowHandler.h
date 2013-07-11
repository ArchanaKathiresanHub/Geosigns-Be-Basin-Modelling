#ifndef _FASTCAULDRON__MULTI_COMPONENT_FLOW_HANDLER__H_
#define _FASTCAULDRON__MULTI_COMPONENT_FLOW_HANDLER__H_

#include <vector>
#include <string>

#include "Subdomain.h"
#include "layer.h"
#include "globaldefs.h"

#include "ExplicitMultiComponentFlowSolver.h"

class MultiComponentFlowHandler {

   /// \brief An array of subdomains.
   typedef std::vector<Subdomain*> SubdomainArray;

public :

   static const std::string& getErrorString ( const DarcyErrorIndicator id );


   MultiComponentFlowHandler ();

   ~MultiComponentFlowHandler ();


   /// \brief Indicates if the multi-component flow equations need to be solved.
   bool solveFlowEquations () const;

   /// \brief Determines the amount of output from the multi-component flow solver.
   ///
   /// The higher the number then the more output is produced, 0 or less indicates no output.
   int getDebugLevel () const;

   /// \brief The 
   double getMaximumPermeability () const;

   /// \brief Indicate that uniform time-stepping is required.
   bool useUniformTimeStepping () const;

   /// \brief Indicate that non-uniform time-stpeping is to be used.
   bool useCflTimeStepping () const;

   /// \brief 
   double cflTimeStepFraction () const;

   /// \brief 
   double getMaximumTimeStepSize () const;

   /// \brief The fraction of the time-step size given by the CFL value.
   double cflimeStepFraction () const;

   /// \brief Indicate whether or not to use time-step smooting when using CFL time-stepping in Darcy.
   ///
   /// Applies only when the time-step size is increasing.
   bool applyTimeStepSmoothing () const;

   /// \brief
   ///
   /// Works only when increasing the time-step size.
   double getTimeStepSmoothingFactor () const;

   /// \brief 
   double getMaximumHCFractionForFlux () const;

   /// \brief Indicate whether or not OTGC should be applied to the HCs that are in the pore-space.
   bool applyOtgc () const;

   /// \brief Indicate whether or not the water-saturation should be included in the over-pressure calculation.
   bool includeWaterSaturationInOp () const;

   /// \brief Indicate whether or not the capillary-pressure should be included in the Darcy flux calculation.
   bool includeCapillaryPressure () const;

   /// \brief Indicate whther or not the grad-pressure should be limited.
   bool limitGradPressure () const;

   /// \brief If grad-pressure is to be limited then use this as the maximum.
   double gradPressureMaximum () const;

   /// \brief Indicate whther or not the permeability should be limited when computing the fluxes.
   bool limitFluxPermeability () const;

   /// \brief If permeability is to be limited then use this as the maximum.
   double maximumFluxPermeability () const;

   /// \brief Indicate whether or not to use the immobile species in the saturation/porosity calculations.
   bool useImmobileSaturation () const;

   /// \brief Determine is the multi-component flow solver is required.
   void determineUsage ();

   /// \brief Set the subdomain(s) for the multi-component flow solver.
   void initialise ();

   /// \brief Set the activity of all subdomain.
   void setSubdomainActivity ( const double currentTime );

   /// \brief Determine if any sub-domain is active.
   bool anySubdomainIsActive () const;

   /// \brief Determine if any source-rock in any darcy sbudomain contains sulphur.
   bool modelContainsSulphur () const;

   /// \brief Determine the number of active sub-domains.
   int numberOfActiveSubdomains () const;

   /// \brief Solve the multi-component flow equations.
   void solve ( const double previousTime, 
                const double currentTime,
                      bool&  errorOccurred );


   /// \brief Get the i-position of the node/needle to be debugged.
   ///
   /// If this function returns -1 then no node was selected.
   int getDebugINode () const;

   /// \brief Get the j-position of the node/needle to be debugged.
   ///
   /// If this function returns -1 then no node was selected.
   int getDebugJNode () const;

   /// \brief Get the k-position of the node to be debugged.
   ///
   /// If this function returns -1 then no node was selected.
   int getDebugKNode () const;

   /// \brief Get the quadrature degree that should be used for the face-flux integrals.
   int getFaceQuadratureDegree () const;

   /// \brief Get the quadrature degree that should be used for the "previous-contributions term" integrals.
   int getPreviousTermQuadratureDegree () const;

   /// \brief Get the quadrature degree that should be used for the source-term integrals.
   int getSourceTermQuadratureDegree () const;

   /// \brief Get the quadrature degree that should be used for the mass-matrix integrals.
   int getMassMatrixQuadratureDegree () const;

   /// \brief Get the amout of time-step sub-smapling required.
   ///
   /// 1 means no sub-sampling.
   int getTimeStepSubSample () const;

   /// \brief Apply time-step sub-sampling to OTGC.
   bool timeStepSubSampleOtgc () const;

   /// \brief Apply time-step sub-sampling to pvt calculation.
   bool timeStepSubSamplePvt () const;

   /// \brief Apply time-step sub-sampling to flux.
   bool timeStepSubSampleFlux () const;

   /// \brief Inticate whether or not averaging of hc-density and hc-viscosity should be applied.
   bool getApplyPvtAveraging () const;

   /// \brief Indicate whether or not the source term should be included in the Darcy simulator.
   ///
   /// This is a testing parameter only.
   bool removeSourceTerm () const;

   /// \brief The age as which the source term should not be included in the Darcy simulator.
   ///
   /// This is a testing parameter only.
   double removeSourceTermAge () const;

   /// \brief Indicate whether or not the hc-transport should be included in the Darcy simulator.
   ///
   /// This is a testing parameter only.
   bool removeHcTransport () const;

   /// \brief The age as which the hc-transport should not be included in the Darcy simulator.
   ///
   /// This is a testing parameter only.
   double removeHcTransportAge () const;

   /// \brief Indicate whether or not the volume calculations should be saved.
   ///
   /// This is a debug option.
   bool saveVolumeOutput () const;

   /// \brief Indicate whether or not the volume of transported hc calculations should be saved.
   ///
   /// This is a debug option.
   bool saveTransportedVolumeOutput () const;

   /// \brief Output maps for Darcy runs.
   ///
   /// Normally only volume results are output during a Darcy run.
   bool outputDarcyMaps () const;

   /// \brief Add mass per element to 3d results.
   ///
   /// Selectable by command line parameter only.
   bool outputElementMasses () const;

   /// \brief Return all command line information required by the multi-component flow solver.
   static std::string getCommandLineOptions ();

   /// \brief Get reference to array of subdomains.
   const SubdomainArray& getSubdomains () const;

   /// \brief Get reference to array of subdomains.
   SubdomainArray& getSubdomains ();

   /// \brief Determine whether or not the formation is contained in a darcy subdomain.
   bool containsFormation ( const LayerProps* formation ) const;

private :

   void getLayers ( LayerList& layers );



   /// \brief Add a subdomain to the set that are to be used in the multi-component flow solver.
   ///
   /// If the subdomain overlaps with any other subdomain already in the list then it will not be added.
   /// The return value indicates whether or not the subdomain was added.
   bool addSubdomain ( Subdomain* subdomain );

   /// \brief Add the subdomains from the list of formations.
   ///
   /// The formation list must not contain a set of overlapping subdomains.
   void addSubdomains ( const int* formationRangeArray,
                        const int  formationCount );

   /// \brief Determine programmatically the subdomains.
   void determineSubdomains ();

   /// \brief Set the subdomain-id for rach sub-domain.
   void numberSubdomains ();


   // void determineSubdomains ();

   // void determineSubdomains ();


   /// \brief Array of subdomains that are to be used in the multi-component flow solver.
   SubdomainArray m_subdomains;

   PetscBool m_solveFlowEquations;

   int m_debugINode;
   int m_debugJNode;
   int m_debugKNode;

   // PetscBool m_userDefinedSubdomains;
   double     m_userDefinedMaximumPermeability;

   PetscBool m_uniformTimeStepping;
   PetscBool m_cflTimeStepping;
   double     m_cflTimeStepFraction;
   double     m_maximumTimeStepSize;
   double     m_maximumHCFractionForFlux;



   int        m_debugLevel;
   PetscBool m_applyOtgc;
   bool       m_includeWaterSaturationInOp;
   bool       m_includeCapillaryPressureInDarcy;
 
   ExplicitMultiComponentFlowSolver* m_explicitFlowSolver;

   int m_faceQuadratureDegree;
   int m_previousContributionTermQuadratureDegree;
   int m_sourceTermQuadratureDegree;
   int m_massMatrixQuadratureDegree;


   bool   m_stopHcContributions;
   double m_stopHcContributionsAge;


   bool   m_stopHcTransport;
   double m_stopHcTransportAge;

   bool   m_applyPvtAveraging;

   bool m_saveVolumeOutput;
   bool m_saveTransportedVolumeOutput;

   /// \brief Indicates whether or not the grad-pressure should be limited.
   bool m_limitGradPressure;

   /// \brief If grad-pressure is to be limited then this is that limit.
   double m_gradPressureMaximum;

   /// \brief Indicates whether or not the permeability should be limited when calculating the fluxes.
   bool m_limitPermeabilityInFlux;

   /// \brief If permeability is to be limited then this is that limit.
   double m_maximumFluxPermeability;

   /// \brief Indicates whether or not the immobile species should reduce the pore-space.
   bool m_useImmobileSaturation;

   /// \brief Indicate whether or not to save project file and results after an error has been detected.
   bool m_outputDarcyMaps;

   /// \brief Indicate whether or not to save project file and results after an error has been detected.
   bool m_outputElementMasses;


   int  m_timeStepSubSample;
   bool m_timeStepSubSampleOtgc;
   bool m_timeStepSubSamplePvt;
   bool m_timeStepSubSampleFlux;


   double m_timeStepSmoothingFactor;
   bool   m_applyTimeStepSmoothing;

};


inline bool MultiComponentFlowHandler::solveFlowEquations () const {
   return m_solveFlowEquations;
}

inline int MultiComponentFlowHandler::getDebugLevel () const {
   return m_debugLevel;
}

inline double MultiComponentFlowHandler::getMaximumTimeStepSize () const {
   return m_maximumTimeStepSize;
}

inline bool MultiComponentFlowHandler::applyOtgc () const {
   return m_applyOtgc;
}

inline double MultiComponentFlowHandler::getMaximumPermeability () const {
   return m_userDefinedMaximumPermeability;
}

// inline bool MultiComponentFlowHandler::userDefinedSubdomains () const {
//    return m_userDefinedSubdomains;
// }

inline bool MultiComponentFlowHandler::useUniformTimeStepping () const {
   return m_uniformTimeStepping;
}

inline bool MultiComponentFlowHandler::useCflTimeStepping () const {
   return not m_uniformTimeStepping;
}

inline double MultiComponentFlowHandler::cflTimeStepFraction () const {
   return m_cflTimeStepFraction;
}

inline bool MultiComponentFlowHandler::applyTimeStepSmoothing () const {
   return m_applyTimeStepSmoothing;
}

inline double MultiComponentFlowHandler::getTimeStepSmoothingFactor () const {
   return m_timeStepSmoothingFactor;
}

inline double MultiComponentFlowHandler::getMaximumHCFractionForFlux () const {
   return m_maximumHCFractionForFlux;
}

inline SubdomainArray& MultiComponentFlowHandler::getSubdomains () {
   return m_subdomains;
}

inline const SubdomainArray& MultiComponentFlowHandler::getSubdomains () const {
   return m_subdomains;
}

inline int MultiComponentFlowHandler::getDebugINode () const {
   return m_debugINode;
}

inline int MultiComponentFlowHandler::getDebugJNode () const {
   return m_debugJNode;
}

inline int MultiComponentFlowHandler::getDebugKNode () const {
   return m_debugKNode;
}

inline bool MultiComponentFlowHandler::includeWaterSaturationInOp () const {
   return m_includeWaterSaturationInOp;
}

inline bool MultiComponentFlowHandler::includeCapillaryPressure () const {
   return m_includeCapillaryPressureInDarcy;
}

inline int MultiComponentFlowHandler::getFaceQuadratureDegree () const {
   return m_faceQuadratureDegree;
}

inline int MultiComponentFlowHandler::getPreviousTermQuadratureDegree () const {
   return m_previousContributionTermQuadratureDegree;
}

inline int MultiComponentFlowHandler::getSourceTermQuadratureDegree () const {
   return m_sourceTermQuadratureDegree;
}

inline int MultiComponentFlowHandler::getMassMatrixQuadratureDegree () const {
   return m_massMatrixQuadratureDegree;
}

inline bool MultiComponentFlowHandler::getApplyPvtAveraging () const {
   return m_applyPvtAveraging;
}

inline bool MultiComponentFlowHandler::removeSourceTerm () const {
   return m_stopHcContributions;
}

inline double MultiComponentFlowHandler::removeSourceTermAge () const {
   return m_stopHcContributionsAge;
}

inline bool MultiComponentFlowHandler::removeHcTransport () const {
   return m_stopHcTransport;
}

inline double MultiComponentFlowHandler::removeHcTransportAge () const {
   return m_stopHcTransportAge;
}

inline bool MultiComponentFlowHandler::saveVolumeOutput () const {
   return m_saveVolumeOutput;
}

inline bool MultiComponentFlowHandler::saveTransportedVolumeOutput () const {
   return m_saveTransportedVolumeOutput;
}

inline bool MultiComponentFlowHandler::limitGradPressure () const {
   return m_limitGradPressure;
}

inline double MultiComponentFlowHandler::gradPressureMaximum () const {
   return m_gradPressureMaximum;
}

inline bool MultiComponentFlowHandler::limitFluxPermeability () const {
   return m_limitPermeabilityInFlux;
}

inline double MultiComponentFlowHandler::maximumFluxPermeability () const {
   return m_maximumFluxPermeability;
}

inline bool MultiComponentFlowHandler::useImmobileSaturation () const {
   return m_useImmobileSaturation;
}

inline int MultiComponentFlowHandler::getTimeStepSubSample () const {
   return m_timeStepSubSample;
}

inline bool MultiComponentFlowHandler::timeStepSubSampleOtgc () const {
   return m_timeStepSubSampleOtgc;
}

inline bool MultiComponentFlowHandler::timeStepSubSamplePvt () const {
   return m_timeStepSubSamplePvt;
}

inline bool MultiComponentFlowHandler::timeStepSubSampleFlux () const {
   return m_timeStepSubSampleFlux;
}

inline bool MultiComponentFlowHandler::outputDarcyMaps () const {
   return m_outputDarcyMaps;
}

inline bool MultiComponentFlowHandler::outputElementMasses () const {
   return m_outputElementMasses;
}


#endif // _FASTCAULDRON__MULTI_COMPONENT_FLOW_HANDLER__H_
