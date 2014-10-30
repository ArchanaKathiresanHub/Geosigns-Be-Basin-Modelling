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

using namespace casa;

MatlabExporter::MatlabExporter( const std::string & dataFileName )
{
   m_fname = dataFileName;
   assert( !m_fname.empty() );
}

MatlabExporter::~MatlabExporter()
{
}


void MatlabExporter::exportDoEInfo( ScenarioAnalysis & sc, std::ofstream & ofs )
{
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );

   // export DoE info
   const std::vector<std::string> & exps = rcs.experimentNames();

   // save DoE names 
   ofs << "DoENames = {\n";
   for ( size_t i = 0; i < exps.size(); ++i ) ofs << "   \'" << exps[i] << "\'\n";
   ofs << "};\n\n";

   // save number of experiments for each DoE
   ofs << "DoECases = [ ";
   for ( size_t i = 0; i < exps.size(); ++i )
   {
      rcs.filterByExperimentName( exps[i] );
      ofs << rcs.size() << " ";
   }
   ofs << "];\n\n";

   rcs.filterByExperimentName( "" ); // reset filter
}


void MatlabExporter::exportParametersInfo( ScenarioAnalysis & sc, std::ofstream & ofs )
{
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );

   // First save name of the variable parameters
   ofs << "ParametersName = {\n";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      SharedParameterPtr prm = rcs[0]->parameter( j );
      if ( !prm  ) continue;
      ofs << "    \'" << prm->name() << "\'\n";      
   }
   ofs << "};\n\n";

   // Second - save parameters dimension
   size_t totPrmsNum = 0;

   ofs << "ParametersDim = [ ";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      SharedParameterPtr prm = rcs[0]->parameter( j );
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:  ofs << " " << prm->asDoubleArray().size(); totPrmsNum += prm->asDoubleArray().size(); break;
         case VarParameter::Categorical: ofs << " " << 1;                         ++totPrmsNum;                                break;
         default: assert( false ); break;
      }
   }
   ofs << " ];\n\n";

   // Parameters ranges - min/max
   // min values
   ofs << "ParametersMinVals = [ ";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      const SharedParameterPtr prm = rcs[0]->parameter( j )->parent()->minValue();
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:
         {
            const std::vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k ) { ofs << "\t" << prmVals[k]; }
         }
         break;
         case VarParameter::Categorical:  ofs << " " << prm->asInteger();  break;
         default: assert( false ); break;
      }
   }
   ofs << " ];\n\n";

   // min values
   ofs << "ParametersMaxVals = [ ";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      const SharedParameterPtr prm = rcs[0]->parameter( j )->parent()->maxValue();
      switch( prm->parent()->variationType() )
      {
         case VarParameter::Discrete:
         case VarParameter::Continuous:
         {
            const std::vector<double> & prmVals = prm->asDoubleArray();
            for ( size_t k = 0; k < prmVals.size(); ++k ) { ofs << "\t" << prmVals[k]; }
         }
         break;
         case VarParameter::Categorical:  ofs << " " << prm->asInteger();  break;
         default: assert( false ); break;
      }
   }
   ofs << " ];\n\n";

   // Parameters values for each case
   ofs << "ParametersVal = [\n";

   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      ofs << i;  // Case number

      for ( size_t j = 0; j < rcs[i]->parametersNumber(); ++j )
      {
         SharedParameterPtr prm = rcs[i]->parameter( j );

         switch( prm->parent()->variationType() )
         {
            case VarParameter::Discrete:
            case VarParameter::Continuous:
               {
                  const std::vector<double> & prmVals = prm->asDoubleArray();
                  for ( size_t k = 0; k < prmVals.size(); ++k ) { ofs << "\t" << prmVals[k]; }
               }
               break;

            case VarParameter::Categorical:  ofs << " " << prm->asInteger();  break;
            default: assert( false ); break;
         }        
     }
     ofs << "\n";
   }
   ofs << "];\n\n";
}


