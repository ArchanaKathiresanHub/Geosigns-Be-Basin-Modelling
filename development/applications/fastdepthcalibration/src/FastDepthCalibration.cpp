//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// to do: add the case when the refence is the sealevel and surface 0 depths are calculated using the seismic velocity of seawater

#include "FastcauldronStartup.h"

// DataAccess library
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

#include "FastDepthCalibration.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"

static const double       s_twtconvFactor = 0.001;
static const string       s_resultsFile = "Inputs.HDF"; // "CalibratedInputs.HDF";
static const string       s_finalResultsFolder = "CalibratedDepthMapsProject";

FastDepthCalibration::FastDepthCalibration( char * projectFileName, int referenceSurface, int endSurface, int argc, char** argv, int rank ) :
                                            m_projectFileName( projectFileName ),
                                            m_referenceSurface( referenceSurface ),
                                            m_endSurface( endSurface ),
                                            m_argc( argc ),
                                            m_argv( argv ),
                                            m_rank( rank ),
                                            m_fullMasterPath( FolderPath( "." ).fullPath( ) ),
                                            m_finalResultsPath( ibs::FolderPath( "." ) << s_finalResultsFolder ),
                                            m_masterResults( FolderPath( "." ).fullPath( ) << s_resultsFile ),
                                            m_finalResults( ibs::FilePath( ibs::FolderPath( "." ) << s_finalResultsFolder ) << s_resultsFile )
{
   // Removing the result file if exist
   if ( s_resultsFile != "Inputs.HDF" && m_masterResults.exists( ) )
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Removing existing " << s_resultsFile;
      m_masterResults.remove( );
   }
}

FastDepthCalibration::~FastDepthCalibration( )
{
}

void FastDepthCalibration::calculateInitialMaps()
{
	// Automatic depth calibration
	// Model: we must have an unique instance of the project database during the runtime, so we load the project several times
	m_mdl.reset(new mbapi::Model());

	// Load the initial model in mdl
	if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(m_projectFileName.c_str()))
	{
		throw T2Zexception() << "Can not load model from " << m_projectFileName << ", " << m_mdl->errorMessage();
	}

	// To be more clear we collect the managers after the load so it is clear to which model they belog to
	mbapi::StratigraphyManager & stMgr = m_mdl->stratigraphyManager();
	mbapi::MapsManager &         mapMgr = m_mdl->mapsManager();

	// Get the surfaces
	const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs = stMgr.surfacesIDs();

	if (m_referenceSurface >= surfacesIDs.size())
	{
		throw T2Zexception() << " The parameter value of -referenceSurface is invalid: " << m_referenceSurface;
	}

	if (m_endSurface <= m_referenceSurface || m_endSurface >= surfacesIDs.size())
	{
		throw T2Zexception() << " The parameter value of -endSurface is invalid: " << m_endSurface;
	}

	// Now check the twt maps are present for all surfaces 
	for (mbapi::StratigraphyManager::SurfaceID s = m_referenceSurface; s <= m_endSurface; ++s)
	{
		// Stores the top twt maps of each surface s
		m_twtMaps[s] = stMgr.twtGridName(s);
		if (m_twtMaps[s].empty()) { throw T2Zexception() << " No twt maps were found for the surface " << s; }
	}

	// Get the current depth of the m_endSurface
	std::vector<double> depthEndSurface;
	if (!m_mdl->getGridMapDepthValues(m_endSurface, depthEndSurface))
	{
		throw T2Zexception() << " Cannot get the depth map for endSurface: " << m_endSurface << ", " << m_mdl->errorMessage();
	}

	// Calculate the "isopacks" for the surfaces below the m_endSurface
	for (mbapi::StratigraphyManager::SurfaceID s = m_endSurface + 1; s < surfacesIDs.size(); ++s)
	{
		// Get the depths
		std::vector<double> bottomDepth;
		if (!m_mdl->getGridMapDepthValues(s, bottomDepth))
		{
			throw T2Zexception() << " Cannot get the depth map for the surface " << s << ", " << m_mdl->errorMessage();
		}

		// Calculate the isopacks (layer thickness)
		for (size_t i = 0; i != bottomDepth.size(); ++i)
		{
			bottomDepth[i] -= depthEndSurface[i];
		}
		m_isoPacks[s] = bottomDepth;
	}

	// create an empty result file on master
	if (ErrorHandler::NoError != mapMgr.inizializeMapWriter(s_resultsFile, (s_resultsFile == "Inputs.HDF" ? true : false)))
	{
		throw T2Zexception() << "Cannot inizialize the map writer";
	}

	if (ErrorHandler::NoError != mapMgr.finalizeMapWriter())
	{
		throw T2Zexception() << "Cannot finalize the map writer";
	}
}

