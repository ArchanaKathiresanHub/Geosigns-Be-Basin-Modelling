#ifndef __PThreadsAllochthonousModellingFactory_HH_
#define __PThreadsAllochthonousModellingFactory_HH_

#include "Interface/ObjectFactory.h"
#include "AllochthonousModellingFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ObjectFactory;
      class ProjectHandle;  
      class SourceRock;
      class AllochthonousLithology;
   }
}

namespace database
{
   class Record;
   class Database;
}


namespace AllochMod {


  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file PThreadsAllochthonousModellingFactory.h
  /// \brief 

  class PThreadsAllochthonousModellingFactory : public AllochthonousModellingFactory {

  public :

    PThreadsAllochthonousModellingFactory ( const int initialNumberOfThreads );

    /// \brief Returns a PThreadsAllochthonousLithology.
    DataAccess::Interface::AllochthonousLithology * produceAllochthonousLithology ( DataAccess::Interface::ProjectHandle * projectHandle,
                                                                                         database::Record * record);

  private :

    /// The number of threads to allocate.
    const int numberOfThreads;

  };

  /** @} */

}


#endif // __PThreadsAllochthonousModellingFactory_HH_
