#ifndef _PROPInterface_H_
#define _PROPInterface_H_

//------------------------------------------------------------//

#include <petscksp.h>

//------------------------------------------------------------//

#include "CompoundLithology.h"
#include "LithologyManager.h"
#include "GeoPhysicsFluidType.h"
#include "CauldronGridDescription.h"

#include "globaldefs.h"
#include "petscts.h"
#include "petscdmda.h"
#include "IBSinterpolator.h"
#include "Polyfunction.h"//from CBM Generics
#include "layer.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "timefilter.h"
#include "filterwizard.h"
#include "utils.h"
#include "profile.h"
#include "fileio.h"
#include "System.h"
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <string>
#include <map>
#include "array.h"

#include "snapshotdata.h"

#include "database.h"

#include "layer_iterators.h"

#include "petscvector_readwrite.h"

#include "milestones.h"

// Data access library.
#include "Interface/GridMap.h"
#include "Interface/RelatedProject.h"

using Interface::MANTLE_HEAT_FLOW;
using Interface::FIXED_BASEMENT_TEMPERATURE;


using namespace std;
using namespace GeoPhysics;

typedef list<double> FloatStack;
typedef vector<int>  LayerStack;
typedef map< string, int, less<string> > String_Container;

using CBMGenerics::Polyfunction;

class Elt2dIndices
{
public:

  bool exists;
  int i[4];
  int j[4];

  bool nodeDefined[4];

  /// Indicate whether the edge is on the boundary of the domain.
  ///
  /// boundaryNumber must be in the range 0 .. 3.
   /// DOES NOT WORK YET! DO NOT USE!
  bool onDomainBoundary ( const int boundaryNumber ) const;

//   /// Indicate whether the edge is on the boundary of the domain
//   bool onDomainBoundary [ 4 ];
};

typedef std::vector < Elt2dIndices > ElementList;

class Related_Project {
 public:
  string   Name;
  double   X_Coord;
  double   Y_Coord;
  int      X_Position;
  int      Y_Position;
  bool     In_Range;
  bool     Exists;
};

typedef Related_Project* Related_Project_Ptr;
typedef vector<Related_Project_Ptr> Related_Project_List;
typedef std::map <const LayerProps*, double> LayerPreferredTimeStepSizeMap;

class FissionTrackCalculator;

class AppCtx {

public:

   AppCtx (int argc, char **argv);

   ~AppCtx ();



   bool readProjectName ();
   bool readProjectFile ();

   void CheckForStartInDebugger (int *argc, char ***args);

   bool nodeIsDefined ( const int i, const int j ) const;

   const Boolean2DArray& getValidNeedles () const;

   void addUndefinedAreas ( const Interface::GridMap* theMap );

   void setValidNodeArray ();

   void printValidNeedles () const;


   void Retrieve_Lithology_ID ();
   void deleteLithologyIDs ();

   bool In_Processor_Range ( const int globalIIndex, const int globalJIndex );

   bool inProcessorRange ( const int  globalIIndex,
			   const int  globalJIndex, 
			   const bool includeLowerIGhostNodes,
			   const bool includeUpperIGhostNodes, 
			   const bool includeLowerJGhostNodes,
			   const bool includeUpperJGhostNodes ) const;

   void Generate_Lithology_Identifier_List (const int Max_Nb_Lithology_Single_Proc, int &Total_Number_Of_Lithologies);
   bool calcNodeDepths (const double time);
   bool setNodeDepths (const double time);
   bool calcNodeVes (const double time);
   bool calcNodeMaxVes (const double time);
   bool calcPorosities (const double time);
   bool createFormationLithologies ( const bool canRunSaltModelling );
   bool calcBasementProperties(const double time);

   void interpolateProperty ( const double       currentTime,
                              const PropertyList property );

   void Examine_Load_Balancing ();
   void Output_Number_Of_Geological_Events();

   bool findActiveElements (const double time);

   LayerProps *findLayer (const string & LayerName) const;

   const LayerProps* findDepositingLayer ( const double time ) const;

   int getLayerIndex ( const string& name ) const;

   int Find_Layer_Index ( const double Deposition_Age,
                          const double tolerance = DefaultAgeTolerance ) const;

   LayerProps* Find_Layer_From_Surface(const string& SurfaceName);

   int findSurfacePosition ( const string& surfaceName ) const;

   int findFormationPosition ( const string& formationName ) const;

