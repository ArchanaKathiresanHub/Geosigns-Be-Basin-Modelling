#include "de.hh"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

int main()
{
  
//================================================
  // Cauldron project related
  Cauldron cauldron = Cauldron();
  //define cauldron options
  bool isMultiObjective = true;
  std::string mainProject3d = "Project.project3d";//the main project3d file for the problem
  cauldron.OutputDir="";
  cauldron.Identifier="Wellcalibtest_multi_"; //experiment name
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
  
  //Read Calibration data   well param x y z observation 
  //make calibrationdata object
  //this is observation data
  CalibrationData cal = CalibrationData(isMultiObjective);
  //std::string wellfile = "wellinput.dat";
  //std::string wellfile  = "vr_calib_data.dat";
  std::string wellfile  = "wellcalibrationTEST.dat";
  std::vector<CalibrationData> calVec = cal.Read(wellfile.c_str());
  
  
  //Read Diff evolution control data 
  DEControls decontrols = DEControls(6,1,2,0.8,0.9,3,1e-2);
  

  //================================================
  //read parameter whose value being optimised for
  //read heat prod initial min max

  Variable var = Variable();
  std::string varfile = "varinput.dat";
  std::vector<Variable> varVec = var.Read(varfile.c_str());


 //================================================
boost::thread_group g;
 
  //Read well data table and using the main .project3d, for each well create a .project3d
  CloneProject3d clone = CloneProject3d(runningProject);
  std::vector<std::string> wellProject3dVec =  clone.CreateWellModel(calVec);
  DifferentialEvolution de;
  for(std::vector<std::string>::size_type i = 0; i != wellProject3dVec.size(); i++)
	{
	  //insertdataminingiotable entries for the calibration data
	  std::string backup = wellProject3dVec[i] + ".BK";
	  std::string c = "cp "+ wellProject3dVec[i] + " " + backup;
	  system(c.c_str());
	  cauldron.Project3d = wellProject3dVec[i];
	  cauldron.Identifier = cauldron.Identifier+calVec[i].WellName();
	  //std::cout<<backup<<std::endl;
	  InsertDataMiningIoTable(backup,wellProject3dVec[i],calVec);
	   FILE *Fp_out;
	   std::string outf=cauldron.Identifier+"-out.dat";
	   Fp_out  = fopen(outf.c_str(),"w");   
	  //Launch each model into different thread or
	   boost::thread *my_thread1 = new boost::thread(boost::bind(&DifferentialEvolution::Run,&de,cauldron,decontrols,varVec,calVec, Fp_out));
	   g.add_thread(my_thread1);

	  
	}
  
  g.join_all();
  


  
  
//   FILE *Fp_in;
//   FILE *Fp_out;
//   std::string outf=cauldron.Identifier+"-out.dat";
//   Fp_in   = fopen("in.dat","r");
//   Fp_out  = fopen(outf.c_str(),"w");             
  
//   //---optimization--------------
//   devol(cauldron,decontrols,calVec, varVec, Fp_out);
  
//   fclose(Fp_in);
//   fclose(Fp_out);



  //rm -f Wellcalibtest_*_drillerlogfile
  //rm -f Wellcalibtest_*_logfile
  //rm -f Wellcalibtest_multi_*.project3d
  //rm -rf *_CauldronOutputDir


	
}


//TODO use timer class from pnet for time
//use mersennet random
//check matlab code for missing details
//implement various strategies
