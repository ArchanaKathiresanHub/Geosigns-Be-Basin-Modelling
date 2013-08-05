#ifndef __CRUSTAL_THICKNESS_CALCULATOR__
#define __CRUSTAL_THICKNESS_CALCULATOR__

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "InterfaceInput.h"
#include "InterfaceOutput.h"
#include "DensityCalculator.h"

#include "Local2DArray.h"

using namespace DataAccess;
using namespace std;

class CrustalThicknessCalculator : public Interface::ProjectHandle {

   typedef GeoPhysics::Local2DArray <bool>  BooleanLocal2DArray;

public :
   // Constructor / Destructor
   CrustalThicknessCalculator( database::Database * database, const std::string & name, const std::string & accessMode );

   ~CrustalThicknessCalculator();

   // Return the reference to the project data.
   static CrustalThicknessCalculator& getInstance ();

   // Open the project file
   static CrustalThicknessCalculator* CreateFrom ( const string& inputFileName );

   // Finish any activity and deallocate the singleton object.
   static void finalise ( const bool saveResults );

   // Parse command line
   bool parseCommandLine();

   // Run calculator
   void run();

   /// Delete property values associated with crustal thickness calculator
   void deleteCTCPropertyValues();

private :

   static CrustalThicknessCalculator* m_crustalThicknessCalculator;

   static string m_projectFileName;
   static string m_outputFileName;
   int    m_outputOptions; 
   bool   m_debug;
   bool   m_applySmoothing; // smooth the WLS map
   int    m_smoothRadius;

   /// I.e. whether it is included in the calculation.
   BooleanLocal2DArray m_validNodes;
   BooleanLocal2DArray m_currentValidNodes;

   // Set requested output properties from the Project file
   void setRequestedOutputProperties( InterfaceOutput & theOutput);

   bool movingAverageSmoothing( GridMap * aMap );

   void setAdditionalOptionsFromCommandLine();

  /// Return whether or not the node is defined.
   bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;
   
   void addUndefinedAreas ( BooleanLocal2DArray & validNodes, const Interface::GridMap* theMap );

   void initialiseValidNodes ( const InterfaceInput &theInterfaceData );
   void initialiseCurrentValidNodes ( const DensityCalculator &theInterfaceData );
};

inline CrustalThicknessCalculator& CrustalThicknessCalculator::getInstance () {
   return *m_crustalThicknessCalculator;
}

inline bool CrustalThicknessCalculator::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return ( m_validNodes ( i, j ) && m_currentValidNodes ( i, j ));
}

#endif
