//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by Shell India Markets Pvt. Ltd.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <gtest/gtest.h>

#include "database.h"
#include "cauldronschemafuncs.h"
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"
#include "ImportFromXML.h"
#include "ExportToXML.h"
#include "VisualizationIO_projectHandle.h"
#include "ImportProjectHandle.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Formation.h"
#include "Property.h"
#include "Snapshot.h"
#include "Reservoir.h"
#include "PropertyValue.h"
#include "Trap.h"
#include "Trapper.h"
#include "GridMap.h"
#include "Surface.h"
#include "SerialGridMap.h"

using namespace CauldronIO;
using namespace std;

class CompareTest : public ::testing::Test
{

public:
   CompareTest(){ m_factoryP3d = new DataAccess::Interface::ObjectFactory(); }
   ~CompareTest() { delete m_factoryP3d; }
   DataAccess::Interface::ObjectFactory *m_factoryP3d;

};

void compareSnapshots(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
   //Comparing Snapshot information
   DataAccess::Interface::SnapshotList *snapShotListP3d = projectP3d->getSnapshots(DataAccess::Interface::MINOR | DataAccess::Interface::MAJOR);
   size_t sizeP3d = snapShotListP3d->size();
   size_t sizeXml = projectXml->getSnapShots().size();
   EXPECT_EQ(sizeP3d, sizeXml);
   for (size_t i = 0; i < sizeP3d; i++)
   {
           const DataAccess::Interface::Snapshot* snapshotP3d;
           std::shared_ptr<CauldronIO::SnapShot> snapshotXml = projectXml->getSnapShots()[i];
           snapshotP3d = projectP3d->findSnapshot(snapshotXml->getAge(), snapshotXml->isMinorShapshot() ? DataAccess::Interface::MINOR : DataAccess::Interface::MAJOR);
           EXPECT_EQ(ImportProjectHandle::getSnapShotKind(snapshotP3d), snapshotXml->getKind());

           //Comparing Trappers information
           if( projectXml->getTrapperTable().size() == 0 ) {
              DataAccess::Interface::TrapperList *trapListP3d = projectP3d->getTrappers(0, snapshotP3d, 0, 0);
              const CauldronIO::TrapperList trapListXml = snapshotXml->getTrapperList();
              size_t sizeP3d1 = trapListP3d->size();
              size_t sizeXml1 = trapListXml.size();
              EXPECT_EQ(sizeP3d1, sizeXml1);

              for (size_t j = 0; j < sizeP3d1; j++)
              {
                 DataAccess::Interface::TrapperList *trapperListP3d;
                 std::shared_ptr<CauldronIO::Trapper> trapperXml = trapListXml.at(j);

                 const DataAccess::Interface::Reservoir *reservoirP3d = projectP3d->findReservoir(trapperXml->getReservoirName());

                 trapperListP3d = projectP3d->getTrappers(reservoirP3d, snapshotP3d, trapperXml->getID(), trapperXml->getPersistentID());
                 const DataAccess::Interface::Trapper *trapperP3d = trapperListP3d->at(0);
                 EXPECT_NEAR(trapperP3d->getDepth(), trapperXml->getDepth(), 0.00001);
                 EXPECT_EQ(trapperXml->getID(), trapperP3d->getId());
                 EXPECT_EQ(trapperXml->getPersistentID(), trapperP3d->getPersistentId());
                 float posXXml, posYXml;
                 double posXP3d, posYP3d;
                 trapperXml->getPosition(posXXml, posYXml);
                 trapperP3d->getPosition(posXP3d, posYP3d);
                 EXPECT_NEAR(posXXml, posXP3d, 0.00001);
                 EXPECT_NEAR(posYXml, posYP3d, 0.00001);
                 EXPECT_EQ(trapperXml->getReservoirName(), trapperP3d->getReservoir()->getName());

                 EXPECT_NEAR(trapperP3d->getSpillDepth(), trapperXml->getSpillDepth(), 0.00001);
                 trapperXml->getSpillPointPosition(posXXml, posYXml);
                 trapperP3d->getSpillPointPosition(posXP3d, posYP3d);
                 EXPECT_NEAR(posXXml, posXP3d, 0.00001);
                 EXPECT_NEAR(posYXml, posYP3d, 0.00001);

                 EXPECT_NEAR(trapperXml->getGOC(), trapperP3d->getGOC(), 0.001);
                 EXPECT_NEAR(trapperXml->getOWC(), trapperP3d->getOWC(), 0.001);
              }
           }
        }
}

void compareFormations(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
   //Comparing Formation information
   DataAccess::Interface::FormationList *formationListP3d = projectP3d->getFormations(0, true);
   size_t sizeP3d = formationListP3d->size();
   size_t sizeXml = projectXml->getFormations().size();
   EXPECT_EQ(sizeP3d, sizeXml);
   for (size_t i = 0; i < sizeP3d; i++)
   {
      const std::shared_ptr<CauldronIO::Formation> formationXml = projectXml->getFormations()[i];
      const DataAccess::Interface::Formation* formationP3d = projectP3d->findFormation(formationXml->getName());

      EXPECT_EQ(formationP3d->getName(), formationXml->getName());
      EXPECT_EQ(formationP3d->isMobileLayer(), formationXml->isMobileLayer());
      EXPECT_EQ(formationP3d->isSourceRock(), formationXml->isSourceRock());

   }

}

void compareReservoirs(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
   //Comparing Reservoir in formation
   DataAccess::Interface::ReservoirList *reservoirListP3d = projectP3d->getReservoirs(0);
   size_t sizeP3d = reservoirListP3d->size();
   size_t sizeXml = projectXml->getReservoirs().size();
   EXPECT_EQ(sizeP3d, sizeXml);
   if (sizeP3d > 0)
   {
      for (size_t i = 0; i < sizeP3d; i++)
      {
         const std::shared_ptr<const CauldronIO::Reservoir> reservoirXml = projectXml->getReservoirs()[i];
         const DataAccess::Interface::Reservoir* reservoirP3d = projectP3d->findReservoir(reservoirXml->getName());

         EXPECT_EQ(reservoirP3d->getName(), reservoirXml->getName());
         EXPECT_EQ(reservoirP3d->getFormation()->getName(), reservoirXml->getFormation()->getName());
         EXPECT_EQ(reservoirP3d->getFormation()->isSourceRock(), reservoirXml->getFormation()->isSourceRock());
         EXPECT_EQ(reservoirP3d->getFormation()->isMobileLayer(), reservoirXml->getFormation()->isMobileLayer());

      }
   }

}

void compareProperties(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
   //Comparing Properties information
   DataAccess::Interface::PropertyList *propListP3d = projectP3d->getProperties();
   size_t sizeP3d = propListP3d->size();
   size_t sizeXml = projectXml->getProperties().size(); // size can be different as some input properties are created

   for (size_t i = 0; i < sizeP3d; i++)
   {
      const DataAccess::Interface::Property* propP3d = propListP3d->at(i);
      std::shared_ptr<const CauldronIO::Property> propXml = projectXml->findProperty(propP3d->getName());
      CauldronIO::PropertyType propTypeP3d = ImportProjectHandle::getPropertyType(propP3d);
      CauldronIO::PropertyAttribute propAttributeP3d = ImportProjectHandle::getPropertyAttribute(propP3d);
      EXPECT_EQ(propP3d->getName(), propXml->getName());
      EXPECT_EQ(propTypeP3d, propXml->getType());
      EXPECT_EQ(propAttributeP3d, propXml->getAttribute());
      EXPECT_EQ(propP3d->getUnit(), propXml->getUnit());
      EXPECT_EQ(propP3d->getUserName(), propXml->getUserName());
      EXPECT_EQ(propP3d->getCauldronName(), propXml->getCauldronName());
   }
}

