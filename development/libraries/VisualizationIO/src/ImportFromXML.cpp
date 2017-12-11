//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ImportFromXML.h"
#include "VisualizationIO_native.h"
#include "DataStore.h"
#include "FilePath.h"
#include "FolderPath.h"

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace CauldronIO;

//////////////////////////////////////////////////////////////////////////
/// Importing from native format
//////////////////////////////////////////////////////////////////////////

std::shared_ptr<Project> CauldronIO::ImportFromXML::importFromXML(const std::string& filename)
{
    if (!ibs::FilePath(filename).exists())
        throw CauldronIOException("Cannot open file");
    
    pugi::xml_document doc;
    std::cout << "Reading XML into pugiXML DOM" << std::endl;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (!result)
        throw CauldronIOException("Error during parsing xml file");

        
    ImportFromXML importExport(ibs::FilePath(filename).filePath());
    std::shared_ptr<Project> project;
    
    try
    {
       project = importExport.getProject(doc);
    }
    catch (CauldronIOException& excp)
    {
        // Just rethrow it
        throw excp;
    }
    catch (...)
    {
        throw CauldronIOException("unspecified error during xml parse");
    }
    
    return project;
}

CauldronIO::ImportFromXML::ImportFromXML(const ibs::FilePath& absPath)
    : m_absPath(absPath)
{
}

