#include "BurialHistory.h"

TEST (WriteReadBurialHistory,WriteReadBurialHistory) 

{ 
	size_t numTimeSteps = 2;
	int firstI=0;
	int lastI=10;
	int firstJ=0;
	int lastJ=10;
	int numLayers=1;
	int numActive=2;
	int iD = 0;
		
	//here write burial history, this is a burHistTimestep struct defined in BurialHistoryGeneric.h
	{
	std::vector<BurialHistoryTimeStep> burHistTimesteps(numTimeSteps); 
	
	burHistTimesteps[0].time			=	0.0;
	burHistTimesteps[0].temperature	=	0.0;
	burHistTimesteps[0].depth			=	0.0;
	burHistTimesteps[0].effStress		=	0.0;
	burHistTimesteps[0].waterSat		=	0.0;
	burHistTimesteps[0].overPressure	=	0.0;
	
	burHistTimesteps[1].time			=	10.0;
	burHistTimesteps[1].temperature	=	10.0;
	burHistTimesteps[1].depth			=	10.0;
	burHistTimesteps[1].effStress		=	10.0;
	burHistTimesteps[1].waterSat		=	10.0;
	burHistTimesteps[1].overPressure	=	10.0;	
		
	//write burial history
	WriteBurial WriteBurial("BurialHistory");
	WriteBurial.writeIndexes(firstI, lastI, firstJ, lastJ, numLayers, numActive);
	WriteBurial.writeBurialHistory(burHistTimesteps, iD, true) ;
	}
	
	//read  Burial History, into the wrapper burHistTimesteps is of Geocosm::TsLib::burHistTimestep type
	ReadBurial ReadBurial("BurialHistory");
   ReadBurial.readIndexes(&firstI, &lastI, &firstJ, &lastJ, &numLayers,&numActive);
   ReadBurial.readNumTimeStepsID(&numTimeSteps, &iD);
   
   std::vector<Geocosm::TsLib::burHistTimestep> burHistTimesteps(numTimeSteps) ;
   ReadBurial.readBurialHistory(burHistTimesteps,numTimeSteps);
   
	//check values
	EXPECT_EQ(0, firstI);
	EXPECT_EQ(10, lastI);
	EXPECT_EQ(0, firstJ);
	EXPECT_EQ(10, lastJ);
	EXPECT_EQ(1, numLayers);
	EXPECT_EQ(2, numTimeSteps);
	EXPECT_EQ(2, numActive);

	EXPECT_DOUBLE_EQ(0.0, burHistTimesteps[0].time);
	EXPECT_DOUBLE_EQ(0.0, burHistTimesteps[0].temperature);
	EXPECT_DOUBLE_EQ(0.0, burHistTimesteps[0].depth);
	EXPECT_DOUBLE_EQ(0.0, burHistTimesteps[0].effStress);
	EXPECT_DOUBLE_EQ(0.0, burHistTimesteps[0].waterSat);
	EXPECT_DOUBLE_EQ(0.0, burHistTimesteps[0].overPressure);

	EXPECT_DOUBLE_EQ(10.0, burHistTimesteps[1].time);
	EXPECT_DOUBLE_EQ(10.0, burHistTimesteps[1].temperature);
	EXPECT_DOUBLE_EQ(10.0, burHistTimesteps[1].depth);
	EXPECT_DOUBLE_EQ(10.0, burHistTimesteps[1].effStress);
	EXPECT_DOUBLE_EQ(10.0, burHistTimesteps[1].waterSat);
	EXPECT_DOUBLE_EQ(10.0, burHistTimesteps[1].overPressure);
}





