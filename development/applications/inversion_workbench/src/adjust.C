#include <iostream>
#include <string>
#include <vector>
#include <typeinfo>
#include <iterator>
#include <algorithm>
#include <cmath>

#include "generalexception.h"
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "cauldronschemafuncs.h"


const std::string basementParameters[] 
  = { "TopAsthenoTemp", "Temperature at the top of the asthenospheric mantle "
                        "(bottom of lithospheric mantle) i.e. melting "
                        "temperature."
    , "TopCrustHeatProd", "The surface Radiogenic heat production of the basement"
    , "CrustHeatPDecayConst", "The decay constant of the surface Radiogenic heat "
                              "production of the basement"
    , "LithoMantleThickness", "Lithospheric Mantle thickness"
    , "InitialLthMntThickns", "Initial Litho Mantle thickness"
    , "FixedCrustThickness", "Fixed Crust Thickness"
    } ;


const std::string lithotypes[] 
  = {"*", "All lithogies" 
    ,"Std. Sandstone" ,     "Standard Sandstone"
    ,"SM. Sandstone",       "Soil Mechanics Sandstone"
    ,"Std. Shale",          "Standard Shale"
    ,"SM.Mudst.40%Clay",    "Soil Mechanics Mudstone 40% Clay"
    ,"SM.Mudst.50%Clay",    "Soil Mechanics Mudstone 50% Clay"
    ,"SM.Mudst.60%Clay",    "Soil Mechanics Mudstone 60% Clay"
    ,"Std. Siltstone",      "Standard Siltstone"
    ,"Std.Grainstone",      "Standard Grainstone (Limestone)"
    ,"Std.Dolo.Grainstone", "Standard Domlomitic Grainstone (Dolostone)"
    ,"Std.Lime Mudstone",   "Standard Calcareous Mudstone (Limestone)"
    ,"Std.Dolo.Mudstone",   "Standard Dolomitic Mudstone (Dolostone)"
    ,"Std. Chalk",          "Standard Chalk"
    ,"Std. Marl",           "Standard Marl"
    ,"Std. Anhydrite",      "Standard Anhydrite"
    ,"Std. Salt",           "Standard Salt"
    ,"Sylvite",             "Sylvite"
    ,"Std. Coal",           "Standard Coal"
    ,"Std. Basalt",         "Standard Basalt"
    ,"Crust",               "Standard Crust"
    ,"Litho. Mantle",       "Lithospheric Mantle"
    ,"Astheno. Mantle",     "Asthenospheric Mantle"
    ,"HEAT Sandstone",      "Sandstone as used by IBS-HEAT"
    ,"HEAT Shale",          "Shale as used by IBS-HEAT"
    ,"HEAT Limestone",      "Limestone as used by IBS-HEAT"
    ,"HEAT Dolostone",      "Dolostone as used by IBS-HEAT"
    ,"HEAT Chalk",          "Chalk as used by IBS-HEAT"
    } ;


