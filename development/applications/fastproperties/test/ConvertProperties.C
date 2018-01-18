#include <iostream>
#include <cmath>
#include <cassert>
#include <boost/foreach.hpp>
#include <cstdlib>

#include <gtest/gtest.h>

#include "../src/Utilities.h"

struct MPIHelper
{
   MPIHelper()
   {
      MPI_Init(NULL, NULL);
   }

   ~MPIHelper()
   {
      MPI_Finalize();
   }
   static MPIHelper & instance()
   {
      static MPIHelper object;
      return object;
   }
   static int rank() 
   { 
      instance();

      int rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      return rank;
   }
};

class TestUnit
{
public:
   TestUnit();
   ~TestUnit();

   void AddDerivedContVolume();

   std::shared_ptr<CauldronIO::Project> vizProject;
   int numIGlobal;
   int numJGlobal;
   int minIGlobal;
   int minJGlobal;

   int numILocal;
   int numJLocal;
   int minILocal;
   int minJLocal;
   int minK;
   int numK;
      
   int dataSize;

   MPI_Op mop;

   const string pname = "Depth";
   const string pnameToAdd = "LithoStaticPressure";

   std::shared_ptr<CauldronIO::FormationInfoList> formInfoList;
   std::shared_ptr<CauldronIO::SnapShot> snapshot;
   std::shared_ptr<CauldronIO::Geometry3D> geometry;
   vector<float> inData;
};

TestUnit::TestUnit() {
   
   int rank = MPIHelper::rank();

   numIGlobal = 10;
   numJGlobal = 10;
   minIGlobal = 0;
   minJGlobal = 0;

   numILocal = (numIGlobal - minIGlobal) / 2;
   numJLocal = (numJGlobal - minJGlobal);

   minILocal = numILocal * rank + minIGlobal;
   minJLocal = 0;

   minK = 0;
   numK = 10;
 
   dataSize = numIGlobal * numJGlobal * numK;

   inData.resize(dataSize);

   vizProject = std::shared_ptr<CauldronIO::Project>(new CauldronIO::Project("testProject", "testDescription", "1.0", CauldronIO::MODE3D, 1, 0));
   snapshot = std::shared_ptr<CauldronIO::SnapShot>(new CauldronIO::SnapShot(10.11, CauldronIO::SYSTEM, false));
   geometry = std::shared_ptr< CauldronIO::Geometry3D>(new CauldronIO::Geometry3D(numIGlobal, numJGlobal, numK - minK, minK, 500, 500, minIGlobal, minJGlobal));
   std::shared_ptr<CauldronIO::Formation> formation1(new CauldronIO::Formation(minK, 3, "Formation1"));
   std::shared_ptr<CauldronIO::Formation> formation2(new CauldronIO::Formation(3, numK, "Formation2"));
   std::shared_ptr<Volume> volume(new Volume(CauldronIO::Sediment));

   snapshot->setVolume(volume);
   vizProject->addFormation(formation1);
   vizProject->addFormation(formation2);
   vizProject->addSnapShot(snapshot);
   vizProject->addGeometry(geometry);

   MPI_Op_create((MPI_User_function *)minmax_op, true, & mop);
 }

TestUnit::~TestUnit() {      

   MPI_Op_free (&mop);  
}
   
