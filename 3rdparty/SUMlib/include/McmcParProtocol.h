// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MCMCPARPROTOCOL_H
#define SUMLIB_MCMCPARPROTOCOL_H

// STD
#include <vector>

// SUMlib
#include "SUMlib.h"
#include "IMcmc.h"
#include "ISerializer.h"
#include "KrigingData.h"
#include "McmcBase.h"

namespace SUMlib
{

class INTERFACE_SUMLIB McmcParProtocolMaster : public IMcmc
{

public:

   /// Constructor
   ///   @param statistics    Object where some results is stored
   McmcParProtocolMaster( McmcStatistics& statistics );

   /// Destructor
   virtual ~McmcParProtocolMaster();

   /// Send a command to the slave
   ///   @param   cmd   command
   void startCommand( std::string& cmd ) const;

   /// Give parameters for communication
   ///   @param[in] id     Id of this communication
   ///   @param[in] ser    Serializer
   ///   @param[in] deser  Deserializer
   void initCommunication( int id, ISerializer* ser, IDeserializer* deser )
            { m_mcmcId = id, m_serialiser = ser; m_deserialiser = deser; }

   /// Get the id of this communication
   int  getId() const { return m_mcmcId; }

   /// Receive the result (status) of the command
   void  checkResult() const;

   /// Create an instance of MC(MC)
   ///   @param seed                Seed for random number generator
   ///   @param ascs                Proxies
   ///   @param sampleSize          Number of required samples
   ///   @param pdf                 Pdf
   ///   @param constraints         Constraints for proxies
   ///   @param mcmcType            Type of algorithm: MC, MCMC or MCSolver
   ///   @param krigingData         Kriging data for proxies
   ///   @param maxNbOfIterations   Maximum number of iterations
   void createMCMC( int seed, std::vector<McmcProxy*>const& ascs, unsigned int sampleSize, const ParameterPdf& pdf, const ParameterBounds& constraints,
            IMcmc::McmcType mcmcType, const KrigingData& krigingData, unsigned int maxNbOfIterations );

   /// Destroy the instance of MC(MC)
   void destroyMCMC();

   // IMcmc implementation


   /// Set the measurement error distristribution to be used when calculating
   /// the likelihood of a simulated result.
   ///
   /// Use the Mixed or Robust method first, until a reasonable convergence has
   /// been attained. Then analyse the match and eliminate the wrong data
   /// (outliers). Then continue with the Normal distribution on the good data.
   ///
   /// @param method The distribution type.
   void setMeasurementDistributionType( IMcmc::MeasurementDistributionType method );

   /// Set the parameter distribution to use in the MCMC acceptance criterium.
   ///
   /// When the prior is largely unknown, and determined only to provide an
   /// initial sampling, setting the distribution to NoPrior allows to ignore
   /// the prior distribution when applying the acceptance criterium in the
   /// MCMC step processes. The assumption then is that any parameter value
   /// within bounds is equally likely a priory.
   ///
   /// @param parameterDistribution The distribution.
   void setParameterDistributionType( IMcmc::ParameterDistributionType parameterDistribution );

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   void setMarginalDistributionType( size_t parameterSeqnb, MarginalProbDistr::Type marginalDistributionType );

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   void setMarginalDistributionType( const std::vector<MarginalProbDistr::Type>& marginalDistribution );


   /// Set the step method type.
   ///
   /// MetropolisHasting is used to obtain a sample of the posterior parameter
   /// distribution. The survival of the fittest is used to obtain the maximum
   /// likelihood parameter values.
   ///
   /// @param stepMethod The step method to use.
   void setStepMethodType( IMcmc::StepMethod stepMethod );

   /// Set Kriging usage: NoMcmcKriging, SmartMcmcKriging, or FullMcmcKriging.
   ///
   /// @param krUsage The indicator specifying Kriging usage.
   void setKrigingUsage( IMcmc::KrigingUsage krUsage );

   /// Setter for the kriging type to apply.
   /// @param [in] krigingType the new kriging type value
   void setKrigingType( KrigingType krigingType );
   
   /// Setter for the proxy kriging type to apply.
   /// @param [in] krigingType the new kriging type value to be used for the proxy
   void setProxyKrigingType( KrigingType krigingType );

   /// Set maximum number of iterations.
   ///
   /// @param maxNbOfIterations The maximum number of iterations.
   void setMaxNbOfIterations( unsigned int maxNbOfIterations );

   /// Set the standard deviation factor to be used.
   ///
   /// @param stdDevFactor The new standard deviation factor.
   void setStdDevFactor( double stddevfac );

   /// Get the global standard deviation factor.
   ///
   /// @return The global standard deviation factor.
   double getStdDevFactor() const;