   const GeoPhysics::FluidType* findFluid (const string & FluidName);

   void Set_Crust_Layer ( CrustFormation* Pointer );

   void Set_Mantle_Layer ( MantleFormation* Pointer );

   CrustFormation*  Crust () const;

   MantleFormation* Mantle () const;

   //----------------------------//

   void setFastCauldronProjectFileName (string projectName);

   bool setProjectFileName (const string & projectFileName);

   bool setOutputDirExtension (const string & extension);

   string getOutputDirectory ();

   const string& getFastCauldronProjectFileName () const
   {
      return m_FastCauldronProjectFileName;
   }

   const string& getProjectFileName () const
   {
      return m_ProjectFileName;
   }

   string Get_Project_Filename_Base ();


   void setModellingMode ( const Interface::ModellingMode modellingMode );

   bool isModellingMode1D () const;

   bool isModellingMode3D () const;

   void Display_Grid_Description();

   void Locate_Related_Project ();


   void print ();
   void printFCT ();
   void printGrids ();
   bool getCommandLineOptions ();

   void setAdditionalCommandLineParameters ();

   void setParametersFromEnvironment ();

   int getUnitTestNumber () const;

   void selectLayers ( const PropertyList currentProperty,
                       const OutputOption range,
                             LayerStack&  selectedLayers );

   void selectSourceRockLayers ( LayerStack& selectedLayers );

   void setLayerBottSurfaceName ();
   void setUp2dEltMapping ();

   bool Get_FCT_Correction ();

   void Print_Nodes_Value_From_Polyfunction ();

   double Temperature_Gradient;

   bool Calculate_Pressure( const double time );

   /// \brief Determine whther or not the geometric should be used in calculation.
   bool isGeometricLoop () const;

   bool computeComponentFlow () const;

   double minimumTimeStep () const;

   double maximumTimeStep () const;

   double fixedTimeStep () const;

   bool cflTimeStepping () const;

   bool useBurialRateTimeStepping () const;


   void   setPermafrost( const double timeStep, const double age );
   void   setPermafrostTimeStep( const double timeStep );
   double permafrostAge () const;
   double permafrostTimeStep () const;
   bool   permafrost() const;

   /// Initialises other minor layer data fields.
   void setLayerData ();

   double Estimate_Temperature_At_Depth( const double Node_Depth, 
					 const double Surface_Temperature,
					 const double Surface_Sea_Bottom_Depth );

   /// Each layer contains a 3D Boolean array that indicates if a node 
   /// is to be included in the mesh or not due to its lithology. The
   /// values stored in the array's are set here.
   void SetIncludedNodeArrays ();


   /// Set the time step used after a snapshot when performing a calculation.
   /// It depends on the maximum burial rate of all of the layers.

   /// Set the size of the time-step that buries a fraction (defined by m_elementFraction)
   /// of an element. The values are saved in a mapping: layer->time-step-size.
   void setInitialTimeStep ();

   /// Return the time-step size for the layer that is currently being deposited.
   /// if none is found then the "initial time-step size" will be returned.
   double getInitialTimeStep ( const double currentTime ) const;

   /// Return the time step computed above.
   // no longer used.
   double initialPressureTimeStep () const;

   bool openProject ();

   bool setRelatedProject ( const Interface::RelatedProject* relatedProject );

   /// \brief Add pseudo-well location at position i,j
   void setRelatedProject ( const int indexI, const int indexJ );

   /// \brief Add pseudo-well location at position x,y
   void setRelatedProject ( const double locationX, const double locationY );

   /// \brief Get the ves scaling factor.
   ///
   /// This factor is used to scale the ves when initialising the solid-thickness (fct).
   double getVesScaling () const;

   int getCrustThinningModel () const;


  void deleteTimeIORecord ( const string& propertyName,
			    const double  age );

  void deleteTimeIORecord ( const double  age );

  void deleteTimeIORecord ( const string& propertyName,
			    const double  age,
			    const string& surfaceName,
			    const string& formationName );

  void initialiseTimeIOTable ( const string& currentOperation );

   database::Record * getRunStatusRecord (void);

   typedef std::vector<double> DoubleVector;

   
   enum ContourType {ISOVRE, ISOTEMPERATURE, NUMBEROFCONTOURTYPES};
	
   IsoLineTable m_theTables[NUMBEROFCONTOURTYPES];
   DoubleVector m_theMilestones[NUMBEROFCONTOURTYPES];
	