void TestUnit::AddDerivedContVolume() {

   int rank = MPIHelper::rank();

   shared_ptr<const CauldronIO::Property> vizProperty(new const CauldronIO::Property(pnameToAdd, pnameToAdd, pnameToAdd, "unit", CauldronIO::FormationProperty,  CauldronIO::Continuous3DProperty));
   vizProject->addProperty(vizProperty);

   std::shared_ptr<CauldronIO::Volume> snapshotVolume = snapshot->getVolume();

   shared_ptr<CauldronIO::VolumeData> volDataNew(new CauldronIO::VolumeDataNative(geometry, CauldronIO::DefaultUndefinedValue));
 
   float *data = &inData[0];
   memset(data, 0, sizeof(float) * dataSize);
      
   volDataNew->setData_IJK(data);
   float * internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());

   CauldronIO::FormationList formations = vizProject->getFormations();

   float sedimentMinValue = (rank == 0 ? 0 : CauldronIO::DefaultUndefinedValue);
   float sedimentMaxValue = rank + 41.0;

   BOOST_FOREACH(const std::shared_ptr<CauldronIO::Formation>& formation, formations)
   {
      int firstK, lastK;
      formation->getK_Range(firstK, lastK);

      for (int j = minJLocal; j <  minJLocal + numJLocal; ++j) {
         for (int i = minILocal; i < minILocal + numILocal; ++i) {
            for (int k = lastK - 1; k >= firstK; --k) {
              internalData[volDataNew->computeIndex_IJK(i, j, k)] = static_cast<float>(rank + 1);
            }
         }
      }

      volDataNew->setSedimentMinMax(sedimentMinValue, sedimentMaxValue);
   }

   CauldronIO::PropertyVolumeData propVolDataNew(vizProperty, volDataNew);
   snapshotVolume->addPropertyVolumeData(propVolDataNew);

   float *dataCollect = new float[dataSize];
   memset(dataCollect, 0, sizeof(float) * dataSize);
   MPI_Reduce((void *)internalData, (void *)dataCollect, dataSize, MPI_FLOAT, MPI_SUM, 0,  MPI_COMM_WORLD);

   if(rank == 0) {
      std::memcpy(internalData, dataCollect, dataSize * sizeof(float));
   }
   delete [] dataCollect;

   // Find the global min and max sediment values and set on rank 0
   float localValues[2];
   float globalValues[2];

   localValues[0] = volDataNew->getSedimentMinValue();
   localValues[1] = volDataNew->getSedimentMaxValue();

   MPI_Reduce(localValues, globalValues, 2, MPI_FLOAT, mop, 0, MPI_COMM_WORLD);
   
   if( rank == 0 ) {
      volDataNew->setSedimentMinMax(globalValues[0], globalValues[1]);
   }
}


TEST(ConvertProperties, AddVolume)
{
   TestUnit oneProject;
     
   EXPECT_EQ(5, oneProject.numILocal);
   EXPECT_EQ(10, oneProject.numJLocal);

   int mILocal = oneProject.minILocal;
   int mJLocal = oneProject.minJLocal;
   int mILocalExp = (MPIHelper::rank() == 0 ? 0 : 5);

   EXPECT_EQ(mILocalExp, mILocal);

   oneProject.AddDerivedContVolume ();
   const std::shared_ptr<Volume> volume = oneProject.snapshot->getVolume();

   PropertyVolumeDataList& propVolList = volume->getPropertyVolumeDataList();
   EXPECT_EQ(1, propVolList.size());

   int rank = MPIHelper::rank();


   if(rank == 0) {
      BOOST_FOREACH(PropertyVolumeData& propVolume, propVolList) {
         std::string propName = propVolume.first->getName();
         std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
         std::shared_ptr<const Geometry3D> geometry = valueMap->getGeometry();
         
         float sedimentMinValue = valueMap->getSedimentMinValue();
         float sedimentMaxValue = valueMap->getSedimentMaxValue();
         
         EXPECT_FLOAT_EQ(0.0, sedimentMinValue);
         EXPECT_FLOAT_EQ(42.0, sedimentMaxValue);

         int lastK  = static_cast<int>(geometry->getNumK() - 1);
         int firstK = static_cast<int>(geometry->getFirstK());
         
         float value;
         float value1 = 1;
         float value2 = 2;
         
         for (int i = 0; i < geometry->getNumI(); ++ i) {
            if(i < 5) { value = value1; }
            else { value = value2; }
            
            for (int j = 0; j < geometry->getNumJ(); ++ j) {
               for (int k = lastK; k >= firstK; -- k) {
                  EXPECT_EQ(value, valueMap->getValue(i, j, k));
               }
            }
         }
      }
   }
   MPI_Barrier(MPI_COMM_WORLD);
}

