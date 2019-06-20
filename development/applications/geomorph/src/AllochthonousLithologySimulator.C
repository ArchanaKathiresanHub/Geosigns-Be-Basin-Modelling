#include "AllochthonousLithologySimulator.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "GeoMorphAllochthonousLithology.h"
#include "GeoMorphAllochthonousLithologyDistribution.h"

#include "Formation.h"
#include "Grid.h"
#include "Formation.h"
#include "Interface.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "AllochthonousLithology.h"
#include "hdf5funcs.h"

#include "WallTime.h"

using namespace DataAccess;
using namespace DataAccess::Interface;

const std::string AllochMod::AllochthonousLithologySimulator::GeomorphRunStatus = "AllochthonousModelling";

const std::string AllochMod::AllochthonousLithologySimulator::ResultsFileName = "InterpolationResults.HDF";

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologySimulator::DistributionMapEarlierThan::operator ()( const GeoMorphAllochthonousLithologyDistribution* m1,
                                                                                          const GeoMorphAllochthonousLithologyDistribution* m2 ) const {
  return m1->getAge () > m2->getAge ();
}

//------------------------------------------------------------//

AllochMod::AllochthonousLithologySimulator::AllochthonousLithologySimulator (database::ProjectFileHandlerPtr database, const std::string & name, const std::string & accessMode, const ObjectFactory* factory)
      : Interface::ProjectHandle (database, name, accessMode, factory)
{
  if ( allochthonousModellingRequired ()) {
     startActivity ( GeomorphRunStatus, getHighResolutionOutputGrid ());
  }

}

//------------------------------------------------------------//