   const IsoLineTable & getContourValueTable(enum ContourType theType) const;
   const DoubleVector & getContourMilestones(enum ContourType theType) const;
   void addIsolinePoint(ContourType theType, IsolinePoint *thePoint);
   void setMilestones(void);
   void writeIsoLinesToDatabase(void);
   void deleteIsoValues(void);
   void addBottomSedimentSurfaceIsoPointsToIsolines(void);

   Polyfunction m_bottomSedimentSurfaceAgeTemperatureCurve;
   Polyfunction m_bottomSedimentSurfaceAgeDepthCurve;
   Polyfunction m_bottomSedimentSurfaceAgeVreCurve;

   //FT stuff
   void writeFissionTrackResultsToDatabase(const FissionTrackCalculator &theFTCalculator);

   inline bool setRunStatus (const string & property, const string & value);

   database::Database * database;
   database::Table * timeIoTbl;
   database::Table * runStatusIoTbl;

   LayerList layers;

   TimeFilter timefilter;
   Filterwizard filterwizard;

   Related_Project_List Related_Projects;

   DM Reference_DA_For_Io_Maps;
   DM *mapDA;

   Vec Defined_FCT_Correction;

   ElementList mapElementList;

   //~~~~~~~~~~~~ Profiling ~~~~~~~~~~~~~~~~~~~~
   int Vec_Allocate;

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   //~~~~~~~~~~~~ RunOptions ~~~~~~~~~~~~~~~~~~~

   double optimalpressdiff;
   double optimaltempdiff;
   double optimalsrtempdiff;
   double timestepsize;
   double timestepincr;
   double timestepdecr;
   double BricksizeEast;
   double BricksizeNorth;
   int MaxNumberOfRunOverpressure;
   int Optimisation_Level;

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   //
   //
   // Fine control over output
   //
   int Output_Level;
   PetscBool debug1, debug2, debug3;

   /// \brief output information iteration behaviour of algorithms that are interesting for performance analyis;
   PetscBool verbose;


   bool       Use_Geometric_Loop;
   bool       includeAdvectiveTerm;
   bool       Do_Chemical_Compaction;

   CalculationMode getCalculationMode () const;

   /// \brief Indicate whether or not the integration of the genex equations is required.
   bool integrateGenexEquations () const;

   /// \brief Indicate whether or not map results for Darcy should be output.
   bool outputDarcyMaps () const;

   /// \brief Indicate whether or not the project file and results should be saved even if there is an error in the Darcy simulator.
   bool saveOnDarcyError () const;


   PetscBool Do_Iteratively_Coupled;
   PetscBool Nonlinear_Temperature;
   PetscBool DoOverPressure;
   PetscBool DoDecompaction;
   PetscBool DoHighResDecompaction;
   PetscBool DoTemperature;
   PetscBool DoGempis;
   PetscBool DoHDFOutput;
   PetscBool doGenex;
   PetscBool m_doOutputAtAge;
   double m_ageToOutput;
	 
   PetscBool allowPressureJacobianReuse;
   int        pressureJacobianReuseCount;
   PetscBool allowIluFillIncrease;
   PetscBool allowSolverChange;

   PetscBool IsCalculationCoupled;
   PetscBool readFCTCorrectionFactor;
   OutputFormat fileOutput;


   // return true if bottomBoundaryCondition == ADVANCED_LITHOSPHERE_CALCULATOR
   bool isALC() const;
   PetscBool bottomBasaltTemp;

   // ModelArea Grid_Window;
   bool useTemisRelPerm () const;

    void Fill_Topmost_Segment_Arrays ( const double Previous_Time, 
                                       const double Current_Time );



   //------------------------------------------------------------//

   //
   //
   // Pressure and Temperature linear solver information
   //
   void setPressureLinearSolver    ( KSP&         pressureLinearSolver,
                                     const double solverTolerance ) const;

   void setTemperatureLinearSolver ( KSP&         temperatureLinearSolver,
                                     const double solverTolerance,
				     const bool   requiredForSteadyStateCalculation ) const;


   int  Temperature_GMRes_Restart;

   KSPType   Temperature_Linear_Solver_Type;

  //------------------------------------------------------------//

  SnapshotData projectSnapshots;

  void setSnapshotInterval ( const SnapshotInterval& interval );


  double Age_Of_Basin ( ) const {
    return (*projectSnapshots.majorSnapshotsBegin ())->time ();
   }