void MatlabExporter::exportObservablesInfo( ScenarioAnalysis & sc, std::ofstream & ofs )
{
   RunCaseSetImpl & rcs = dynamic_cast<RunCaseSetImpl&>( sc.doeCaseSet() );

   // save observables value for each case
   // First save name of the observables
   ofs << "ObservablesName = {\n";
   for ( size_t j = 0; j < rcs[0]->observablesNumber(); ++j )
   {
      ObsValue * obv = rcs[0]->obsValue( j );
      
      if ( !obv || !obv->isDouble()  ) continue;  // skip observables which is not double

      ofs << "    \'" << obv->observable()->name() << "\'\n";      
   }
   ofs << "};\n\n";

   // Second - value for observables for each case
   ofs << "ObservablesDim = [ ";
   for ( size_t j = 0; j < rcs[0]->observablesNumber(); ++j )
   {
      ObsValue * obv = rcs[0]->obsValue( j );
      
      if ( !obv || !obv->isDouble()  ) continue;  // skip observables which is not double

      ofs << " " << obv->asDoubleArray().size();      
   }
   ofs << " ];\n\n";

   ofs << "ObservablesRefValue = [\n";
   for ( size_t i = 0; i < sc.obsSpace().size(); ++i )
   {
      const Observable * obs = sc.obsSpace().observable( i );
      ofs << "    { [ ";
      if ( obs->hasReferenceValue() )
      {
         const ObsValue * obv = obs->referenceValue();
         const std::vector<double> & rv = obv->asDoubleArray();
         for ( size_t j = 0; j < rv.size(); ++j )
         {
            ofs << rv[j] << " ";
         }
      }
      ofs << " ], " << obs->stdDeviationForRefValue() << " }\n";
   }
   ofs << " ];\n\n";

   ofs << "ObservablesVal = [\n";
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      ofs << i;

      for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
      {
         ObsValue * obv = rcs[i]->obsValue( j );

         if ( obv && obv->isDouble() )
         {
            const std::vector<double> & vals = obv->asDoubleArray();
            for ( size_t k = 0; k < vals.size(); ++k ) ofs << "\t" << vals[k];
         }
      }

      ofs << std::endl;
   }
   ofs << "];\n\n";

   // Third - observables MSE for each case
   ofs << "ObservablesMSE = [\n";
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      ofs << i;

      for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
      {
         ObsValue * obv = rcs[i]->obsValue( j );

         if ( obv && obv->isDouble() )
         {
            ofs << "\t" << obv->MSE();
         }
      }

      ofs << std::endl;
   }
   ofs << "];\n\n";

}

void MatlabExporter::exportRSAProxies( ScenarioAnalysis & sc, std::ofstream & ofs )
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
 
   ofs << "ProxiesList = {\n";
   for ( size_t i = 0; i < proxyNames.size(); ++i )
   {
      ofs << "   \"" << proxyNames[i] << "\"\n";
   }
   ofs << "};\n\n";

   for ( size_t i = 0; i < proxyNames.size(); ++i )
   {
      const RSProxyImpl * rs = dynamic_cast< const RSProxyImpl *> ( sc.rsProxySet().rsProxy( proxyNames[i].c_str() ) );
      const RSProxy::CoefficientsMapList & cml = rs->getCoefficientsMapList();

      if ( rs->kriging() != RSProxy::NoKriging )
      {
         ofs << "warning( 'Proxy: " << proxyNames[i] << " uses kring interpolation, but kriging interpolation can not be exported to Matlab function.' );\n";
      }
      // create function to calculater proxy value for observable
      ofs << "function ret = proxy_" << proxyNames[i] << "( "; 
      
      // it will take all parameters, but use some of them
      for ( size_t k = 0; k < totPrmsNum; ++k ) ofs << "prm_" << k+1 << (k+1 < totPrmsNum ? ", " : " )\n" );

      for ( size_t j = 0; j < cml.size(); ++j )
      {
         const RSProxy::CoefficientsMap & cmap = cml[j];

         // some trick for easy print formating
         ofs << "   ret(" << j+1 << ") = 0.0";

         for ( RSProxy::CoefficientsMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it )
         {
            const std::vector< unsigned int > & prmsLst = it->first;

            ofs << " " << (it->second < 0.0 ? "" : "+") << it->second << " ";
            for( size_t k = 0; k < prmsLst.size(); ++k ) 
            {
               ofs << "* prm_" << prmsLst[k]+1;
            }
         }
         ofs << ";\n";
      }
      ofs << "end\n\n";
   }
}

