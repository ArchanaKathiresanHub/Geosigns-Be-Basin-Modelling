#ifndef __CRUSTAL_THICKNESS_CALCULATOR__
#define __CRUSTAL_THICKNESS_CALCULATOR__

#include "ProjectFileHandler.h"

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "InterfaceInput.h"
#include "InterfaceOutput.h"

#include "GeoPhysicsProjectHandle.h"

using namespace DataAccess;
using namespace std;

const string CrustalThicknessCalculatorActivityName = "CrustalThicknessCalculator";

const int XYZ = 0x0001;
const int HDF = 0x0002;
const int SUR = 0x0004;

void displayTime ( const double timeToDisplay, const char * msgToDisplay );

class CrustalThicknessCalculator : public GeoPhysics::ProjectHandle {

public :
   // Constructor / Destructor
   CrustalThicknessCalculator( database::ProjectFileHandlerPtr database, const std::string & name, const std::string & accessMode, ObjectFactory* factory );

   ~CrustalThicknessCalculator();

   // Return the reference to the project data.
   static CrustalThicknessCalculator& getInstance ();

   // Open the project file
   static CrustalThicknessCalculator* CreateFrom ( const string& inputFileName, ObjectFactory* factory );

   // Finish any activity and deallocate the singleton object.
   static void finalise ( const bool saveResults );

   // Parse command line
   bool parseCommandLine();

   // Run calculator
   void run();

   /// Delete property values associated with crustal thickness calculator
   void deleteCTCPropertyValues();


   /// Merge output files if nessecary
   ///
   /// \b Must be called at end of calculation.
   bool mergeOutputFiles ();
private :

   static CrustalThicknessCalculator* m_crustalThicknessCalculator;

   static string m_projectFileName;
   static string m_outputFileName;
   int    m_outputOptions;
   bool   m_debug;
   bool   m_applySmoothing; // smooth the WLS map

   // Set requested output properties from the Project file
   void setRequestedOutputProperties( InterfaceOutput & theOutput);

   bool movingAverageSmoothing( GridMap * aMap );
};

inline CrustalThicknessCalculator& CrustalThicknessCalculator::getInstance () {
   return *m_crustalThicknessCalculator;
}

#endif
