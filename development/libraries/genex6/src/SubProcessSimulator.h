#ifndef _GENEX6__SUB_PROCESS_SIMULATOR__H_
#define _GENEX6__SUB_PROCESS_SIMULATOR__H_

#ifdef USE_PRIORITY_QUEUE
#include <priority_queue>
#endif
#include <vector>

#include "Input.h"
#include "SimulatorStateBase.h"

namespace Genex6 {

   /// \brief Base class for all sub-processes.
   class SubProcessSimulator {

   public :

      /// \brief Destructor.
      virtual ~SubProcessSimulator () {}

      /// \brief Compute the sub-process.
      virtual void compute ( const Input&        input,
                             SimulatorStateBase* state ) = 0;

      /// \brief Priority of sub-process.
      ///
      /// The higher the priority then the earlier the sub-process will be computed.
      virtual unsigned int getPriority () const = 0;

      /// \brief Determine if deallocation of this object is responsibility of the sub-process list.
      ///
      /// If result is true then it will be the responsibility of the allocator to deallocate 
      /// this object. Otherwise (result is false) the simulator will delete the sub-process-simulator.
      virtual bool managed () const = 0;

   private :

   };


   /// \brief Functor comparator class for comparing priorities of two sub-process simulators.
   class SubProcessPriorityComparator {

   public :

      /// \brief Compare the priority of two sub-process simulators
      bool operator ()( const SubProcessSimulator* sp1,
                        const SubProcessSimulator* sp2 ) const;

   };

   /// \typdedef SubProcessSimulatorList
   /// \brief Container for sub-process simulators.
#ifdef USE_PRIORITY_QUEUE
   typedef std::priority_queue<SubProcessSimulator*, std::vector<SubProcessSimulator*>, SubProcessPriorityComparator> SubProcessSimulatorList;
#else
   typedef std::vector<SubProcessSimulator*> SubProcessSimulatorList;
#endif

} // 

inline bool Genex6::SubProcessPriorityComparator::operator ()( const SubProcessSimulator* sp1,
                                                               const SubProcessSimulator* sp2 ) const {
   return sp1->getPriority () < sp2->getPriority ();
}

#endif // _GENEX6__SUB_PROCESS_SIMULATOR__H_