std::shared_ptr<Project> CauldronIO::ImportFromXML::getProject(const pugi::xml_document& ptDoc)
{
    pugi::xml_node pt = ptDoc.child("project");
    if (!pt) 
        throw CauldronIOException("Invalid xml file or format");

    std::string projectName = pt.child_value("name");
    std::string projectDescript = pt.child_value("description");
    std::string projectVersion = pt.child_value("programversion");
    std::string outputPath = pt.child_value("outputpath");
    
    ModellingMode mode = (ModellingMode)pt.child("modelingmode").text().as_int();

    // Check XML versions
    // dataXmlVersion > xml_version : actual data generated with code newer than this code, requires forward compatibility
    // dataXmlVersion < xml_version : actual data generated with code older than this code, requires backward compatibility
    //////////////////////////////////////////////////////////////////////////

    int dataXmlVersionMinor = 0;
    int dataXmlVersionMajor = 0;
    pugi::xml_node xmlOptional = pt.child("xml-version");
    if (xmlOptional)
    {
        dataXmlVersionMinor = xmlOptional.attribute("minor").as_int();
        dataXmlVersionMajor = xmlOptional.attribute("major").as_int();
    }

    bool forwardCompatible = dataXmlVersionMajor > xml_version_major || (dataXmlVersionMajor == xml_version_major && dataXmlVersionMinor > xml_version_minor);
    bool backwardCompatible = dataXmlVersionMajor < xml_version_major || (dataXmlVersionMajor == xml_version_major && dataXmlVersionMinor < xml_version_minor);

    if (forwardCompatible)
        throw CauldronIOException("Xml format not forward compatible");
    if (backwardCompatible)
        throw CauldronIOException("Xml format not backward compatible"); // we should try to fix that when the time comes

    ibs::FilePath fullOutputPath(m_absPath);
    if (m_absPath.path() != ".")
    {
        fullOutputPath << outputPath;
    }
    else
    {
        fullOutputPath = ibs::FilePath(outputPath);
    }

    // Create the project
    m_project.reset(new Project(projectName, projectDescript, projectVersion, mode, dataXmlVersionMajor, dataXmlVersionMinor));

    // Read all properties
    pugi::xml_node propertiesNode = pt.child("properties");
    for (pugi::xml_node propertyNode = propertiesNode.child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
    {
        std::shared_ptr<const Property> property = getProperty(propertyNode);
        m_project->addProperty(property);
    }
    
    // Read all geometries
    pugi::xml_node geometriesNode = pt.child("geometries");
    if (!geometriesNode)
    {
        throw CauldronIOException("Cannot find any geometries, please reconvert your project to reflect latest xml format");
    }
    for (pugi::xml_node geometryNode = geometriesNode.child("geometry"); geometryNode; geometryNode = geometryNode.next_sibling("geometry"))
    {
        std::shared_ptr<const Geometry2D> geometry = getGeometry2D(geometryNode);
        m_project->addGeometry(geometry);
    }

    // Read all formations
    pugi::xml_node formationsNode = pt.child("formations");
    for (pugi::xml_node formationNode = formationsNode.child("formation"); formationNode; formationNode = formationNode.next_sibling("formation"))
    {
        std::shared_ptr<Formation> formation = getFormation(formationNode, fullOutputPath);
        m_project->addFormation(formation);
    }

    // Read all reservoirs
    pugi::xml_node hasReservoirs = pt.child("reservoirs");
    if (hasReservoirs)
    {
        for (pugi::xml_node reservoirNode = hasReservoirs.child("reservoir"); reservoirNode; reservoirNode = reservoirNode.next_sibling("reservoir"))
        {
            std::shared_ptr<const Reservoir> reservoir = getReservoir(reservoirNode);
            m_project->addReservoir(reservoir);
        }
    }

    // Read strat table
    pugi::xml_node stratTableNode = pt.child("stratigraphytable");
    if (stratTableNode)
    {
        for (pugi::xml_node_iterator it = stratTableNode.begin(); it != stratTableNode.end(); ++it)
        {
            pugi::xml_node entry = (*it);
            if (std::string(entry.name()) == "surface")
            {
                std::shared_ptr<Surface> surface = getSurface(entry, fullOutputPath);
                StratigraphyTableEntry item;
                item.setSurface(surface);
                m_project->addStratigraphyTableEntry(item);
            }
            else if (std::string(entry.name()) == "formation")
            {
                // Get the name and find it in the list
                const std::string formationName(entry.attribute("name").as_string());
                std::shared_ptr<Formation> formation = m_project->findFormation(formationName);
                assert(formation);

                StratigraphyTableEntry item;
                item.setFormation(formation);
                m_project->addStratigraphyTableEntry(item);
            }
        }

        // Loop over formations to connect surfaces; surfaces should already be connected
        for (size_t index = 1; index < m_project->getStratigraphyTable().size(); index += 2)
        {
            const CauldronIO::StratigraphyTableEntry& entry = m_project->getStratigraphyTable().at(index);
            assert(bool(entry.getFormation()));

            std::shared_ptr<CauldronIO::Surface> topSurface = m_project->getStratigraphyTable().at(index - 1).getSurface();
            std::shared_ptr<CauldronIO::Formation> formation = entry.getFormation();

            formation->setTopSurface(topSurface.get());

            if (index + 1 < m_project->getStratigraphyTable().size())
            {
                std::shared_ptr<CauldronIO::Surface> bottomSurface = m_project->getStratigraphyTable().at(index + 1).getSurface();
                formation->setBottomSurface(bottomSurface.get());
            }
        }
    }

    // Parse migration event table
    pugi::xml_node migrationEventsNode = pt.child("migrationEvents");
    if (migrationEventsNode)
    {
       size_t nrEvents = (size_t)migrationEventsNode.attribute("number").as_int();
       size_t record_size = migrationEventsNode.attribute("record_size").as_int();
       size_t totalSize = nrEvents * record_size;

       if (record_size != sizeof(MigrationEvent))
          throw CauldronIOException("Invalid record size for MigrationEvent");
       
       // Uncompress the data
       pugi::xml_node datastoreNode = migrationEventsNode.child("datastore");
       DataStoreParams *params = DataStoreLoad::getDatastoreParams(datastoreNode, fullOutputPath);
       DataStoreLoad datastore(params);
       char* data = (char*)datastore.getData(totalSize);
       
       // Reconstruct migrationEvents
       size_t dataIndex = 0;
       for (size_t index = 0; index < nrEvents; ++index, dataIndex += record_size)
       {
          std::shared_ptr<MigrationEvent> event(new MigrationEvent());
          
          void* source = (void*)(&data[dataIndex]);
          void* dest = (void*)(event.get());
          memcpy(dest, source, record_size);
          
          m_project->addMigrationEvent(event);
       }
       
       delete params;
       delete[] data;
    }
    
    // Parse trapper table
    pugi::xml_node trapperNode = pt.child("trapper");
    if (trapperNode)
    {
       size_t nrEvents = (size_t)trapperNode.attribute("number").as_int();
       size_t record_size = trapperNode.attribute("record_size").as_int();
       size_t totalSize = nrEvents * record_size;

       if (record_size != sizeof(Trapper))
          throw CauldronIOException("Invalid record size for Trapper");
       
       // Uncompress the data
       pugi::xml_node datastoreNode = trapperNode.child("datastore");
       DataStoreParams *params = DataStoreLoad::getDatastoreParams(datastoreNode, fullOutputPath);
       DataStoreLoad datastore(params);
       char* data = (char*)datastore.getData(totalSize);
       
       // Reconstruct trapper
       size_t dataIndex = 0;
       for (size_t index = 0; index < nrEvents; ++index, dataIndex += record_size)
       {
          std::shared_ptr<Trapper> event(new Trapper());
          
          void* source = (void*)(&data[dataIndex]);
          void* dest = (void*)(event.get());
          memcpy(dest, source, record_size);
          
          m_project->addTrapper(event);
       }
       
       delete params;
       delete[] data;
    }

    // Parse trap table
    pugi::xml_node trapNode = pt.child("trap");
    if (trapNode)
    {
       size_t nrEvents = (size_t)trapNode.attribute("number").as_int();
       size_t record_size = trapNode.attribute("record_size").as_int();
       size_t totalSize = nrEvents * record_size;

       if (record_size != sizeof(Trap))
          throw CauldronIOException("Invalid record size for Trap");
       
       // Uncompress the data
       pugi::xml_node datastoreNode = trapNode.child("datastore");
       DataStoreParams *params = DataStoreLoad::getDatastoreParams(datastoreNode, fullOutputPath);
       DataStoreLoad datastore(params);
       char* data = (char*)datastore.getData(totalSize);
       
       // Reconstruct trap
       size_t dataIndex = 0;
       for (size_t index = 0; index < nrEvents; ++index, dataIndex += record_size)
       {
          std::shared_ptr<Trap> event(new Trap());
          
          void* source = (void*)(&data[dataIndex]);
          void* dest = (void*)(event.get());
          memcpy(dest, source, record_size);
          
          m_project->addTrap(event);
       }
       
       delete params;
       delete[] data;
    }

    // Parse genex history files references
    pugi::xml_node genexHistoryNode = pt.child("genexHistoryFiles");
    if (genexHistoryNode)
    {
       for (pugi::xml_node_iterator it = genexHistoryNode.begin(); it != genexHistoryNode.end(); ++it)
       {
          pugi::xml_node entry = (*it);
          if( std::string(entry.name()) == "filepath" ) {
             const std::string fileName(entry.attribute("file").as_string());
             m_project->addGenexHistoryRecord( fileName );
          }
       }
    }
    
    // Parse burial history files references
    pugi::xml_node burialHistoryNode = pt.child("burialHistoryFiles");
    if (burialHistoryNode)
    {
       for (pugi::xml_node_iterator it = burialHistoryNode.begin(); it != burialHistoryNode.end(); ++it)
       {
          pugi::xml_node entry = (*it);
          if( std::string(entry.name()) == "filepath" ) {
             const std::string fileName(entry.attribute("file").as_string());
             m_project->addBurialHistoryRecord( fileName );
          }
       }
    }
    
    // Parse mass balance file reference
    pugi::xml_node massBalanceNode = pt.child("massBalance");
    if (massBalanceNode)
    {
       m_project->setMassBalance ( massBalanceNode.attribute("file").as_string() );
    }
    
    // Parse display contour table
    pugi::xml_node displayContourNode = pt.child("displayContour");
    if (displayContourNode)
    {
       for (pugi::xml_node oneRecord = displayContourNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<DisplayContour> entry(new DisplayContour);
          std::string name = oneRecord.attribute("propertyName").value();
          std::string colour = oneRecord.attribute("colour").value();
          float value = oneRecord.attribute("value").as_float();
          
          entry->setPropertyName(name);
          entry->setContourColour(colour);
          entry->setContourValue(value);
          
          m_project->addDisplayContour(entry);
       }
    }

    // Parse TemperatureIso table
    pugi::xml_node temperatureIsoNode = pt.child("temperatureIso");
    if (temperatureIsoNode)
    {
       
       for (pugi::xml_node oneRecord = temperatureIsoNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<IsoEntry> entry(new IsoEntry);
          
          float age = oneRecord.attribute("age").as_float();
          float value = oneRecord.attribute("value").as_float();
          double sum = oneRecord.attribute("sum").as_double();
          int numberOfPoints = oneRecord.attribute("NP").as_int();
          
          entry->setAge(age);
          entry->setNP(numberOfPoints);
          entry->setContourValue(value);
          entry->setSum(sum);
          
          m_project->addTemperatureIsoEntry(entry);
       }
    }

    // Parse VRIso table
    pugi::xml_node vrIsoNode = pt.child("vrIso");
    if (vrIsoNode)
    {
       for (pugi::xml_node oneRecord = vrIsoNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<IsoEntry> entry(new IsoEntry);
          
          float age = oneRecord.attribute("age").as_float();
          float value = oneRecord.attribute("value").as_float();
          double sum = oneRecord.attribute("sum").as_double();
          int numberOfPoints = oneRecord.attribute("NP").as_int();
          
          entry->setAge(age);
          entry->setNP(numberOfPoints);
          entry->setContourValue(value);
          entry->setSum(sum);
          
          m_project->addVrIsoEntry(entry);
          
       }
    }
    
    // Parse FtSample table
    pugi::xml_node ftNode = pt.child("ftSample");
    if (ftNode)
    {
       
       for (pugi::xml_node oneRecord = ftNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<FtSample> entry(new FtSample);
          
          std::string id = oneRecord.attribute("id").value();
          entry->setFtSampleId(id);
          float depthInd = oneRecord.attribute("depthInd").as_float();
          entry->setDepthIndex(depthInd);
          float zeta = oneRecord.attribute("zeta").as_float();
          entry->setFtZeta(zeta);
          float ustglTrackDensity = oneRecord.attribute("ustglTrackDensity").as_float();
          entry->setFtUstglTrackDensity(ustglTrackDensity);
          float predictedAge = oneRecord.attribute("predictedAge").as_float();
          entry->setFtPredictedAge(predictedAge);
          float pooledAge = oneRecord.attribute("pooledAge").as_float();
          entry->setFtPooledAge(pooledAge);
          float pooledAgeErr = oneRecord.attribute("pooledAgeErr").as_float();
          entry->setFtPooledAgeErr(pooledAgeErr);
          float ageChi2 = oneRecord.attribute("ageChi2").as_float();
          entry->setFtAgeChi2(ageChi2);
          int degreeOfFreedom = oneRecord.attribute("degreeOfFreedom").as_int();
          entry->setFtDegreeOfFreedom(degreeOfFreedom);
          float pAgeChi2 = oneRecord.attribute("pAgeChi2").as_float();
          entry->setFtPAgeChi2(pAgeChi2);
          float corrCoeff = oneRecord.attribute("corrCoeff").as_float();
          entry->setFtCorrCoeff(corrCoeff);
          float varianceSqrtNs = oneRecord.attribute("varianceSqrtNs").as_float();
          entry->setFtVarianceSqrtNs(varianceSqrtNs);
          float varianceSqrtNi = oneRecord.attribute("varianceSqrtNi").as_float();
          entry->setFtVarianceSqrtNi(varianceSqrtNi);
          float nsDivNiErr = oneRecord.attribute("nsDivNiErr").as_float();
          entry->setFtNsDivNiErr(nsDivNiErr);
          float nsDivNi = oneRecord.attribute("nsDivNi").as_float();
          entry->setFtNsDivNi(nsDivNi);
          float meanRatio = oneRecord.attribute("meanRatio").as_float();
          entry->setFtMeanRatio(meanRatio);
          float meanRatioErr = oneRecord.attribute("meanRatioErr").as_float();
          entry->setFtMeanRatioErr(meanRatioErr);
          float centralAge = oneRecord.attribute("centralAge").as_float();
          entry->setFtCentralAge(centralAge);
          float centralAgeErr = oneRecord.attribute("centralAgeErr").as_float();
          entry->setFtCentralAgeErr(centralAgeErr);
          float meanAge = oneRecord.attribute("meanAge").as_float();
          entry->setFtMeanAge(meanAge);
          float meanAgeErr = oneRecord.attribute("meanAgeErr").as_float();
          entry->setFtMeanAgeErr(meanAgeErr);
          float lengthChi2 = oneRecord.attribute("lengthChi2").as_float();
          entry->setFtLengthChi2(lengthChi2);
          std::string apatiteYield  = oneRecord.attribute("apatiteYield").value();
          entry->setFtApatiteYield(apatiteYield);
          
          m_project->addFtSample(entry);
          
       }
    }
    // Parse FtGrain table
    pugi::xml_node ftgNode = pt.child("ftGrain");
    if (ftgNode)
    {
       
       for (pugi::xml_node oneRecord = ftgNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<FtGrain> entry(new FtGrain);
          
          std::string id = oneRecord.attribute("sampleId").value();
          entry->setFtSampleId(id);
          int grainId = oneRecord.attribute("grainId").as_int();
          entry->setFtGrainId(grainId);
          int trackNo = oneRecord.attribute("spontTrackNo").as_int();
          entry->setFtSpontTrackNo(trackNo);
          int itrackNo = oneRecord.attribute("inducedTrackNo").as_int();
          entry->setFtInducedTrackNo(itrackNo);
          float clWeightPerc = oneRecord.attribute("clWeightPerc ").as_float();
          entry->setFtClWeightPerc(clWeightPerc);
          float grainAge = oneRecord.attribute("grainAge").as_float();
          entry->setFtGrainAge(grainAge);
          
          m_project->addFtGrain(entry);
          
       }
    }
    // Parse FtPredLengthCountsHist table
    pugi::xml_node ftpNode = pt.child("ftPredLengthCountsHist");
    if(ftpNode)
    {
       
       for (pugi::xml_node oneRecord = ftpNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<FtPredLengthCountsHist> entry(new FtPredLengthCountsHist);
          
          std::string id = oneRecord.attribute("sampleId").value();
          entry->setFtSampleId(id);
          int histId = oneRecord.attribute("id").as_int();
          entry->setFtPredLengthHistId(histId);
          float clWeightPerc = oneRecord.attribute("clWeightPerc").as_float();
          entry->setFtClWeightPerc(clWeightPerc);
          float binStart = oneRecord.attribute("binStart").as_float();
          entry->setFtPredLengthBinStart(binStart);
          float binWidth = oneRecord.attribute("binWidth").as_float();
          entry->setFtPredLengthBinStart(binWidth);
          int binNum = oneRecord.attribute("binNum").as_int();
          entry->setFtPredLengthBinNum(binNum);
          
          m_project->addFtPredLengthCountsHist(entry);
          
       }
    }
    // Parse FtPredLengthCountsHistData table
    pugi::xml_node ftpdNode = pt.child("ftPredLengthCountsHistData");
    if(ftpdNode)
    {
       
       for (pugi::xml_node oneRecord = ftpdNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<FtPredLengthCountsHistData> entry(new FtPredLengthCountsHistData);
          
          int histId = oneRecord.attribute("id").as_int();
          entry->setFtPredLengthHistId(histId);
          int binIndex = oneRecord.attribute("binIndex").as_int();
          entry->setFtPredLengthBinIndex(binIndex);
          float binCount = oneRecord.attribute("binCount").as_float();
          entry->setFtPredLengthBinCount(binCount);
          
          m_project->addFtPredLengthCountsHistData(entry);
          
       }
    }
    // Parse FtClWeightPercBins table
    pugi::xml_node ftbNode = pt.child("ftClWeightPercBins");
    if(ftbNode)
    {
       
       for (pugi::xml_node oneRecord = ftbNode.child("record"); oneRecord; oneRecord = oneRecord.next_sibling("record"))
       {
          std::shared_ptr<FtClWeightPercBins> entry(new FtClWeightPercBins);
          
          double start = oneRecord.attribute("start").as_double();
          entry->setFtClWeightBinStart(start);
          double width = oneRecord.attribute("width").as_double();
          entry->setFtClWeightBinWidth(width);
          
          m_project->addFtClWeightPercBins(entry);
       }
    }
    // Parse timeIo1D table
    pugi::xml_node timeIo1DNode = pt.child("timeIo1D");
    if (timeIo1DNode)
    {
       size_t nrEvents = (size_t)timeIo1DNode.attribute("number").as_int();
       size_t record_size = timeIo1DNode.attribute("record_size").as_int();
       size_t totalSize = nrEvents * record_size;

       if (record_size != sizeof(TimeIo1D))
          throw CauldronIOException("Invalid record size for TimeIo1D");
       
       // Uncompress the data
       pugi::xml_node datastoreNode = timeIo1DNode.child("datastore");
       DataStoreParams *params = DataStoreLoad::getDatastoreParams(datastoreNode, fullOutputPath);
       DataStoreLoad datastore(params);
       char* data = (char*)datastore.getData(totalSize);
       
       // Reconstruct the record
       size_t dataIndex = 0;
       for (size_t index = 0; index < nrEvents; ++index, dataIndex += record_size)
       {
          std::shared_ptr<TimeIo1D> event(new TimeIo1D);
          void* source = (void*)(&data[dataIndex]);
          void* dest = (void*)(event.get());
          memcpy(dest, source, record_size);
          
          m_project->add1DTimeIo(event);
       }
       
       delete params;
       delete[] data;
    }

    // Read all snapshots
    pugi::xml_node snapshotsNode = pt.child("snapshots");
    for (pugi::xml_node snapShotNode = snapshotsNode.child("snapshot"); snapShotNode; snapShotNode = snapShotNode.next_sibling("snapshot"))
    {
        double age = snapShotNode.attribute("age").as_double();
        SnapShotKind kind = (SnapShotKind)snapShotNode.attribute("kind").as_int();
        bool isminor = snapShotNode.attribute("isminor").as_bool();

        // Create the snapshot
        std::shared_ptr<SnapShot> snapShot(new SnapShot(age, kind, isminor));

        // Find all surfaces
        /////////////////////////////////////////
        pugi::xml_node hasSurfaces = snapShotNode.child("surfaces");
        if (hasSurfaces)
        {
            for (pugi::xml_node surfaceNode = hasSurfaces.child("surface"); surfaceNode; surfaceNode = surfaceNode.next_sibling("surface"))
            {
                std::shared_ptr<Surface> surface = getSurface(surfaceNode, fullOutputPath);
                
                // Add to snapshot
                snapShot->addSurface(surface);
            }
        }
        
        // Find all (continuous) volumes
        /////////////////////////////////////////
        pugi::xml_node hasVolumes = snapShotNode.child("volume");
        if (hasVolumes)
        {
            std::shared_ptr<Volume> volume = getVolume(hasVolumes, fullOutputPath);
            snapShot->setVolume(volume);
        }

        // Get formation volumes
        //////////////////////////////////////////////////////////////////////////
        pugi::xml_node hasFormationVolumes = snapShotNode.child("formvols");

        // Get all property volume data
        if (hasFormationVolumes)
        {
            for (pugi::xml_node formVolNode = hasFormationVolumes.child("formvol"); formVolNode; formVolNode = formVolNode.next_sibling("formvol"))
            {
                std::string formationName = formVolNode.attribute("formation").value();
                std::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName);
                assert(formationIO);

                // There should be a volume
                pugi::xml_node volumeNode = formVolNode.child("volume");
                std::shared_ptr<Volume> volume = getVolume(volumeNode, fullOutputPath);

                // Add it to the list
                FormationVolume formVolume(formationIO, volume);
                snapShot->addFormationVolume(formVolume);
            }
        }

        // Get trappers
        //////////////////////////////////////////////////////////////////////////

        pugi::xml_node hasTrappers = snapShotNode.child("trappers");
        if (hasTrappers)
        {
            for (pugi::xml_node trapperNode = hasTrappers.child("trapper"); trapperNode; trapperNode = trapperNode.next_sibling("trapper"))
            {
                int ID = trapperNode.attribute("id").as_int();
                int persistentID = trapperNode.attribute("persistentID").as_int(); 
                int downstreamTrapperID = trapperNode.attribute("downstreamtrapper").as_int(); 
                float depth = trapperNode.attribute("depth").as_float();
                float spillDepth = trapperNode.attribute("spillDepth").as_float(); 
                std::string reservoirname = trapperNode.attribute("reservoirname").value();
                        
                float x      = trapperNode.attribute("posX").as_float();
                float y      = trapperNode.attribute("posY").as_float();
                float spillX = trapperNode.attribute("spillPosX").as_float();
                float spillY = trapperNode.attribute("spillPosY").as_float();
                float goc    = trapperNode.attribute("goc").as_float();
                float owc    = trapperNode.attribute("owc").as_float();

                std::shared_ptr<Trapper> trapperIO(new Trapper(ID, persistentID));
                trapperIO->setDownStreamTrapperID(downstreamTrapperID);
                trapperIO->setReservoirName(reservoirname);
                trapperIO->setSpillDepth(spillDepth);
                trapperIO->setSpillPointPosition(spillX, spillY);
                trapperIO->setDepth(depth);
                trapperIO->setPosition(x, y);
                trapperIO->setOWC(owc);
                trapperIO->setGOC(goc);

                snapShot->addTrapper(trapperIO);
            }
        }

        m_project->addSnapShot(snapShot);
    }
    
    return m_project;
}