void FastDepthCalibration::calibrateDepths()
{
	// variables that are used in the main depth calibration loop
	std::vector<double>   refDepths;
	std::vector<double>   newDepths;
	std::vector<double>   refTwt;
	std::vector<double>   tarTwt;

	MPI_Barrier(PETSC_COMM_WORLD); // sync point

	//-------------------------------------------------//
	// The start of the main depth calibration loop
	//-------------------------------------------------//

	for (mbapi::StratigraphyManager::SurfaceID currentSurface = m_referenceSurface; currentSurface < m_endSurface; ++currentSurface)
	{
		mbapi::StratigraphyManager::SurfaceID   nextSurface = currentSurface + 1;
		mbapi::StratigraphyManager::LayerID     currentLayer(currentSurface);
		// Destroy, create and reset the model all times, so only one copy of the database is present
		m_mdl.reset(new mbapi::Model());
		if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(m_projectFileName.c_str()))
		{
			throw T2Zexception() << "Can not load project from: " << m_projectFileName << ", " << m_mdl->errorMessage();
		}
		MPI_Barrier(PETSC_COMM_WORLD);

		mbapi::LithologyManager    & litMgrLocal = m_mdl->lithologyManager();
		mbapi::StratigraphyManager & stMgrLocal = m_mdl->stratigraphyManager();
		mbapi::MapsManager         & mapsMgrLocal = m_mdl->mapsManager();

		// Prepare mdl to run the automatic depth calibration, always load the master project projectName
		try { prepareProject(); } 
		catch (const T2Zexception & ex) { throw T2Zexception() << " Project could not be prepared for the surface " << currentSurface << ", error code: " << ex.what(); }

		// Modify StratIoTbl and GridMapIo tbl
		try { modifyTables(nextSurface); }
		catch (const T2Zexception & ex) { throw T2Zexception() << " Tables could not be modified for the surface " << currentSurface << ", error code: " << ex.what(); }

		// Get the maximum seismic velocity of currentLayer
		std::vector<std::string>   lithoList;
		std::vector<double>        lithoPercent;
		std::vector<std::string>   lithoPercMap;
		if (ErrorHandler::ReturnCode::NoError != stMgrLocal.layerLithologiesList(currentLayer, lithoList, lithoPercent, lithoPercMap))
		{
			throw ErrorHandler::Exception(stMgrLocal.errorCode()) << "Can not read the lithologies for the current layer: " << stMgrLocal.errorMessage();
		}
		double maxSeisVel = 0.0;
		for (int lith = 0; lith != lithoList.size(); ++lith)
		{
			mbapi::LithologyManager::LithologyID lithID = litMgrLocal.findID(lithoList[lith]);
			if ( IsValueUndefined( lithID ) )
			{
				throw ErrorHandler::Exception(litMgrLocal.errorCode()) << "Can not find the id for the lithology " << lith
					<< ", " << litMgrLocal.errorMessage();
			}
			// For these surfaces get the rock seismic velocity
			double seisVel = litMgrLocal.seisVelocity(lithID);
			if ( IsValueUndefined( seisVel ) )
			{
				throw ErrorHandler::Exception(litMgrLocal.errorCode()) << "Can not find the seismic velocity for the lithology " << lith
					<< ", " << litMgrLocal.errorMessage();
			}
			maxSeisVel < seisVel ? maxSeisVel = seisVel : maxSeisVel = maxSeisVel;
		}

		// Get the depths of the top surface, if currentSurface == m_referenceSurface retrive the depths, 
		// otherwise reuse the ones of the previous iteration as reference
		if (currentSurface == m_referenceSurface)
		{
			if (!m_mdl->getGridMapDepthValues(currentSurface, refDepths))
			{
				throw T2Zexception() << " Cannot get the depth values of the reference surface, " << m_mdl->errorMessage();
			}
			newDepths.resize(refDepths.size());
		}
		else
		{
			refDepths = newDepths;
		}

		// Get the measured TWT of the top surface
		mbapi::MapsManager::MapID refTwtID = mapsMgrLocal.findID(m_twtMaps[currentSurface]);
		if (ErrorHandler::ReturnCode::NoError != mapsMgrLocal.mapGetValues(refTwtID, refTwt))
		{
			throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << " Cannot get the measured twt map for the current surface " << currentSurface
				<< ", message: " << mapsMgrLocal.errorMessage();
		}
		// Get the measured TWT of the bottom surface
		mbapi::MapsManager::MapID twtMapID = mapsMgrLocal.findID(m_twtMaps[nextSurface]);
		if (ErrorHandler::ReturnCode::NoError != mapsMgrLocal.mapGetValues(twtMapID, tarTwt))
		{
			throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << " Cannot get the measured twt map for the next surface " << nextSurface
				<< ", message: " << mapsMgrLocal.errorMessage();
		}

		// Increase the depths
		std::vector<double> increasedDepths(refDepths.size());
		// For the topmost layer we take the top depths and we do not need to make twt differences
		if (currentSurface == 0)
		{
			for (size_t i = 0; i != refDepths.size(); ++i)
			{
				if (refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue || tarTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue)
				{
					increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
				}
				else
				{
					tarTwt[i] = tarTwt[i] * s_twtconvFactor;
					increasedDepths[i] = refDepths[i] + tarTwt[i] * maxSeisVel * 0.5;
				}
			}
		}
		else
		{
			for (size_t i = 0; i != refDepths.size(); ++i)
			{
				if (refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue ||
					tarTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue ||
					refTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue
					)
				{
					increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
				}
				else
				{
					tarTwt[i] = tarTwt[i] * s_twtconvFactor; // convert twt
					refTwt[i] = refTwt[i] * s_twtconvFactor; // convert twt 
					increasedDepths[i] = refDepths[i] + (tarTwt[i] - refTwt[i]) * maxSeisVel * 0.5;
				}
			}
		}

		// Create the name of the new folder where to store the case
		std::string mapName = "Surface_" + std::to_string(nextSurface);
		ibs::FolderPath casePath = ibs::FilePath(".") << mapName;

		ibs::FilePath casePathResults = ibs::FilePath(casePath) << s_resultsFile;            // Results path
		ibs::FilePath caseProject = ibs::FilePath(casePath) << m_mdl->projectFileName(); // Project path

		// Create the case
		try { createCase(casePath, caseProject, casePathResults); }
		catch (const T2Zexception & ex) { throw T2Zexception() << "Case could not be created, error code: " << ex.what(); }

		// Reload the model so the map manager gets updated (with the correct map sequence number)
		MPI_Barrier(PETSC_COMM_WORLD);
		m_mdl.reset(new mbapi::Model());
		if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(caseProject.cpath()))
		{
			throw ErrorHandler::Exception(m_mdl->errorCode()) << m_mdl->errorMessage();
		}
		MPI_Barrier(PETSC_COMM_WORLD);
		mbapi::MapsManager & mapsMgrLocalReloaded = m_mdl->mapsManager();

		// Create the new map with the increased depths
		m_correctedMapsNames[nextSurface] = mapName;
		size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
