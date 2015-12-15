#ifndef DATAMININGIOTABLE_HH
#define DATAMININGIOTABLE_HH

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "calibrationdata.hh"

class DataMiningIoTable
{

  std::string bpakey,propertyname,reservoirname,formationname,surfacename,propertyunit;
  double time,xcoord,ycoord,zcoord,value;
public:

  DataMiningIoTable() : bpakey(""),propertyname(""),reservoirname(""),formationname(""),surfacename(""),propertyunit("")
  {
  }
  void SetData(std::string b, double t, double x,double y,double z,
			   std::string r, std::string f, std::string s,
			   std::string pn, std::string pu, double v);

  //void Create(std::string welltable);
  //void Create();
 
  std::string BPAKey()
  {
	return bpakey;
  }
  double Time()
  {
	return time;	
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
  std::string ReservoirName()
  {
	return reservoirname;
  }
  std::string FormationName()
  {
	return formationname;
  }
  std::string SurfaceName()
  {
	return surfacename;
  }
  std::string PropertyName()
  {
	return propertyname;
  }
  std::string PropertyUnit()
  {
	return propertyunit;
  }
  double Value()
  {
	return value;
  }	
     
};

void DataMiningIoTable::SetData(std::string b, double t, double x,double y,
								double z, std::string r, std::string f,
								std::string s, std::string pn, std::string pu, double v)
{
  propertyname = pn;
  propertyunit=pu;
  xcoord=x;
  ycoord=y;
  zcoord=z;
  value=v;
  bpakey=b;
  time=t;
  reservoirname=r;
  surfacename=s;
  formationname=f;
}

// void Create(std::string welltable)
// {
// }

// std::vector<CalibrationData> Create()
// {
//   CalibrationData cal;
//   std::string p = "Temperature";
//   double x = 2300;
//   double y = 4500;
//   double ov = -9999;
//   double v = -9999;
//   cal.SetData(p,x,y,z,ov,v);
//   std::vector<CalibrationData> calVec(0);
//   calVec.push_back(cal);
//   return calVec;
// }



#endif