std::shared_ptr<const Reservoir> CauldronIO::ImportFromXML::getReservoir(pugi::xml_node reservoirNode) const
{
    std::shared_ptr<Reservoir> reservoir;

    std::string name = reservoirNode.attribute("name").value();
    std::string formation = reservoirNode.attribute("formation").value(); 

    std::shared_ptr<const Formation> formationIO = m_project->findFormation(formation);
    assert(formationIO);

    reservoir.reset(new Reservoir(name, formationIO));
    return reservoir;
}

std::shared_ptr<const Geometry2D> CauldronIO::ImportFromXML::getGeometry2D(pugi::xml_node geometryNode) const
{
    std::shared_ptr<const Geometry2D> geometry;

    double minI, minJ, deltaI, deltaJ;
    size_t numI, numJ;

    numI = (size_t)geometryNode.attribute("numI").as_uint();
    numJ = (size_t)geometryNode.attribute("numJ").as_uint();
    minI =   geometryNode.attribute("minI").as_double();
    minJ =   geometryNode.attribute("minJ").as_double();
    deltaI = geometryNode.attribute("deltaI").as_double();
    deltaJ = geometryNode.attribute("deltaJ").as_double();

    // Cell-centered?
    bool cellCentered = false;
    pugi::xml_attribute isCellCenteredNode = geometryNode.attribute("cell-centered");
    if (isCellCenteredNode)
    {
        cellCentered = isCellCenteredNode.as_bool();
    }

    return std::shared_ptr<const Geometry2D>(new Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ, cellCentered));
}