void showUsage ( const char* message, const char * parameter=0 ) 
{ 
   std::cerr << '\n';

   if ( message ) {
      std::cerr << "ERROR: "  << message ;
      
      if (parameter)
	std::cerr << ' ' << parameter ;
      
      std::cerr << "\n\n";
   }

   std::cerr << "NAME\n"
     	     << "\tadjust - Let's you change input parameters in a Cauldron Project3d file from the command line"
	     << '\n'
             << "SYNOPSIS\n"
             << "\tUsage: adjust [OPTION] ... \n"
	     << '\n'
	     << "OPTIONS\n"
             << "\t--project <cauldron-project-file>   Specify input file. (in which case you do not need to give a name as the last parameter)\n" 
             << "\t--output <cauldron-project-file>    Specify output file (by default is equal to input file).\n" 
	     << "\t--set-basement-property <parameter-name>=<new-value>\n"
             << "\t                                    Set a specified property in the BasementIoTbl. See below for a list of parameter names\n"
	     << "\t--set-crust-thickness <series of ages, series of thickness >\n"
	     << "\t                                    Specify the thicnkess of the crust at a specified moment in history, e.g.:\n"
	     << "\t                                      $ adjust --set-crust-thickness 0 100 200 300, 30000 20000 25000 35000 MyProject.project3d\n"
	     << "\t                                    Note: As a (necessary) side effect, it will also clear the snapshot table\n"
	     << "\t--adjust-thermal-conductivity <lithotype>=<multiplication factor>\n"
	     << "\t                                    Adjust the thermal conductity of one or all lithogies in them model. See below for a list of lithotype names.\n"
	     << "\t--show-erosion-formations           Outputs the names of the formations that are erosions.\n"
	     << "\t--set-erosion <formation-name>=<thickness>[,<t0>,<t1>,<t2>]\n"
	     << "\t                                    Specify the thickness and age of an erosion and coinciding eroded layer. t0 marks the beginning of\n"
	     << "\t                                    the eroded formation, t1 the end of the eroded formation and the beginning of the erosion, and t2\n"
	     << "\t                                    the end of the erosion.\n"
     	     << "\t                                    Note: As a (necessary) side effect, it will also clear the snapshot table\n"
	     << "\t--add-erosion <thickness>,<erosion start>,<erosion length>\n"
	     << "\t--help                              to print this message.\n"
             << std::endl;

   std::cerr << "BASEMENT PARAMETERS\n";
   for (size_t i = 0 ; i < sizeof(basementParameters)/sizeof(basementParameters[0]) / 2; ++i)
   {
     std::cerr << "\t" << i << ". '" << basementParameters[2*i] << "' -> '" << basementParameters[2*i+1] << "'\n";
   }
   std::cerr << "\n";

   std::cerr << "LITHOTYPE NAMES\n";
   for (size_t i = 0 ; i < sizeof(lithotypes)/sizeof(lithotypes[0]) / 2; ++i)
     std::cerr << "\t" << i << ". '" << lithotypes[2*i] << "' -> '" << lithotypes[2*i + 1] << "'\n";
   std::cerr << std::endl;
}

struct AdjustException : BaseException< AdjustException > 
{ AdjustException() { *this << "Error adjusting parameter in project file: "; } };

class Project
{
public:
  Project(const std::string & input, const std::string & output )
    : m_inputFileName(input)
    , m_outputFileName(output)
    , m_projectHandle(DataAccess::Interface::OpenCauldronProject( m_inputFileName, "rw" ))
  {
    if (!m_projectHandle)
      throw AdjustException() << "Could not load project file '" << m_inputFileName << "'";
  }

  ~Project()
  {
    try
    {
      discard();
    }
    catch(...)
    {
      /* ignore all exceptions. FIXME: should be logged */
    }
  }


  void close()
  {
    if (m_projectHandle)
      m_projectHandle->saveToFile(m_outputFileName);

    m_projectHandle = 0;
  }

  void discard()
  {
    m_projectHandle = 0;
  }

  double setBasementProperty(const std::string & parameter, double newValue)
  {
    database::Table * table = m_projectHandle->getTable("BasementIoTbl");
    if (!table)
      throw AdjustException() << "Project file '" << m_inputFileName << "' seems to be incompatible, "
	    << "because the BasementIoTbl tabel could not be found.";

    database::Record * record = table->getRecord(0);

    if (!record)
      throw AdjustException() << "Project file '" << m_inputFileName << "' seems to be incompatible, "
	    << "because the BasementIoTbl does not have any records.";

    if (database::getBottomBoundaryModel(record) != "Fixed Temperature")
      throw AdjustException() << "Currently only 'fixed temperature' bottom boundary models are supported, "
			      << "while this project has a '" << database::getBottomBoundaryModel(record)
			      << "' bottom boundary model.";

    double oldValue = record->getValue<double>( parameter );
    record->setValue(parameter, newValue);
    return oldValue;
  }

  typedef double Time; // in mA (million of years ago)
  typedef double Thickness; // in meters.


  void setCrustThickness( const std::vector< std::pair<Time, Thickness> > & series)
  {
    database::Table * table = m_projectHandle->getCrustIoTable();
    if (!table)
      throw AdjustException() << "Project file '" << m_inputFileName << "' seems to be incompatible, "
	    << "because the CrustIoTbl tabel could not be found.";

    // remove all records from the table;
    table->clear();

    // add new records
    for (size_t i = 0 ; i < series.size(); ++i)
    {
      database::Record * newRecord = table->createRecord();
      assert( newRecord && "Could not allocate new record in Crust thickness table");

      database::setAge( newRecord, series[i].first );
      database::setThickness(newRecord, series[i].second);
      database::setThicknessGrid(newRecord, "");

      using DataAccess::Interface::DefaultUndefinedScalarValue;
      database::setCalibThickness(newRecord, DefaultUndefinedScalarValue );
      database::setOptimThickness(newRecord, DefaultUndefinedScalarValue );
      database::setErrThickness(newRecord, DefaultUndefinedScalarValue );
    }
  }