;
		m_correctedMapsIDs[nextSurface] = mapsMgrLocalReloaded.generateMap("StratIoTbl", mapName, increasedDepths, mapsSequenceNbr, s_resultsFile);
		if ( IsValueUndefined( m_correctedMapsIDs[nextSurface] ) )
		{
			throw T2Zexception() << " Cannot generate the map with increased depths for the surface " << nextSurface;
		}
		if (ErrorHandler::ReturnCode::NoError != m_mdl->setTableValue("StratIoTbl", nextSurface, "DepthGrid", mapName))
		{
			throw T2Zexception() << "Cannot set the map " << mapName << " as the new depth map of surface " << nextSurface << " in the StratIoTbl";
		}
		m_correctedMapsSequenceNbr[nextSurface] = mapsSequenceNbr;

		// Save the project and input data to folder with the new currentSurface map
		MPI_Barrier(PETSC_COMM_WORLD);
		if (m_rank == 0)
		{
			m_mdl->saveModelToProjectFile(caseProject.cpath(), true);
		}
		MPI_Barrier(PETSC_COMM_WORLD);

		// Change to the case directory
		if (!casePath.setPath())
		{
			throw T2Zexception() << "Cannot change to the case directory " << casePath.fullPath().path();
		}

		// Inform that Fastcauldron will run
		std::string layerName = m_mdl->stratigraphyManager().layerName(currentLayer);
		if (m_rank == 0)
		{
			LogHandler(LogHandler::INFO_SEVERITY) << "Running Fastcauldron to adjust the surface: " << nextSurface;
		}

		// Run Fastcauldron -----------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------//

		try { runFastCauldron(layerName, refDepths, tarTwt, newDepths); }
		catch (const T2Zexception & ex) { throw T2Zexception() << "runFastCauldron failed, error code: " << ex.what(); }

		//-----------------------------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------//

		// Go back to the master path
		if (!m_fullMasterPath.setPath())
		{
			throw T2Zexception() << " Cannot change to the master directory " << m_fullMasterPath.fullPath().path();
		}

		// Now write the corrected map (note that the project file path will be appended to the HDF map name,
		// so we must be in the master path to do this operation!)
		mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
		if ( IsValueUndefined( mapsMgrLocalReloaded.generateMap( "StratIoTbl", mapName, newDepths, mapsSequenceNbr, s_resultsFile ) ) )
		{
			throw T2Zexception() << " Cannot generate the map with corrected depths for the surface " << nextSurface;
		}

		// Copy the update map file to the master path
		MPI_Barrier(PETSC_COMM_WORLD);
		if (m_rank == 0)
		{
			m_masterResults.remove();
			if (!casePathResults.copyFile(m_masterResults))
			{
				throw T2Zexception() << "Cannot copy the result file to " << m_masterResults.fullPath().path();
			}
		}
		MPI_Barrier(PETSC_COMM_WORLD);
	}

	// save a copy of the depths of the bottom most surface to append the isopacks later
	m_depthsEndSurface.resize(newDepths.size());
	m_depthsEndSurface = newDepths;

	//-----------------------------------------------//
	// The end of the main depth calibration loop
	//-----------------------------------------------//
}