void compare2dPropertyValues(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
   DataAccess::Interface::PropertyValueList *propValueListP3d = projectP3d->getPropertyValues();
   size_t sizeP3d = propValueListP3d->size();
   size_t nSnapShotsXml = projectXml->getSnapShots().size();

   //Used as string separator in maps for keys
   string sep = "$#!";
   //Map to store 2D property values with index
   std::map <string, int> propValue2DMapP3d;

   //Creating map for 2D property values in project3d
   for (size_t i = 0; i < sizeP3d; i++)
   {
      const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(i);
      if ((propValueP3d->getStorage() != DataAccess::Interface::PropertyStorage::TIMEIOTBL))
      {
         continue;
      }
      double ageP3d = propValueP3d->getSnapshot()->getTime();
      string propNameP3d = propValueP3d->getProperty()->getName();
      string userNameP3d = propValueP3d->getProperty()->getUserName();
      string cauldronNameP3d = propValueP3d->getProperty()->getCauldronName();
      string formationNameP3d = propValueP3d->getFormation() ? propValueP3d->getFormation()->getName() : "";

      formationNameP3d = ((formationNameP3d.empty()) && (propValueP3d->getReservoir())) ? propValueP3d->getReservoir()->getFormation()->getName() : formationNameP3d;
      string surfaceNameP3d = propValueP3d->getSurface() ? propValueP3d->getSurface()->getName() : "";

      string key = std::to_string(ageP3d) + sep + propNameP3d + sep + userNameP3d + sep + cauldronNameP3d + sep + formationNameP3d + sep + surfaceNameP3d;
      ASSERT_TRUE(propValue2DMapP3d.count(key) == 0) << "Error:: Key not unique: " << key;
      propValue2DMapP3d.insert(std::make_pair(key, static_cast<int>(i)));

   }

   //Comparing 2D property values
   for (size_t i = 0; i < nSnapShotsXml; i++)
   {
      size_t nSurfacesXml = projectXml->getSnapShots()[i]->getSurfaceList().size();
      for (size_t j = 0; j < nSurfacesXml; j++)
      {
         size_t nPropSurfaceDataXml = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList().size();
         string surfaceNameXml = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getName();
         double ageXml = projectXml->getSnapShots()[i]->getAge();
         const CauldronIO::PropertySurfaceDataList propSurfaceDataListXml = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList();
         for (size_t k = 0; k < nPropSurfaceDataXml; k++)
         {
            const CauldronIO::PropertySurfaceData propSurfaceDataXml = propSurfaceDataListXml.at(k);
            std::shared_ptr<const CauldronIO::Property> propXml = propSurfaceDataXml.first;
            std::shared_ptr<CauldronIO::SurfaceData> surfaceDataXml = propSurfaceDataXml.second;

            string formationNameXml = surfaceDataXml->getFormation() ? surfaceDataXml->getFormation()->getName() : "";
            formationNameXml = ((formationNameXml.empty()) && (surfaceDataXml->getReservoir())) ? surfaceDataXml->getReservoir()->getFormation()->getName() : formationNameXml;

            string key = std::to_string(ageXml) + sep + propXml->getName() + sep + propXml->getUserName() + sep + propXml->getCauldronName() + sep + formationNameXml + sep + surfaceNameXml;

            std::map<std::string, int>::iterator itP3d = propValue2DMapP3d.find(key);

            int indexP3d = itP3d->second;
            int numIXml = static_cast<int>(surfaceDataXml->getGeometry()->getNumI());
            int numJXml = static_cast<int>(surfaceDataXml->getGeometry()->getNumJ());

            const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(indexP3d);
            DataAccess::Interface::GridMap* gridP3d = propValueP3d->getGridMap();
            int numIP3d = gridP3d->numI();
            int numJP3d = gridP3d->numJ();

            EXPECT_EQ(numIXml, numIP3d);
            EXPECT_EQ(numJXml, numJP3d);
            if (surfaceDataXml->isRetrieved())
            {
               for (int indexI = 0; indexI < numIXml; indexI++)
               {
                  for (int indexJ = 0; indexJ < numJXml; indexJ++)
                  {
                     EXPECT_EQ(gridP3d->getValue((unsigned int)indexI, indexJ), surfaceDataXml->getValue(indexI, indexJ));
                  }
               }
            }
         }
      }
   }
   cout << "2d property value comparison done" << endl;
}

std::map<string, int> createFormationMap(std::shared_ptr<CauldronIO::Project> projectXml)
{
   //Map to store formation names and kStart to perform depth conversions for 3D continuous property values
   std::map<string, int> formationMapXml;
   size_t nFormationsXml = projectXml->getFormations().size();
   for (size_t i = 0; i < nFormationsXml; i++)
   {
      int kStart, kEnd;
      projectXml->getFormations()[i]->getK_Range(kStart, kEnd);
      formationMapXml.insert(std::make_pair(projectXml->getFormations()[static_cast<int>(i)]->getName().c_str(), kStart));
   }
   return formationMapXml;
}

