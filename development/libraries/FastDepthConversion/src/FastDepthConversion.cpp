//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// to do: add the case when the reference is the sealevel and surface 0 depths are calculated using the seismic velocity of seawater

#include "FastDepthConversion.h"
#include "CommonDefinitions.h"

#include "database.h"

#include "petsc.h"

// DataAccess library
#include "Formation.h"
#include "Grid.h"
#include "GridMap.h"
#include "Interface.h"
#include "Surface.h"

//utilities library
#include "LogHandler.h"

namespace fastDepthConversion
{

namespace
{

static const double s_tolerance = 1.e-12;

bool isNotErosion(const double tarDepthI, const double tarTwtI, const double checkDepthI, const double checkTwtI, const double erosionI)
{
  return (tarDepthI == DataAccess::Interface::DefaultUndefinedMapValue ||
          tarTwtI == DataAccess::Interface::DefaultUndefinedMapValue ||
          checkDepthI == DataAccess::Interface::DefaultUndefinedMapValue ||
          checkTwtI == DataAccess::Interface::DefaultUndefinedMapValue ||
          std::fabs(tarTwtI - checkTwtI) >= s_tolerance ||
          tarDepthI - checkDepthI >= erosionI);
}

} // namespace

FastDepthConversion::FastDepthConversion(char* projectFileName, int referenceSurface, int endSurface,
                                           bool noCalculatedTWToutput, bool preserveErosionFlag, bool noExtrapolationFlag,
                                           int argc, char** argv, int rank) :
  m_referenceSurface(referenceSurface),
  m_endSurface(endSurface),
  m_noCalculatedTWToutput(noCalculatedTWToutput),
  m_preserveErosionFlag(preserveErosionFlag),
  m_noExtrapolationFlag(noExtrapolationFlag),
  m_argc(argc),
  m_argv(argv),
  m_rank(rank),
  m_depthsEndSurface(),
  m_fdcProjectManager(projectFileName),  
  m_fdcMapFieldProperties(m_fdcProjectManager, m_referenceSurface, m_endSurface),
  m_caseStorageManager(m_fdcProjectManager, projectFileName, rank),
  m_fdcVectorFieldProperties(m_fdcProjectManager, referenceSurface),
  m_fdcLithoProperties(m_fdcProjectManager)
{
  checkReferenceAndEndSurfaceBoundsAreValid();
  LogHandler(LogHandler::INFO_SEVERITY) << "options: referenceSurface : " << m_referenceSurface << ", endSurface: " << m_endSurface
                                        << ", NoCalculatedTWToutput: " << m_noCalculatedTWToutput << ", preserveErosion: " << m_preserveErosionFlag
                                        << ", NoExtrapolation: " << m_noExtrapolationFlag;
}

FastDepthConversion::~FastDepthConversion() {}

void FastDepthConversion::run()
{ 
  m_fdcMapFieldProperties.calculateInitialMaps(m_caseStorageManager.masterResultsFilePath(), m_preserveErosionFlag);
  calibrateDepths();
  FinalCaseManager finalCaseManager(m_fdcProjectManager, m_fdcMapFieldProperties, m_caseStorageManager, m_rank);
  finalCaseManager.writeFinalProject(m_endSurface, m_depthsEndSurface, m_noCalculatedTWToutput);
}

void FastDepthConversion::checkReferenceAndEndSurfaceBoundsAreValid() const
{  
  const int nSurfacesIDs = m_fdcProjectManager.surfacesIDs().size();

  if (m_referenceSurface >= nSurfacesIDs)
  { throw T2Zexception() << " The parameter value of -referenceSurface is invalid: " << m_referenceSurface; }

  if (m_endSurface <= m_referenceSurface || m_endSurface >= nSurfacesIDs)
  { throw T2Zexception() << " The parameter value of -endSurface is invalid: " << m_endSurface; }
}

void FastDepthConversion::reloadModel()
{
  try
  {    
    m_fdcProjectManager.reloadModel(m_caseStorageManager.caseProjectFilePath());        
  }
  catch (const T2Zexception & ex)
  {
    throw T2Zexception() << "Model could not be reloaded! error code: " << ex.what();
  }
}

void FastDepthConversion::prepareModelToRunAutomaticDepthConversion()
{
  try
  {
    m_fdcProjectManager.prepareProject();
  }
  catch (const T2Zexception & ex)
  {
    throw T2Zexception() << " Project could not be prepared for layer " << m_fdcLithoProperties.currentLayerName()
                         << " with top surface " << m_fdcLithoProperties.currentTopName() << ", error code: " << ex.what();
  }
}

bool FastDepthConversion::checkForHiatus(const mbapi::StratigraphyManager::SurfaceID currentSurface)
{
  if (m_fdcMapFieldProperties.hiatusID(currentSurface) > -1)
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "Skipping hiatus for surface " << m_fdcLithoProperties.nextTopName();
    return true;
  }
  return false;
}

