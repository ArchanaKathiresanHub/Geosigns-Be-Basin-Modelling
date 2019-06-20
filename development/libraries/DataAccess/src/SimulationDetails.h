#ifndef INTERFACE__SIMULATION_DETAILS__H
#define INTERFACE__SIMULATION_DETAILS__H

#include "DAObject.h"
#include "Interface.h"

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

         /// \brief Get the mode of the simulation.
         ///
         /// Currently only fastcaudron is capable of running in different modes.
         /// HydrostaticDecompaction, HydrostaticTemperature, ...
         /// All other simulators will have the value Default.
         virtual const std::string& getSimulatorMode () const;

         /// \brief Get the number of cores used in the simulation.
         virtual int getNumberOfCores () const;

         /// \brief Get a string of all the command line parameters passed to the simulator.
         virtual const std::string& getSimulatorCommandLineParameters () const;

      private :

         /// \brief Contains the name of the simulator.
         std::string m_simulatorName;

         /// \brief Contains the name of the mode the simulator was run.
         std::string m_simulatorMode;

         /// \brief Contains all of the command line parameters that were passed to the simulator.
         std::string m_commandLineParams;

      };

      /// \brief Used to order a sequence of simulation-details.
      class SimulationDetailsComparison {
      public :
         /// \brief Determine order of SimulationDetails.
         ///
         /// The simulation sequence number is used to determine the order.
         /// No two numbers should be the same.
         bool operator ()( const SimulationDetails* left, const SimulationDetails* right ) const;
      };

   }

}

#endif // INTERFACE__SIMULATION_DETAILS__H