void FastDepthCalibration::writeFinalProject()
{
	m_mdl.reset(new mbapi::Model());
	if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(m_projectFileName.c_str()))
	{
		throw T2Zexception() << " wrong input file name ";
	}
	MPI_Barrier(PETSC_COMM_WORLD);
	mbapi::StratigraphyManager & strMgrMaster = m_mdl->stratigraphyManager();
	const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDFinal = strMgrMaster.surfacesIDs();

	// Modify StratIoTbl and GridMapIoTbl with the corrected new maps names
	try { modifyTables(surfacesIDFinal.size() - 1); }
	catch (const T2Zexception & ex) { throw T2Zexception() << "Tables could not be modified for the final project, error code: " << ex.what(); }

	// Create the final case
	ibs::FilePath finalProject = ibs::FilePath(m_finalResultsPath) << m_mdl->projectFileName();
	try { createCase(m_finalResultsPath, finalProject, m_finalResults); }
	catch (const T2Zexception & ex) { throw T2Zexception() << "Case could not be created for the final project, error code: " << ex.what(); }

	// Reload the model so the map manager gets updated (with the correct map sequence numbers)
	MPI_Barrier(PETSC_COMM_WORLD);
	m_mdl.reset(new mbapi::Model());
	if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(finalProject.path().c_str()))
	{
		throw ErrorHandler::Exception(m_mdl->errorCode()) << m_mdl->errorMessage();
	}
	MPI_Barrier(PETSC_COMM_WORLD);

	mbapi::MapsManager &         mapsMgrFinal = m_mdl->mapsManager();
	mbapi::StratigraphyManager & strMgrFinal = m_mdl->stratigraphyManager();

	// For the surfaces below the endSurface append the "isopacks" previously calculated
	for (mbapi::StratigraphyManager::SurfaceID s = m_endSurface + 1; s < surfacesIDFinal.size(); ++s)
	{
		std::string mapName = "IsoSurface_" + std::to_string(s);
		std::string surfaceName = strMgrFinal.surfaceName(s);
		if (m_rank == 0)
		{
			LogHandler(LogHandler::INFO_SEVERITY) << " Appending isopack for surface " << s;
		}
		size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
		for (size_t i = 0; i != m_depthsEndSurface.size(); ++i)
		{
			m_isoPacks[s][i] = m_depthsEndSurface[i] + m_isoPacks[s][i];
		}
		m_correctedMapsNames[s] = mapName;
		if ( IsValueUndefined( mapsMgrFinal.generateMap("StratIoTbl", mapName, m_isoPacks[s], mapsSequenceNbr, s_resultsFile ) ) )
		{
			throw T2Zexception() << " Cannot generate the map for the iso surface " << s;
		}
		m_correctedMapsSequenceNbr[s] = mapsSequenceNbr;
		// Modify record with the new name for the depth gris map     
		if (ErrorHandler::ReturnCode::NoError != m_mdl->setTableValue("StratIoTbl", s, "DepthGrid", mapName))
		{
			throw T2Zexception() << "Cannot set the map " << mapName << " as new depth iso surface in the StratIoTbl";
		}
	}

	// Save the project and input data to folder with the new maps
	MPI_Barrier(PETSC_COMM_WORLD);
	if (m_rank == 0)
	{
		m_mdl->saveModelToProjectFile(finalProject.path().c_str(), true);
	}
	MPI_Barrier(PETSC_COMM_WORLD);
}