  void clearSnapshotTable()
  {
    // Clear the Snapshot table.
    // This is necessary, when the time of events change. These influence time stepping
    // which has a large effect on precision.
    database::Table * snapshotTable = m_projectHandle->getTable("SnapshotIoTbl");

    if (!snapshotTable)
      throw AdjustException() << "Project file '" << m_inputFileName << "' seems to be incompatible, "
	    << "because the SnapshotIoTbl tabel could not be found.";

    snapshotTable->clear();
  }

  // Changes the thermal conductivity for a specific lithotype.
  // Lithotypes that are always used:
  //   - "Crust"
  //   - "Litho. Mantle"
  //   - "Astheno. Mantle"
  //   
  // Beside those, you can have many others, e.g.
  //   - "Std. Sandstone"
  //   - "Std. Shale"
  //   - ...
  //
  // The resulting thermal conductivity is the product of correctionFactor with
  // the previous thermal conductivity number
  void adjustThermalConductivity( const std::string & lithotype, double correctionFactor)
  {
    database::Table * table = m_projectHandle->getTable("LithotypeIoTbl");
    if (!table)
      throw AdjustException() << "Project file '" << m_inputFileName << "' seems to be incompatible, "
	    << "because the LithotypeIoTbl tabel could not be found.";

    // look for the lithotype
    for (size_t i = 0 ; i < table->size(); ++i)
    {
      database::Record * record = table->getRecord(i);
      assert( record && "'record' cannot be null, because 'i' can never grow beyond the size of the table.");

      // NewStpThCond = correctionFactor * OldStpThCond
      if ( lithotype == "*" || lithotype == database::getLithotype(record) )
        database::setStpThCond(record, database::getStpThCond(record) * correctionFactor );
    }
  }

#if 0
  std::vector<std::string> getErosionFormations() const
  {
    // get all formations, but without the basement formation
    std::auto_ptr<DataAccess::Interface::FormationList> formations( m_projectHandle->getFormations(0,false) );

    // select the formations which have an erosion (thickness < 0.0)
    std::vector<std::string> result;
    for (size_t i  = 0; i < formations->size(); ++i)
    {
      double minThickness = 0.0, maxThickness = 0.0;
      (*formations)[i]->getInputThicknessMap()->getMinMaxValue(minThickness, maxThickness);

      if (minThickness < 0.0)
	result.push_back( (*formations)[i]->getName());
    }

    return result;
  }
#endif

  struct Formation : public std::string
  {
    double m_minThickness, m_maxThickness;
    double m_minAge, m_maxAge;
    bool m_constant;

    Formation( const DataAccess::Interface::Formation * formation)
      : std::string( formation->getName() )
      , m_minThickness(0.0)
      , m_maxThickness(0.0)
      , m_minAge( formation->getTopSurface()->getSnapshot()->getTime() )
      , m_maxAge( formation->getBottomSurface()->getSnapshot()->getTime() )
      , m_constant(false)
    {
      const DataAccess::Interface::GridMap * thickness = formation->getInputThicknessMap();
      thickness->getMinMaxValue(m_minThickness, m_maxThickness);
      m_constant = thickness->isConstant();
    }
  };

  // Returns which formations erode which other formations
  std::map< Formation, std::vector< Formation > > getErosionFormations() const
  {
    // get all formations, but without the basement formation
    std::auto_ptr<DataAccess::Interface::SurfaceList> surfaces( m_projectHandle->getSurfaces(0,false) );

    // select the formations which have an erosion (thickness < 0.0)
    std::map< Formation, std::vector<Formation> > result;
    for (size_t i  = 0; i < surfaces->size(); ++i)
    {
      const DataAccess::Interface::Surface * topSurface = (*surfaces)[i];

      for (size_t j = i + 1; j < surfaces->size(); ++j)
      { 
	const DataAccess::Interface::Surface * bottomSurface = (*surfaces)[j];

	SurfaceOrder order = compare(topSurface, bottomSurface);

	assert( order != UNDETERMINED );

	if ( order == ABOVE || order == EQUAL )
	{
	  // then the formation under topSurface does not erode this layer,
	  // so we can stop
	  break;
	}

	// otherwise 'topSurface' does erode 'bottomSurface'
	result[ topSurface->getBottomFormation() ].push_back( bottomSurface->getBottomFormation() );
      }
    }

    return result;
  }

