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
#if 0 //these methods are not releavent...as there will no be any upgrade performed on node counts
int Prograde::ProjectIoModelConverter::upgradeNodeX(const std::string& modellingMode, const int& nodeX, const int& OriginalwindowXMax, int& NewWindowXMax)
{
   int upgradeNodeX, node_diff;
   if (modellingMode == "3d" || modellingMode=="Both")
   {
	   if (nodeX < 3)
	   {
		   upgradeNodeX = 3; 
		   node_diff = upgradeNodeX - nodeX;
		   NewWindowXMax = OriginalwindowXMax + node_diff;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> The number of x-nodes specified is less than 3. Resetting values of nodeX from " << nodeX << " to " << upgradeNodeX << " , simulation window max size from "<< OriginalwindowXMax<< " to "<< NewWindowXMax;
	   }
	   else
	   {
		   upgradeNodeX = nodeX;
		   NewWindowXMax = OriginalwindowXMax;
	   }
   }
   else if (modellingMode == "1d")
   {
	   if (nodeX != 2)
	   {
		   upgradeNodeX = 2;
		   node_diff = upgradeNodeX - nodeX;
		   NewWindowXMax= OriginalwindowXMax + node_diff;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Default value for number of x-nodes is not found. Resetting its value from " << nodeX << " to " << upgradeNodeX << " as it is a 1d scenario";
	   }
	   else
	   {
		   upgradeNodeX = nodeX;
		   NewWindowXMax = OriginalwindowXMax;
	   }
   }
   else
   {
	   upgradeNodeX = nodeX;//Need to be added when the mapping is ready for 1Dand3D scenarios
	   NewWindowXMax = OriginalwindowXMax;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Unknown modelling mode is found. Not a valid scenario";
   }
   return upgradeNodeX;
}
int Prograde::ProjectIoModelConverter::upgradeNodeY(const std::string& modellingMode, const int& nodeY, const int& OriginalwindowYMax, int& NewWindowYMax)
{
   int upgradeNodeY, node_diff;
   if (modellingMode == "3d" || modellingMode == "Both")
   {
	   if (nodeY < 3)
	   {
		   upgradeNodeY = 3; 
		   node_diff = 3 - nodeY;	   
		   NewWindowYMax = OriginalwindowYMax + node_diff;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> The number of y-nodes specified is less 3. Resetting values of nodeX from " << nodeY << " to " << upgradeNodeY << " , simulation window max size from " << OriginalwindowYMax << " to " << NewWindowYMax;;
	   }
	   else
	   {
		   upgradeNodeY = nodeY;
		   NewWindowYMax = OriginalwindowYMax;
	   }
   }
   else if (modellingMode == "1d")
   {
	   if (nodeY != 2)
	   {
		   upgradeNodeY = 2;
		   node_diff = upgradeNodeY - nodeY;
		   NewWindowYMax = OriginalwindowYMax + node_diff;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Default value for number of y-nodes is not found. Resetting its value from " << nodeY << " to " << upgradeNodeY << " as it is a 1d scenario";
	   }
	   else
	   {
		   upgradeNodeY = nodeY;
		   NewWindowYMax = OriginalwindowYMax;
	   }
   }
   else
   {
	   upgradeNodeY = nodeY;//May needed to be updated when the mapping is ready for 1Dand3D scenarios
	   NewWindowYMax = OriginalwindowYMax;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Unknown modelling mode is found. Not a valid scenario";
   }
   return upgradeNodeY;
}
#endif

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
std::string Prograde::ProjectIoModelConverter::upgradeModellingMode(const std::string& originalModellingMode)
{
   std::string upgradeModellingMode;

   if (originalModellingMode == "1d" || originalModellingMode=="Both")
   {
      upgradeModellingMode = "3d";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Deprecated modelling mode is identified";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Modelling mode is upgraded from "<< originalModellingMode<<" to "<< upgradeModellingMode;
   }
   else if (originalModellingMode == "3d")
   {
      upgradeModellingMode = originalModellingMode;
   }
   else//This is unlikely to found any other modelling mode but is added here to track if unknownly this field is got edited 
   {
	   upgradeModellingMode = "3d";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Unknown ModellingMode (" << originalModellingMode << ") is found; upgraded to 3d";
   }
   return upgradeModellingMode;
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