bool FastDepthConversion::setTwtMapsIfMissingInBottomSurface()
{
  const mbapi::StratigraphyManager::SurfaceID nextSurface = m_fdcLithoProperties.nextSurface();
  if(!m_fdcMapFieldProperties.twtMaps(nextSurface).empty())
  { return false; }

  m_fdcMapFieldProperties.setMissingTwtForSurface(nextSurface);
  m_fdcVectorFieldProperties.setTarTwts(m_fdcMapFieldProperties.missingTwtMaps(nextSurface));
  m_fdcProjectManager.appendToAddedTwtMapNames(m_fdcLithoProperties.nextTopName());
  m_fdcProjectManager.appendToAddedTwtSurfaceNames(m_fdcLithoProperties.nextTopName());
  return true;
}

void FastDepthConversion::setDepthAndTwtMapsForNextSurfaceInTables(std::string & mapName, const bool generated_Twt, const std::vector<double> & increasedDepths)
{
  const mbapi::StratigraphyManager::SurfaceID nextSurface = m_fdcLithoProperties.nextSurface();
  mapName = m_fdcProjectManager.t2ZTemporaryMapName(m_fdcLithoProperties.nextTopName());
  m_fdcMapFieldProperties.setCorrectedMapsNames(nextSurface, mapName);
  size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
  m_fdcProjectManager.generateMapInGridMapIoTbl(mapName, increasedDepths, mapsSequenceNbr, m_caseStorageManager.resultsMapFileName(), nextSurface);
  m_fdcProjectManager.setMapNameInStratIoTbl(nextSurface, mapName);
  m_fdcMapFieldProperties.setCorrectedMapsSequenceNbr(nextSurface, mapsSequenceNbr);

  if (!(generated_Twt && !m_noCalculatedTWToutput)) { return; }

  mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
  if(IsValueUndefined(m_fdcProjectManager.generateMapInTwoWayTimeIoTbl(m_fdcVectorFieldProperties.tarTwts(), mapsSequenceNbr, m_caseStorageManager.resultsMapFileName())))
  { throw T2Zexception() << " Cannot generate the twt map for the surface " << nextSurface; }
  m_fdcMapFieldProperties.setAddedTwtmapsequenceNbr(mapsSequenceNbr);
}

void FastDepthConversion::runFastCauldronAndCalculateNewDpeths()
{
  if (m_rank == 0) { LogHandler(LogHandler::INFO_SEVERITY) << "Running Fastcauldron to adjust the surface: " << m_fdcLithoProperties.nextTopName(); }

  try
  {
    MPI_Barrier(PETSC_COMM_WORLD);
    FDCFastCauldronManager fdcFastCauldronManager(m_argc, m_argv, m_fdcProjectManager.xScalingFactor(), m_fdcProjectManager.yScalingFactor(), m_noExtrapolationFlag);
    fdcFastCauldronManager.prepareAndRunFastCauldron();
    DataAccess::Interface::GridMap * twtGridMap = fdcFastCauldronManager.getPropertyGridMap("TwoWayTime", m_fdcLithoProperties.currentLayerName());
    DataAccess::Interface::GridMap * depthGridMap = fdcFastCauldronManager.getPropertyGridMap("Depth", m_fdcLithoProperties.currentLayerName());

    if (twtGridMap == nullptr || depthGridMap == nullptr)
    { throw T2Zexception() << "property grid maps could not be set after fastcauldron run"; }
    calculateNewDepths(twtGridMap, depthGridMap);

    fdcFastCauldronManager.finalizeFastCauldronStartup();
    MPI_Barrier(PETSC_COMM_WORLD);
  }
  catch (const T2Zexception & ex)
  {
    throw T2Zexception() << "runFastCauldronAndGetGridMaps  failed, error code: " << ex.what();
  }
}

void FastDepthConversion::writeNewDepthAndCorrectedMapstoCaseFileInMasterDirectory(const std::string & mapName)
{  
  size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
  m_fdcProjectManager.generateMapInGridMapIoTbl(mapName, m_fdcVectorFieldProperties.newDepths(), mapsSequenceNbr, m_caseStorageManager.resultsMapFileName(), m_fdcLithoProperties.nextSurface());  
}