  void setErosionThickness(const std::string & formationName, double thickness, double t0, double t1, double t2 )
  {
    assert( thickness >= 0.0 );
    assert( isnan(t0) || isnan(t1) || t0 > t1 );
    assert( isnan(t1) || isnan(t2) || t1 > t2 );
    typedef std::map< Formation, std::vector< Formation > > Formations;
   
    Formations erosionFormations = getErosionFormations();

    // look for eroding formation that erodes exactly whole other formation
    for (Formations::const_iterator i = erosionFormations.begin(); i != erosionFormations.end(); ++i)
    {
      const Formation & erosion = i->first;
      const std::vector< Formation > & eroded=i->second;

      if ( eroded.size() == 1 
	  && erosion.m_constant 
	  && eroded[0].m_minThickness 
	  && erosion.m_minThickness == 0.0 - eroded[0].m_minThickness
	  && ( erosion == formationName || eroded[0] == formationName)
	 )
      {
	database::Table * table = m_projectHandle->getTable("StratIoTbl");
	assert( table );

	for (size_t j = 0; j < table->size(); ++j)
	{
	  database::Record * record = table->getRecord(j);
	  assert( record );

	  if (getLayerName(record) == erosion)
	  {
	    assert( j+2 < table->size()); // because otherwise there wouldn't be an eroded layer beneath it

	    if (j >= 1 && !isnan(t2) && getDepoAge(table->getRecord(j-1)) >= t2 )
	      throw AdjustException() << "Cannot move deposition age of " << erosion << " to a later time (" 
		<< t2 << " Ma), because an other layer is completed at " << getDepoAge(table->getRecord(j-1)) << " Ma.";

	    if (j + 3 < table->size() && !isnan(t0) && getDepoAge(table->getRecord(j+3)) <= t0 )
	      throw AdjustException() << "Cannot move deposition age of " << eroded[0] << " to an earlier time (" 
		<< t0 << " Ma), because an other layer starts at " << getDepoAge(table->getRecord(j+3)) << " Ma.";

	    if (!isnan(t2))
	      setDepoAge(record, t2);

	    setDepth(record, getDepth(table->getRecord(j+2)));

	    // move to the formation below 
	    record = table->getRecord(j+1);
	    assert(getLayerName(record) == eroded[0]);
	    
	    if (!isnan(t1))
	      setDepoAge(record, t1 );

	    setDepth(record, DataAccess::Interface::DefaultUndefinedScalarValue);
	    setThickness(record, thickness);

	    // move to the formation below
	    if (!isnan(t0))
	    {
	      record = table->getRecord(j+2);
	      setDepoAge(record, t0);
	    }

	    return;
	  }
	}

	assert( false ); // The StratIoTbl didn't contain the specified formations

        return;
      }
    }
   
    throw AdjustException() << "Cannot change erosion formation '" << formationName 
      << "', because it doesn't exist or doesn't satisfy the conditions.";
  } 


