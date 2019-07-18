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

double Prograde::ProjectIoModelConverter::upgradeProjectOriginX(const std::string& modellingMode, const double& originX, const double& deltaX)
{
   double upgradeOriginX;
   if (modellingMode == "3d")
   {
      upgradeOriginX = originX - (deltaX/2.0);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "A 3d scenario is found and needs upgradation of origin's x-coordinates";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The legacy XCoord of the origin is converted from cellcentroidal to node";
   }
   else
   {
      upgradeOriginX = originX;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "A 1d scenario is found. No upgradation of origin's x-coordinates is needed";
      //Need to take care for the 1d/3d scenarios once the mapping is finalized
   }
   return upgradeOriginX;  
}

double Prograde::ProjectIoModelConverter::upgradeProjectOriginY(const std::string& modellingMode, const double& originY, const double& deltaY)
{
   double upgradeOriginY;
   
   if (modellingMode == "3d")
   {
      upgradeOriginY = originY - (deltaY / 2.0);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "A 3d scenario is found and needs upgradation of origin's y-coordinates";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The legacy YCoord of the origin is converted from cellcentroidal to node";
   }
   else
   {
      upgradeOriginY = originY;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "A 1d scenario is found. No upgradation of origin's y-coordinates is needed";
   }
   //Need to take care for the 1d/3d scenarios once the mapping is finalized
   return upgradeOriginY;
}

int Prograde::ProjectIoModelConverter::upgradeNodeX(const std::string& modellingMode, const int& nodeX)
{
   int upgradeNodeX;
   if (modellingMode == "3d")
   {
	   if (nodeX < 3)
	   {
		   upgradeNodeX = 3;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of x-nodes specified is less than the valid range of BPA2. Resetting its value from " << nodeX << " to " << upgradeNodeX << " as it is a 3d scenario";
	   }
	   else
	   {
		   upgradeNodeX = nodeX;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of x-nodes specified is within the valid range of BPA2 for 3d scenarios. No upgrade is required";
	   }
   }
   else if (modellingMode == "1d")
   {
	   if (nodeX != 2)
	   {
		   upgradeNodeX = 2;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of x-nodes is not found. Resetting its value from " << nodeX << " to " << upgradeNodeX << " as it is a 1d scenario";
	   }
	   else
	   {
		   upgradeNodeX = nodeX;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of x-nodes is found for 1d scenarios. No upgrade is required";
	   }
   }
   else
   {
	   upgradeNodeX = nodeX;//Need to be added when the mapping is ready for 1Dand3D scenarios
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of x-nodes specified is within the valid range of BPA2 for 3d scenarios. No upgrade is required";
   }
   return upgradeNodeX;
}
int Prograde::ProjectIoModelConverter::upgradeNodeY(const std::string& modellingMode, const int& nodeY)
{
   int upgradeNodeY;
   if (modellingMode == "3d")
   {
	   if (nodeY < 3)
	   {
		   upgradeNodeY = 3;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of y-nodes specified is less than the valid range of BPA2. Resetting its value from " << nodeY << " to " << upgradeNodeY << " as it is a 3d scenario";
	   }
	   else
	   {
		   upgradeNodeY = nodeY;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of y-nodes specified is within the valid range of BPA2 for 3d scenarios. No upgrade is required";
	   }
   }
   else if (modellingMode == "1d")
   {
	   if (nodeY != 2)
	   {
		   upgradeNodeY = 2;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of y-nodes is not found. Resetting its value from " << nodeY << " to " << upgradeNodeY << " as it is a 1d scenario";
	   }
	   else
	   {
		   upgradeNodeY = nodeY;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value for number of y-nodes is found for 1d scenarios. No upgrade is required";
	   }
   }
   else
   {
	   upgradeNodeY = nodeY;//May needed to be updated when the mapping is ready for 1Dand3D scenarios
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The number of y-nodes specified is within the valid range of BPA2 for 3d scenarios. No upgrade is required";
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
std::string Prograde::ProjectIoModelConverter::upgradeModellingModeFor1D(const std::string& originalModellingMode)
{
   std::string upgradeModellingMode;

   if (originalModellingMode == "1d")
   {
      upgradeModellingMode = "3d";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deprecated modelling mode ("<< originalModellingMode<<") is identified";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The modelling mode is upgraded to "<< upgradeModellingMode;
   }
   else
   {
      upgradeModellingMode = originalModellingMode;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "ModellingMode is up to date. No upgrade required";
   }
   //Need to take care for the 1d/3d scenarios once the mapping is finalized 
   return upgradeModellingMode;
}
std::string Prograde::ProjectIoModelConverter::upgradeDescription(const std::string& orignalDescription)
{
   std::string upgradedDescription;
   if (orignalDescription.empty())
   {
      upgradedDescription = "Migrated from BPA";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is empty";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description";
   }
   else
   {
      upgradedDescription.append(orignalDescription);
      upgradedDescription.append(": Migrated from BPA");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Project description is not empty";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgraded the description by appending";
   }

   return upgradedDescription;

}