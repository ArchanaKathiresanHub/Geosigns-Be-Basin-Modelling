//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "casaAPI.h"
#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"
#include "RSProxyImpl.h"
#include "RSProxySetImpl.h"
#include "RunCase.h"
#include "RunCaseSetImpl.h"
#include "RunManagerImpl.h"
#include "SUMlibUtils.h"

#include "MatlabExporter.h"

#include <iomanip>

using namespace casa;

MatlabExporter::MatlabExporter( const std::string & dataFileName )
{
   m_fname = dataFileName;
   assert( !m_fname.empty() );

   m_ofs.open( m_fname.c_str(), std::ios_base::out | std::ios_base::trunc );

   if ( m_ofs.fail() ) throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can not open file for writing: " << m_fname;
}

MatlabExporter::~MatlabExporter()
{
   m_ofs.close();
}


void MatlabExporter::exportDoEInfo( ScenarioAnalysis & sc )
{
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );

   // export DoE info
   const std::vector<std::string> & exps = rcs.experimentNames();

   // save DoE names 
   m_ofs << "DoENames = {\n";
   for ( size_t i = 0; i < exps.size(); ++i ) m_ofs << "   \'" << exps[i] << "\'\n";
   m_ofs << "};\n\n";

   // save number of experiments for each DoE
   m_ofs << "DoECases = [ ";
   for ( size_t i = 0; i < exps.size(); ++i )
   {
      rcs.filterByExperimentName( exps[i] );
      m_ofs << rcs.size() << " ";
   }
   m_ofs << "];\n\n";

   rcs.filterByExperimentName( "" ); // reset filter
}


void MatlabExporter::exportParametersInfo( ScenarioAnalysis & sc )
{
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );

   // First save name of the variable parameters
   m_ofs << "ParametersName = {\n";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      SharedParameterPtr prm = rcs[0]->parameter( j );
      if ( !prm  ) continue;
      m_ofs << "    \'" << prm->name() << "\'\n";      
   }
   m_ofs << "};\n\n";

   // Second - save parameters dimension
   size_t totPrmsNum = 0;

   m_ofs << "ParametersDim = [ ";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      SharedParameterPtr prm = rcs[0]->parameter( j );
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:  m_ofs << " " << prm->asDoubleArray().size(); totPrmsNum += prm->asDoubleArray().size(); break;
         case VarParameter::Categorical: m_ofs << " " << 1;                         ++totPrmsNum;                                break;
         default: assert( false ); break;
      }
   }
   m_ofs << " ];\n\n";

   m_ofs << "SubParametersName = {\n";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      const VarParameter * parent = rcs[0]->parameter( j )->parent();
      if ( !parent  ) continue;
      const std::vector<std::string> & pnames = parent->name();
      for ( size_t k = 0; k < pnames.size(); ++ k )
      {
         m_ofs << "    \'" << pnames[k] << "\'\n";
      }
   }
   m_ofs << "};\n\n";

   // Parameters ranges - min/max
   // min values
   m_ofs << "ParametersMinVals = [ ";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      const SharedParameterPtr prm = rcs[0]->parameter( j )->parent()->minValue();
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:
         {
            const std::vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k ) { m_ofs << "\t" << prmVals[k]; }
         }
         break;
         case VarParameter::Categorical:  m_ofs << " " << prm->asInteger();  break;
         default: assert( false ); break;
      }
   }
   m_ofs << " ];\n\n";

   // min values
   m_ofs << "ParametersMaxVals = [ ";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      const SharedParameterPtr prm = rcs[0]->parameter( j )->parent()->maxValue();
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:
         {
            const std::vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k ) { m_ofs << "\t" << prmVals[k]; }
         }
         break;
         case VarParameter::Categorical:  m_ofs << " " << prm->asInteger();  break;
         default: assert( false ); break;
      }
   }
   m_ofs << " ];\n\n";

   // Parameters values for each case
   m_ofs << "ParametersVal = [\n";

   const std::vector<std::string> & doeList = rcs.experimentNames();
   for ( size_t e = 0, expID = 0; e < doeList.size(); ++e )
   {
      rcs.filterByExperimentName( doeList[e] );

      for ( size_t i = 0; i < rcs.size(); ++i )
      {
         m_ofs << expID++;  // Case number

         for ( size_t j = 0; j < rcs[i]->parametersNumber(); ++j )
         {
            SharedParameterPtr prm = rcs[i]->parameter( j );

            switch( prm->parent()->variationType() )
            {
               case VarParameter::Discrete:
               case VarParameter::Continuous:
                  {
                     const std::vector<double> & prmVals = prm->asDoubleArray();
                     for ( size_t k = 0; k < prmVals.size(); ++k ) { m_ofs << "\t" << prmVals[k]; }
                  }
                  break;

               case VarParameter::Categorical:  m_ofs << " " << prm->asInteger();  break;
               default: assert( false ); break;
            }
         }
         m_ofs << "\n";
      }
   }
   rcs.filterByExperimentName( "" );
   m_ofs << "];\n\n";

   // Parameters values for the base case
   m_ofs << "ParametersValBaseCase = [\n";

   const casa::RunCase * cs = sc.baseCaseRunCase();
   if ( cs )
   {
      for ( size_t j = 0; j < cs->parametersNumber(); ++j )
      {
         SharedParameterPtr prm = cs->parameter( j );

         switch( prm->parent()->variationType() )
         {
            case VarParameter::Discrete:
            case VarParameter::Continuous:
               {
                  const std::vector<double> & prmVals = prm->asDoubleArray();
                  for ( size_t k = 0; k < prmVals.size(); ++k ) { m_ofs << "\t" << prmVals[k]; }
               }
               break;

            case VarParameter::Categorical:  m_ofs << " " << prm->asInteger();  break;
            default: assert( false ); break;
         }        
     }
     m_ofs << "\n";
   }
   m_ofs << "];\n\n";
}