  void addErosion( double thickness, double t0, double duration )
  {
    assert( thickness > 0.0 );

    // get all formations, but without the basement formation
    std::auto_ptr<DataAccess::Interface::SurfaceList> surfaces( m_projectHandle->getSurfaces(0,false) );

    const double t1 = t0 - duration;

    // Find the place where to insert the erosion event.
    // walk over the surface list, starting with the oldest surface
    typedef DataAccess::Interface::Formation Formation;
    typedef DataAccess::Interface::SurfaceList::const_reverse_iterator SurfIt;

    const Formation * erodedLayer = 0;    
    for (SurfIt surface = surfaces->rbegin(); surface != surfaces->rend(); ++surface)
    {
      if (t0 > (*surface)->getSnapshot()->getTime())
      {
	erodedLayer = (*surface)->getBottomFormation();

	if (t1 <= (*surface)->getSnapshot()->getTime())
	  throw AdjustException() << "The erosion must start and end within one layer";

	break;
      }
    }
    

    if (! erodedLayer->getInputThicknessMap()->isConstant() )
      throw AdjustException() << "The formation '" << erodedLayer->getName() << "' has a non-"
	  << "constant thickness map, which this tool cannot handle";

    // h: the thickness of the eroded layer
    const double h = erodedLayer->getInputThicknessMap()->getConstantValue();

    // the period of the eroded layer.
    const double t_e1 = erodedLayer->getTopSurface()->getSnapshot()->getTime();
    const double t_e0 = erodedLayer->getBottomSurface()->getSnapshot()->getTime();

    // the depth at which we interupt the eroded layer
    const double d0 = h / (t_e0 - t_e1) * (t_e0 - t0);

    // the depth at which we continue depositing the layer
    const double d1 = d0 - thickness;

    // baseDepth: the depth of the surface below the eroded layer
    if ( !erodedLayer->getBottomSurface()->getInputDepthMap()->isConstant() ||
	! erodedLayer->getTopSurface()->getInputDepthMap()->isConstant() )
      throw AdjustException() << "The surfaces bounding the formation '" << erodedLayer->getName() 
	<< "' have a non constant depth-map, which this tool cannot handle";

    const double baseDepth = erodedLayer->getBottomSurface()->getInputDepthMap()->getConstantValue();
    const double topDepth = erodedLayer->getTopSurface()->getInputDepthMap()->getConstantValue();

    if (d1 < 0.0)
      throw AdjustException() << "The eroded formation '" << erodedLayer->getName() << "' is not thick "
	<< "enough at " << t0 << "Ma. It is then only " << d0 << " meters thick, while the erosion is "
	<< thickness << " meters.";

    // Now we know enough to alter the StratIoTbl.

    database::Table * table = m_projectHandle->getTable("StratIoTbl");
    assert( table );

    database::Record * erodedLayerRecord = table->findRecord("LayerName", erodedLayer->getName() );
    
    using DataAccess::Interface::DefaultUndefinedScalarValue;

    // alter the erodedLayer
    setDepoAge(erodedLayerRecord, t0);
    setDepth(erodedLayerRecord, baseDepth - d0);
    setThickness(erodedLayerRecord, DefaultUndefinedScalarValue);

    // construct the erosion layer
    database::Record * erosionRecord = table->createRecord();
    assert(erosionRecord);
    erosionRecord->copyFrom( *erodedLayerRecord );
    setSurfaceName(erosionRecord, erodedLayer->getName() + "ErosionSurface");
    setLayerName(erosionRecord, erodedLayer->getName() + "Erosion");
    setDepoAge(erosionRecord, t1);
    setDepth(erosionRecord, baseDepth - d1 );
    setThickness(erosionRecord, DefaultUndefinedScalarValue );

    // construct the continuation of the eroded layer.
    database::Record * erodedLayerContinuationRecord = table->createRecord();
    assert(erodedLayerContinuationRecord);
    erodedLayerContinuationRecord->copyFrom( *erodedLayerRecord );
    setSurfaceName(erodedLayerContinuationRecord, erodedLayer->getName() + "ContinuationSurface");
    setLayerName(erodedLayerContinuationRecord, erodedLayer->getName() + "Continuation");
    setDepoAge(erodedLayerContinuationRecord, t_e1);
    setDepth(erodedLayerContinuationRecord, topDepth);

    // move record to correct position
    table->moveRecord(erosionRecord, erodedLayerRecord);
    table->moveRecord(erodedLayerContinuationRecord, erosionRecord);
    
    // fix depo sequence numbers
    for (size_t i = 0; i < table->size(); ++i)
      setDepoSequence( table->getRecord(i), table->size() - i);
  }

private:
  enum SurfaceOrder { UNDETERMINED, ABOVE, UNDER, INTERSECTING, EQUAL };

  static SurfaceOrder compare( const DataAccess::Interface::Surface * a, const DataAccess::Interface::Surface * b )
  {
    const DataAccess::Interface::GridMap * gridA = a->getInputDepthMap(), * gridB = b->getInputDepthMap();

    assert( gridA->numI() == gridB->numI() );
    assert( gridA->numJ() == gridB->numJ() );
    assert( gridA->getDepth() == 1 && "surfaces are 2D"); 
    assert( gridB->getDepth() == 1 && "surfaces are 2D");

    const unsigned int I = gridA->numI();
    const unsigned int J = gridB->numJ();

    SurfaceOrder state = UNDETERMINED;

    for (unsigned int i = 0; i < I; ++i)
    {
      for (unsigned int j = 0; j < J; ++j)
      {
	const double depthA = gridA->getValue(i,j);
	const double depthB = gridB->getValue(i,j);
	
	switch(state)
	{
	  case UNDETERMINED: case EQUAL:
	    if (depthA < depthB)
	      state = ABOVE;
	    else if (depthA > depthB)
	      state = UNDER;
	    else
	      state = EQUAL;
	    break;

	  case ABOVE:
	    if (depthA > depthB)
	      return INTERSECTING;
	    break;

	  case UNDER:
	    if (depthA < depthB )
	      return INTERSECTING;
	    break;

	  case INTERSECTING:
	    assert(false && "Function should have returned already");
	    break;

	  default:
	    assert( false && "Unknown value of 'state'");
	    break;
	}
      }
    }

    return state;
  }