void FastDepthConversion::calibrateDepths()
{  
  for (mbapi::StratigraphyManager::SurfaceID currentSurface = m_referenceSurface; currentSurface < m_endSurface; ++currentSurface)
  {
    m_caseStorageManager.setOriginalMasterPath();

    // Note: reset and reload model in each loop, so that only one copy of the database is present
    reloadModel();

    m_fdcLithoProperties.setLithoSurfaces(currentSurface);

    if (checkForHiatus(currentSurface)) { continue; }

    const mbapi::StratigraphyManager::LayerID   currentLayer = m_fdcLithoProperties.currentLayer();
    const mbapi::StratigraphyManager::SurfaceID nextSurface = m_fdcLithoProperties.nextSurface();
    const std::string currentTopName = m_fdcLithoProperties.currentTopName();
    const std::string nextTopName = m_fdcLithoProperties.nextTopName();
    const std::string currentLayerName = m_fdcLithoProperties.currentLayerName();

    LogHandler(LogHandler::INFO_SEVERITY) << "Running depth conversion between " << currentTopName << " and " << nextTopName << " for Layer: " << currentLayerName;
    prepareModelToRunAutomaticDepthConversion();

    m_fdcLithoProperties.setlayerLithologiesListForCurrentLayer();
    m_fdcVectorFieldProperties.setTopSurfaceProperties(currentSurface, m_referenceSurface, m_fdcMapFieldProperties.twtMaps(currentSurface));
    m_fdcMapFieldProperties.setPropertyHistoryMaps(currentLayer, m_fdcVectorFieldProperties.refDepths(), m_fdcVectorFieldProperties.refTwts());

    bool generated_Twt = setTwtMapsIfMissingInBottomSurface();
    if (!generated_Twt)
    {
      const std::vector<double> tarTwts = m_fdcVectorFieldProperties.getMeasuredTwtAtSpecifiedSurface(nextSurface, m_fdcMapFieldProperties.twtMaps(nextSurface));
      m_fdcVectorFieldProperties.setTarTwts(tarTwts);
    }

    const double maxSeisVel = m_fdcLithoProperties.getMaxSeismicVelocityForCurrentLayer();
    const std::vector<double> increasedDepths = m_fdcVectorFieldProperties.calculateIncreasedDepthsIncludingPreservedErosion(currentSurface, maxSeisVel);

    if (m_preserveErosionFlag)
    {
      m_fdcVectorFieldProperties.setPreservedErosion(calculateErosion(nextSurface));
    }

    try
    {
      m_fdcProjectManager.modifyTables(nextSurface, m_fdcMapFieldProperties.hiatusAll(), m_fdcMapFieldProperties.correctedMapsNames(),
                                       m_fdcMapFieldProperties.correctedMapsSequenceNbr(), m_fdcMapFieldProperties.addedTwtmapsequenceNbr(),
                                       m_caseStorageManager.resultsMapFileName(), m_noCalculatedTWToutput);
    }
    catch (const T2Zexception & ex)
    {
      throw T2Zexception() << " Tables could not be modified for layer " << currentLayerName <<" with top surface "
                           << currentTopName << ", error code: " << ex.what();
    }

    try
    {
      m_caseStorageManager.createTemporaryCase(nextSurface);
    }
    catch (const T2Zexception & ex)
    {
      throw T2Zexception() << "Case could not be created, error code: " << ex.what();
    }

    // Reload the model so the map manager gets updated (with project file in newly created temporary folder)
    reloadModel();

    std::string mapName;
    setDepthAndTwtMapsForNextSurfaceInTables(mapName, generated_Twt, increasedDepths);
    m_caseStorageManager.saveModelToCaseProjectFile();

    m_caseStorageManager.changeToTemporaryCaseDirectoryPath();
    runFastCauldronAndCalculateNewDpeths();
    m_caseStorageManager.changeToMasterDirectoryPath();

    writeNewDepthAndCorrectedMapstoCaseFileInMasterDirectory(mapName);
    m_caseStorageManager.copyTemporaryToMasterHDFMaps();
  }

  // save a copy of the depths of the bottom most surface to append the isopacks later
  m_depthsEndSurface.resize(m_fdcVectorFieldProperties.newDepths().size());
  m_depthsEndSurface = m_fdcVectorFieldProperties.newDepths();

  m_caseStorageManager.setOriginalMasterPath();
  reloadModel();
}