void MatlabExporter::exportObservablesInfo( ScenarioAnalysis & sc )
{
   // save observables value for each case
   // First save name of the observables
   m_ofs << "ObservablesName = {\n";
   for ( size_t j = 0; j < sc.obsSpace().size(); ++j )
   {
      // export untransformed observable name first
      const std::vector<std::string> & obsNames = sc.obsSpace().observable( j )->name();
      for ( size_t k = 0; k < obsNames.size(); ++k )
      {
         m_ofs << "    \'" << obsNames[k] << "\'\n";      
      }
   }
   m_ofs << "};\n\n";

   // Second - value for observables for each case
   m_ofs << "ObservablesDim = [ ";
   for ( size_t j = 0; j < sc.obsSpace().size(); ++j )
   {
      m_ofs << " " << sc.obsSpace().observable( j )->dimension();      
   }
   m_ofs << " ];\n\n";

   m_ofs << "ObservablesRefValue = [\n";
   for ( size_t i = 0; i < sc.obsSpace().size(); ++i )
   {
      const Observable * obs = sc.obsSpace().observable( i );
      m_ofs << "    { [ ";
      if ( obs->hasReferenceValue() )
      {
         const ObsValue * obv = obs->referenceValue();
         const std::vector<double> & rv = obv->asDoubleArray();
         for ( size_t j = 0; j < rv.size(); ++j )
         {
            m_ofs << rv[j] << " ";
         }
      }
      m_ofs << " ], " << obs->stdDeviationForRefValue() << " }\n";
   }
   m_ofs << " ];\n\n";

   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );

   m_ofs << "ObservablesVal = [\n";
   const std::vector<std::string> & doeList = rcs.experimentNames();
   for ( size_t e = 0, expID = 0; e < doeList.size(); ++e )
   {
      rcs.filterByExperimentName( doeList[e] );

      for ( size_t i = 0; i < rcs.size(); ++i )
      {
         m_ofs << expID++;

         for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
         {
            ObsValue * obv = rcs[i]->obsValue( j );

            if ( obv && obv->isDouble() )
            {
               const std::vector<double> & vals = obv->asDoubleArray();
               for ( size_t k = 0; k < vals.size(); ++k ) m_ofs << "\t" << vals[k];
            }
         }
         m_ofs << std::endl;
      }
   }
   rcs.filterByExperimentName( "" );
   m_ofs << "];\n\n";

   m_ofs << "ObservablesValBaseCase = [\n";
   casa::RunCase * cs = sc.baseCaseRunCase();
   if ( cs )
   {
      for ( size_t j = 0; j < cs->observablesNumber(); ++j )
      {
         ObsValue * obv = cs->obsValue( j );

         if ( obv && obv->isDouble() )
         {
            const std::vector<double> & vals = obv->asDoubleArray();
            for ( size_t k = 0; k < vals.size(); ++k ) m_ofs << "\t" << vals[k];
         }
      }
      m_ofs << std::endl;
   }
   m_ofs << "];\n\n";


   // Third - observables MSE for each case
   m_ofs << "ObservablesMSE = [\n";
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      m_ofs << i;

      for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
      {
         ObsValue * obv = rcs[i]->obsValue( j );

         if ( obv && obv->isDouble() )
         {
            m_ofs << "\t" << obv->MSE();
         }
      }

      m_ofs << std::endl;
   }
   m_ofs << "];\n\n";
}

