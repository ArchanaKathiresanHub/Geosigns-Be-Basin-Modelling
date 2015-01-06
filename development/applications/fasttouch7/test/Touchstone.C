#include "Touchstone.h"

const int numberOfTouchstoneProperties = 7;
const int numberOfStatisticalOutputs   = 29;

TEST (WriteReadTouchstone,WriteReadTouchstone) 
{ 

   size_t numTimeSteps = 1; 
   
	std::map <int, int> categoriesMappingOrder;
	categoriesMappingOrder.insert(std::pair<int, int>(0,0));
	categoriesMappingOrder.insert(std::pair<int, int>(1,1));
	categoriesMappingOrder.insert(std::pair<int, int>(2,2));
	categoriesMappingOrder.insert(std::pair<int, int>(3,3));
	categoriesMappingOrder.insert(std::pair<int, int>(4,4));
	categoriesMappingOrder.insert(std::pair<int, int>(5,5));

	// Write Results
   {
   std::vector<double> outputProperties( numberOfTouchstoneProperties * numberOfStatisticalOutputs, 10.0);
	TouchstoneFiles WriteTouchstone("Results");
	WriteTouchstone.writeOrder(categoriesMappingOrder);
	WriteTouchstone.writeNumTimeSteps(numTimeSteps);
	WriteTouchstone.writeArray( outputProperties );	
	}
	
	// Read Results
   TouchstoneFiles ReadTouchstone("Results");
   std::vector<int> vec;
   std::vector<double> outputProperties( numberOfTouchstoneProperties * numberOfStatisticalOutputs);
	ReadTouchstone.readOrder(vec);
	ReadTouchstone.readNumTimeSteps(&numTimeSteps);
   ReadTouchstone.readArray(outputProperties);

   // Perform Test
	EXPECT_EQ(vec[0], 0);
	EXPECT_EQ(vec[1], 1);
	EXPECT_EQ(vec[2], 2);
	EXPECT_EQ(vec[3], 3);
	EXPECT_EQ(vec[4], 4);
	EXPECT_EQ(vec[5], 5);
	EXPECT_EQ(numTimeSteps, 1);
	
	for(int ii = 0; ii < numberOfTouchstoneProperties * numberOfStatisticalOutputs; ++ii)
   {
      EXPECT_DOUBLE_EQ(10.0, outputProperties [ ii ]);
   }   
}
