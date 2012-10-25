#ifndef CALIBRATIONDATA_HH
#define CALIBRATIONDATA_HH
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "tokenize.hh"

class CalibrationData
{

  bool isMO;
  std::string wellname,property;
  double xcoord,ycoord,zcoord,obsvalue,compvalue;
public:

  CalibrationData():isMO(false){}
  CalibrationData(bool mo):isMO(mo){}
  void SetData(std::string w,std::string p,double x,double y,double z,double ov, double cv);
  std::vector<CalibrationData> Read(const std::string& );
  
  std::string WellName()
  {
	return wellname;
  }
  std::string PropertyName()
  {
	return property;
  }
  double XCoord()
  {
	return xcoord;
	
  }
  double YCoord()
  {
	return ycoord;
	
  }
  double ZCoord()
  {
	return zcoord;
	
  }
  double ObservedValue()
  {
	return obsvalue;
	
  }
double ComputedValue()
  {
	return compvalue;	
  }
  bool isMultiObjective()
  {
	return isMO;
  }
  
  
};

void CalibrationData::SetData(std::string w, std::string p,double x,double y,double z,double ov, double cv)
{
  wellname=w;
  property = p;
  xcoord=x;
  ycoord=y;
  zcoord=z;
  obsvalue=ov;
  compvalue=cv;
}

std::vector<CalibrationData> CalibrationData::Read(const std::string& wellinputdata)
{

//check if the input file present or not
  try {
	std::ifstream file_up;
	file_up.open(wellinputdata.c_str());
	if(!file_up)
	  throw(wellinputdata);//If the file is not found, this calls the "catch"
  }
  catch(std::string wellinputdata)
	//This catches the fileup and exit process
	{
	  std::cout << "ERROR: File not found.. well input data"<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}
  std::ifstream inputFile(wellinputdata.c_str());
    std::string str;
	std::vector<CalibrationData> calVec(0);	
  while(!inputFile.eof())
	{
	  //read a line and store into str
	  getline(inputFile, str);
	   std::vector<std::string> vecStr;
	  tokenize(str, vecStr, " ");
	  
	  if(vecStr.size() > 0)//if not empty lines
		{
		  if(vecStr[0]=="#" ||  vecStr[0][0]=='#')
			continue;
		  
		  //std::cout<<str<<std::endl;
		  CalibrationData cal = CalibrationData();
		  double x,y,z,ov;
		  std::istringstream xs(vecStr[2]);
		  xs>>x;
		   std::istringstream ys(vecStr[3]);
		  ys>>y;
		   std::istringstream zs(vecStr[4]);
		  zs>>z;
		  std::istringstream ovs(vecStr[5]);
		  ovs>>ov;		  
		  cal.SetData(vecStr[0],vecStr[1],x,y,z,ov,-9999 );
		  calVec.push_back(cal);
		}
	}

  return calVec;
}

#endif