   /// Adjust the standard deviation factor assuming a perfect fit.
   ///
   /// Adjust the standard deviation factor assuming a perfect fit, ie use the
   /// square-root of the current chi-square as the standard deviation factor.
   /// This will, for reasonable amounts of data, bring the goodness-of-fit
   /// close to a half. The goodness-of-fit indicator is subsequently not useful
   /// anymore. SUM does however calculate the goodness-of-fit without using the
   /// factor for diagnostic purposes.
   void adaptStdDevFactor();

   /// Get the values of ySample
   /// @return the values of ySample
   const std::vector<std::vector<double> >& getYSample() const;

   /// Get the values of ySample sorted by RMSE (same order as getBestMatches)
   /// @return the values of ySample sorted by RMSE (same order as getBestMatches)
   const std::vector<std::vector<double> > getSortedYSample() const;
   
   /// Get the average values of pSample
   /// @return the average values of pSample
   const std::vector<double>& getPSampleAvg() const;

   /// Get the average values of pSample per categorical combination
   /// @return the average values of pSample per categorical combination
   const std::vector< std::vector<double> >& getPCatSampleAvgs() const;

   /// Get the covariance values of pSample per categorical combination
   /// @return the covariance values of pSample per categorical combination
   const std::vector<std::vector<std::vector<double> > >& getPCatSampleCovs() const;

   /// Get the best matches
   /// @return the best matches
   const IMcmc::ParameterRanking& getBestMatches() const;

   /// Get the iteration count
   /// @return the iteration count
   unsigned int getIterationCount();

   /// Get the categorical combinations
   /// @return the categorical combination
   std::vector< std::vector< unsigned int > > getCatCombi();


   /// Performs multiple iterations until the MCMC process converges.
   ///
   /// Depending on the value of m_stepMethod, accept proposals using the
   /// Metropolis Hasting algorithm or accept only proposals that increase
   /// the likelihood.
   ///
   /// Metropolis Hasting is used to converge the sample towards a sample
   /// of the posterior distribution when assessing the uncertainties.
   ///
   /// Survival of the Fittest is used to converge the sample towards maximum
   /// likelihood parameters. The parameters end up in (local) optima.
   /// Distinct history matches can be found this way.
   ///
   /// @returns the number of iterations needed for convergence
   unsigned int execute();

   /// Performs a single iteration in the MCMC process. An iteration loops
   /// over a limited number of cycles each of which performing a limited
   /// number of steps. The step size is tuned such that the last steps of
   /// the cycles can be assumed uncorrelated. To establish a step, cheap
   /// "proposal steps" are generated first until one is accepted.
   ///
   /// @returns the iteration counter (0 if process has converged)
   unsigned int iterateOnce();

   /**
    * Returns for every McmcProxy the P10 to P90 values.
    * In addition the corresponding scenarios are returned.
    *
    * The rows of the matrices are in the same order as in the
    * McmcProxy vector ascs.
    * Dimension m: [ascs.size()][9]
    * Dimension s: [ascs.size()][9][pSize]
    */
   void getP10toP90( IMcmc::P10ToP90Values& values, IMcmc::P10ToP90Parameters& parameters );

private:

   unsigned int      m_mcmcId;
   ISerializer*      m_serialiser;
   IDeserializer*    m_deserialiser;

   McmcStatistics&   m_statistics;  ///< Kept here for McmcBase

   // These objects need to be mutable, because the interface requires that the
   // methods in which these objects are changed are const
   mutable std::vector<std::vector<double> >   m_ysample;
   mutable std::vector<double>                 m_psampleavg;
   mutable std::vector<std::vector<double> >   m_pcatsampleavgs;
   mutable std::vector< std::vector<std::vector<double> > >  m_pcatsamplecovs;
   mutable ParameterRanking                    m_bestMatches;
};


class INTERFACE_SUMLIB McmcParProtocolSlave
{

public:

   /// Constructor
   ///   @param mcmcId        Id of this instance
   ///   @param serializer    Object for sending to the master
   ///   @param deserializer  Object for receiving from the master
   McmcParProtocolSlave( unsigned int mcmcId, ISerializer* serializer, IDeserializer* deserializer );

   /// Destructor
   virtual ~McmcParProtocolSlave();

   /// Loop for handling the commands that are send
   void handleCommands();

   /// Generic code for sending an error to the master
   ///   @param msg  Information about the error
   void setError( std::string msg );

private:
   /// Send an OK result to the master
   void setResultOK();

   // IMcmc implementation

   /// Create an instance of MC(MC)
   void createMCMC();

   /// Set the measurement error distristribution to be used when calculating
   /// the likelihood of a simulated result.
   ///
   /// Use the Mixed or Robust method first, until a reasonable convergence has
   /// been attained. Then analyse the match and eliminate the wrong data
   /// (outliers). Then continue with the Normal distribution on the good data.
   ///
   /// @param method The distribution type.
   void setMeasurementDistributionType();

