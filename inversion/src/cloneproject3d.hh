#ifndef CLONEPROJECT3D_HH
#define CLONEPROJECT3D_HH
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "calibrationdata.hh"
#include "helperfunctions.hh"
#include "grid.hh"
#include "cmath"
class CloneProject3d
{
 
  std::string _project3d;
  Grid GetGridDetails();
  
public:

  CloneProject3d(){}
  CloneProject3d(std::string& project3dfile):_project3d(project3dfile){}

  std::string Project3d()
  {
	return _project3d;
  }
  std::vector<std::string> CreateWellModel(std::vector<CalibrationData>& calVec);
  void ModifyHeatProd(std::string inputProject3d, std::string outputProject3d,double wellHeatProd);
  void ModifyModelWindow(std::string inputProject3d, std::string outputProject3d,int wXMin,int wXMax, int wYMin, int wYMax);
  
};

Grid CloneProject3d::GetGridDetails()
{
  //check if the input file present or not
  try
	{
	  std::ifstream file_up;
	  file_up.open(_project3d.c_str());
	  if(!file_up)
		throw(_project3d);//If the file is not found, this calls the "catch"
	}
  catch(std::string p)
	//This catches the fileup and exit process
	{
	  std::cout << "ERROR: File not found for Grid details."<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}
  std::ifstream project3dFile(_project3d.c_str());
  std::string str;
  Grid grid;
  while(!project3dFile.eof())
	{
	  //read a line and store into str
	  getline(project3dFile, str);
	  
	  std::vector<std::string> vecStr;
	  tokenize(str, vecStr, " ");
	  if(vecStr.size() > 0)//if not empty lines
		{
		  //store variable names to a vector
		  if(vecStr[0]=="[ProjectIoTbl]")
			{
			  //skip reading semicolon, labels, units 
			  getline(project3dFile, str);
			  getline(project3dFile, str);
			  getline(project3dFile, str);
			  			  
			  //read the required fields
			  getline(project3dFile, str);
			  //std::cout<<"str "<<str<<std::endl;
			  //tokenize
			  tokenize(str, vecStr, " ");

			  //for(int i =0; i<vecStr.size(); ++i)
			  //std::cout<<"grid string "<<"i: "<<i<<" "<<vecStr[i]<<std::endl;


			  
			   grid.XCoord=ToNumber<double>(vecStr[12]);
			   grid.YCoord=ToNumber<double>(vecStr[13]);
			   grid.NumberX=ToNumber<int>(vecStr[14]);
			   grid.NumberY=ToNumber<int>(vecStr[15]);
			   grid.DeltaX=ToNumber<double>(vecStr[16]);
			   grid.DeltaY=ToNumber<double>(vecStr[17]);
			   grid.ScaleX=ToNumber<double>(vecStr[18]);
			   grid.ScaleY=ToNumber<double>(vecStr[19]);
			   grid.OffsetX=ToNumber<double>(vecStr[20]);
			   grid.OffsetY=ToNumber<double>(vecStr[21]);
			   grid.WindowXMin=ToNumber<int>(vecStr[22]);
			   grid.WindowXMax=ToNumber<int>(vecStr[23]);
			   grid.WindowYMin=ToNumber<int>(vecStr[24]);
			   grid.WindowYMax=ToNumber<int>(vecStr[25]);
			   
			   return grid;
			   
			}
		  
		}
	}
}