std::shared_ptr<Volume> CauldronIO::ImportFromXML::getVolume(pugi::xml_node volumeNode, const ibs::FilePath& path)
{
    SubsurfaceKind surfaceKind = (SubsurfaceKind)volumeNode.attribute("subsurfacekind").as_int();

    // Create the volume
    std::shared_ptr<Volume> volume(new Volume(surfaceKind));

    // Get all property volume data
    pugi::xml_node propertyVolNodes = volumeNode.child("propertyvols");
    assert(propertyVolNodes);

    for (pugi::xml_node propertyVolNode = propertyVolNodes.child("propertyvol"); propertyVolNode; propertyVolNode = propertyVolNode.next_sibling("propertyvol"))
    {
        std::string propertyName = propertyVolNode.attribute("property").value();
        std::shared_ptr<const Property> property = m_project->findProperty(propertyName);
        assert(property);

        // Get the geometry
        size_t geometryIndex = (size_t)propertyVolNode.attribute("geom-index").as_int();
        std::shared_ptr<const Geometry2D> geometry = m_project->getGeometries().at(geometryIndex);

        // Get K range
        pugi::xml_attribute firstK_attrib = propertyVolNode.attribute("firstK");
        if (!firstK_attrib) throw CauldronIOException("Cannot find firstK attribute in property-volume");
        size_t firstK = firstK_attrib.as_uint();
        pugi::xml_attribute numK_attrib = propertyVolNode.attribute("numK");
        if (!numK_attrib) throw CauldronIOException("Cannot find numK attribute in property-volume");
        size_t numK = numK_attrib.as_uint();

        // Get min/max value
        float minValue = DefaultUndefinedValue;
        float maxValue = DefaultUndefinedValue;

        pugi::xml_attribute minValueAttr = propertyVolNode.attribute("min");
        if (minValueAttr)
            minValue = minValueAttr.as_float();

        pugi::xml_attribute maxValueAttr = propertyVolNode.attribute("max");
        if (maxValueAttr)
            maxValue = maxValueAttr.as_float();

        // Create the VolumeData
        std::shared_ptr<Geometry3D> geometry3D(new Geometry3D(geometry->getNumI(), geometry->getNumJ(), numK, firstK, geometry->getDeltaI(),
            geometry->getDeltaJ(), geometry->getMinI(), geometry->getMinJ()));
        std::shared_ptr<VolumeData> volData(new VolumeDataNative(geometry3D, minValue, maxValue));

	pugi::xml_attribute sedimentMinValueAttr = propertyVolNode.attribute("sedimentMin");
	pugi::xml_attribute sedimentMaxValueAttr = propertyVolNode.attribute("sedimentMax");
	if(sedimentMinValueAttr && sedimentMaxValueAttr)
	{
	  volData->setSedimentMinMax(
	    sedimentMinValueAttr.as_float(),
	    sedimentMaxValueAttr.as_float());
	}

        // Get the datastore xml node or constantvalue
        pugi::xml_attribute constantVal = propertyVolNode.attribute("constantvalue");
        if (constantVal)
            volData->setConstantValue(constantVal.as_float());
        else
            DataStoreLoad::getVolume(propertyVolNode, volData, path);

        PropertyVolumeData propVolData(property, volData);
        volume->addPropertyVolumeData(propVolData);
    }
    
    return volume;
}