  std::string m_inputFileName, m_outputFileName;
  DataAccess::Interface::ProjectHandle * m_projectHandle;
};

struct OptionException : BaseException< OptionException > 
{ OptionException() { *this << "Error reading command line parameter: "; } };

class OptionParser
{
public:
  static const std::string PARAMETER;  

  typedef std::map< std::string, std::vector< std::string > > Options;

  OptionParser(int argc, char ** argv, const std::string & optionFlag, const std::string & separators)
    : m_options()
    , m_pos(0)
  {
    std::string currentOptionName = PARAMETER;

    for (int i = 1; i < argc; ++i)
    {
      std::string tok( argv[i] );

      if ( tok.size() > optionFlag.size() && tok.find( optionFlag ) == 0 )
      { // then we found a new option flag
	currentOptionName = tok.substr( optionFlag.size());
	m_options.insert( std::make_pair( currentOptionName, std::vector<std::string>() ));
	continue;
      }

      // else tokenize the parameters of the option
      size_t pos = 0, previousPos = 0;
      while(pos < tok.size())
      {
	previousPos = pos;
	pos = tok.find_first_of(separators, pos);

	if (pos != std::string::npos)
	{
	  if (pos != previousPos)
	  {
  	    m_options[currentOptionName].push_back( tok.substr(previousPos, pos - previousPos));
	  }
	  m_options[currentOptionName].push_back( tok.substr(pos, 1));
	  pos++;
	}
	else
	{
	  m_options[currentOptionName].push_back( tok.substr(previousPos) );
	  break;
	}
      }
    }
  }

  class OptionValues
  { friend class OptionParser;
  public:

    const std::string & operator[]( size_t i)
    {
      if (i >= m_it->second.size() )
	throw OptionException() << m_name << " needs an additional parameter";

      return m_it->second[i];
    }

    size_t size() const
    {
      return m_it->second.size();
    }

  private:
    OptionValues(const OptionParser & parser, const std::string & name)
      : m_name( name )
      ,	m_it( parser.m_options.find(name) )
    {
    	if (m_it == parser.m_options.end() )
	  throw OptionException() << m_name;
    }

    std::string m_name;
    Options::const_iterator m_it;
  };

  OptionValues operator[]( const std::string & name ) const
  {
    return OptionValues(*this, name);
  }

  bool defined( const std::string & name) const
  {
    return m_options.find(name) != m_options.end();
  }

  bool empty() const
  {
    return m_options.empty(); 
  }

  void erase( const std::string & name) 
  {
    m_options.erase( name );
  }

  std::vector<std::string> definedNames() const
  {
    std::vector<std::string> result;
    for (Options::const_iterator i = m_options.begin(); i != m_options.end(); ++i )
      result.push_back( i->first );

    return result;
  }

private:
  Options m_options ;
  int m_pos;
};

const std::string
OptionParser
  :: PARAMETER = " a positional parameter ";


void terminateHandler()
try
{
  throw;
}
catch( std::exception & e)
{
  showUsage( e.what() );
  std::exit(1);
}

struct ConversionException : BaseException< ConversionException > {};			     

template <typename T>
T fromString(const std::string & x)
{
  T y;
  std::istringstream s(x);
  s >> y;

  if (!s)
    throw ConversionException() << "Error while converting std::string '" << x << "' to type '" << typeid(y).name() << "'" ;

  return y;
}