// Modify the tables
void FastDepthCalibration::modifyTables(const mbapi::StratigraphyManager::SurfaceID nextSurface)
{
	const ErrorHandler::ReturnCode ok(ErrorHandler::ReturnCode::NoError);

	mbapi::StratigraphyManager & stMgr = m_mdl->stratigraphyManager();
	const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs = stMgr.surfacesIDs();

	// Change the deposequence from the top surface to nextSurface - 1
	for (mbapi::StratigraphyManager::SurfaceID s = 0; s < nextSurface; ++s)
	{
		long deposequence = nextSurface - s;
		if (NoError != m_mdl->setTableValue("StratIoTbl", s, "MobileLayer", (long)0)) { throw T2Zexception() << "Cannot set MobileLayer "; }
		if (NoError != m_mdl->setTableValue("StratIoTbl", s, "DepoSequence", deposequence)) { throw T2Zexception() << "Cannot set DepoSequence"; }
	}

	// Set the deposequence of the nextSurface surface to -9999
	if (NoError != m_mdl->setTableValue("StratIoTbl", nextSurface, "MobileLayer", (long)0)) { throw T2Zexception() << "Cannot set MobileLayer "; }
	if (NoError != m_mdl->setTableValue("StratIoTbl", nextSurface, "DepoSequence", (long)-9999)) { throw T2Zexception() << "Cannot set DepoSequence "; }

	// Clean the records below nextSurface (note that erase is used so the nextSurface + 1 record is deleted recursivly )
	for (mbapi::StratigraphyManager::SurfaceID s = nextSurface + 1; s < surfacesIDs.size(); ++s)
	{
		if (NoError != m_mdl->removeRecordFromTable("StratIoTbl", nextSurface + 1)) { throw T2Zexception() << "Cannot remove records from the StratIoTbl "; }
	}

	// Append the correct maps names if present
	for (auto it = m_correctedMapsNames.begin(); it != m_correctedMapsNames.end(); ++it)
	{
		// StratIoTbl
		if (NoError != m_mdl->setTableValue("StratIoTbl", it->first, "DepthGrid", it->second)) { throw T2Zexception() << "Cannot set DepthGrid "; }
		// GridMapIoTbl
		if (NoError != m_mdl->addRowToTable("GridMapIoTbl")) { throw T2Zexception() << "Cannot add a new row in GridMapIoTbl"; }
		if (NoError != m_mdl->setTableValue("GridMapIoTbl", m_correctedMapsIDs[it->first], "ReferredBy", "StratIoTbl")) { throw T2Zexception() << "Cannot set ReferredBy in GridMapIoTbl "; }
		if (NoError != m_mdl->setTableValue("GridMapIoTbl", m_correctedMapsIDs[it->first], "MapName", m_correctedMapsNames[it->first])) { throw T2Zexception() << "Cannot set MapName in GridMapIoTbl "; }
		if (NoError != m_mdl->setTableValue("GridMapIoTbl", m_correctedMapsIDs[it->first], "MapType", "HDF5")) { throw T2Zexception() << "Cannot set MapType in GridMapIoTbl "; }
		if (NoError != m_mdl->setTableValue("GridMapIoTbl", m_correctedMapsIDs[it->first], "MapFileName", s_resultsFile)) { throw T2Zexception() << "Cannot set MapFileName in GridMapIoTbl "; }
		if (NoError != m_mdl->setTableValue("GridMapIoTbl", m_correctedMapsIDs[it->first], "FileId", (long)-1)) { throw T2Zexception() << "Cannot set FileId in GridMapIoTbl "; }
		if (NoError != m_mdl->setTableValue("GridMapIoTbl", m_correctedMapsIDs[it->first], "MapSeqNbr", (long)m_correctedMapsSequenceNbr[it->first])) { throw T2Zexception() << "Cannot set MapSeqNbr in GridMapIoTbl "; }
	}
}