void MatlabExporter::exportRSAProxies( ScenarioAnalysis & sc )
{
   // Save proxies as set of function for each proxy_observable
   const std::vector<std::string> & proxyNames = sc.rsProxySet().names();

   // calculate total parameters number
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );
   size_t totPrmsNum = 0;
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      SharedParameterPtr prm = rcs[0]->parameter( j );
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:    totPrmsNum += prm->asDoubleArray().size(); break;
         case VarParameter::Categorical: ++totPrmsNum;                                break;
         default: assert( false ); break;
      }
   }
 
   m_ofs << "ProxiesList = {\n";
   for ( size_t i = 0; i < proxyNames.size(); ++i )
   {
      m_ofs << "   \"" << proxyNames[i] << "\"\n";
   }
   m_ofs << "};\n\n";

   m_ofs << "%export proxy polynomial coefficients as a list of matrix\n";
   for ( size_t i = 0; i < proxyNames.size(); ++i )
   {
      const RSProxyImpl * rs = dynamic_cast< const RSProxyImpl *> ( sc.rsProxySet().rsProxy( proxyNames[i].c_str() ) );
      const RSProxy::CoefficientsMapList & cml = rs->getCoefficientsMapList();

      for ( size_t j = 0; j < cml.size(); ++j )
      {
         const RSProxy::CoefficientsMap & cmap = cml[j];

         std::vector<int> numCoefPerOrd;

         // get polynomial order 
         size_t ord = 0;
         for ( RSProxy::CoefficientsMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it )
         {
            const std::vector< unsigned int > & prmsLst = it->first;
            ord = ord < prmsLst.size() ? prmsLst.size() : ord;
            if ( numCoefPerOrd.size() < prmsLst.size()+1 ) numCoefPerOrd.resize( prmsLst.size()+1, 0 );
            numCoefPerOrd[prmsLst.size()] += 1; 
         }

         // allocate zero matrix with rank equals to polynomial order
         m_ofs << proxyNames[i] << "(" << j+1 << ").ord0 = 0.0;\n"; // zero order always
         for ( size_t k = 0; k < ord; ++k  )
         {
            m_ofs << proxyNames[i] << "(" << j+1 << ").ord" << k+1 << " = zeros(";
            if ( k == 0 ) m_ofs << "1, ";
            else  m_ofs << numCoefPerOrd[k] << ",";
            m_ofs << k+2 << ");\n";
         }

         numCoefPerOrd.assign( numCoefPerOrd.size(), 0 ); // reset counters

         for ( RSProxy::CoefficientsMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it )
         {
            const std::vector< unsigned int > & prmsLst = it->first;
            double coef = it->second.first;
            //double errr = it->second.second;
            m_ofs << proxyNames[i] << "(" << j + 1 << ").ord" << prmsLst.size();
            if ( prmsLst.empty() ) { m_ofs << " = " << coef << ";\n"; }
            else
            {
               m_ofs << "(" << numCoefPerOrd[prmsLst.size()]+1 << ",:) = [";
               numCoefPerOrd[prmsLst.size()] += 1;
               for ( size_t k = 0; k < prmsLst.size(); ++k ) m_ofs << prmsLst[k]+1 << ",";
               m_ofs << coef << "];\n";
            }
         }
      }
   }
   m_ofs << "\n\n";

   for ( size_t i = 0; i < proxyNames.size(); ++i )
   {
      const RSProxyImpl * rs = dynamic_cast< const RSProxyImpl *> ( sc.rsProxySet().rsProxy( proxyNames[i].c_str() ) );
      const RSProxy::CoefficientsMapList & cml = rs->getCoefficientsMapList();

      if ( rs->kriging() != RSProxy::NoKriging )
      {
         m_ofs << "warning( 'Proxy: " << proxyNames[i] << " uses kring interpolation, but kriging interpolation can not be exported to Matlab function.' );\n";
      }
      // create function to calculater proxy value for observable
      m_ofs << "function ret = proxy_" << proxyNames[i] << "( "; 
      
      // it will take all parameters, but use some of them
      for ( size_t k = 0; k < totPrmsNum; ++k ) m_ofs << "prm_" << k+1 << (k+1 < totPrmsNum ? ", " : " )\n" );

      for ( size_t j = 0; j < cml.size(); ++j )
      {
         const RSProxy::CoefficientsMap & cmap = cml[j];

         // some trick for easy print formating
         m_ofs << "   ret(" << j+1 << ") = 0.0";

         for ( RSProxy::CoefficientsMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it )
         {
            const std::vector< unsigned int > & prmsLst = it->first;
            double coef = it->second.first;
            // double errr = it->second.second;

            m_ofs << " " << (coef < 0.0 ? "" : "+ ") << coef << " ";
            for( size_t k = 0; k < prmsLst.size(); ++k ) 
            {
               m_ofs << "* prm_" << prmsLst[k]+1;
            }
         }
         m_ofs << ";\n";
      }
      m_ofs << "end\n\n";
   }
}

