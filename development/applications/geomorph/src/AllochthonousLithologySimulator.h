#ifndef __AllochthonousLithologySimulator_HH__
#define __AllochthonousLithologySimulator_HH__

#include "ProjectFileHandler.h"

// Data access
#include "Interface.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "GeoMorphAllochthonousLithology.h"

#include "GeoMorphAllochthonousLithologyDistribution.h"

#include "hdf5.h"

using namespace DataAccess;
using namespace DataAccess::Interface;


namespace AllochMod {

  static const double IBSNULLVALUE = -9999.0;

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file AllochthonousLithologySimulator.h
  /// \brief Allochthonous modelling capability.

  /// Inherits from the DataAccess::ProjectHandle class adding allochthonous lithology modelling capability.
  class AllochthonousLithologySimulator : public Interface::ProjectHandle {

    /// \brief Used in sorting the distribution maps by age.
    class DistributionMapEarlierThan {

    public :

      /// \brief Returns whether the age if m1 is eariler than that of m2 or not.
      bool operator ()( const GeoMorphAllochthonousLithologyDistribution* m1,
                        const GeoMorphAllochthonousLithologyDistribution* m2 ) const;

    };

  public :

     /// \var GeomorphRunStatus
     /// \brief The string found in the run-status-io-table, indicating that geomorph ahs been run.
     static const std::string GeomorphRunStatus;

    /// \var ResultsFileName
    /// \brief The name of the HDF file in which the interpolation results will be stored.
    ///
    /// The file name here also includes the HDF extension.
    static const std::string ResultsFileName;


    /// Constructor
    AllochthonousLithologySimulator (database::ProjectFileHandlerPtr database, const std::string & name, const std::string & accessMode, const ObjectFactory* factory);

    ~AllochthonousLithologySimulator (void);

    /// \brief Open the project file and return a pointed to the allochthonous lithology modelling simulator.
    static AllochthonousLithologySimulator* CreateFrom ( const std::string& projectFileName, ObjectFactory* factory );


    /// \brief Calculate the 3-d implicit surface for each of the layers.
    bool execute ( const int debugLevel );

    /// \brief Output the interior, exterior and surface maps for each of the distribution maps.
    void printMaps ( std::ostream& output );

    /// \brief Save the interpolation results bask to the project file, specified by the parameter.
    bool saveToFile ( const std::string& fileName );

    /// \brief Print some information on the stream.
    void printOn (std::ostream & ostr) const;

    /// Determine if the global switch, in the run options io table, is on or off.
    bool allochthonousModellingRequired () const;


  protected :

    ///
    void getAllochthonousLithologyDistributionList ( const Interface::AllochthonousLithology*     theAllochthonousLithology,
                                                           AllochthonousLithologyDistributionSequence& associatedLithologyDistributions );
  };

  /** @} */

}

#endif // __AllochthonousLithologySimulator_HH__
