//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ProjectIoModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "ProjectDataManager.h"

using namespace mbapi;

void Prograde::ProjectIoModelConverter::upgradeSimulationWindow(const std::string& legacyModellingMode, int& windowXMin, int& windowXMax, int nodeX, int & stepX)
{
	// For a 3d scenario to import successfully in BPA2, the minimum nodes should be >=3 in the simulation window defined. This also depends on the value of the subsampled size.
	// If the the combination of original simulation window with/without subsampling doesn't satisfy the BPA2 requirement then upgrade the simulation window as follows
	//       1) First increase the max-window parameters considering the subsampled size keeping the min-window fixed. 
	//       2) If the upgraded new max-window is going out the allowed value which is bounded by the number of total nodes in each direction, then update the minimum-window and max-window is kept unchanged.
	//       3) If both step 1-2 are not falling within the acceptable limit then check for the subsampling size and reduce it value until the updated window falls within the acceptable limit.
	int nodesInOriginalWindow = windowXMax - windowXMin + 1;
	int newWindowMax = windowXMax;
	int newWindowMin = windowXMin;
	int originalStepX = stepX;
	int minimumRequiredNode = (stepX * 3)- stepX + 1;//nodes needed when subsampled 

	while (minimumRequiredNode >= nodeX && stepX > 1)//This is needed for scenarios where the legacy simulation window may be acceptable to import in BPA2 but the combination of sub-sampling and simulation window is not acceptable 
	{
		stepX--;
		minimumRequiredNode = (stepX * 3) - stepX + 1;
	}

	if (nodesInOriginalWindow < minimumRequiredNode and (legacyModellingMode =="3d" or (legacyModellingMode=="Both")) and nodeX > 2)//if the window area itself is smaller than the allowable limit...update the window area.. 
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Simulation window defined in the legacy scenario is not acceptable to BPA2";
		
		if (nodeX == 3 )
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Simulation window is updated from [" << windowXMin << " , "<< windowXMax<<"] to [0 , 2]";
			windowXMin = 0;
			windowXMax = 2;
			if (stepX != originalStepX)
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Subsampling size is updated from " << originalStepX << " to 1";
				stepX = 1;
			}
			
		}
		else {
			while(minimumRequiredNode > nodesInOriginalWindow){
				newWindowMax = windowXMin + minimumRequiredNode - 1;
				newWindowMin = windowXMin;
				if (newWindowMax > (nodeX - 1))
				{
					newWindowMax = windowXMax;
					newWindowMin = windowXMax - (minimumRequiredNode - 1);
					
					if (newWindowMin < 0)
					{
						if (stepX > 1)//StepX must be always greater than equal to 1
						{
							stepX--;
							minimumRequiredNode = (stepX * 3) - stepX + 1;
						}
						else
							throw ErrorHandler::Exception(ErrorHandler::ReturnCode::ValidationError) << "Invalid inputs wrt maximum possible simulation window";
					}
					else
						break;
				}
				else
					break;
			}
			if ((newWindowMax - newWindowMin + 1) < 3)
			{
				throw ErrorHandler::Exception(ErrorHandler::ReturnCode::ValidationError) << "Incompatible sub-sampling size wrt maximum possible simulation window.";
			}

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Simulation window is updated from [" << windowXMin << " , " << windowXMax << "] to [" << newWindowMin << " , " << newWindowMax << "]";

			windowXMin = newWindowMin;
			windowXMax = newWindowMax;
			
			if (originalStepX != stepX)
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Subsampling size is updated from " << originalStepX << " to " << stepX;
		}
		
	}
	else {
		if (originalStepX != stepX)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Subsampling size is updated from " << originalStepX << " to " << stepX;
	}

}

void Prograde::ProjectIoModelConverter::preProcessSimulationWindow(int& legacyWindowMin, int& legacyWindowMax, int nodeCount)
{
	if (legacyWindowMax > (nodeCount - 1) )
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Original maximum simulation window value is greater than the maximum allowed value bounded by the node count ("<< nodeCount<<"). Resetting the value from " << legacyWindowMax<<" to "<<(nodeCount - 1);
		legacyWindowMax = nodeCount - 1;
	}
	if(legacyWindowMin < 0)
	{
		legacyWindowMin = 0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Original minimum simulation window value is less than the minimum allowed value (" << nodeCount << "). Resetting the value from " << legacyWindowMin << " to 0";
	}
}


double Prograde::ProjectIoModelConverter::upgradeDeltaX(const std::string& modellingMode, const double& deltaX, const int nodeX)
{
   double upgradeDeltaX = deltaX;
   if ( (modellingMode == "1d" or modellingMode == "Both") and nodeX == 2 and deltaX != 100)
   {
	   upgradeDeltaX = 100;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Default value of DeltaX for scenario with 2 nodes is not found. Resetting its value from " << deltaX << " to " << upgradeDeltaX;
   }
   
   return upgradeDeltaX;
}
double Prograde::ProjectIoModelConverter::upgradeDeltaY(const std::string& modellingMode, const double& deltaY, const int nodeY)
{
   double upgradeDeltaY = deltaY;
   if ((modellingMode == "1d" or modellingMode == "Both") and nodeY == 2 and deltaY != 100)
   {
	   upgradeDeltaY = 100;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Default value of DeltaY for scenario with 2 nodes is not found. Resetting its value from " << deltaY << " to " << upgradeDeltaY;
   }

   return upgradeDeltaY;
}
std::string Prograde::ProjectIoModelConverter::upgradeDescription(const std::string& ModellingMode, const std::string& orignalDescription)
{
   std::string upgradedDescription;
   if (orignalDescription.empty())
   {
	   if (ModellingMode == "3d" || ModellingMode == "1d")
	   {
		   upgradedDescription = "Migrated from BPA";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Project description is empty; Description is upgraded";
	   }
	   else if (ModellingMode == "Both")
	   {
		   upgradedDescription = "Migrated from BPA (Please note that the original modelling mode was 1Dand3D)";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Project description is empty; Description is upgraded";
	   }
	   else
	   {
		   upgradedDescription = "Migrated from BPA";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Project description is empty with unknown modelling mode; Description is upgraded but not the modelling mode";
	   }
   }
   else
   {
	   if (ModellingMode == "3d" || ModellingMode == "1d")
	   {
		   upgradedDescription.append(orignalDescription);
		   upgradedDescription.append(": Migrated from BPA");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Project description is not empty; Description is upgraded by appending";
	   }
	   else if (ModellingMode == "Both")
	   {
		   upgradedDescription.append(orignalDescription);
		   upgradedDescription.append(": Migrated from BPA (Please note that the original modelling mode was 1Dand3D)");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Project description is not empty; Description is upgraded by appending";
	   }
	   else
	   {
		   upgradedDescription.append(orignalDescription);
		   upgradedDescription.append(": Migrated from BPA");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Project description is empty with unknown modelling mode; Description is upgraded but not the modelling mode";
	   }
   }

   return upgradedDescription;

}