void FastDepthCalibration::prepareProject()
{

	// get project subsampling values
	long scX, scY;
	m_mdl->subsampling(scX, scY);

	// Set full resolution
	if (scX > 1 || scY > 1)
	{
		LogHandler(LogHandler::WARNING_SEVERITY) << "The project is subsampled. For the depth calibration the full resolution is required. "
			<< "Changing ScaleX and ScaleY to run in full resolution";
		if (NoError != m_mdl->setSubsampling(1, 1)) { throw T2Zexception() << "Cannot set subsampling to 1, 1 "; }
	}

	// Set full window
	long dimI, dimJ, winMinI, winMinJ, winMaxI, winMaxJ;

	m_mdl->hiresGridArealSize(dimI, dimJ);
	m_mdl->window(winMinI, winMaxI, winMinJ, winMaxJ);

	if (winMinI > 1 || winMinJ > 1 || winMaxI < dimI - 1 || winMaxJ < dimJ - 1)
	{
		LogHandler(LogHandler::WARNING_SEVERITY) << "The project is windowed. For the depth calibration the entire domain is required."
			<< " Changing WindowX and WindowY in to run the simulation over the entire domain";
		if (NoError != m_mdl->setWindow(1, dimI - 1, 1, dimJ - 1)) { throw T2Zexception() << "Cannot set window"; }
	}
	// Clear tables
	if (NoError != m_mdl->clearTable("TimeIoTbl")) { throw T2Zexception() << "Cannot clear the table TimeIoTbl "; }
	if (NoError != m_mdl->clearTable("3DTimeIoTbl")) { throw T2Zexception() << "Cannot clear the table 3DTimeIoTbl "; }
	if (NoError != m_mdl->clearTable("1DTimeIoTbl")) { throw T2Zexception() << "Cannot clear the table 1DTimeIoTbl "; }
	if (NoError != m_mdl->clearTable("ReservoirIoTbl")) { throw T2Zexception() << "Cannot clear the table ReservoirIoTbl "; }
	if (NoError != m_mdl->clearTable("MobLayThicknIoTbl")) { throw T2Zexception() << "Cannot clear the table MobLayThicknIoTbl "; }
	if (NoError != m_mdl->clearTable("FilterTimeIoTbl")) { throw T2Zexception() << "Cannot clear the table FilterTimeIoTbl "; }
	if (NoError != m_mdl->clearTable("SnapshotIoTbl")) { throw T2Zexception() << "Cannot clear the table SnapshotIoTbl "; }

	// Set the properties we want to save in the FilterTimeIoTbl
	if (NoError != m_mdl->propertyManager().requestPropertyInSnapshots("Depth", "SedimentsOnly")) { throw T2Zexception() << "Cannot set the table property Depth in the FilterTimeIoTbl "; }
	if (NoError != m_mdl->propertyManager().requestPropertyInSnapshots("TwoWayTime", "SedimentsOnly")) { throw T2Zexception() << "Cannot set the table property TwoWayTime in the FilterTimeIoTbl "; }
	if (NoError != m_mdl->propertyManager().requestPropertyInSnapshots("TwoWayTimeResidual", "SedimentsOnly")) { throw T2Zexception() << "Cannot set the table property TwoWayTimeResidual in the FilterTimeIoTbl "; }

}


