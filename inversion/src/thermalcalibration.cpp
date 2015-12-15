#include <iostream>      
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <decontrols.hh>
#include <calibrationdata.hh>
#include <cauldron.hh>
#include <variable.hh>
#include <datamining.hh>
#include "de.hh"
int main()
{
  
//================================================
  // Cauldron project related
   // Cauldron project related
  Cauldron cauldron = Cauldron();
  //define cauldron options
  bool isMultiObjective = true;
  std::string mainProject3d = "Dalsnuten.project3d";//the main project3d file for the problem
  cauldron.OutputDir="";
  cauldron.Identifier="Thermaltest_multi_"; //experiment name
  cauldron.LaunchDir="/scratch/";
  cauldron.CommandOptions="-v2011.11nightly -onlyat 0.0";//command line option, version 
  cauldron.Processors = 1;
  cauldron.Cluster = "LOCAL";
  cauldron.CauldronMode="-itcoupled";
  //rename based on identifier
  std::string runningProject = cauldron.Identifier+mainProject3d;
  std::string command = "cp "+ mainProject3d + " " +runningProject;
   system(command.c_str());
  cauldron.Project3d=runningProject;
  //================================================
  //make a backup
  std::string project3dbackup = mainProject3d + ".BK";
   command = "cp "+ mainProject3d + " " +project3dbackup;
   system(command.c_str());
  
//Read Calibration data   well param x y z observation data
  //make calibrationdata object
  //this is observation data
  CalibrationData cal = CalibrationData(isMultiObjective);
  //std::string wellfile = "wellinput.dat";
  //std::string wellfile  = "vr_calib_data.dat";
  std::string wellfile  = "vr_t_data.dat";
  std::vector<CalibrationData> calVec = cal.Read(wellfile.c_str());
  
  
  //Read Diff evolution control data 
  DEControls decontrols = DEControls(15,1,30,0.8,0.9,3);

   

  
  // for(std::vector<CalibrationData>::size_type i = 0; i != calVec.size(); i++)
// 				{
// 				  std::cout<<calVec[i].XCoord()<<" "<<calVec[i].YCoord()<<" "
// 						   <<calVec[i].ObservedValue()<<std::endl;
// 				}


  //================================================
  //read parameter whose value being optimised for
  //read heat prod initial min max

  // std::string heat_prod="heat_prod";
  //double min = 1.1;
  //double max = 3;
  //double initial=2;
  //create variables for number of parameters decontrols.Parameters()
  //Variable var = Variable(heat_prod, min, max,initial);
  Variable var = Variable();
  std::string varfile = "varinput.dat";
  std::vector<Variable> varVec = var.Read(varfile.c_str());

  // for(std::vector<Variable>::size_type i = 0; i != varVec.size(); i++)
// 				{
// 				  std::cout<<varVec[i].Name()<<" "<<varVec[i].Min()<<" "
// 						   <<varVec[i].Max()<<" "<<varVec[i].Value()<<std::endl;
// 				}

 //================================================

 //insert datamineiotable to the main project file


  //insertdataminingiotable entries for the calibration data
  InsertDataMiningIoTable(project3dbackup,cauldron.Project3d,calVec);

  
  
  // FILE *Fp_in;
  FILE *Fp_out;
  std::string outf=cauldron.Identifier+"-out.dat";
  //Fp_in   = fopen("in.dat","r");
  Fp_out  = fopen(outf.c_str(),"w");             
  
  //---optimization--------------
  DifferentialEvolution devol = DifferentialEvolution();
  devol.Run(cauldron,decontrols,varVec,calVec,Fp_out);
  
 
  fclose(Fp_out);
  
  
  
}


//TODO use timer class from pnet for time
//use mersennet random
//check matlab code for missing details
//implement various strategies