void MatlabExporter::exportMCResults( ScenarioAnalysis & sc, std::ofstream & ofs )
{
   ofs << "MCAlgoName = '";
   switch ( sc.mcSolver().algorithm() )
   {
      case MonteCarloSolver::MonteCarlo:  ofs << "MC";          break;
      case MonteCarloSolver::MCMC:        ofs << "MCMC";        break;
      case MonteCarloSolver::MCLocSolver: ofs << "MCLocSolver"; break;
   }
   ofs << "';\n\n";

   ofs << "MCKrigingType = '";
   switch ( sc.mcSolver().kriging() )
   {
      case MonteCarloSolver::NoKriging:     ofs << "Polynomial";    break;
      case MonteCarloSolver::SmartKriging:  ofs << "SmartKriging";  break;
      case MonteCarloSolver::GlobalKriging: ofs << "GlobalKriging"; break;
   }
   ofs << "';\n\n";

   ofs << "MCMeasurementDistr = '";
   switch ( sc.mcSolver().measurementDistrib() )
   {
      case MonteCarloSolver::NoMeasurements: ofs << "Undefined"; break;
      case MonteCarloSolver::Normal:         ofs << "Normal";    break; 
      case MonteCarloSolver::Robust:         ofs << "Robust";    break;
      case MonteCarloSolver::Mixed:          ofs << "Mixed";     break;
   }
   ofs << "';\n\n";
 
   ofs << "MCPriorDistribution = '";
   switch( sc.mcSolver().priorDistribution() )
   {
      case MonteCarloSolver::NoPrior:           ofs << "NoPrior";           break;
      case MonteCarloSolver::MarginalPrior:     ofs << "MarginalPrior";     break;
      case MonteCarloSolver::MultivariatePrior: ofs << "MultivariatePrior"; break;
   }
   ofs << "';\n\n";

   ofs << "MCGOF                = " << sc.mcSolver().GOF()                  << ";\n\n";
   ofs << "MCStdDevFact         = " << sc.mcSolver().stdDevFactor()         << ";\n\n";
   ofs << "MCProposedStdDevFact = " << sc.mcSolver().proposedStdDevFactor() << ";\n\n";

   // export MC samples
   const MonteCarloSolver::MCResults & mcSamples = sc.mcSolver().getSimulationResults();

   ofs << "MCSamplingPrmsVal = [\n";

   std::vector<double> mcRMSEs;
   
   for ( size_t i = 0; i < mcSamples.size(); ++i )
   {
      ofs << i;  // Case number

      for ( size_t j = 0; j < mcSamples[i].second->parametersNumber(); ++j )
      {
         SharedParameterPtr prm = mcSamples[i].second->parameter( j );

         switch( prm->parent()->variationType() )
         {
            case VarParameter::Continuous:
            case VarParameter::Discrete:
               {
                  const std::vector<double> & prmVals = prm->asDoubleArray();
                  for ( size_t k = 0; k < prmVals.size(); ++k ) { ofs << "\t" << prmVals[k]; }
               }
               break;

            case VarParameter::Categorical:  ofs << " " << prm->asInteger();  break;
            default: assert( false ); break;
         }
         mcRMSEs.push_back( mcSamples[i].first );
     }
     ofs << "\n";
   }
   ofs << "];\n\n";

   // save RMSE array
   ofs << "MCSamplingRMSE = [\n";
   for ( size_t i = 0; i < mcRMSEs.size(); ++i )
   {
      ofs << mcRMSEs[i] << "\n";
   }
   ofs << "];\n\n";

   // export MC samplings observables value
   ofs << "MCSamplingObsVal = [\n";

   for ( size_t i = 0; i < mcSamples.size(); ++i )
   {
      ofs << i;  // Case number
      for ( size_t j = 0; j < mcSamples[i].second->observablesNumber(); ++j )
      {
         ObsValue * obv = mcSamples[i].second->obsValue( j );

         if ( obv && obv->isDouble() )
         {
            const std::vector<double> & vals = obv->asDoubleArray();
            for ( size_t k = 0; k < vals.size(); ++k ) ofs << "\t" << vals[k];
         }
      }
      ofs << std::endl;
   }
   ofs << "];\n\n";
}

void MatlabExporter::exportScenario( ScenarioAnalysis & sc, const std::string & baseCaseName, const std::string & location )
{
   std::ofstream ofs( m_fname.c_str(), std::ios_base::out | std::ios_base::trunc );
   
   if ( ofs.fail() ) return;

   // export globals
   ofs << "BaseCaseName  = '" << baseCaseName << "';\n";
   ofs << "PathToCaseSet = '" << location << "';\n";

   sc.doeCaseSet().filterByExperimentName( "" );
   if ( !sc.doeCaseSet().size() )
   {
      ofs << "# Empty case set no data to print\n\n";
      return;
   }

   // export DoE
   exportDoEInfo( sc, ofs );

   // export cases parameters
   exportParametersInfo( sc, ofs );

   // export cases observables
   exportObservablesInfo( sc, ofs );

   // export proxies as functions
   exportRSAProxies( sc, ofs );

   // export MC/MCMC results
   exportMCResults( sc, ofs );

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