// Run Fastcauldron and truncate the depths
void FastDepthCalibration::runFastCauldron( const std::string         & layerName,
											const std::vector<double> & refDepths,
											const std::vector<double> & tarTwt,
											std::vector<double>       & newDepths )
{
	// Instantiate FastcauldronStartup, prepare and sturtup
	FastcauldronStartup fastcauldronStartup(m_argc, m_argv);

	if (!fastcauldronStartup.getPrepareStatus()) { throw T2Zexception() << "Fail to prepare fastcauldron run "; }

	if (!fastcauldronStartup.getStartUpStatus()) { throw T2Zexception() << "Fail to startup fastcauldron "; }

	// Run Fastcauldroun
	fastcauldronStartup.run();

	// Get the run status
	if (!fastcauldronStartup.getRunStatus()) { throw T2Zexception() << "Fail to run fastcauldron "; }

	// if fastcauldron run successfully adjust the depth
	FastcauldronSimulator & fcApp = FastcauldronSimulator::getInstance();
	const DataAccess::Interface::Formation * formation = fcApp.findFormation(layerName);
	
	// get the present day snapshot
	const Interface::Snapshot              * presentDaySnapshot = fcApp.findOrCreateSnapshot(0.0);
	if (presentDaySnapshot == 0) { throw T2Zexception() << "No present day snapshot found"; }

	// get the depth
	const DataAccess::Interface::Property * depthProperty = fcApp.findProperty("Depth");
	std::unique_ptr<const DataAccess::Interface::PropertyValueList> depthPropertyValues(fcApp.getPropertyValues(FORMATION, depthProperty, presentDaySnapshot, 0, formation, 0, VOLUME));

	// get the two way time
	const DataAccess::Interface::Property * twtProperty = fcApp.findProperty("TwoWayTime");
	std::unique_ptr<const DataAccess::Interface::PropertyValueList> twtPropertyValues(fcApp.getPropertyValues(FORMATION, twtProperty, presentDaySnapshot, 0, formation, 0, VOLUME));

	if ((*depthPropertyValues).size() != 1) { throw T2Zexception() << "Invalid size of the depth property value"; }
	if ((*twtPropertyValues).size() != 1) { throw T2Zexception() << "Invalid size of the twt property value"; }

	const DataAccess::Interface::GridMap * twtGridMap = (*(twtPropertyValues.get()))[0]->getGridMap();
	const DataAccess::Interface::GridMap * depthGridMap = (*(depthPropertyValues.get()))[0]->getGridMap();

	std::vector<double> twtSim(refDepths.size());
	std::vector<double> twtSimUpper(refDepths.size());
	std::vector<double> depthSim(refDepths.size());
	std::vector<double> depthSimUpper(refDepths.size());

	// Start from the top
	unsigned int maxK = twtGridMap->getDepth() - 1;
	fillArray(twtGridMap, twtSim, maxK, s_twtconvFactor);
	fillArray(depthGridMap, depthSim, maxK, 1.0);

	bool searching = false;
	for (size_t i = 0; i < newDepths.size(); ++i)
	{
		newDepths[i] = DataAccess::Interface::DefaultUndefinedScalarValue;
		if (twtSim[i] > tarTwt[i])
		{
			newDepths[i] = abs(twtSim[i]) > 1e-12 ? (depthSim[i] / twtSim[i] * tarTwt[i]) : depthSim[i];
		}
		if (!searching && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue) { searching = true; }
	}

	twtSimUpper = twtSim;
	depthSimUpper = depthSim;
	for (int k = maxK - 1; k >= 0; --k)
	{
		if (!searching) break; // all depths are set, no need to retrive other data
		searching = false;

		fillArray(twtGridMap, twtSim, k, s_twtconvFactor);
		fillArray(depthGridMap, depthSim, k, 1.0);

		for (size_t i = 0; i != newDepths.size(); ++i)
		{
			if (twtSim[i] > tarTwt[i] && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue)
			{
				double twtSimDiff = twtSim[i] - twtSimUpper[i];
				double twtMeasDiffUpper = tarTwt[i] - twtSimUpper[i];
				double twtMeasDiffLower = twtSim[i] - tarTwt[i];

				double depthSimDiff = depthSim[i] - depthSimUpper[i];

				if (abs(twtSimDiff) > 1e-12)
				{
					newDepths[i] = depthSimDiff / twtSimDiff * twtMeasDiffUpper + depthSimUpper[i];
				}
				else
				{
					newDepths[i] = abs(twtMeasDiffLower) < abs(twtMeasDiffUpper) ? depthSim[i] : depthSimUpper[i];
				}
			}
			if (!searching && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue) { searching = true; }
		}
		twtSimUpper = twtSim;
		depthSimUpper = depthSim;
	}

	for (size_t i = 0; i < newDepths.size(); ++i)
	{
		// Set to the last deepmost simulated depth (k == 0)
		if (DataAccess::Interface::DefaultUndefinedScalarValue == newDepths[i]) { newDepths[i] = depthSim[i]; }
		// If everything goes wrong, use the refDepths value 
		if (newDepths[i] - refDepths[i] < 0.0) { newDepths[i] = refDepths[i]; }
	}

	// Finalize fastcauldronStartup
	fastcauldronStartup.finalize();
}


