//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTPROPERTIES__VISUALIZATION_PROPERTIES_CALCULATOR__H
#define FASTPROPERTIES__VISUALIZATION_PROPERTIES_CALCULATOR__H

//std library
#include <string>
#include <sstream>
#include <vector>

#include "AbstractPropertiesCalculator.h"

//DataModel library
#include "PropertyAttribute.h"

//VisualizationIO library
#include "ExportToXML.h"

class VisualizationPropertiesCalculator : public AbstractPropertiesCalculator {

   const std::string xmlExt = ".xml";

public :
   VisualizationPropertiesCalculator( int aRank );

   virtual ~VisualizationPropertiesCalculator();

private:

   std::shared_ptr<CauldronIO::Project> m_vizProject;
   std::shared_ptr<CauldronIO::Project> m_vizProjectExisting;

   CauldronIO::ExportToXML * m_export;
   pugi::xml_document m_doc;
   pugi::xml_node m_pt;
   pugi::xml_node m_snapShotNodes;

   bool m_vizFormatHDFonly;
   bool m_vizListXml;
   bool m_minorPrimary;      ///< If true: convert and output primary properties at minor snapshots

   string m_fileNameXml;
   string m_filePathXml;
   string m_fileNameExistingXml;
   string m_outputNameXml;

   std::vector<float> m_data; ///< Buffer to convert a property to visualization format
   std::shared_ptr<CauldronIO::FormationInfoList> m_formInfoList; ///< List of formations grid info

   MPI_Op m_op; ///< Custom MPI operation to find maximum and minimum of sediment properties
   MPI_Op m_ind;

   int m_localIndices[2];
   int m_globalIndices[2];

   float m_minValue;
   float m_maxValue;

   std::vector<string> m_existingProperties;

   /// \brief Check command-line parameters consistency
   bool checkParameters();

   /// \brief Set the XML file name and path
   void setXmlName();

   /// \brief Create visualization format project: import projectHandle and create xml file
   void createXML();
   /// \brief Import projectHandle into visualization format
   std::shared_ptr<CauldronIO::Project> createStructureFromProjectHandle( bool verbose );

   /// \brief Finish export to visualization format: add project related information to xml file and save it on disk
   void saveXML();

   /// \brief Create a property map in visualization format
   bool createVizSnapshotResultPropertyValueMap ( OutputPropertyValuePtr propertyValue,
                                                  const Interface::Snapshot* snapshot, const Interface::Formation * formation,
                                                  const Interface::Surface * surface );

   /// \brief Create a continuous property in visualization format
   bool createVizSnapshotResultPropertyValueContinuous ( OutputPropertyValuePtr propertyValue,
                                                         const Interface::Snapshot* snapshot, const Interface::Formation * formation );

   /// \brief Create a discontinuous property in visualization format
   bool  createVizSnapshotResultPropertyValueDiscontinuous ( OutputPropertyValuePtr propertyValue,
                                                             const Interface::Snapshot* snapshot, const Interface::Formation * formation );

   /// \brief Create list of properties for a formation/surface pair at a snapshot age in visualization format
   void  createVizSnapshotFormationData( const Interface::Snapshot * snapshot, const FormationSurface & formationItem,
                                         DataAccess::Interface::PropertyList & properties,
                                         SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues );

   /// \brief Create a property for a formation/surface pair at a snapshot age in visualization format
   bool createVizSnapshotResultPropertyValue ( OutputPropertyValuePtr propertyValue,
                                               const Interface::Snapshot* snapshot, const Interface::Formation * formation,
                                               const Interface::Surface * surface );

   /// \brief Calculate k-range (offsets in depth) for all stratigraphy formations.
   void updateFormationsKRange();

   /// \brief Collect continuous volume on rank 0
   void collectVolumeData( const std::shared_ptr<CauldronIO::SnapShot>& snapShot );

   /// \brief Create a property in visualization format or find it if already exists
   shared_ptr<const CauldronIO::Property> findOrCreateProperty(OutputPropertyValuePtr propertyValue, CauldronIO::PropertyAttribute attrib);

   /// \brief Create a formation in visualization format or find it if already exists
   shared_ptr<CauldronIO::Formation> findOrCreateFormation(std::shared_ptr<CauldronIO::FormationInfo>& info);

   /// \brief Create a reservoir in visualization format or find it if already exists
   shared_ptr<const CauldronIO::Reservoir> findOrCreateReservoir(const Interface::Reservoir* reserv, std::shared_ptr<const CauldronIO::Formation> formationIO);

   /// \brief Create a trapper list and add to visualization format snapshot
   void createTrappers(const Interface::Snapshot * snapshot, std::shared_ptr<CauldronIO::SnapShot>& snapShotIO);

   /// \brief Update a constant value for a snapshot volume data
   void updateConstantValue( std::shared_ptr< CauldronIO::SnapShot>& snapshot );

   /// \brief Add MigrationIo, TrapIo, TrapperIo, mass balance etc. to the xml file
   void addTables ();

   /// \brief Returns true
   bool getProperiesActivity() const;

   /// \brief Locally convert volume to viz format
   bool computeFormationVolume (OutputPropertyValuePtr propertyValue, const DataModel::AbstractGrid* grid,
                                int firstK, int lastK, int minK, int depthK, bool computeMinMax);

   /// \brief Compute global boundaries of the viz volume
   void computeVolumeBounds (const DataModel::AbstractGrid* grid, int firstK, int lastK, int minK);

   /// \brief Compute global minimum/maximum values
   void computeMinMax ();

   /// \brief Globally convert map
   bool computeFormationMap (OutputPropertyValuePtr propertyValue, const DataModel::AbstractGrid* grid, int kIndex, float * dest);

   /// \brief Acquire property names for a given property type attribute
   void acquireAttributeProperties (DataModel::PropertyOutputAttribute outputAttribute);

public:

   bool parseCommandLine(int argc, char ** argv);

   void calculateProperties( FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots );

   /// \brief Acquire property names for a simulator (if defined)
   void acquireSimulatorProperties();

   /// \brief Acquire the names of the existing properties in the XML file
   bool acquireExistingProperties();

   /// \brief Check if the property is already in the XML file
   bool propertyExisting(const string &name);

   /// \brief Remove the existing properties from the list of the properties to be calculated
   void refinePropertyNames(Interface::PropertyList& properties);

   /// \brief Update the constant value for all snapshots
   void updateVizSnapshotsConstantValue();

   bool hdfonly() const;
   bool vizFormat() const;
   bool listXml() const;

   void writeToHDF();
   void listXmlProperties();

   bool copyFiles();
};

inline bool VisualizationPropertiesCalculator::hdfonly() const {
   return m_vizFormatHDFonly;
}

inline bool VisualizationPropertiesCalculator::listXml() const {
   return m_vizListXml;
}

inline bool VisualizationPropertiesCalculator::getProperiesActivity() const {
   return true;
}

inline bool VisualizationPropertiesCalculator::copyFiles() {
   return true;
}

#endif // FASTPROPERTIES__VISUALIZATION_PROPERTIES_CALCULATOR__H