  ///
  /// Delete all of the minor snapshot times, for the properties in the list, from the time io table.
  ///
  void deleteMinorSnapshotsFromTimeIOTable ( const snapshottimeContainer& savedMinorSnapshotTimes,
                                             const PropListVec&           properties );


  //------------------------------------------------------------//

   bool Create_Reference_DA_for_Io_Maps ();

  // bool updateRunStatusIOTable ( const string& lastRunStatus );

 private:

   AppCtx ();

   void setMantleDepth ( const double presentDayMantleThickness,
                         const int    i,
                         const int    j,
                         const double time,
                         const int    numberOfNodes,
                               PETSC_3D_Array& depth ) const;

   double Minimum_Permeability ( const LayerProps* Current_Layer ) const;


   int Layer_Position ( const string& Formation_Name ) const;

   void clearOperationFromTimeIOTable ( database::Table* table,
                                        const string&    mapFileName );


   void printHelp () const;

   void shareValidNeedleData ();

   void loadLateralStressFile ( const std::string& lateralStressFileName );

   void computeLowPermeabilitySubdomains ( const double lowPermeability );


   MantleFormation* Mantle_Layer;
   CrustFormation* Crust_Layer;

   CalculationMode currentCalculationMode;

   string m_FastCauldronProjectFileName;
   string m_ProjectFileName;
   string m_dirExtension;
   string m_OutputDirectory;

   double m_presentDayMantleThickness;
   double m_maximumBasementThickness;
   double m_maximumCrustThinningRatio;
   bool   m_minCrustThicknessIsZero;

   int m_crustThinningModel;
   int m_numberOfSuperiorMantleElements;
   int m_numberOfInferiorMantleElements;
   double m_inferiorMantleElementHeightScaling;
   bool m_computeComponentFlow;

   /// The initial time step that is dependant on the maximum burial rate in the model.
   double m_computedInitialPressureTimeStep;

   bool m_saveOnDarcyError;

   //------------------------------------------------------------//

   /// These arrays contain the partitioning information of the coarse grid DA's
   /// if we are doing a coupled high res decompaction calculation.
   int* xCoarseGridPartitioning;
   int* yCoarseGridPartitioning;


   Boolean2DArray m_nodeIsDefined;
   LayerPreferredTimeStepSizeMap m_layerPreferredTimeStepSize;

   double m_minimumTimeStep;
   double m_maximumTimeStep;
   double m_fixedTimeStep;
   bool m_cflTimeStepping;
   bool m_burialRateTimeStepping;
   double m_elementFraction;
   double m_elementErosionFraction;
   double m_elementHiatusFraction;
   double m_vesScaling;

   double m_permafrostTimeStep;
   double m_permafrostAge;
   bool   m_permafrost;

   PetscBool m_useTemisRelPerm;

   int m_unitTestNumber;
};

/*  template <class Type> */
bool AppCtx::setRunStatus (const string & property, const string & value)
{
   database::Record * record = getRunStatusRecord ();
   record->setValue (property, value);
   return true;
}

inline CalculationMode AppCtx::getCalculationMode () const {
   return currentCalculationMode;
}

inline bool AppCtx::nodeIsDefined ( const int i, const int j ) const {
   return m_nodeIsDefined ( i, j );
}

inline double AppCtx::getVesScaling () const {
   return m_vesScaling;
}

inline int AppCtx::getCrustThinningModel () const {
   return m_crustThinningModel;
}

inline CrustFormation* AppCtx::Crust () const { 
   return Crust_Layer;
}

inline MantleFormation* AppCtx::Mantle () const { 
   return Mantle_Layer;
}

inline int AppCtx::getUnitTestNumber () const {
   return m_unitTestNumber;
}


inline bool AppCtx::isGeometricLoop () const {
   return Use_Geometric_Loop;
}

inline bool AppCtx::useTemisRelPerm () const {
   return bool ( m_useTemisRelPerm );
}

inline bool AppCtx::integrateGenexEquations () const {
   return doGenex;
}

inline bool AppCtx::saveOnDarcyError () const {
   return m_saveOnDarcyError;
}

inline bool AppCtx::isALC () const {
   return ( FastcauldronSimulator::getInstance ().getBottomBoundaryConditions() == Interface::ADVANCED_LITHOSPHERE_CALCULATOR );
}

#endif /* _PROPInterface_H_ */