void FastDepthCalibration::fillArray( const DataAccess::Interface::GridMap * grid, std::vector<double> & v, int k, const double convFact )
{
   grid->retrieveData( );
   int numI = grid->lastI( ) + 1;

   for ( unsigned int j = grid->firstJ( ); j <= grid->lastJ( ); ++j )
   {
      for ( unsigned int i = grid->firstI( ); i <= grid->lastI( ); ++i )
      {
         v[i + j * numI] = grid->getValue( i, j, (unsigned int)k ) * convFact;
      }
   }
   grid->restoreData( );
}


// create a case folder with the input files
void FastDepthCalibration::createCase(
   ibs::FolderPath & casePath,
   ibs::FilePath   & caseProject,
   ibs::FilePath   & casePathResults )
{
	// Save the project and input data to folder (remove it if it is already present). 
	// The file containing the maps needs to be copied separatly because the map manager does not know it yet (only after the reload).
	// Only rank 0 must perform this operation.
	if (m_rank == 0)
	{
		if (casePath.exists())
		{
			LogHandler(LogHandler::WARNING_SEVERITY) << "Folder " << casePath.fullPath().path() << " will be deleted";
			casePath.remove();
		}
		casePath.create();

		if (ErrorHandler::NoError != m_mdl->saveModelToProjectFile(caseProject.path().c_str(), true))
		{
			throw ErrorHandler::Exception(m_mdl->errorCode()) << m_mdl->errorMessage();
		}

		if (!casePathResults.exists() && !m_masterResults.copyFile(casePathResults))
		{
			throw T2Zexception() << "Fail to copy file: " << casePathResults.path();
		}
	}
}