CauldronIO::PropertySurfaceData CauldronIO::ImportFromXML::getPropertySurfaceData(pugi::xml_node &propertyMapNode, const ibs::FilePath& fullOutputPath) const
{
    std::string propertyName = propertyMapNode.attribute("property").value();
    std::shared_ptr<const Property> property = m_project->findProperty(propertyName);
    assert(property);

    // Get the geometry
    size_t geometryIndex = (size_t)propertyMapNode.attribute("geom-index").as_int();
    std::shared_ptr<const Geometry2D> geometry = m_project->getGeometries().at(geometryIndex);

    // Get min/max value
    float minValue = DefaultUndefinedValue;
    float maxValue = DefaultUndefinedValue;

    pugi::xml_attribute minValueAttr = propertyMapNode.attribute("min");
    if (minValueAttr)
        minValue = minValueAttr.as_float();

    pugi::xml_attribute maxValueAttr = propertyMapNode.attribute("max");
    if (maxValueAttr)
        maxValue = maxValueAttr.as_float();

    // Create the surface data
    std::shared_ptr<SurfaceData> surfaceData(new MapNative(geometry, minValue, maxValue));

    // Contains formation?
    pugi::xml_attribute formationName = propertyMapNode.attribute("formation");
    if (formationName)
    {
        std::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName.value());
        surfaceData->setFormation(formationIO);
    }

    // Find the reservoir object, if name is present
    std::shared_ptr<const Reservoir> reservoirIO;
    pugi::xml_attribute reservoirName = propertyMapNode.attribute("reservoir");
    if (reservoirName)
    {
        reservoirIO = m_project->findReservoir(reservoirName.value());
        assert(reservoirIO);
        surfaceData->setReservoir(reservoirIO);
    }

    // Get the datastore xml node or constantvalue
    pugi::xml_attribute constantVal = propertyMapNode.attribute("constantvalue");
    if (constantVal)
        surfaceData->setConstantValue(constantVal.as_float());
    else
        DataStoreLoad::getSurface(propertyMapNode, surfaceData, fullOutputPath);

    return PropertySurfaceData(property, surfaceData);
}