void MatlabExporter::exportMCResults( ScenarioAnalysis & sc )
{
   m_ofs << "MCAlgoName = '";
   switch ( sc.mcSolver().algorithm() )
   {
      case MonteCarloSolver::MonteCarlo:  m_ofs << "MC";          break;
      case MonteCarloSolver::MCMC:        m_ofs << "MCMC";        break;
      case MonteCarloSolver::MCLocSolver: m_ofs << "MCLocSolver"; break;
   }
   m_ofs << "';\n\n";

   m_ofs << "MCKrigingType = '";
   switch ( sc.mcSolver().kriging() )
   {
      case MonteCarloSolver::NoKriging:     m_ofs << "Polynomial";    break;
      case MonteCarloSolver::SmartKriging:  m_ofs << "SmartKriging";  break;
      case MonteCarloSolver::GlobalKriging: m_ofs << "GlobalKriging"; break;
   }
   m_ofs << "';\n\n";

   m_ofs << "MCMeasurementDistr = '";
   switch ( sc.mcSolver().measurementDistrib() )
   {
      case MonteCarloSolver::NoMeasurements: m_ofs << "Undefined"; break;
      case MonteCarloSolver::Normal:         m_ofs << "Normal";    break; 
      case MonteCarloSolver::Robust:         m_ofs << "Robust";    break;
      case MonteCarloSolver::Mixed:          m_ofs << "Mixed";     break;
   }
   m_ofs << "';\n\n";
 
   m_ofs << "MCPriorDistribution = '";
   switch( sc.mcSolver().priorDistribution() )
   {
      case MonteCarloSolver::NoPrior:           m_ofs << "NoPrior";           break;
      case MonteCarloSolver::MarginalPrior:     m_ofs << "MarginalPrior";     break;
      case MonteCarloSolver::MultivariatePrior: m_ofs << "MultivariatePrior"; break;
   }
   m_ofs << "';\n\n";

   m_ofs << "MCGOF                = " << sc.mcSolver().GOF()                  << ";\n\n";
   m_ofs << "MCStdDevFact         = " << sc.mcSolver().stdDevFactor()         << ";\n\n";
   m_ofs << "MCProposedStdDevFact = " << sc.mcSolver().proposedStdDevFactor() << ";\n\n";

   // export MC samples
   const MonteCarloSolver::MCResults & mcSamples = sc.mcSolver().getSimulationResults();

   m_ofs << "MCSamplingPrmsVal = [\n";

   std::vector<double> mcRMSEs;
   
   for ( size_t i = 0; i < mcSamples.size(); ++i )
   {
      m_ofs << i;  // Case number

      for ( size_t j = 0; j < mcSamples[i].second->parametersNumber(); ++j )
      {
         SharedParameterPtr prm = mcSamples[i].second->parameter( j );

         switch( prm->parent()->variationType() )
         {
            case VarParameter::Continuous:
            case VarParameter::Discrete:
               {
                  const std::vector<double> & prmVals = prm->asDoubleArray();
                  for ( size_t k = 0; k < prmVals.size(); ++k ) { m_ofs << "\t" << prmVals[k]; }
               }
               break;

            case VarParameter::Categorical:  m_ofs << " " << prm->asInteger();  break;
            default: assert( false ); break;
         }
      }
      mcRMSEs.push_back(mcSamples[i].first);
      m_ofs << "\n";
   }
   m_ofs << "];\n\n";

   // save RMSE array
   m_ofs << "MCSamplingRMSE = [\n";
   for ( size_t i = 0; i < mcRMSEs.size(); ++i )
   {
      m_ofs << std::scientific << std::setprecision( 10 )  << std::setfill( '0' ) << mcRMSEs[i] << "\n";
   }
   m_ofs << "];\n\n";

   // export MC samplings observables value
   m_ofs << "MCSamplingObsVal = [\n";

   for ( size_t i = 0; i < mcSamples.size(); ++i )
   {
      m_ofs << i;  // Case number
      for ( size_t j = 0; j < mcSamples[i].second->observablesNumber(); ++j )
      {
         ObsValue * obv = mcSamples[i].second->obsValue( j );

         if ( obv && obv->isDouble() )
         {
            const std::vector<double> & vals = obv->asDoubleArray();
            for ( size_t k = 0; k < vals.size(); ++k ) m_ofs << "\t" << vals[k];
         }
      }
      m_ofs << std::endl;
   }
   m_ofs << "];\n\n";

   // save CDFs per observable
   m_ofs << "MC_P10P90_CDF = [\n";
   const std::vector< std::vector< double > > & cdf = sc.mcSolver().p10p90CDF();
   for ( size_t i = 0; i < cdf.size(); ++i )
   {
      m_ofs << "  ";
      for ( size_t j = 0; j < cdf[i].size(); ++j )
      {
         m_ofs << " " << cdf[i][j];
      }
      m_ofs << std::endl;
   }
   m_ofs << "];\n\n";
}

