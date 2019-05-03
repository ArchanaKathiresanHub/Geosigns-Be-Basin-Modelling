#ifndef __AllochthonousModellingFactory_HH_
#define __AllochthonousModellingFactory_HH_

#include "ProjectFileHandler.h"

#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ObjectFactory;
      class ProjectHandle;
      class SourceRock;
      class AllochthonousLithology;
      class AllochthonousLithologyDistribution;
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

  /// \file AllochthonousModellingFactory.h
  /// \brief An object factory creating objects necessary for allochthonous modelling.

  /// \brief An object factory creating objects necessary for allochthonous modelling.
  class AllochthonousModellingFactory : public DataAccess::Interface::ObjectFactory {

  public :

    /// Returns the project-handle.
    DataAccess::Interface::ProjectHandle * produceProjectHandle ( database::ProjectFileHandlerPtr database,
                                                                  const string & name,
                                                                  const string & accessMode) const;

    /// Returns an allochtohonous lithology
    DataAccess::Interface::AllochthonousLithology * produceAllochthonousLithology ( DataAccess::Interface::ProjectHandle * projectHandle,
                                                                                    database::Record * record) const;

    /// Returns an allochtohonous lithology distribution.
    DataAccess::Interface::AllochthonousLithologyDistribution * produceAllochthonousLithologyDistribution ( DataAccess::Interface::ProjectHandle * projectHandle,
                                                                                                            database::Record * record) const;

  };

  /** @} */


}


#endif // __AllochthonousModellingFactory_HH_
