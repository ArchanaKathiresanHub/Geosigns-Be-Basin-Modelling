#ifndef INTERFACE__SIMULATION_DETAILS__H
#define INTERFACE__SIMULATION_DETAILS__H

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

#include <string>

namespace DataAccess {

   namespace Interface {

      /// \brief Contains details about a simulation.
      ///
      /// For example
      ///    - the name of the simulator, e.g. fastcauldron, fastmig;
      ///    - simulation mode, e.g. HydrostaticDecompaction;
      ///    - the number of cores used in the simulation;
      ///    - the command line parameter passed to the simulator.
      class SimulationDetails : public DAObject {

      public :

         /// constructor.
         SimulationDetails (ProjectHandle * projectHandle, database::Record * record);

         /// destructor.
         virtual ~SimulationDetails ();

         /// \brief Get the sequence number of the simulation.
         ///
         /// This is used to order the sequence of simulations.
         /// A larger number indicates a simulation that was performed later in the sequence.
         virtual int getSimulationSequenceNumber () const;

         /// \brief Get the name of the simulator.
         virtual const std::string& getSimulatorName () const;

         /// \brief Get the most of the simulation.
         virtual const std::string& getSimulatorMode () const;

         /// \brief Get the number of cores used in the simulation.
         virtual int getNumberOfCores () const;

         /// \brief Get a string of all the command line parameters passed to the simulator.
         virtual const std::string& getSimulatorCommandLineParameters () const;

      private :

         std::string m_simulatorName;
         std::string m_simulatorMode;
         std::string m_commandLineParams;

      };

      /// \brief Used to order a sequence of simulation-details.
      class SimulationDetailsComparison {
      public :
         bool operator ()( const SimulationDetails* left, const SimulationDetails* right ) const;
      };

   }

}

#endif // INTERFACE__SIMULATION_DETAILS__H