void compare3dDiscontinuousPropertyValues(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{
   DataAccess::Interface::PropertyValueList *propValueListP3d = projectP3d->getPropertyValues();
   size_t sizeP3d = propValueListP3d->size();
   size_t nSnapShotsXml = projectXml->getSnapShots().size();

   //Map to store formation names and kStart to perform depth conversions for 3D continuous property values
   std::map<string, int> formationMapXml = createFormationMap(projectXml);

   //Used as string separator in maps for keys
   string sep = "$#!";

   //Map to store 3D discontinuous property values and their indices
   std::map <string, int> propValue3DMapDiscontinuousP3d;
   for (size_t i = 0; i < sizeP3d; i++)
   {
      const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(i);
      if (propValueP3d->getStorage() != DataAccess::Interface::PropertyStorage::SNAPSHOTIOTBL && propValueP3d->getStorage() != DataAccess::Interface::PropertyStorage::THREEDTIMEIOTBL)
      {
         continue;
      }
      if ((propValueP3d->getProperty()->getPropertyAttribute() == DataModel::PropertyAttribute::DISCONTINUOUS_3D_PROPERTY) && (propValueP3d->getSurface() == NULL))
      {
         double ageP3d = propValueP3d->getSnapshot()->getTime();
         string propNameP3d = propValueP3d->getProperty()->getName();
         string userNameP3d = propValueP3d->getProperty()->getUserName();
         string cauldronNameP3d = propValueP3d->getProperty()->getCauldronName();

         string formationNameP3d = propValueP3d->getFormation() ? propValueP3d->getFormation()->getName() : "";
         formationNameP3d = ((formationNameP3d.empty()) && (propValueP3d->getReservoir())) ? propValueP3d->getReservoir()->getFormationName() : formationNameP3d;

         string key = std::to_string(ageP3d) + sep + propNameP3d + sep + userNameP3d + sep + cauldronNameP3d + sep + formationNameP3d;
         ASSERT_TRUE(propValue3DMapDiscontinuousP3d.count(key) == 0) << "Error:: Key not unique: " << key;
         propValue3DMapDiscontinuousP3d.insert(std::make_pair(key, static_cast<int>(i)));

      }
   }

   //Comparing 3D discontinuous property values
   for (size_t i = 0; i < nSnapShotsXml; i++)
   {
      size_t nFormationVolumeXml = projectXml->getSnapShots()[i]->getFormationVolumeList().size();
      const CauldronIO::FormationVolumeList formationVolumeListXml = projectXml->getSnapShots()[i]->getFormationVolumeList();

      for (size_t j = 0; j < nFormationVolumeXml; j++)
      {
         double ageXml = projectXml->getSnapShots()[i]->getAge();
         const CauldronIO::FormationVolume formationVolumeXml = formationVolumeListXml.at(j);
         std::shared_ptr<const CauldronIO::Formation> formationXml = formationVolumeXml.first;

         std::shared_ptr<CauldronIO::Volume> volumeXml = formationVolumeXml.second;
         const CauldronIO::PropertyVolumeDataList propVolDataListXml = volumeXml->getPropertyVolumeDataList();
         string formationNameXml = formationXml->getName();

         size_t nPropVolDataListXml = volumeXml->getPropertyVolumeDataList().size();
         for (size_t k = 0; k < nPropVolDataListXml; k++)
         {

            const CauldronIO::PropertyVolumeData propVolDataXml = propVolDataListXml.at(k);
            std::shared_ptr<const CauldronIO::Property> propXml = propVolDataXml.first;
            std::shared_ptr<CauldronIO::VolumeData> volumeDataXml = propVolDataXml.second;

            string key = std::to_string(ageXml) + sep + propXml->getName() + sep + propXml->getUserName() + sep + propXml->getCauldronName() + sep + formationNameXml;
            std::map<std::string, int>::iterator itP3d = propValue3DMapDiscontinuousP3d.find(key);
            int indexP3d = itP3d->second;
            int numIXml = static_cast<int>(volumeDataXml->getGeometry()->getNumI());
            int numJXml = static_cast<int>(volumeDataXml->getGeometry()->getNumJ());
            int numKXml = static_cast<int>(volumeDataXml->getGeometry()->getNumK());

            const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(indexP3d);
            DataAccess::Interface::GridMap* gridP3d = propValueP3d->getGridMap();
            int numIP3d = gridP3d->numI();
            int numJP3d = gridP3d->numJ();
            int lastKP3d = gridP3d->lastK();
            int firstKP3d = gridP3d->firstK();
            EXPECT_EQ(numIXml, numIP3d);
            EXPECT_EQ(numJXml, numJP3d);
            EXPECT_EQ(numKXml, lastKP3d - firstKP3d + 1);

            if (volumeDataXml->isRetrieved())
            {
               for (int indexI = 0; indexI < numIXml; indexI++)
               {
                  for (int indexJ = 0; indexJ < numJXml; indexJ++)
                  {
                     for (int indexK = 0; indexK < numKXml; indexK++)
                     {
                        EXPECT_EQ(gridP3d->getValue((unsigned int)indexI, (unsigned int)indexJ, (unsigned int)indexK), volumeDataXml->getValue(indexI, indexJ, indexK));

                     }
                  }
               }
            }
            else
            {
               continue;
            }


         }

      }
   }

   cout << "3d discontinuous property value comparison done" << endl;
}

void compare3dContinuousPropertyValues(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d)
{

   DataAccess::Interface::PropertyValueList *propValueListP3d = projectP3d->getPropertyValues();
   size_t sizeP3d = propValueListP3d->size();
   size_t nSnapShotsXml = projectXml->getSnapShots().size();
   //Used as string separator in maps for keys
   string sep = "$#!";

   //Map to store formation names and kStart to perform depth conversions for 3D continuous property values
   std::map<string, int> formationMapXml = createFormationMap(projectXml);

   //Map to store 3D continuous property values and their indices
   std::map <string, vector<int>> propValue3DMapContinuousP3d;
   for (size_t i = 0; i < sizeP3d; i++)
   {
      const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(i);
      if (propValueP3d->getStorage() != DataAccess::Interface::PropertyStorage::SNAPSHOTIOTBL && propValueP3d->getStorage() != DataAccess::Interface::PropertyStorage::THREEDTIMEIOTBL)
      {
         continue;
      }
      if ((propValueP3d->getProperty()->getPropertyAttribute() == DataModel::PropertyAttribute::CONTINUOUS_3D_PROPERTY) && (propValueP3d->getSurface() == NULL))
      {
         double ageP3d = propValueP3d->getSnapshot()->getTime();
         string propNameP3d = propValueP3d->getProperty()->getName();
         string userNameP3d = propValueP3d->getProperty()->getUserName();
         string cauldronNameP3d = propValueP3d->getProperty()->getCauldronName();
         string key = std::to_string(ageP3d) + sep + propNameP3d + sep + userNameP3d + sep + cauldronNameP3d;
         if (propValue3DMapContinuousP3d.count(key) == 0)
         {
            vector<int> vecValues{ static_cast<int>(i) };
            propValue3DMapContinuousP3d.insert(make_pair(key, vecValues));
         }
         else
         {
            propValue3DMapContinuousP3d.find(key)->second.push_back(static_cast<int>(i));
         }
      }
   }

   //Comparing 3D continuous property values
   for (size_t i = 0; i < nSnapShotsXml; i++)
   {
      if (projectXml->getSnapShots()[i]->getVolume())
      {
         size_t nPropVolDataXml = projectXml->getSnapShots()[i]->getVolume()->getPropertyVolumeDataList().size();
         double ageXml = projectXml->getSnapShots()[i]->getAge();
         const CauldronIO::PropertyVolumeDataList propVolumeDataListXml = projectXml->getSnapShots()[i]->getVolume()->getPropertyVolumeDataList();

         for (size_t j = 0; j < nPropVolDataXml; j++)
         {
            const CauldronIO::PropertyVolumeData propVolumeDataXml = propVolumeDataListXml.at(j);
            std::shared_ptr<const CauldronIO::Property> propXml = propVolumeDataXml.first;
            std::shared_ptr<CauldronIO::VolumeData> volumeDataXml = propVolumeDataXml.second;

            int numIXml = static_cast<int>(volumeDataXml->getGeometry()->getNumI());
            int numJXml = static_cast<int>(volumeDataXml->getGeometry()->getNumJ());
            int numKXml = static_cast<int>(volumeDataXml->getGeometry()->getNumK());

            string key = std::to_string(ageXml) + sep + propXml->getName() + sep + propXml->getUserName() + sep + propXml->getCauldronName();
            std::map<std::string, vector<int>>::iterator itP3d = propValue3DMapContinuousP3d.find(key);
            vector<int> indexValsP3d = itP3d->second;

            for (int k = 0; k < indexValsP3d.size(); k++)
            {

               const DataAccess::Interface::PropertyValue* propValueP3d = propValueListP3d->at(indexValsP3d.at(k));
               DataAccess::Interface::GridMap* gridP3d = propValueP3d->getGridMap();
               int numIP3d = gridP3d->numI();
               int numJP3d = gridP3d->numJ();
               int lastKP3d = gridP3d->lastK();
               int firstKP3d = gridP3d->firstK();
               EXPECT_EQ(numIXml, numIP3d);
               EXPECT_EQ(numJXml, numJP3d);

               string formationKeyP3d = propValueP3d->getFormation()->getName();
               int firstKXml = formationMapXml.find(formationKeyP3d)->second;

               if (volumeDataXml->isRetrieved())
               {
                  for (int indexI = 0; indexI < numIXml; indexI++)
                  {
                     for (int indexJ = 0; indexJ < numJXml; indexJ++)
                     {
                        for (int indexK = 0; indexK < lastKP3d - firstKP3d + 1; indexK++)
                        {
                           EXPECT_EQ(gridP3d->getValue((unsigned int)indexI, (unsigned int)indexJ, (unsigned int)indexK), volumeDataXml->getValue(indexI, indexJ, indexK + firstKXml));

                        }
                     }
                  }
               }
               else
               {
                  continue;
               }

            }

         }
      }
   }
   cout << "3d continuous property value comparison done" << endl;
}

vector<int> updatePropertySurfaceData(shared_ptr<CauldronIO::Project> projectXml, shared_ptr<const Property> prop, int &size)
{
   size_t nSnapshots = projectXml->getSnapShots().size();
   vector<int> indexWithUpdatedPropSurfaceData;
   //Adding PropertySurfaceData
   bool modified = false;
   for (size_t i = 0; i < nSnapshots; i++)
   {
      size_t nSurfaces = projectXml->getSnapShots()[i]->getSurfaceList().size();
      for (size_t j = 0; j < nSurfaces; j++)
      {
         if ((projectXml->getSnapShots()[i]->getSurfaceList()[j]) && (projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList().size()>0))
         {
            size_t nPropSurfaceData = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList().size();
            bool canAdd = true;

            for (size_t k = 0; k < nPropSurfaceData; k++)
            {
               PropertySurfaceData propSurface = projectXml->getSnapShots()[i]->getSurfaceList()[j]->getPropertySurfaceDataList()[k];
               if (propSurface.first->getName() == prop->getName())
               {
                  canAdd = false;
                  break;
               }

            }
            if (canAdd == true)
            {
               indexWithUpdatedPropSurfaceData.push_back(static_cast<int>(i));
               indexWithUpdatedPropSurfaceData.push_back(static_cast<int>(j));
               const shared_ptr<Surface> surfaceExisting = projectXml->getSnapShots()[i]->getSurfaceList()[j];
               shared_ptr<const Geometry2D> geo2DExisting = surfaceExisting->getPropertySurfaceDataList()[0].second->getGeometry();
               shared_ptr<SurfaceData> valueMap(new MapNative(geo2DExisting));
               size = static_cast<int>(geo2DExisting->getNumI()*geo2DExisting->getNumJ());
               float *surfaceDataValue = new float[size];
               for (int p = 0; p < size; p++)
               {
                  surfaceDataValue[p] = 1.1f + p;
               }

               PropertySurfaceData propSurfaceNew = PropertySurfaceData(prop, valueMap);
               valueMap->setData_IJ(surfaceDataValue);
               projectXml->getSnapShots()[i]->getSurfaceList()[j]->addPropertySurfaceData(propSurfaceNew);
               modified = true;
               break;
            }
         }
      }
      if (modified == true)
      {
         break;
      }
   }
   return indexWithUpdatedPropSurfaceData;
}

vector<int> updatePropertyVolumeData(shared_ptr<CauldronIO::Project> projectXml, shared_ptr<const Property> prop, int &size)
{
   size_t nSnapshots = projectXml->getSnapShots().size();
   //Adding PropertyVolumeData
   vector<int> indexAddedPropVolData;
   bool modified = false;
   for (size_t i = 0; i < nSnapshots; i++)
   {
      if (projectXml->getSnapShots()[i]->getFormationVolumeList().size()>0)
      {
         size_t nFormationVolumes = projectXml->getSnapShots()[i]->getFormationVolumeList().size();
         CauldronIO::FormationVolumeList formVolListExisting = projectXml->getSnapShots()[i]->getFormationVolumeList();
         for (size_t j = 0; j < nFormationVolumes; j++)
         {
            bool canAdd = true;
            std::shared_ptr<CauldronIO::Volume> volExisting = formVolListExisting.at(j).second;
            const CauldronIO::PropertyVolumeDataList propVolumeDataListExisting = volExisting->getPropertyVolumeDataList();
            size_t nPropVolData = propVolumeDataListExisting.size();
            for (size_t k = 0; k < nPropVolData; k++)
            {
               const CauldronIO::PropertyVolumeData propVolDataExisting = propVolumeDataListExisting.at(k);
               if (propVolDataExisting.first->getName() == prop->getName())
               {
                  canAdd = false;
                  break;
               }
            }
            if (canAdd == true)
            {
               indexAddedPropVolData.push_back(static_cast<int>(i));
               indexAddedPropVolData.push_back(static_cast<int>(j));
               const shared_ptr<Geometry3D> geo3dExisting = volExisting->getPropertyVolumeDataList().at(0).second->getGeometry();
               shared_ptr<VolumeData> volDataNew(new VolumeDataNative(geo3dExisting));
               size = static_cast<int>(geo3dExisting->getNumI() * geo3dExisting->getNumJ()*geo3dExisting->getNumK());
               float *volumeDataValue = new float[size];
               for (int p = 0; p < size; p++)
               {
                  volumeDataValue[p] = 1.1f + p;
               }
               volDataNew->setData_KIJ(volumeDataValue);

               PropertyVolumeData propVolDataNew(prop, volDataNew);
               volExisting->addPropertyVolumeData(propVolDataNew);

               modified = true;
               break;
            }

         }
      }
      if (modified == true)
      {
         break;
      }
   }
   return indexAddedPropVolData;

}

//Function to update property value in an existing snapshot and check if it has been updated correctly
void addToExistingData(string xmlFileName)
{
   cout << "Starting import from XML" << endl;
   shared_ptr<CauldronIO::Project> projectXml = CauldronIO::ImportFromXML::importFromXML(xmlFileName);
   cout << "File read complete" << endl;

   //Finding property which can be added with PropertySurfaceData
   shared_ptr<const Property> prop;
   for (int i = 0; i < projectXml->getProperties().size(); i++)
   {
      if (projectXml->getProperties().at(i)->getAttribute() == Discontinuous3DProperty)
      {
         prop = projectXml->getProperties().at(i);
         break;
      }
   }
   int sizeSurfaceData = 0;
   vector<int> indexWithUpdatedPropSurfaceData = updatePropertySurfaceData(projectXml,prop,sizeSurfaceData);

   float *surfaceDataValue = sizeSurfaceData > 0 ? new float[sizeSurfaceData] : nullptr;
   for (int i = 0; i < sizeSurfaceData; i++)
   {
      surfaceDataValue[i] = 1.1f + i;
   }
   //Finding property which can be added with PropertyVolumeData
   shared_ptr<const Property> prop2;
   for (size_t i = 0; i < projectXml->getProperties().size(); i++)
   {
      if ((projectXml->getProperties().at(i)->getAttribute() == Discontinuous3DProperty) && (projectXml->getProperties().at(i)->getName() != prop->getName()))
      {
         prop2 = projectXml->getProperties().at(i);
         break;
      }
   }

   int sizeVolumeData = 0;
   vector<int> indexAddedPropVolData = updatePropertyVolumeData(projectXml, prop2, sizeVolumeData);
   float *volumeDataValue = sizeVolumeData > 0 ? new float[sizeVolumeData] : nullptr;
   for (int i = 0; i < sizeVolumeData; i++)
   {
      volumeDataValue[i] = 1.1f + i;
   }

   //Exporting data
   shared_ptr<CauldronIO::Project> projectExisting;
   bool status = CauldronIO::ExportToXML::exportToXML(projectXml, projectExisting, xmlFileName);
   ASSERT_EQ(true, status);

   //Importing data to check if data has been added correctly
   projectXml = CauldronIO::ImportFromXML::importFromXML(xmlFileName);

   if (indexWithUpdatedPropSurfaceData.size() > 0)
   {
      //Retrieving the surface to which property surface data was added
      int index = indexWithUpdatedPropSurfaceData.at(0);
      shared_ptr<SnapShot> snapShotNew = projectXml->getSnapShots()[index];
      index = indexWithUpdatedPropSurfaceData.at(1);
      shared_ptr<Surface> surfaceNew = snapShotNew->getSurfaceList().at(index);
      size_t nPropSurfaceData = surfaceNew->getPropertySurfaceDataList().size();
      //Comparing the property surface data which was added
      for (size_t i = 0; i < nPropSurfaceData; i++)
      {
         PropertySurfaceData propSurfaceDataNew = surfaceNew->getPropertySurfaceDataList().at(i);

         if (prop->getName() == propSurfaceDataNew.first->getName())
         {
            shared_ptr<const Property> propNew = propSurfaceDataNew.first;
            shared_ptr<SurfaceData> surfaceDataNew = propSurfaceDataNew.second;

            EXPECT_EQ(prop->getName(), propNew->getName());
            EXPECT_EQ(prop->getCauldronName(), propNew->getCauldronName());
            EXPECT_EQ(prop->getAttribute(), propNew->getAttribute());
            EXPECT_EQ(prop->getType(), propNew->getType());
            EXPECT_EQ(prop->getUnit(), propNew->getUnit());
            EXPECT_EQ(prop->getUserName(), propNew->getUserName());
            surfaceDataNew->retrieve();
            int numI = static_cast<int>(surfaceDataNew->getGeometry()->getNumI());
            int numJ = static_cast<int>(surfaceDataNew->getGeometry()->getNumJ());
            index = 0;
            for (int j = 0; j < numJ; j++)
            {
               for (int k = 0; k < numI && index < sizeSurfaceData; k++)
               {
                  EXPECT_EQ(surfaceDataValue[index++], surfaceDataNew->getValue(k, j));
               }
            }
            break;
         }
      }
   }

   if (indexAddedPropVolData.size() > 0)
   {
      //Retrieving volume data to which property volume data was added
      int index = indexAddedPropVolData.at(0);
      shared_ptr<SnapShot> snapShotNew = projectXml->getSnapShots()[index];
      index = indexAddedPropVolData.at(1);
      std::shared_ptr<CauldronIO::Volume> volNew = snapShotNew->getFormationVolumeList().at(index).second;

      //Comparing and checking if the property volume data has been added correctly
      size_t nPropVolData = volNew->getPropertyVolumeDataList().size();
      for (size_t i = 0; i < nPropVolData; i++)
      {
         PropertyVolumeData propVolDataNew = volNew->getPropertyVolumeDataList().at(i);

         if (propVolDataNew.first->getName() == prop2->getName())
         {
            shared_ptr<const Property> propNew = propVolDataNew.first;
            shared_ptr<VolumeData> volDataNew = propVolDataNew.second;

            EXPECT_EQ(prop2->getName(), propNew->getName());
            EXPECT_EQ(prop2->getCauldronName(), propNew->getCauldronName());
            EXPECT_EQ(prop2->getAttribute(), propNew->getAttribute());
            EXPECT_EQ(prop2->getType(), propNew->getType());
            EXPECT_EQ(prop2->getUnit(), propNew->getUnit());
            EXPECT_EQ(prop2->getUserName(), propNew->getUserName());
            volDataNew->retrieve();
            int numI = static_cast<int>(volDataNew->getGeometry()->getNumI());
            int numJ = static_cast<int>(volDataNew->getGeometry()->getNumJ());
            int numK = static_cast<int>(volDataNew->getGeometry()->getNumK());
            int firstK = static_cast<int>(volDataNew->getGeometry()->getFirstK());
            index = 0;
            for (int p = 0; p < numJ; p++)
            {
               for (int q = 0; q < numI; q++)
               {
                  for (int r = firstK; r < numK + firstK && index < sizeVolumeData; r++)
                  {
                     EXPECT_EQ(volumeDataValue[index++], volDataNew->getValue(q, p, r));
                  }
               }
            }
            break;
         }
      }
   }
}

//Function to add a new snapshot with binary data and check if it has been added correctly or not
void addNewData(string xmlFileName)
{
   //Import from XML
   cout << "Starting import from XML" << endl;
   shared_ptr<CauldronIO::Project> projectXml = CauldronIO::ImportFromXML::importFromXML(xmlFileName);

   //Preparing new snapshot to be added
   size_t nSnapshots = projectXml->getSnapShots().size();
   double age = projectXml->getSnapShots()[nSnapshots - 1]->getAge();
   age = age + 5;
   shared_ptr<SnapShot> snapShot(new SnapShot(age, CauldronIO::SnapShotKind::SYSTEM, true));

   //Preparing surface to be added
   const string surfaceName = "waterbottom";
   const string propName = "Depth";
   const string unit = "m";
   shared_ptr<const Property> prop(new Property(propName, propName, propName, unit, FormationProperty, Continuous3DProperty));
   shared_ptr<const Geometry2D> geometry(new Geometry2D(2, 2, 1, 1, 0, 0));
   shared_ptr<SurfaceData> valueMap(new MapNative(geometry));
    projectXml->addGeometry(geometry);

   //Preparing property surface data
   float *data1 = new float[2 * 2];
   for (int i = 0; i < 4; i++)
   {
      data1[i] = 1.1f + i;
   }
   valueMap->setData_IJ(data1);

   //Adding surface to Snapshot
   shared_ptr<Surface> surface(new Surface(surfaceName, CauldronIO::SubsurfaceKind::Sediment));
   PropertySurfaceData propSurface = PropertySurfaceData(prop, valueMap);
   surface->addPropertySurfaceData(propSurface);
   snapShot->addSurface(surface);

   //Preparing formation to be added
   int kStart, kEnd;
   const string formName = projectXml->getFormations()[0]->getName();
   projectXml->getFormations()[0]->getK_Range(kStart, kEnd);
   shared_ptr<const Formation> formation(new Formation(kStart, kEnd, formName));
   shared_ptr<Volume> vol(new Volume(Sediment));

   //Preparing property volume data
   shared_ptr<const Property> prop2(new Property(projectXml->getProperties()[0]->getName(), projectXml->getProperties()[0]->getUserName(), projectXml->getProperties()[0]->getCauldronName(), projectXml->getProperties()[0]->getUnit(), projectXml->getProperties()[0]->getType(), projectXml->getProperties()[0]->getAttribute()));
   const shared_ptr<Geometry3D> geo3d(new Geometry3D(1, 2, 3, 4, 1.1, 2.1, 0.5, 2.5));
   shared_ptr<VolumeData> volData(new VolumeDataNative(geo3d));
    projectXml->addGeometry(geo3d);
   float data[6] = { 1.1f, 2.1f, 1.2f ,1.3f,2.2f,2.3f};
   volData->setData_KIJ(data);

   //Adding formation volume
   PropertyVolumeData propVolData(prop2, volData);
   vol->addPropertyVolumeData(propVolData);
   CauldronIO::FormationVolume formVol = FormationVolume(formation, vol);
   snapShot->addFormationVolume(formVol);

   //Adding snapshot
   projectXml->addSnapShot(snapShot);

   //Exporting data
   shared_ptr<CauldronIO::Project> projectExisting;
   bool status = CauldronIO::ExportToXML::exportToXML(projectXml, projectExisting, xmlFileName);
   ASSERT_EQ(true, status);

   //Importing data to check if data is added correctly
   projectXml = CauldronIO::ImportFromXML::importFromXML(xmlFileName);

   //Comparing snapshot
   nSnapshots = projectXml->getSnapShots().size();
   shared_ptr<SnapShot> snapShotNew = projectXml->getSnapShots()[nSnapshots - 1];
   EXPECT_EQ(age, snapShotNew->getAge());
   EXPECT_EQ(SYSTEM, snapShotNew->getKind());
   EXPECT_EQ(true, snapShotNew->isMinorShapshot());

   //Comparing surface and property surface data
   size_t nSurfacesNew = snapShotNew->getSurfaceList().size();
   EXPECT_EQ(1, nSurfacesNew);

   shared_ptr<Surface> surfaceNew = snapShotNew->getSurfaceList()[0];
   EXPECT_EQ(surfaceName, surfaceNew->getName());
   EXPECT_EQ(CauldronIO::SubsurfaceKind::Sediment, surfaceNew->getSubSurfaceKind());
   size_t nPropSurfaceDataNew = surfaceNew->getPropertySurfaceDataList().size();

   EXPECT_EQ(1, nPropSurfaceDataNew);
   PropertySurfaceData propSurfaceDataNew = surfaceNew->getPropertySurfaceDataList()[0];
   shared_ptr<const Property> propNew = propSurfaceDataNew.first;
   shared_ptr<SurfaceData> valueMapNew = propSurfaceDataNew.second;
   valueMapNew->retrieve();

   EXPECT_EQ(propName, propNew->getName());
   EXPECT_EQ(propName, propNew->getUserName());
   EXPECT_EQ(propName, propNew->getCauldronName());
   EXPECT_EQ(unit, propNew->getUnit());
   EXPECT_EQ(FormationProperty, propNew->getType());
   EXPECT_EQ(Continuous3DProperty, propNew->getAttribute());

   EXPECT_EQ(2, valueMapNew->getGeometry()->getNumI());
   EXPECT_EQ(2, valueMapNew->getGeometry()->getNumJ());
   EXPECT_EQ(1, valueMapNew->getGeometry()->getDeltaI());
   EXPECT_EQ(1, valueMapNew->getGeometry()->getDeltaJ());
   EXPECT_EQ(0, valueMapNew->getGeometry()->getMinI());
   EXPECT_EQ(0, valueMapNew->getGeometry()->getMinJ());

   int index = 0;
   for (int i = 0; i < 2; i++)
   {
      for (int j = 0; j < 2; j++)
      {
         EXPECT_EQ(data1[index++], valueMapNew->getValue(j, i));
      }
   }
   //Comparing formation volume data
   size_t nFormationVolumesNew = snapShotNew->getFormationVolumeList().size();
   EXPECT_EQ(1, nFormationVolumesNew);

   CauldronIO::FormationVolume formVolNew = snapShotNew->getFormationVolumeList()[0];
   shared_ptr<const Formation> formationNew = formVolNew.first;
   int kStartNew, kEndNew;
   formationNew->getK_Range(kStartNew, kEndNew);
   shared_ptr<Volume> volNew = formVolNew.second;
   EXPECT_EQ(formName, formationNew->getName());
   EXPECT_EQ(false, formationNew->isMobileLayer());
   EXPECT_EQ(false, formationNew->isSourceRock());
   EXPECT_EQ(kStart, kStartNew);
   EXPECT_EQ(kEnd, kEndNew);
   EXPECT_EQ(Sediment, volNew->getSubSurfaceKind());
   size_t nPropVolDataNew = volNew->getPropertyVolumeDataList().size();
   EXPECT_EQ(1, nPropVolDataNew);

   PropertyVolumeData propVolDataNew = volNew->getPropertyVolumeDataList()[0];
   shared_ptr<const Property> propNew2 = propVolDataNew.first;
   shared_ptr<VolumeData> volDataNew = propVolDataNew.second;

   EXPECT_EQ(prop2->getName(), propNew2->getName());
   EXPECT_EQ(prop2->getUserName(), propNew2->getUserName());
   EXPECT_EQ(prop2->getCauldronName(), propNew2->getCauldronName());
   EXPECT_EQ(prop2->getUnit(), propNew2->getUnit());
   EXPECT_EQ(prop2->getType(), propNew2->getType());
   EXPECT_EQ(prop2->getAttribute(), propNew2->getAttribute());

   EXPECT_EQ(1, volDataNew->getGeometry()->getNumI());
   EXPECT_EQ(2, volDataNew->getGeometry()->getNumJ());
   EXPECT_EQ(3, volDataNew->getGeometry()->getNumK());
   EXPECT_EQ(4, volDataNew->getGeometry()->getFirstK());
   EXPECT_EQ(1.1, volDataNew->getGeometry()->getDeltaI());
   EXPECT_EQ(2.1, volDataNew->getGeometry()->getDeltaJ());
   EXPECT_EQ(0.5, volDataNew->getGeometry()->getMinI());
   EXPECT_EQ(2.5, volDataNew->getGeometry()->getMinJ());

   int firstKNew = static_cast<int>(volDataNew->getGeometry()->getFirstK());
   int numKNew = static_cast<int>(volDataNew->getGeometry()->getNumK());
   volDataNew->retrieve();
   index = 0;
   for (int i = 0; i < 1; i++)
   {
      for (int j = 0; j < 2; j++)
      {
         for (int k = firstKNew; k < firstKNew + numKNew; k++)
         {
            EXPECT_EQ(data[index++], volDataNew->getValue(i, j, k));
         }
      }
   }
}

//Function to copy directory recursively
bool copyDir(boost::filesystem::path const & source, boost::filesystem::path const & destination)
{
   namespace fs = boost::filesystem;
   // Check whether the source path is valid
   if (!fs::exists(source) || !fs::is_directory(source))
   {
      std::cerr << "Source directory " << source.string()
         << " does not exist or is not a directory." << '\n'
         ;
      return false;
   }
   if (fs::exists(destination))
   {
      std::cerr << "Destination directory " << destination.string()
         << " already exists." << '\n'
         ;
      return false;
   }
   // Create the destination directory
   if (!fs::create_directory(destination))
   {
      std::cerr << "Unable to create destination directory"
         << destination.string() << '\n'
         ;
      return false;
   }

   // Iterate through the source directory
   for (fs::directory_iterator file(source); file != fs::directory_iterator(); ++file)
   {
      fs::path current(file->path());
      if (fs::is_directory(current))
      {
         // Found directory: Recursion
         if (!copyDir(current,destination / current.filename()))
         {
            return false;
         }
      }
      else
      {
         // Found file: Copy
         fs::copy_file(current,destination / current.filename());
      }
   }
   return true;
}

//Function to create temporary copy of dataset
string createTemporaryDataset(string xmlFileName)
{
   boost::filesystem::path originalPath(xmlFileName);
   boost::filesystem::path destPath = originalPath.parent_path();
   destPath = destPath.parent_path();
   destPath /= "testData";
   boost::filesystem::path sourcePath = originalPath.parent_path();
   if (boost::filesystem::exists(destPath))
   {
      boost::filesystem::remove_all(destPath);
   }
   bool status = copyDir(sourcePath, destPath);
   if (status == true)
   {
      return destPath.string();
   }
   else
   {
      cout << "Copying failed" << endl;
      return "error";
   }
}

int convertToXML(string filePathP3d, DataAccess::Interface::ObjectFactory *factoryP3d)
{
   //Converting project3d to xml

   // Try to open the projectHandle
   cout << "Opening project handle " << endl;
   unique_ptr<DataAccess::Interface::ProjectHandle> projectHandle(DataAccess::Interface::OpenCauldronProject(filePathP3d, factoryP3d));
   if (!projectHandle)
   {
      cerr << "Could not open the project handle" << endl;
      return 1;
   }
   cout << "Finished opening project handle" << endl;

   // Import from ProjectHandle
   cout << "Importing from project handle (requires reading depth formations)" << endl;
   shared_ptr<CauldronIO::Project> project = ImportProjectHandle::createFromProjectHandle(*projectHandle, false);
   cout << "Finished import " << endl;

   // Export to native format: it will retrieve data when needed
   cout << "Writing to xml format" << endl;

   // Construct output path
   ibs::FilePath absPath(filePathP3d);
   int numThreads = 1;
   shared_ptr<CauldronIO::Project> existingProject;
   bool status = CauldronIO::ExportToXML::exportToXML(project, existingProject, absPath.path(), numThreads);
   if (status == true)
   {
      cout << "Wrote to xml format " << endl;
      return 0;
   }
   else
   {
      cout << "Unable to write in xml format" << endl;
      return 1;
   }
}

void compareTrappers(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d) {

   DataAccess::Interface::TrapperList * trapperList3d =  projectP3d->getTrappers(0, 0, 0, 0);
   const CauldronIO::TrapperList * trapperList = & (projectXml->getTrapperTable());
   EXPECT_EQ(trapperList3d->size(), trapperList->size() );
   for (size_t index = 0; index < trapperList3d->size(); ++ index)
   {
      const DataAccess::Interface::Trapper* trapper3d = trapperList3d->at(index);
      std::shared_ptr<CauldronIO::Trapper> trapper = trapperList->at(index);
      EXPECT_EQ( trapper3d->getId(), trapper->getID() );
      EXPECT_EQ( trapper3d->getPersistentId(), trapper->getPersistentID() );

      if( trapper3d->getDownstreamTrapper() ) {
         EXPECT_EQ( trapper3d->getDownstreamTrapper()->getPersistentId(), trapper->getDownStreamTrapperID() );
      } else {
         EXPECT_EQ( trapper->getDownStreamTrapperID(), -1 );
      }

      EXPECT_FLOAT_EQ( (float)trapper3d->getVolume( PhaseId::OIL, PhaseId::GAS ), trapper->getSolutionGasVolume() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getVolume( PhaseId::GAS, PhaseId::GAS ), trapper->getFreeGasVolume() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getVolume( PhaseId::OIL, PhaseId::OIL ), trapper->getStockTankOilVolume() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getVolume( PhaseId::GAS, PhaseId::OIL ), trapper->getCondensateVolume() );

      EXPECT_FLOAT_EQ( (float)trapper3d->getViscosity( PhaseId::OIL, PhaseId::GAS ), trapper->getSolutionGasViscosity() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getViscosity( PhaseId::GAS, PhaseId::GAS ), trapper->getFreeGasViscosity() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getViscosity( PhaseId::OIL, PhaseId::OIL ), trapper->getStockTankOilViscosity() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getViscosity( PhaseId::GAS, PhaseId::OIL ), trapper->getCondensateViscosity() );

      EXPECT_FLOAT_EQ( (float)trapper3d->getDensity( PhaseId::OIL, PhaseId::GAS ), trapper->getSolutionGasDensity() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getDensity( PhaseId::GAS, PhaseId::GAS ), trapper->getFreeGasDensity() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getDensity( PhaseId::OIL, PhaseId::OIL ), trapper->getStockTankOilDensity() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getDensity( PhaseId::GAS, PhaseId::OIL ), trapper->getCondensateDensity() );

      EXPECT_FLOAT_EQ( (float)trapper3d->getMass( PhaseId::OIL, PhaseId::GAS ), trapper->getSolutionGasMass() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getMass( PhaseId::GAS, PhaseId::GAS ), trapper->getFreeGasMass() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getMass( PhaseId::OIL, PhaseId::OIL ), trapper->getStockTankOilMass() );
      EXPECT_FLOAT_EQ( (float)trapper3d->getMass( PhaseId::GAS, PhaseId::OIL ), trapper->getCondensateMass() );

      for( int compId = 0; compId < CauldronIO::SpeciesNamesId::NUMBER_OF_SPECIES; ++ compId ) {
         EXPECT_EQ( trapper3d->getMass( PhaseId::OIL, PhaseId::GAS, (ComponentId)compId ), trapper->getSolutionGasMass( (CauldronIO::SpeciesNamesId)compId) );
         EXPECT_EQ( trapper3d->getMass( PhaseId::GAS, PhaseId::GAS, (ComponentId)compId ), trapper->getFreeGasMass( (CauldronIO::SpeciesNamesId)compId) );
         EXPECT_EQ( trapper3d->getMass( PhaseId::GAS, PhaseId::OIL, (ComponentId)compId ), trapper->getCondensateMass( (CauldronIO::SpeciesNamesId)compId) );
         EXPECT_EQ( trapper3d->getMass( PhaseId::OIL, PhaseId::OIL, (ComponentId)compId ), trapper->getStockTankOilMass( (CauldronIO::SpeciesNamesId)compId) );
      }

      EXPECT_FLOAT_EQ( trapper->getVolumeOil(), (float)trapper3d->getVolume(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getVolumeGas(), (float)trapper3d->getVolume(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getMassVapour(), (float)trapper3d->getMass(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getMassLiquid(), (float)trapper3d->getMass(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getViscosityVapour(), (float)trapper3d->getViscosity(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getViscosityLiquid(), (float)trapper3d->getViscosity(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getDensityVapour(), (float)trapper3d->getDensity(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getDensityLiquid(), (float)trapper3d->getDensity(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getCEPGas(), (float)trapper3d->getCEP(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getCEPOil(), (float)trapper3d->getCEP(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getCriticalTemperatureOil(), (float)trapper3d->getCriticalTemperature(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getCriticalTemperatureGas(), (float)trapper3d->getCriticalTemperature(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getInterfacialTensionOil(), (float)trapper3d->getInterfacialTension(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trapper->getInterfacialTensionGas(), (float)trapper3d->getInterfacialTension(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trapper->getOilAPI(), (float)trapper3d->getOilAPI() );
      EXPECT_FLOAT_EQ( trapper->getCGR(), (float)trapper3d->getCGR() );
      EXPECT_FLOAT_EQ( trapper->getGOR(), (float)trapper3d->getGOR() );
      EXPECT_FLOAT_EQ( trapper->getFracturePressure(), (float)trapper3d->getFracturePressure() );
      EXPECT_FLOAT_EQ( trapper->getBuoyancy(), (float)trapper3d->getBuoyancy() );
      EXPECT_FLOAT_EQ( trapper->getWCSurface(), (float)trapper3d->getWCSurface() );
      EXPECT_STREQ( trapper->getReservoirName ().c_str(), trapper3d->getReservoir ()->getName().c_str());;

      double x3d, y3d;
      float  x, y;
      trapper3d->getPosition(x3d, y3d);
      trapper->getPosition(x, y);
      EXPECT_FLOAT_EQ((float)x3d, x);
      EXPECT_FLOAT_EQ((float)y3d, y);

      trapper3d->getSpillPointPosition(x3d, y3d);
      trapper->getSpillPointPosition(x, y);
      EXPECT_FLOAT_EQ((float)x3d, x);
      EXPECT_FLOAT_EQ((float)y3d, y);

      EXPECT_FLOAT_EQ( trapper->getTrapCapacity(), (float)trapper3d->getCapacity());
      EXPECT_FLOAT_EQ( trapper->getDepth(), (float)trapper3d->getDepth());
      EXPECT_FLOAT_EQ( trapper->getGOC(),(float)trapper3d->getGOC());
      EXPECT_FLOAT_EQ( trapper->getOWC(), (float)trapper3d->getOWC());
      EXPECT_FLOAT_EQ( trapper->getSpillDepth(),(float)trapper3d->getSpillDepth());
      EXPECT_FLOAT_EQ( trapper->getPressure(),(float)trapper3d->getPressure());
      EXPECT_FLOAT_EQ( trapper->getTemperature(),(float)trapper3d->getTemperature());
      EXPECT_FLOAT_EQ( trapper->getPermeability(),(float)trapper3d->getPermeability());
      EXPECT_FLOAT_EQ( trapper->getSealPermeability(),(float)trapper3d->getSealPermeability());
      EXPECT_FLOAT_EQ( trapper->getPorosity(),(float)trapper3d->getPorosity());
      EXPECT_FLOAT_EQ( trapper->getNetToGross(),(float)trapper3d->getNetToGross());
      EXPECT_FLOAT_EQ( trapper->getAge(),(float)trapper3d->getSnapshot()->getTime());
   }
   if( trapperList3d->size() > 0 ) {
      cout << "TrapperIoTbl comparison done" << endl;
   }
}

void compareTraps(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d) {

   DataAccess::Interface::TrapList * trapList3d =  projectP3d->getTraps(0, 0, 0);
   const CauldronIO::TrapList * trapList = & (projectXml->getTrapTable());
   EXPECT_EQ(trapList3d->size(), trapList->size() );
   for (size_t index = 0; index < trapList3d->size(); ++ index)
   {
      const DataAccess::Interface::Trap* trap3d = trapList3d->at(index);
      database::Record* record = trap3d->getRecord();
      std::shared_ptr<CauldronIO::Trap> trap = trapList->at(index);

      EXPECT_EQ( trap3d->getId(), trap->getID() );
      EXPECT_STREQ( trap->getReservoirName ().c_str(), trap3d->getReservoir ()->getName().c_str());;
      for( int compId = 0; compId < CauldronIO::SpeciesNamesId::NUMBER_OF_SPECIES; ++ compId ) {
         EXPECT_EQ( trap3d->getMass( (ComponentId)compId ), trap->getMass( (CauldronIO::SpeciesNamesId)compId) );
      }
      EXPECT_FLOAT_EQ( trap->getVolumeOil(), (float)trap3d->getVolume(PhaseId::OIL) );
      EXPECT_FLOAT_EQ( trap->getVolumeGas(), (float)trap3d->getVolume(PhaseId::GAS) );
      EXPECT_FLOAT_EQ( trap->getCEPGas(), (float)getCEPGas(record));
      EXPECT_FLOAT_EQ( trap->getCEPOil(), (float)getCEPOil(record));
      EXPECT_FLOAT_EQ( trap->getCriticalTemperatureOil(), (float)getCriticalTemperatureOil(record));
      EXPECT_FLOAT_EQ( trap->getCriticalTemperatureGas(), (float)getCriticalTemperatureGas(record));
      EXPECT_FLOAT_EQ( trap->getInterfacialTensionOil(), (float)getInterfacialTensionOil(record));
      EXPECT_FLOAT_EQ( trap->getInterfacialTensionGas(), (float)getInterfacialTensionGas(record));
      EXPECT_FLOAT_EQ( trap->getFracturePressure(), (float)getFracturePressure(record));
      EXPECT_FLOAT_EQ( trap->getFractSealStrength(), (float)getFractSealStrength(record));
      EXPECT_FLOAT_EQ( trap->getWCSurface(), (float)getWCSurface(record));

      double x3d, y3d;
      float  x, y;
      trap3d->getPosition(x3d, y3d);
      trap->getPosition(x, y);
      EXPECT_FLOAT_EQ( (float)x3d, x );
      EXPECT_FLOAT_EQ( (float)y3d, y );

      trap3d->getSpillPointPosition(x3d, y3d);
      trap->getSpillPointPosition(x, y);
      EXPECT_FLOAT_EQ( (float)x3d, x );
      EXPECT_FLOAT_EQ( (float)y3d, y );

      EXPECT_FLOAT_EQ( trap->getTrapCapacity(), (float)getTrapCapacity(record));
      EXPECT_FLOAT_EQ( trap->getDepth(), (float)trap3d->getDepth());
      EXPECT_FLOAT_EQ( trap->getGOC(), (float)trap3d->getGOC());
      EXPECT_FLOAT_EQ( trap->getOWC(), (float)trap3d->getOWC());
      EXPECT_FLOAT_EQ( trap->getSpillDepth(), (float)trap3d->getSpillDepth());
      EXPECT_FLOAT_EQ( trap->getPressure(), (float)trap3d->getPressure());
      EXPECT_FLOAT_EQ( trap->getTemperature(), (float)trap3d->getTemperature());
      EXPECT_FLOAT_EQ( trap->getPermeability(), (float)getPermeability(record));
      EXPECT_FLOAT_EQ( trap->getSealPermeability(), (float)getSealPermeability(record));
      EXPECT_FLOAT_EQ( trap->getNetToGross(), (float)getNetToGross(record));
      EXPECT_FLOAT_EQ( trap->getAge(), (float)trap3d->getSnapshot()->getTime());
   }
   if( trapList3d->size() > 0 ) {
      cout << "TrapIoTbl comparison done" << endl;
   }
}

void compareDataDriller(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d) {

   float value;

   value =  projectXml->getPropertyAtLocation( 0, "Temperature", 2000, 2000, 38992.9, "", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(550.65936, value);

   value =  projectXml->getPropertyAtLocation( 15, "Vr", 6500, 6500, DefaultUndefinedScalarValue, "", "s2", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(0.70490456, value);

   value =  projectXml->getPropertyAtLocation( 15, "Temperature", 6500, 6500, DefaultUndefinedScalarValue, "", "s2", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(123.25272, value);

   value =  projectXml->getPropertyAtLocation( 15, "Temperature", 2000, 2000, DefaultUndefinedScalarValue, "", "s4", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(237.16568, value);

   value =  projectXml->getPropertyAtLocation( 15, "VolumeSGIIP", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(5820988849.02242f, value);

   value =  projectXml->getPropertyAtLocation( 15, "MassSTOIIP", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(22505439209.0994f, value);

   value =  projectXml->getPropertyAtLocation( 15, "DensitySTOIIP", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ( 815.849296424602f, value);

   value =  projectXml->getPropertyAtLocation( 15, "MassLiquid", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ(  29576816190.6786f, value);

   value =  projectXml->getPropertyAtLocation( 15, "MassVapour", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ( 0, value);

   value =  projectXml->getPropertyAtLocation( 15, "OilAPI", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ( 41.9388944381188f, value);

   value =  projectXml->getPropertyAtLocation( 15, "Depth", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ( 2258.05810546875f, value);

   value =  projectXml->getPropertyAtLocation( 15, "Porosity", 5000, 5000, DefaultUndefinedScalarValue, "reservoir 1", "", "");
   if( value == DefaultUndefinedValue) value = DefaultUndefinedScalarValue;
   EXPECT_FLOAT_EQ( 47.2822151184082f, value);

   cout << "DataDriller comparison done" << endl;

}

void compareGenexHistory(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d) {

   const std::vector<std::string> &historyFiles = projectXml->getGenexHistoryList();

   EXPECT_EQ( historyFiles.size(), 2 );

   ibs::FilePath folderPath(projectP3d->getFullOutputDir());

   folderPath << "History_ARD_simple-test_genex_sr_5500_4500.dat";
   const std::string record1 = folderPath.path();

   folderPath.cutLast();
   folderPath << "HistoryGenex.txt";
   const std::string record2 = folderPath.path();

   bool recordExist = std::find(historyFiles.begin(), historyFiles.end(), record1) != historyFiles.end();
   EXPECT_EQ(recordExist, true);

   recordExist = std::find(historyFiles.begin(), historyFiles.end(), record2) != historyFiles.end();
   EXPECT_EQ(recordExist, true);
}

void compareMassBalance(std::shared_ptr<CauldronIO::Project> projectXml, std::unique_ptr<DataAccess::Interface::ProjectHandle> &projectP3d) {

   const std::string& massBalance = projectXml->getMassBalance();
   const std::string  massBalance3D = projectP3d->getProjectName() + "_MassBalance";

   EXPECT_STREQ( massBalance.c_str(), massBalance3D.c_str() );
}

// Test to compare metadata and property values in binary data stored in project3d and XML formats. The property values include both 2D and 3D.

TEST_F(CompareTest, CompareData1)
{
   boost::filesystem::path filePathP3d = "data/ARD_simple-test.project3d";
   string fileNameP3d = filePathP3d.filename().string();

   cout << "Creating temporary copy of data" << endl;
   string testDataPath = createTemporaryDataset(filePathP3d.string());
   ASSERT_NE("error", testDataPath);
   boost::filesystem::path newFilePathP3d(testDataPath);
   newFilePathP3d /= fileNameP3d;

   int check = convertToXML(newFilePathP3d.string(),m_factoryP3d);
   ASSERT_EQ(check, 0);
   string fileNameXml = newFilePathP3d.stem().string() + ".xml";
   boost::filesystem::path newFilePathXml(newFilePathP3d.parent_path());
   newFilePathXml /= fileNameXml;

   //Open from project3d
   cout << "Opening project handle " << endl;
   std::unique_ptr<DataAccess::Interface::ProjectHandle> projectP3d(DataAccess::Interface::OpenCauldronProject(newFilePathP3d.string(), m_factoryP3d));
   ASSERT_EQ(true, (bool)projectP3d);

   //Import from XML
   cout << "Starting import from XML" << endl;
   std::shared_ptr<CauldronIO::Project> projectXml = CauldronIO::ImportFromXML::importFromXML(newFilePathXml.string());
   cout << "Comparing data" << endl;
   //Comparing Project information
   EXPECT_EQ(projectP3d->getModellingMode(), projectXml->getModelingMode());

   compareSnapshots(projectXml, projectP3d);
   compareFormations(projectXml, projectP3d);
   compareReservoirs(projectXml, projectP3d);
   compareProperties(projectXml, projectP3d);
   compare2dPropertyValues(projectXml, projectP3d);
   compare3dDiscontinuousPropertyValues(projectXml, projectP3d);
   compare3dContinuousPropertyValues(projectXml, projectP3d);
   compareTrappers(projectXml, projectP3d);
   compareTraps(projectXml, projectP3d);
        compareGenexHistory(projectXml, projectP3d);
        compareMassBalance(projectXml, projectP3d);

        compareDataDriller(projectXml, projectP3d);

   newFilePathP3d = testDataPath;

   cout << "Deleting temporary copy of data" << endl;
   boost::filesystem::remove_all(newFilePathP3d);
}

// Test to add and update a property value in an existing snapshot and check if it has been added and updated correctly
TEST_F(CompareTest, CheckAddToExisting1)
{
   boost::filesystem::path filePathP3d = "data/ARD_simple-test.project3d";
   string fileNameP3d = filePathP3d.filename().string();

   cout << "Creating temporary copy of data" << endl;
   string testDataPath = createTemporaryDataset(filePathP3d.string());
   ASSERT_NE("error", testDataPath);
   boost::filesystem::path newFilePathP3d(testDataPath);
   newFilePathP3d /= fileNameP3d;

   int check = convertToXML(newFilePathP3d.string(),m_factoryP3d);
   ASSERT_EQ(check, 0);
   string fileNameXml = newFilePathP3d.stem().string() + ".xml";
   boost::filesystem::path newFilePathXml(newFilePathP3d.parent_path());
   newFilePathXml /= fileNameXml;

   cout << "Adding data to existing snapshot" << endl;
   addToExistingData(newFilePathXml.string());
   newFilePathP3d = testDataPath;

   cout << "Deleting temporary copy of data" << endl;
   boost::filesystem::remove_all(newFilePathP3d);

}

// Test to add a new snapshot with binary data and check if it has been added correctly
TEST_F(CompareTest, CheckAddNew1)
{
   boost::filesystem::path filePathP3d = "data/ARD_simple-test.project3d";
   string fileNameP3d = filePathP3d.filename().string();

   cout << "Creating temporary copy of data" << endl;
   string testDataPath = createTemporaryDataset(filePathP3d.string());
   ASSERT_NE("error", testDataPath);
   boost::filesystem::path newFilePathP3d(testDataPath);
   newFilePathP3d /= fileNameP3d;

   int check = convertToXML(newFilePathP3d.string(),m_factoryP3d);
   ASSERT_EQ(check, 0);
   string fileNameXml = newFilePathP3d.stem().string() + ".xml";
   boost::filesystem::path newFilePathXml(newFilePathP3d.parent_path());
   newFilePathXml /= fileNameXml;

   cout << "Adding new data" << endl;
   addNewData(newFilePathXml.string());

   newFilePathP3d = testDataPath;

   cout << "Deleting temporary copy of data" << endl;
   boost::filesystem::remove_all(newFilePathP3d);
}


// Test unsigned long long support
TEST(VerifyData, CheckUnsignedLongLong)
{
   static_assert(sizeof(unsigned long long) >= 8, "Size of unsigned long long is less than 8 bytes");

   unsigned long long value = 0xffffffffffffffff;// std::numeric_limits<unsigned long long>::max();
   const std::string fileName = "projectUnsignedLongLong.xml";

   // Write XML with unsigned long long attribute
   {
      pugi::xml_document docWrite;
      pugi::xml_node root = docWrite.append_child("project");
      root.append_attribute("size") = value;

      EXPECT_TRUE( docWrite.save_file(fileName.c_str()) );
   }

   {
      pugi::xml_document docRead;
      pugi::xml_parse_result result = docRead.load_file(fileName.c_str());
      EXPECT_EQ( result.status, pugi::xml_parse_status::status_ok );

      unsigned long long valueRead = docRead.child("project").attribute("size").as_ullong();
      EXPECT_EQ( valueRead, value );
   }
}