void MatlabExporter::exportScenario( ScenarioAnalysis & sc, const std::string & baseCaseName, const std::string & location )
{
   // export globals
   m_ofs << "ScenarioID    = '" << sc.scenarioID() << "';\n";
   m_ofs << "BaseCaseName  = '" << baseCaseName << "';\n";
   m_ofs << "PathToCaseSet = '" << location << "';\n";

   sc.doeCaseSet().filterByExperimentName( "" );
   if ( !sc.doeCaseSet().size() )
   {
      m_ofs << "# Empty case set no data to print\n\n";
      return;
   }

   // export DoE
   exportDoEInfo( sc );

   // export cases parameters
   exportParametersInfo( sc );

   // export cases observables
   exportObservablesInfo( sc );

   // export proxies as functions
   exportRSAProxies( sc );

   // export MC/MCMC results
   exportMCResults( sc );

}

void MatlabExporter::exportObsValues( const std::string & fName, const std::vector<casa::RunCase*> & rcs )
{
   std::ofstream ofs( fName.c_str(), std::ios_base::out | std::ios_base::trunc );
   
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
      {
         const casa::ObsValue * obs = rcs[i]->obsValue( j );
         if ( obs->isDouble() )
         {
            const std::vector<double> & vals = obs->asDoubleArray();
            for ( size_t k = 0; k < vals.size(); ++k )
            {
               ofs << vals[k] << " ";
            }
         }
      }
      ofs << "\n";
   }
}

std::string MatlabExporter::correctName( std::string name )
{
   std::string::iterator it = std::remove(  name.begin(), name.end(), ')' );
   it = std::remove(  name.begin(), it, '+' );
   name.resize( it - name.begin() );

   std::replace( name.begin(), name.end(), '/', '-' );
   std::replace( name.begin(), name.end(), ':', '-' );
   std::replace( name.begin(), name.end(), ' ', '-' );
   std::replace( name.begin(), name.end(), '_', '-' );
   std::replace( name.begin(), name.end(), '(', '-' );
   std::replace( name.begin(), name.end(), ',', '-' );
   return name;
}


