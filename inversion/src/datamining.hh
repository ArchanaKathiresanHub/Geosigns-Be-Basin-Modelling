#ifndef DATAMINING_HH
#define DATAMINING_HH
#include "calibrationdata.hh"
#include "dataminingiotable.hh"
#include "helperfunctions.hh"



void InsertDataMiningIoTable(std::string inputProject3d, std::string  outputProject3d,
				 std::vector<CalibrationData>& calVec)
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
	  std::cout << "ERROR: File not found.. "<<inputProject3d <<" input project3d to insertdataminingiotable"<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}

    
  
  std::ifstream project3dFile(inputProject3d.c_str());
  std::ofstream outFile(outputProject3d.c_str());

  std::string str;
  bool foundTable = 0;
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
		  if(vecStr[0]=="[DataMiningIoTbl]")
			{
			  foundTable=1;
			  //skip reading semicolon, labels, units 

			  getline(project3dFile, str);
			  outFile<<str<<std::endl;
			  getline(project3dFile, str);
			  outFile<<str<<std::endl;
			  getline(project3dFile, str);
			  outFile<<str<<std::endl;
				  
			  //get the calibdata
			  for(std::vector<CalibrationData>::size_type i = 0; i != calVec.size(); i++)
				{

				  //note that propertyname in the welldatainput file should be in quotes
				  //print out the line
				  outFile<<"\"\""<<" "<<"0"<<" "<<calVec[i].XCoord()
						 <<" "<<calVec[i].YCoord()<<" "<<calVec[i].ZCoord()
						 <<" "<<"\"\""<<" "<<"\"\""<<" "<<"\"\""<<" "
						 <<calVec[i].PropertyName()<<" "<<"\"\""<<" "<<"-9999"<<std::endl;
					  
				}
			  while(vecStr[0] != "[End]")
				{
				  //skip already exisiting fields if any
				  getline(project3dFile, str);
				  tokenize(str, vecStr, " ");
				}
			  outFile<<vecStr[0]<<std::endl;
			  
			}
		}
	}

  if(!foundTable)
	{
	  std::cout << "ERROR: project3d file does not have [DataMiningIoTbl]"<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}
}

std::vector<DataMiningIoTable> GetDataMiningIoTable(std::string inputProject3d)
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
	  std::cout << "ERROR: File not found."<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}

    
  std::ifstream project3dFile(inputProject3d.c_str());
  std::vector<DataMiningIoTable> tableVec(0);
 

  std::string str;
  bool foundTable = 0;
  while(!project3dFile.eof() || !foundTable)
	{
	  //read a line and store into str
	  getline(project3dFile, str);
	  
	  std::vector<std::string> vecStr;
	  tokenize(str, vecStr, " ");
	  if(vecStr.size() > 0)//if not empty lines
		{
		  //store variable names to a vector
		  if(vecStr[0]=="[DataMiningIoTbl]")
			{

			  
			  //skip reading semicolon, labels, units 
			  getline(project3dFile, str);
			  getline(project3dFile, str);
			  getline(project3dFile, str);
			  

			  //double to string
			  //std::ostringstream strs;
			  //strs << dbl;
			  //std::string str = strs.str();

			 
			  int calibPoints=0;
			  getline(project3dFile, str);
			  tokenize(str, vecStr, " ");
			  while(vecStr[0] != "[End]")
				{
				  //std::cout<<"Strtblvec= "<<str<<std::endl;
				  std::string b = vecStr[0];
				  
				  double t = ToNumber<double>(vecStr[1]);
				  double x = ToNumber<double>(vecStr[2]);
				  double y = ToNumber<double>(vecStr[3]);
				  double z = ToNumber<double>(vecStr[4]);
				  double v = ToNumber<double>(vecStr[10]);
				  
				  
				
				  std::string r = vecStr[5];
				  std::string f = vecStr[6];
				  std::string s = vecStr[7];
				  std::string pn = vecStr[8];
				  std::string pu = vecStr[9];
				 
				  DataMiningIoTable table =  DataMiningIoTable();
				  table.SetData(b, t, x, y, z, r,  f, s, pn, pu, v);
				  tableVec.push_back(table);
					 
					//get new line
				  getline(project3dFile, str);
				  tokenize(str, vecStr, " ");
				  calibPoints++;
				}
			  foundTable=1;

			}
		}
	}

  return tableVec;
}



#endif 