void FastDepthConversion::calculateNewDepths(const DataAccess::Interface::GridMap * twtGridMap,
                                              const DataAccess::Interface::GridMap * depthGridMap)
{
  const mbapi::StratigraphyManager::LayerID & layerID = m_fdcLithoProperties.currentLayer();
  const std::vector<double> & refDepths = m_fdcVectorFieldProperties.refDepths();
  const std::vector<double> & tarTwt = m_fdcVectorFieldProperties.tarTwts();
  const std::vector<double> & erosion = m_fdcVectorFieldProperties.preservedErosion();
  const std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double>> & checkErosionDepths = m_fdcMapFieldProperties.refDepthsHistory();
  const std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double>> & checkTwt = m_fdcMapFieldProperties.refTwtHistory();

  std::vector<double> newDepths = m_fdcVectorFieldProperties.newDepths();

  std::vector<double> twtSim(refDepths.size());
  std::vector<double> twtSimUpper(refDepths.size());
  std::vector<double> depthSim(refDepths.size());
  std::vector<double> depthSimUpper(refDepths.size());

  const unsigned int maxK = twtGridMap->getDepth() - 1;
  fillArray(twtGridMap, twtSim, maxK,  m_fdcVectorFieldProperties.twtconvFactor());
  fillArray(depthGridMap, depthSim, maxK, 1.0);

  bool searching = false;

  // Start from the top
  for (size_t i = 0; i < newDepths.size(); ++i)
  {
    // account for shark teeth AOI cut out due to subsampling runs!
    if(depthSim[i]== depthGridMap->getUndefinedValue() || twtSim[i] == twtGridMap->getUndefinedValue())
    {
      newDepths[i] = depthGridMap->getUndefinedValue();
      continue;
    }
    newDepths[i] = DataAccess::Interface::DefaultUndefinedScalarValue;

    if (twtSim[i] > tarTwt[i]* m_fdcVectorFieldProperties.twtconvFactor())
    {
      // in case TWT output gets re-dated to sea level
      newDepths[i] = ((twtSim[i]) > s_tolerance && tarTwt[i] >= 0.0) ? (depthSim[i] / twtSim[i] * tarTwt[i]* m_fdcVectorFieldProperties.twtconvFactor()) : depthSim[i];
    }
    if (!searching && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue) { searching = true; }
  }

  int globalReady = 0;
  int localReady = searching ? 1 : 0;
  MPI_Allreduce( &localReady, &globalReady, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD);
  searching = globalReady > 0;

  twtSimUpper = twtSim;
  depthSimUpper = depthSim;
  for (int k = maxK - 1; k >= 0; --k)
  {
    if (!searching) break; // all depths are set, no need to retrieve other data
    searching = false;

    fillArray(twtGridMap, twtSim, k,  m_fdcVectorFieldProperties.twtconvFactor());
    fillArray(depthGridMap, depthSim, k, 1.0);

    for (size_t i = 0; i != newDepths.size(); ++i)
    {
      if (twtSim[i] > tarTwt[i]* m_fdcVectorFieldProperties.twtconvFactor() && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue)
      {
        const double twtSimDiff = twtSim[i] - twtSimUpper[i];
        const double twtMeasDiffUpper = (tarTwt[i]* m_fdcVectorFieldProperties.twtconvFactor()) - twtSimUpper[i];
        const double twtMeasDiffLower = twtSim[i] - (tarTwt[i]* m_fdcVectorFieldProperties.twtconvFactor());

        const double depthSimDiff = depthSim[i] - depthSimUpper[i];

        if (std::fabs(twtSimDiff) > s_tolerance)
        {
          newDepths[i] = depthSimDiff / twtSimDiff * twtMeasDiffUpper + depthSimUpper[i];
        }
        else
        {
          newDepths[i] = fabs(twtMeasDiffLower) < fabs(twtMeasDiffUpper) ? depthSim[i] : depthSimUpper[i];
        }
      }

      if (!searching && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue) { searching = true; }
    }

    globalReady = 0;
    localReady = searching ? 1 : 0;
    MPI_Allreduce( &localReady, &globalReady, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD);
    searching = globalReady > 0;

    twtSimUpper = twtSim;
    depthSimUpper = depthSim;
  }

  for (size_t i = 0; i < newDepths.size(); ++i)
  {
    // Set to the last deepmost simulated depth (k == 0)
    if (DataAccess::Interface::DefaultUndefinedScalarValue == newDepths[i]) { newDepths[i] = depthSim[i]; }

    // If everything goes wrong, use the refDepths value (or unwanted erosion)
    if(newDepths[i] != depthGridMap->getUndefinedValue())
    {
      double corrected_depth = DataAccess::Interface::DefaultUndefinedScalarValue;
      double tmpLimit = 0.0;
      mbapi::StratigraphyManager::LayerID workingID = layerID;
      while( true )
      {
        if ( m_fdcMapFieldProperties.hiatusID(workingID) != DataAccess::Interface::DefaultUndefinedScalarIntValue &&
             m_fdcMapFieldProperties.hiatusID(workingID) == -1 &&  (newDepths[i] - checkErosionDepths.at(workingID).at(i) < tmpLimit ||
                                                                    fabs(tarTwt[i]- checkTwt.at(workingID).at(i)<1e-6 )))
        {
          corrected_depth = checkErosionDepths.at(workingID).at(i);
          tmpLimit = ((newDepths[i] - checkErosionDepths.at(workingID).at(i)) < 0.0) ? (newDepths[i] - checkErosionDepths.at(workingID).at(i)) : 0.0;
        }
        if (workingID == 0)
        {
          break;
        }

        --workingID;
      }

      if (corrected_depth != DataAccess::Interface::DefaultUndefinedScalarValue)
      {
        newDepths[i] = corrected_depth;
      }

      // add the preserved erosion.
      newDepths[i] += erosion[i];
    }
  }

  m_fdcVectorFieldProperties.setNewDepths(newDepths);  
}

