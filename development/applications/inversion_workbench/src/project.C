#include "project.h"
#include "formattingexception.h"
#include "Crust.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "cauldronschemafuncs.h"

#include <cmath>

struct AdjustException : formattingexception::BaseException< AdjustException > 
{ AdjustException() { *this << "Error adjusting parameter in project file: "; } };

Project
   ::  Project(const std::string & input, const std::string & output )
   : m_inputFileName(input)
   , m_outputFileName(output)
   , m_projectHandle(DataAccess::Interface::OpenCauldronProject( m_inputFileName, "rw" ))
   , m_crust()                                                                                      
{
   if (!m_projectHandle)
      throw AdjustException() << "Could not load project file '" << m_inputFileName << "'";
}

Project
   :: ~Project()
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


void
Project
   :: close()
{
   if (m_crust)
      setCrustThickness( m_crust->getThicknessHistory() );

   if (m_projectHandle)
      m_projectHandle->saveToFile(m_outputFileName);

   m_projectHandle.reset();
}

void
Project
   :: discard()
{
   m_projectHandle.reset();
}

double
Project
   :: setBasementProperty(const std::string & parameter, double newValue)
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

void
Project
   :: setCrustThickness( const std::vector< std::pair<Time, Thickness> > & series)
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

void
Project
   :: addCrustThicknessThinningEvent(double startTime, double duration, double thickness, double ratio)
{
   if (!m_crust)
   {
      m_crust.reset( new Crust(thickness) );
   }

   m_crust->addThinningEvent(startTime, duration, ratio);
}

void
Project
   :: clearSnapshotTable()
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

void
Project
   :: adjustThermalConductivity( const std::string & lithotype, double correctionFactor)
{
   setLithotypeProperty("StpThCond", lithotype, 0.0, correctionFactor);
}

void
Project
   :: setLithotypeProperty( const std::string & property, const std::string & lithotype, double increment, double multiplicationFactor)
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
       record->setValue(property,  increment + multiplicationFactor * record->getValue<double>(property) );
   }
}

Project :: Formation 
   :: Formation( const DataAccess::Interface::Formation * formation)
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

// Returns which formations erode which other formations
std::map< Project :: Formation, std::vector< Project :: Formation > >
Project
   :: getErosionFormations() const
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

void
Project
   :: setErosionThickness(const std::string & formationName, double thickness, double t0, double t1, double t2 )
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


void 
Project
   :: addErosion( double thickness, double t0, double duration )
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


Project :: SurfaceOrder 
Project
   :: compare( const DataAccess::Interface::Surface * a, const DataAccess::Interface::Surface * b )
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