std::shared_ptr<CauldronIO::Surface> ImportFromXML::getSurface(pugi::xml_node surfaceNode, const ibs::FilePath& fullOutputPath) const
{
    // Read some xml attributes
    std::string surfaceName = surfaceNode.attribute("name").value();
    SubsurfaceKind surfaceKind = (SubsurfaceKind)surfaceNode.attribute("subsurfacekind").as_int();

    // Find formations, if present
    std::shared_ptr<Formation> topFormationIO;
    pugi::xml_attribute topFormationName = surfaceNode.attribute("top-formation");
    if (topFormationName)
        topFormationIO = m_project->findFormation(topFormationName.value());
    std::shared_ptr<Formation> bottomFormationIO;
    pugi::xml_attribute bottomFormationName = surfaceNode.attribute("bottom-formation");
    if (bottomFormationName)
        bottomFormationIO = m_project->findFormation(bottomFormationName.value());

    // Construct the surface
    std::shared_ptr<Surface> surface(new Surface(surfaceName, surfaceKind));
    if (topFormationIO) surface->setFormation(topFormationIO.get(), true);
    if (bottomFormationIO) surface->setFormation(bottomFormationIO.get(), false);

    // Find age, if present
    pugi::xml_attribute ageAttrib = surfaceNode.attribute("age");
    if (ageAttrib)
        surface->setAge(ageAttrib.as_float());

    // Get all property surface data
    pugi::xml_node propertyMapNodes = surfaceNode.child("propertymaps");
    for (pugi::xml_node propertyMapNode = propertyMapNodes.child("propertymap"); propertyMapNode; propertyMapNode = propertyMapNode.next_sibling("propertymap"))
    {
        PropertySurfaceData propSurfaceData = getPropertySurfaceData(propertyMapNode, fullOutputPath);
        surface->addPropertySurfaceData(propSurfaceData);
    }

    return surface;
}