std::vector<std::string> CloneProject3d::CreateWellModel(std::vector<CalibrationData>& calVec)
{

  Grid grid=GetGridDetails();
  std::vector<std::string> wellProject3dVec;
  for(size_t w = 0; w < calVec.size(); ++w)
	{
	  std::string wellproject3d = "well_"+calVec[w].WellName()+".project3d";
	  std::ifstream project3dFile(_project3d.c_str());
	  std::ofstream outFile(wellproject3d.c_str());

	  //find new winodow for each wellmodel

	  int nx=static_cast<int>(floor((calVec[w].XCoord()-grid.XCoord)/grid.DeltaX));
	  int ny=static_cast<int>(floor((calVec[w].YCoord()-grid.YCoord)/grid.DeltaY));
	  
	  // CHECK FOR Max and Min index and skip in that case
	  //foreach model, find model area

	  //model area 5x5 cell by default
   int windowXMin = nx-2;
   int windowXMax = nx+2;
   int windowYMin = ny-2;
   int windowYMax = ny+2;
   //TODO check model area going beyond the big model, for example if well is in the border of the domain
   
   ModifyModelWindow(_project3d, wellproject3d,windowXMin,windowXMax,windowYMin,windowYMax);
   wellProject3dVec.push_back(wellproject3d);
	 	  	  
	}//end of well loop
  return wellProject3dVec;
}
void CloneProject3d::ModifyModelWindow(std::string inputProject3d, std::string outputProject3d,int wXMin,int wXMax, int wYMin, int wYMax)
{
  //check if the input file present or not
  try {
	std::ifstream file_up;
	file_up.open(inputProject3d.c_str());
	if(!file_up)
	  throw(inputProject3d);//If the file is not found, this calls the "catch"
  }
  catch(std::string inputProject3de)
	//This catches the fileup and exit process
	{
	  std::cout << "ERROR: File not found while modifying model window"<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}
  std::ifstream project3dFile(inputProject3d.c_str());
  std::ofstream outFile(outputProject3d.c_str());
  std::string str;
  while(!project3dFile.eof())
	{
	  //read a line and store into str
	  getline(project3dFile, str);
	  outFile<<str<<std::endl;
	  std::vector<std::string> vecStr;
	  tokenize(str, vecStr, " ");
	  if(vecStr.size() > 0)//if not empty lines
		{
//store variable names to a vector
			  if(vecStr[0]=="[ProjectIoTbl]")
				{
				  //skip reading semicolon, labels, units 
				  getline(project3dFile, str);
				  outFile<<str<<std::endl;
				  getline(project3dFile, str);
				  outFile<<str<<std::endl;
				  getline(project3dFile, str);
				  outFile<<str<<std::endl;
				  
				  //read the required fields
				  getline(project3dFile, str);
				  //std::cout<<"str "<<str<<std::endl;
				  //tokenize
				  tokenize(str, vecStr, " ");
				  
				  //for(int i =0; i<vecStr.size(); ++i)
				  //std::cout<<"i: "<<i<<" "<<vecStr[i]<<std::endl;
			   

			   for(size_t i =0; i<vecStr.size(); ++i)
				 {
				   if(i<22)
					 {
					   outFile<<vecStr[i]<<" ";
					 }
				 }			   
				 outFile<<wXMin<<" "<<wXMax<<" "<<wYMin<<" "<<wYMax<<std::endl;					 			  
				}

// 			  if(vecStr[0]=="[BasementIoTbl]")
// 				{
// 				  //skip reading semicolon, labels, units 
// 				  getline(project3dFile, str);
// 				  outFile<<str<<std::endl;
// 				  getline(project3dFile, str);
// 				  outFile<<str<<std::endl;
// 				  getline(project3dFile, str);
// 				  outFile<<str<<std::endl;
				  
// 				  //read the required fields
// 				  getline(project3dFile, str);
// 				  //std::cout<<"str "<<str<<std::endl;
// 				  //tokenize
// 				  tokenize(str, vecStr, " ");

				  
// 				  for(int i =0; i<vecStr.size(); ++i)
// 					{
// 					  //write TopCrustHeatProd
// 					  if(i==3)
// 					 {
// 					   outFile<<wellHeatProd<<" ";
// 					 }
// 				   else if(i==4)
// 					 {
// 					   //write TopCrustHeatProdGrid as ""
// 					   outFile<<"\"\""<<" ";
// 					 }
// 				   else
// 					 {
// 					   outFile<<vecStr[i]<<" ";
// 					 }
// 					}			   
// 				   outFile<<std::endl;
// 				}
				 
		}
	  
	}
}

  


void CloneProject3d::ModifyHeatProd(std::string inputProject3d, std::string outputProject3d,double wellHeatProd)
{

 //check if the input file present or not
  try {
	std::ifstream file_up;
	file_up.open(inputProject3d.c_str());
	if(!file_up)
	  throw(inputProject3d);//If the file is not found, this calls the "catch"
  }
  catch(std::string inputProject3de)
	//This catches the fileup and exit process
	{
	  std::cout << "ERROR: File not found.. modifying heat prod data"<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}
  std::ifstream project3dFile(inputProject3d.c_str());
  std::ofstream outFile(outputProject3d.c_str());
  std::string str;
  while(!project3dFile.eof())
	{
	  //read a line and store into str
	  getline(project3dFile, str);
	  outFile<<str<<std::endl;
	  std::vector<std::string> vecStr;
	  tokenize(str, vecStr, " ");
	  if(vecStr.size() > 0)//if not empty lines
		{

				   if(vecStr[0]=="[BasementIoTbl]")
				{
				  //skip reading semicolon, labels, units 
				  getline(project3dFile, str);
				  outFile<<str<<std::endl;
				  getline(project3dFile, str);
				  outFile<<str<<std::endl;
				  getline(project3dFile, str);
				  outFile<<str<<std::endl;
				  
				  //read the required fields
				  getline(project3dFile, str);
				  //std::cout<<"str "<<str<<std::endl;
				  //tokenize
				  tokenize(str, vecStr, " ");

				  
				  for(size_t i =0; i<vecStr.size(); ++i)
					{
					  //write TopCrustHeatProd
					  if(i==3)
					 {
					   outFile<<wellHeatProd<<" ";
					 }
				   else if(i==4)
					 {
					   //write TopCrustHeatProdGrid as ""
					   outFile<<"\"\""<<" ";
					 }
				   else
					 {
					   outFile<<vecStr[i]<<" ";
					 }
					}			   
				   outFile<<std::endl;
			   }
				 
		}
	  
	}
}

  

  
#endif