AllochMod::AllochthonousLithologySimulator::~AllochthonousLithologySimulator (void){
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologySimulator::saveToFile ( const std::string& fileName ) {

   if ( allochthonousModellingRequired ()) {
      setSimulationDetails ( "geomorph", "Default", "" );
      finishActivity ();
   }

   getProjectFileHandler ()->saveToFile ( fileName );
   return true;
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologySimulator::printOn (std::ostream & ostr) const {

  ostr << " AllochthonousLithologySimulator " << std::endl;

}

//------------------------------------------------------------//

AllochMod::AllochthonousLithologySimulator* AllochMod::AllochthonousLithologySimulator::CreateFrom ( const std::string & projectFileName, DataAccess::Interface::ObjectFactory* factory ) {

   return dynamic_cast<AllochthonousLithologySimulator*>(Interface::OpenCauldronProject ( projectFileName, "rw", factory ));
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologySimulator::allochthonousModellingRequired () const {

  database::Table* runOptionsTable = getTable ( "RunOptionsIoTbl" );
  Interface::MutableAllochthonousLithologyList::const_iterator allochthonousLithologyIter;

  bool globalAllochthonousSwitchIsOn = database::getAllochthonousModelling ( runOptionsTable, 0 ) == 1;
  bool anyLayerAllochthonousSwitchIsOn = false;


  for ( allochthonousLithologyIter = m_allochthonousLithologies.begin();
        allochthonousLithologyIter != m_allochthonousLithologies.end();
        ++allochthonousLithologyIter) {

     if ((*allochthonousLithologyIter)->getFormation ()->hasAllochthonousLithology ()) {
        anyLayerAllochthonousSwitchIsOn = true;
        // Found one for which the 'has allothchonous lithology' switch is on so no need to check for others.
        break;
     }

  }

  return globalAllochthonousSwitchIsOn and anyLayerAllochthonousSwitchIsOn;
}

//------------------------------------------------------------//

bool AllochMod::AllochthonousLithologySimulator::execute ( const int debugLevel ) {

  if ( ! allochthonousModellingRequired ()) {
    return true;
  }

  WallTime::Time startTime;
  WallTime::Time endTime;
  WallTime::Duration executionTime;
  int    hours;
  int    minutes;
  double seconds;

  Interface::MutableAllochthonousLithologyList::iterator allochthonousLithologyIter;
  AllochthonousLithologyDistributionSequence allochthonousLithologyDistributionSequence;

  database::Table* allochthonousInterpTbl = getTable ( "AllochthonLithoInterpIoTbl" );

  // Clear the interpolation results table, since they need to be recomputed.
  allochthonousInterpTbl->clear ();

  std::string fullResultFileName = getFullOutputDir () + '/' + ResultsFileName;

  // Create directory, if it does not already exist.
  makeOutputDir();

  cout << endl;

  hid_t fileId = H5Fcreate ( fullResultFileName.c_str (), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );

  for ( allochthonousLithologyIter = m_allochthonousLithologies.begin();
        allochthonousLithologyIter != m_allochthonousLithologies.end();
        ++allochthonousLithologyIter) {

     if ((*allochthonousLithologyIter)->getFormation ()->hasAllochthonousLithology ()) {
        std::cout << " Computation of halokinesis for layer " << (*allochthonousLithologyIter)->getFormationName () << std::endl;

        startTime = WallTime::clock ();

        // Get the distributions (maps, ages, ...) for this formation.
        getAllochthonousLithologyDistributionList ( *allochthonousLithologyIter,
                                                    allochthonousLithologyDistributionSequence );

        ((GeoMorphAllochthonousLithology*)(*allochthonousLithologyIter))->computeInterpolant ( allochthonousLithologyDistributionSequence, debugLevel );
        ((GeoMorphAllochthonousLithology*)(*allochthonousLithologyIter))->saveInterpolant ( fileId, ResultsFileName );

        endTime = WallTime::clock ();
        executionTime = endTime - startTime;

        executionTime.separate ( hours, minutes, seconds );

        std::cout << " Total computation time: "
                  <<  hours << " hours "
                  << minutes << " minutes "
                  << seconds << " seconds "
                  << std::endl
                  << std::endl;
     }

  }

  H5Fclose (fileId);


  return true;
}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologySimulator::printMaps ( std::ostream& output ) {

  Interface::MutableAllochthonousLithologyList::iterator allochthonousLithologyIter;
  AllochthonousLithologyDistributionSequence allochthonousLithologyDistributionSequence;

  for ( allochthonousLithologyIter = m_allochthonousLithologies.begin();
        allochthonousLithologyIter != m_allochthonousLithologies.end();
        ++allochthonousLithologyIter) {

     if ((*allochthonousLithologyIter)->getFormation ()->hasAllochthonousLithology ()) {
        // Get the distributions (maps, ages, ...) for this formation.
        getAllochthonousLithologyDistributionList ( *allochthonousLithologyIter,
                                                    allochthonousLithologyDistributionSequence );

        ((GeoMorphAllochthonousLithology*)(*allochthonousLithologyIter))->printDistributionMaps ( allochthonousLithologyDistributionSequence, output );
     }

  }



}

//------------------------------------------------------------//

void AllochMod::AllochthonousLithologySimulator::getAllochthonousLithologyDistributionList ( const Interface::AllochthonousLithology*     theAllochthonousLithology,
                                                                                                   AllochthonousLithologyDistributionSequence& associatedLithologyDistributions ) {


  Interface::MutableAllochthonousLithologyDistributionList::iterator distributionIter;

  associatedLithologyDistributions.clear ();

  // If the allochthonous lithology-record is null or the formation from which it comes
  // is not taking part in the allochthonous-modelling then return with the empty list.
  if ( theAllochthonousLithology == 0 ||
       ! theAllochthonousLithology->getFormation ()->hasAllochthonousLithology ()) {
    return;
  }

  for ( distributionIter = m_allochthonousLithologyDistributions.begin (); distributionIter != m_allochthonousLithologyDistributions.end (); ++distributionIter ) {

    GeoMorphAllochthonousLithologyDistribution* lithologyDistribution = (GeoMorphAllochthonousLithologyDistribution*)(*distributionIter);

    if ( theAllochthonousLithology->getFormationName () == lithologyDistribution->getFormationName ()) {
      lithologyDistribution->loadGridMap ();
      associatedLithologyDistributions.push_back ( lithologyDistribution );
    }

  }

  std::sort ( associatedLithologyDistributions.begin (), associatedLithologyDistributions.end (), DistributionMapEarlierThan ());
}


//------------------------------------------------------------//