/// Retrieve and adjust the value of the parameter in the project file.
int main( int argc, char ** argv)
{
  std::set_terminate( & terminateHandler );

  std::vector< std::pair< std::string, double > > basementParameters;
  std::vector< std::pair< double, double > > crustThicnkessSeries;
  double value = 0.0;

  OptionParser options(argc, argv, "--", "=," );

  if (options.defined("help"))
  {
    showUsage(0);
    return EXIT_SUCCESS;
  }


  // Determining Input and Output file
  std::string inputProject, outputProject;
  
  outputProject = inputProject = options["project"][0];
  options.erase( "project" );

  try
  { 
    outputProject = options["output"][0];
    options.erase("output");
  }
  catch (OptionException & e)
  {
    /* ignore */
  }


  Project project( inputProject, outputProject);

  // Set basement property
  if (options.defined("set-basement-property"))
  {
    OptionParser::OptionValues values = options["set-basement-property"];
    if (values.size() % 3 != 0 )
    {
      throw OptionException() << "The parameter of --set-basement-property must be of the form NAME=VALUE";
    }

    for (int i = 0; i < values.size(); i+= 3)
    {
      if (values[i+1] != "=")
      {
        throw OptionException() << "The parameter of --set-basement-property must be of the form NAME=VALUE";
      }

      double x2 = 0.0;

      try
      {
	x2 = fromString<double>(values[i+2]);
      }
      catch (ConversionException & e)
      {
        throw OptionException() << "The value in the NAME=VALUE parameter of the --set-basement-property option must be real number.";
      }

      double x1 = project.setBasementProperty(values[i], x2);
    }

    options.erase("set-basement-property");
  }

  if (options.defined("set-crust-thickness"))
  {
    OptionParser::OptionValues values = options["set-crust-thickness"];

    std::vector< double > timeSeries;

    size_t N = 0;
    for (; values[N] != ","; ++N)
    {
      timeSeries.push_back( fromString<double>( values[N] ) );
      if (N > 0 && timeSeries[N-1] >= timeSeries[N])
	throw OptionException() << "The age series in the --set-crust-thickness parameter must be strictly increasing: "
	       << "age[" << N-2 << "] = " << timeSeries[N-1] << " should come after age[" << N << "] = " << timeSeries[N];

    }

    std::vector< double > thicknessSeries;
    for (size_t i = 0; i < N; ++i)
    {
      thicknessSeries.push_back( fromString<double>( values[N+i+1] ));
    }


    std::vector< std::pair<double, double > > series(N);
    for (size_t i = 0 ; i < N; ++i)
    {
      series[i] = std::make_pair( timeSeries[i], thicknessSeries[i]);
    }

    project.setCrustThickness( series );
    project.clearSnapshotTable();

    options.erase("set-crust-thickness");
  }

  if (options.defined("adjust-thermal-conductivity"))
  {
    OptionParser::OptionValues values = options["adjust-thermal-conductivity"];
    if (values.size() % 3 != 0 )
    {
      throw OptionException() << "The parameter of --adjust-thermal-conductivity must be of the form LITHOTYPE=CORRECTION";
    }

    for (int i = 0; i < values.size(); i+= 3)
    {
      if (values[i+1] != "=")
      {
        throw OptionException() << "The parameter of --adjust-thermal-conductivity must be of the form LITHOTYPE=CORRECTION";
      }

      double x2 = 0.0;

      try
      {
	x2 = fromString<double>(values[i+2]);
      }
      catch (ConversionException & e)
      {
        throw OptionException() << "The correction in the LITHOTYPE=CORRECTION parameter of the --adjust-thermal-conductivity option must be real number.";
      }

      project.adjustThermalConductivity(values[i], x2);
    }

    options.erase("adjust-thermal-conductivity");
  }

  if (options.defined("show-erosion-formations"))
  {
    typedef std::map< Project::Formation, std::vector< Project::Formation > > Map;
    Map formations = project.getErosionFormations();

    std::cout << "Erosion formations are:\n";
    for (Map::const_iterator formation = formations.begin(); formation != formations.end(); ++formation)
    {
      const Project::Formation & erosion = formation->first;
      const std::vector< Project::Formation > & eroded = formation->second;

      std::cout << '\'' << erosion << "' [" << erosion.m_minAge << " Ma - " << erosion.m_maxAge << " Ma] erodes ";
      if (erosion.m_constant)
	std::cout << erosion.m_minThickness;
      else
	std::cout << "between " << erosion.m_minThickness << " and " << erosion.m_maxThickness;
      
      std::cout << " meters from the following formation"
	<< (eroded.size() > 1 ? "s" : "") << ": " ;

      for (size_t i = 0; i < eroded.size(); ++i)
      {
	if (i > 0)
	{
	  std::cout << ", ";

	  if (i < eroded.size() - 1)
	    std::cout << "and ";
	}

	std::cout << "'" << eroded[i] << "' [" << eroded[i].m_minAge << " Ma - " << eroded[i].m_maxAge 
	  << " Ma] with thickness ";
	
	if (eroded[i].m_constant)
	  std::cout << "of " << eroded[i].m_minThickness ;
	else
	  std::cout << "between " << eroded[i].m_minThickness << " and  " << eroded[i].m_maxThickness ;

	std::cout << " meters";
      }

      // Mark all erosion formations that can be tweaked with a '*'
      if (eroded.size() == 1 && erosion.m_constant && eroded[0].m_minThickness && erosion.m_minThickness == 0.0 - eroded[0].m_minThickness)
	std::cout << " (*)";

      std::cout << "\n";
    }
    std::cout << "Note: entries marked with a (*) can be changed with the --set-erosion parameter\n";
    std::cout << std::endl;

    options.erase("show-erosion-formations");
  }

  if (options.defined("set-erosion"))
  {
    OptionParser::OptionValues values = options["set-erosion"];

    int i = 0;
    while( i < values.size() )
    {
      std::string formation = values[i];

      if (i +2 >= values.size() || values[i+1] !=  "=" )
	throw OptionException() << "The parameter of --set-erosion must be of the form NAME=THICKNESS[,T0,T1,T2]";
      
      i+=2;

      double thickness = fromString<double>(values[i]);
      if (thickness < 0.0)
        throw OptionException() << "The erosion thickness must be a positive real number.";

      i+=1;

      double t0 = NAN, t1 = NAN, t2 = NAN;

      if (i < values.size() && values[i] == ",")
      {
	++i;
	if (i + 5 > values.size() || values[i+1] != "," || values[i+3] != ",")
	  throw OptionException() << "The parameter of --set-erosion must be of the form NAME=THICKNESS[,T0,T1,T2]";


	t0 = fromString<double>(values[i]);
	t1 = fromString<double>(values[i+2]);
	t2 = fromString<double>(values[i+4]);

	if (t0 <= t1 || t1 <= t2)
	  throw OptionException() << "T0, T1, and T2 must be in Ma and in chronological order. In practice it means that T0 > T1 > T2 must be true.";
	
	i+=5;
      }

      project.setErosionThickness(formation, thickness, t0, t1, t2);
    }

    // clear the snapshot table, because time of certain events change.
    project.clearSnapshotTable();
    options.erase("set-erosion");
  }

  // --add-erosion <thickness>,<erosion start>,<erosion length>
  if (options.defined("add-erosion"))
  {
    OptionParser :: OptionValues values = options["add-erosion"];

    if (values.size() % 5 != 0 )
    {
      throw OptionException() << "The parameter of --add-erosion must a triples of <thickness>, <erosion-start>, <erosion duration>";
    }

    for (int i = 0; i < values.size(); i+= 5)
    {
      if (values[i+1] != "," || values[i+3] != "," )
      {
      	throw OptionException() << "The parameter of --add-erosion must a triples of <thickness>, <erosion-start>, <erosion duration>";
      }

      double thickness = 0.0;

      try
      {
	thickness = fromString<double>(values[i]);
      }
      catch (ConversionException & e)
      {
        throw OptionException() << "Thickness must be a positive real number";
      }

      if (thickness < 0.0)
        throw OptionException() << "Thickness must be a positive real number";

      double t0 = 0.0, duration = 0.0;
      try
      {
	t0 = fromString<double>(values[i+2]);
	duration = fromString<double>(values[i+4]);
      }
      catch(ConversionException & e)
      {
	throw OptionException() << "Start time and duration of erosion event must be real numbers";
      }

      if (duration < 0.0)
	throw OptionException() << "Duration of erosion event must be a positive real number.";

      project.addErosion(thickness, t0, duration);
    }

    project.clearSnapshotTable();
    options.erase("add-erosion");
  }

  project.close();

  if (! options.empty() )
  {
    std::vector< std::string > unusedParams = options.definedNames();
    std::cerr << "WARNING: there " << (unusedParams.size() > 1 ? "are " : "is ") 
       << unusedParams.size() << " parameter" << (unusedParams.size() > 1? "s" : "") << " ignored:\n";

    for (size_t i = 0; i < unusedParams.size(); ++i)
      std::cerr << "WARNING: --" << unusedParams[i] << '\n';

    std::cerr << std::endl;
  }

  return EXIT_SUCCESS;
}