std::shared_ptr<Property> CauldronIO::ImportFromXML::getProperty(pugi::xml_node propertyNode) const
{
    std::shared_ptr<Property> property;

    std::string name = propertyNode.attribute("name").value();
    std::string cauldronname = propertyNode.attribute("cauldronname").value(); 
    std::string username = propertyNode.attribute("username").value(); 
    std::string unit = propertyNode.attribute("unit").value();  
    PropertyType type = (PropertyType)propertyNode.attribute("type").as_int();
    PropertyAttribute attrib = (PropertyAttribute)propertyNode.attribute("attribute").as_int();

    property.reset(new Property(name, username, cauldronname, unit, type, attrib));
    return property;
}

std::shared_ptr<Formation> CauldronIO::ImportFromXML::getFormation(pugi::xml_node formationNode, const ibs::FilePath& fullOutputPath) const
{
    std::shared_ptr<Formation> formation;

    unsigned int start, end;
    std::string name = formationNode.attribute("name").value();
    start = formationNode.attribute("kstart").as_uint();
    end = formationNode.attribute("kend").as_uint();
    formation.reset(new Formation(start, end, name));

    // Read thicknessmap if present
    pugi::xml_node subNode = formationNode.child("thicknessmap");
    if (subNode)
    {
        // read the propertySurface data
        PropertySurfaceData propSurfaceData = getPropertySurfaceData(subNode, fullOutputPath);
        formation->setThicknessMap(propSurfaceData);
    }

    // Read mixingHI map if present
    subNode = formationNode.child("mixingHImap");
    if (subNode)
    {
        // read the propertySurface data
        PropertySurfaceData propSurfaceData = getPropertySurfaceData(subNode, fullOutputPath);
        formation->setSourceRockMixingHIMap(propSurfaceData);
    }

    // Read lithotype percentage maps
    subNode = formationNode.child("lithoType1Perc1map");
    if (subNode)
    {
        // read the propertySurface data
        PropertySurfaceData propSurfaceData = getPropertySurfaceData(subNode, fullOutputPath);
        formation->setLithoType1PercentageMap(propSurfaceData);
    }
    subNode = formationNode.child("lithoType1Perc2map");
    if (subNode)
    {
        // read the propertySurface data
        PropertySurfaceData propSurfaceData = getPropertySurfaceData(subNode, fullOutputPath);
        formation->setLithoType2PercentageMap(propSurfaceData);
    }
    subNode = formationNode.child("lithoType1Perc3map");
    if (subNode)
    {
        // read the propertySurface data
        PropertySurfaceData propSurfaceData = getPropertySurfaceData(subNode, fullOutputPath);
        formation->setLithoType3PercentageMap(propSurfaceData);
    }

    // Find additional properties
    pugi::xml_attribute attrib = formationNode.attribute("sr1name");
    if (attrib)
    {
        formation->setSourceRock1Name(attrib.as_string());
    }
    attrib = formationNode.attribute("sr2name");
    if (attrib)
    {
        formation->setSourceRock2Name(attrib.as_string());
    }
    attrib = formationNode.attribute("fluid_type");
    if (attrib)
    {
        formation->setFluidType(attrib.as_string());
    }
    attrib = formationNode.attribute("sr_mixing");
    if (attrib)
    {
        formation->setEnableSourceRockMixing(attrib.as_bool());
    }
    attrib = formationNode.attribute("allocht_lith");
    if (attrib)
    {
        formation->setAllochthonousLithology(attrib.as_bool());
    }
    attrib = formationNode.attribute("allocht_lith_name");
    if (attrib)
    {
        formation->setAllochthonousLithologyName(attrib.as_string());
    }
    attrib = formationNode.attribute("constrained_op");
    if (attrib)
    {
        formation->setConstrainedOverpressure(attrib.as_bool());
    }
    attrib = formationNode.attribute("chem_compact");
    if (attrib)
    {
        formation->setChemicalCompaction(attrib.as_bool());
    }
    attrib = formationNode.attribute("ignious_intr");
    if (attrib)
    {
        formation->setIgneousIntrusion(attrib.as_bool());
    }
    attrib = formationNode.attribute("ignious_intr_age");
    if (attrib)
    {
        formation->setIgneousIntrusionAge(attrib.as_double());
    }
    attrib = formationNode.attribute("depo_sequence");
    if (attrib)
    {
        formation->setDepoSequence(attrib.as_int());
    }
    attrib = formationNode.attribute("elem_refinement");
    if (attrib)
    {
        formation->setElementRefinement(attrib.as_int());
    }
    attrib = formationNode.attribute("mixing_model");
    if (attrib)
    {
        formation->setMixingModel(attrib.as_string());
    }
    attrib = formationNode.attribute("litho1_name");
    if (attrib)
    {
        formation->setLithoType1Name(attrib.as_string());
    }
    attrib = formationNode.attribute("litho2_name");
    if (attrib)
    {
        formation->setLithoType2Name(attrib.as_string());
    }
    attrib = formationNode.attribute("litho3_name");
    if (attrib)
    {
        formation->setLithoType3Name(attrib.as_string());
    }
    attrib = formationNode.attribute("isSR");
    if (attrib)
    {
        formation->setIsSourceRock(attrib.as_bool());
    }
    attrib = formationNode.attribute("isML");
    if (attrib)
    {
        formation->setIsMobileLayer(attrib.as_bool());
    }

    return formation;
}

