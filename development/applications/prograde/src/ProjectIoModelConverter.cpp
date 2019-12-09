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
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of x-nodes specified is less than 3. Resetting values of nodeX from " << nodeX << " to " << upgradeNodeX << " , simulation window max size from "<< OriginalwindowXMax<< " to "<< NewWindowXMax;
	   }
	   else
	   {
		   upgradeNodeX = nodeX;
		   NewWindowXMax = OriginalwindowXMax;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of x-nodes specified is within the valid range of BPA2 for 3d scenarios. No upgrade is required";
	   }
   }
   else if (modellingMode == "1d")
   {
	   if (nodeX != 2)
	   {
		   upgradeNodeX = 2;
		   node_diff = upgradeNodeX - nodeX;
		   NewWindowXMax= OriginalwindowXMax + node_diff;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of x-nodes is not found. Resetting its value from " << nodeX << " to " << upgradeNodeX << " as it is a 1d scenario";
	   }
	   else
	   {
		   upgradeNodeX = nodeX;
		   NewWindowXMax = OriginalwindowXMax;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of x-nodes is found for 1d scenarios. No upgrade is required";
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
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of y-nodes specified is less 3. Resetting values of nodeX from " << nodeY << " to " << upgradeNodeY << " , simulation window max size from " << OriginalwindowYMax << " to " << NewWindowYMax;;
	   }
	   else
	   {
		   upgradeNodeY = nodeY;
		   NewWindowYMax = OriginalwindowYMax;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of y-nodes specified is within the valid range of BPA2 for 3d scenarios. No upgrade is required";
	   }
   }
   else if (modellingMode == "1d")
   {
	   if (nodeY != 2)
	   {
		   upgradeNodeY = 2;
		   node_diff = upgradeNodeY - nodeY;
		   NewWindowYMax = OriginalwindowYMax + node_diff;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of y-nodes is not found. Resetting its value from " << nodeY << " to " << upgradeNodeY << " as it is a 1d scenario";
	   }
	   else
	   {
		   upgradeNodeY = nodeY;
		   NewWindowYMax = OriginalwindowYMax;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of y-nodes is found for 1d scenarios. No upgrade is required";
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
double Prograde::ProjectIoModelConverter::upgradeDeltaX(const std::string& modellingMode, const double& deltaX)
{
   double upgradeDeltaX;
   if (modellingMode == "1d")
   {
      if (deltaX != 100)
      {
         upgradeDeltaX = 100;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of DeltaX for this 1d scenario is not found. Resetting its value from " << deltaX << " to " << upgradeDeltaX;
      }
      else
      {
         upgradeDeltaX = deltaX;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of DeltaX for this 1d scenario is found. No upgrade required";
      }
   }
   else
   {
      upgradeDeltaX = deltaX;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "DeltaX value is upto date for this scenario, no upgradation required";
   }
   
   return upgradeDeltaX;
}
double Prograde::ProjectIoModelConverter::upgradeDeltaY(const std::string& modellingMode, const double& deltaY)
{
   double upgradeDeltaY;
   if (modellingMode == "1d")
   {
      if (deltaY != 100)
      {
         upgradeDeltaY = 100;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of DeltaY for this 1d scenario is not found. Resetting its value from " << deltaY << " to " << upgradeDeltaY;
      }
      else
      {
         upgradeDeltaY = deltaY;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of DeltaY for this 1d scenario is found. No upgrade required";
      }
   }
   else
   {
      upgradeDeltaY = deltaY;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "DeltaY value is upto date for this scenario, no upgradation required";
   }

   return upgradeDeltaY;
}
std::string Prograde::ProjectIoModelConverter::upgradeModellingMode(const std::string& originalModellingMode)
{
   std::string upgradeModellingMode;

   if (originalModellingMode == "1d" || originalModellingMode=="Both")
   {
      upgradeModellingMode = "3d";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deprecated modelling mode ("<< originalModellingMode<<") is identified";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The modelling mode is upgraded to "<< upgradeModellingMode;
   }
   else if (originalModellingMode == "3d")
   {
      upgradeModellingMode = originalModellingMode;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deprecated modelling mode is not found. No upgrade required";
   }
   else//This is unlikely to found any other modelling mode but is added here to track if unknownly this field is got edited 
   {
	   upgradeModellingMode = originalModellingMode;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Unknown ModellingMode (" << originalModellingMode << ") is found. No upgradation is done as it is not a valid scenario";
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
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is empty";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description";
	   }
	   else if (ModellingMode == "Both")
	   {
		   upgradedDescription = "Migrated from BPA (Please note that the original modelling mode was 1Dand3D)";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is empty";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description";
	   }
	   else
	   {
		   upgradedDescription = "Migrated from BPA";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is empty with unknown modelling mode";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description but not the modelling mode";
	   }
   }
   else
   {
	   if (ModellingMode == "3d" || ModellingMode == "1d")
	   {
		   upgradedDescription.append(orignalDescription);
		   upgradedDescription.append(": Migrated from BPA");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is not empty";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description by appending";
	   }
	   else if (ModellingMode == "Both")
	   {
		   upgradedDescription.append(orignalDescription);
		   upgradedDescription.append(": Migrated from BPA (Please note that the original modelling mode was 1Dand3D)");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is not empty";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description by appending";
	   }
	   else
	   {
		   upgradedDescription.append(orignalDescription);
		   upgradedDescription.append(": Migrated from BPA");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is empty with unknown modelling mode";
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description but not the modelling mode";
	   }
   }

   return upgradedDescription;

}