   /// Set the parameter distribution to use in the MCMC acceptance criterium.
   ///
   /// When the prior is largely unknown, and determined only to provide an
   /// initial sampling, setting the distribution to NoPrior allows to ignore
   /// the prior distribution when applying the acceptance criterium in the
   /// MCMC step processes. The assumption then is that any parameter value
   /// within bounds is equally likely a priory.
   ///
   /// @param parameterDistribution The distribution.
   void setParameterDistributionType();

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   void setMarginalDistributionType2();

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   void setMarginalDistributionType();


   /// Set the step method type.
   ///
   /// MetropolisHasting is used to obtain a sample of the posterior parameter
   /// distribution. The survival of the fittest is used to obtain the maximum
   /// likelihood parameter values.
   ///
   /// @param stepMethod The step method to use.
   void setStepMethodType();

   /// Set Kriging usage: NoMcmcKriging, SmartMcmcKriging, or FullMcmcKriging.
   ///
   /// @param krUsage The indicator specifying Kriging usage.
   void setKrigingUsage();

   /// Setter for the kriging type to apply.
   /// @param [in] krigingType the new kriging type value
   void setKrigingType();
   
   /// Setter for the proxy kriging type to apply.
   /// @param [in] krigingType the new kriging type value to be used for the proxy
   void setProxyKrigingType();

   /// Set maximum number of iterations.
   ///
   /// @param maxNbOfIterations The maximum number of iterations.
   void setMaxNbOfIterations();

   /// Set the standard deviation factor to be used.
   ///
   /// @param stdDevFactor The new standard deviation factor.
   void setStdDevFactor();

   /// Get the global standard deviation factor.
   ///
   /// @return The global standard deviation factor.
   void getStdDevFactor();

   /// Adjust the standard deviation factor assuming a perfect fit.
   ///
   /// Adjust the standard deviation factor assuming a perfect fit, ie use the
   /// square-root of the current chi-square as the standard deviation factor.
   /// This will, for reasonable amounts of data, bring the goodness-of-fit
   /// close to a half. The goodness-of-fit indicator is subsequently not useful
   /// anymore. SUM does however calculate the goodness-of-fit without using the
   /// factor for diagnostic purposes.
   void adaptStdDevFactor();

   /// Get the values of ySample
   /// @return the values of ySample
   void getYSample();

   /// Get the values of ySample sorted by RMSE (same order as getBestMatches)
   /// @return the values of ySample sorted by RMSE (same order as getBestMatches)
   void getSortedYSample();
   
   /// Get the average values of pSample
   /// @return the average values of pSample
   void getPSampleAvg();

   /// Get the average values of pSample per categorical combination
   /// @return the average values of pSample per categorical combination
   void getPCatSampleAvgs();

   /// Get the covariance values of pSample per categorical combination
   /// @return the covariance values of pSample per categorical combination
   void getPCatSampleCovs();

   /// Get the best matches
   /// @return the best matches
   void getBestMatches();

   /// Get the iteration count
   /// @return the iteration count
   void getIterationCount();

   /// Get the categorical combinations
   /// @return the categorical combination
   void getCatCombi();


   /// Performs multiple iterations until the MCMC process converges.
   ///
   /// Depending on the value of m_stepMethod, accept proposals using the
   /// Metropolis Hasting algorithm or accept only proposals that increase
   /// the likelihood.
   ///
   /// Metropolis Hasting is used to converge the sample towards a sample
   /// of the posterior distribution when assessing the uncertainties.
   ///
   /// Survival of the Fittest is used to converge the sample towards maximum
   /// likelihood parameters. The parameters end up in (local) optima.
   /// Distinct history matches can be found this way.
   ///
   /// @returns the number of iterations needed for convergence
   void execute();

   /// Performs a single iteration in the MCMC process. An iteration loops
   /// over a limited number of cycles each of which performing a limited
   /// number of steps. The step size is tuned such that the last steps of
   /// the cycles can be assumed uncorrelated. To establish a step, cheap
   /// "proposal steps" are generated first until one is accepted.
   ///
   /// @returns the iteration counter (0 if process has converged)
   void iterateOnce();

   /**
    * Returns for every McmcProxy the P10 to P90 values.
    * In addition the corresponding scenarios are returned.
    *
    * The rows of the matrices are in the same order as in the
    * McmcProxy vector ascs.
    * Dimension m: [ascs.size()][9]
    * Dimension s: [ascs.size()][9][pSize]
    */
   void getP10toP90();

private:

   unsigned int         m_mcmcId;
   ISerializer*         m_serialiser;
   IDeserializer*       m_deserialiser;
   McmcBase*            m_mcmc;

   // Permanent members of m_mcmc copied from master
   std::vector<McmcProxy*> m_ascs;
   ParameterPdf            m_pdf;
   ParameterBounds         m_constraints;
   McmcStatistics          m_statistics;
   KrigingData             m_krigingData;
};

} // namespace SUMlib

#endif // SUMLIB_MCMCPARPROTOCOL_H