void FastDepthConversion::fillArray( const DataAccess::Interface::GridMap * grid, std::vector<double> & v, int k, const double convFact )
{
  grid->retrieveData();

  int it = 0;
  for ( unsigned int j = grid->firstJ( ); j <= grid->lastJ( ); ++j )
  {
    for ( unsigned int i = grid->firstI( ); i <= grid->lastI( ); ++i )
    {
      v[it++] = grid->getValue( i, j, (unsigned int)k ) * convFact;
    }
  }
  grid->restoreData( );
}

std::vector<double> FastDepthConversion::calculateErosion(const mbapi::StratigraphyManager::SurfaceID surfID)
{
  std::vector<double> erosion;
  //LogHandler(LogHandler::INFO_SEVERITY) << "Running FastDepthConversion::calculateErosion for surfID " << surfID << "";
  std::vector<double> tarTwt;
  if (!m_fdcMapFieldProperties.twtMaps(surfID).empty())
  {
    tarTwt = m_fdcVectorFieldProperties.tarTwts();
  }
  else
  {
    if (m_fdcMapFieldProperties.missingTwtMaps(surfID).empty())
    {
      throw T2Zexception() << "Failed to get the reference TWT map (calculateErosion) for surfID: " << surfID << "!";
    }
    tarTwt = m_fdcMapFieldProperties.missingTwtMaps(surfID);
  }
  const std::vector<double> tarDepth = m_fdcProjectManager.getGridMapDepthValues(surfID);

  std::vector<double> checkTwt;
  erosion.resize(tarDepth.size(), 0.0);
  for (mbapi::StratigraphyManager::SurfaceID checkSurface = surfID-1; checkSurface >= m_referenceSurface; --checkSurface)
  {
    if( !m_fdcMapFieldProperties.twtMaps(checkSurface).empty())
    {
      checkTwt = m_fdcVectorFieldProperties.getMeasuredTwtAtSpecifiedSurface(checkSurface, m_fdcMapFieldProperties.twtMaps(checkSurface));
    }
    else
    {
      if (m_fdcMapFieldProperties.missingTwtMaps(checkSurface).empty())
      {
        throw T2Zexception() << "Failed to get the reference TWT map (calculateErosion) for surfID: "<<checkSurface<<"!";
      }
      checkTwt = m_fdcMapFieldProperties.missingTwtMaps(checkSurface);
    }

    std::vector<double> checkDepth = m_fdcProjectManager.getGridMapDepthValues(checkSurface);
    for (size_t i = 0; i != erosion.size(); ++i)
    {
      if (isNotErosion(tarDepth[i], tarTwt[i], checkDepth[i], checkTwt[i], erosion[i]))
      {
        continue;
      }
      erosion[i] = tarDepth[i] - checkDepth[i];
    }

    if (checkSurface == 0) break;
  }

  MPI_Barrier(PETSC_COMM_WORLD);
  return erosion;
}

} // namespace fastDepthConversion
