#ifndef EVALUATECAULDRONBATCH_HH
#define EVALUATECAULDRONBATCH_HH
#include <sstream>
#include <string>
#include "writeproject3d.hh"
#include "population.hh"
#include "helperfunctions.hh"
#include "evaluateobjectivefunction.hh"
#include "cauldron.hh"
#include "datamining.hh"
#include <assert.h>


int FindJobId(std::string currentLine)
{
  std::string str2="Job <";
  size_t found =currentLine.find(str2);
  if((int)found != -1)
	{
	  size_t start =currentLine.find("<");
	  size_t end = currentLine.find(">");
	  size_t npos = end-(start+1);
	  std::string id = currentLine.substr(start+1,npos);

	  return ToNumber<int>(id);
	}
  return -9999;
}

int CheckRunStatus(std::string logfile)
{
  bool status = 0;
  bool completed = 0;
  bool converged = 1;
 
  try {
	std::ifstream file_up;
	file_up.open(logfile.c_str());
	if(!file_up)
	  throw(logfile);//If the file is not found, this calls the "catch"
  }
  catch(std::string logfile)
	//This catches the fileup and exit process
	{
	  std::cout << "ERROR: Log file not found."<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}

  std::string completed_string        = "STATE COMPLETE";
  std::string non_convergence_string  = "MeSsAgE WARNING Maximum number of geometric iterations, 10, exceeded and geometry has not converged";
  std::string Job_Id_string           = "submitted to default queue <untouchable>";
  std::string no_working_head_node = "Could not detect a working head node";
  std::string mpi_wrapper_error = "ERROR MPI Wrapper";
  std::string result_error = "RESULT ERROR";
  //open
std::ifstream logFile(logfile.c_str());
std::string str;
//std::cout<<"Reading log file for run status"<<std::endl;
  while(!logFile.eof() && completed==0)
	{
	  //std::cout<<"completed: "<<completed<<std::endl;
	  //read a line and store into str
	  getline(logFile, str);
	  //std::cout<<str<<std::endl;
	  size_t found =	  str.find(non_convergence_string);
	  if((int)found != -1)
		{
		  converged = 0; 
		}
	  found =  str.find(completed_string);
	  if((int)found != -1)
		{
		  completed = 1; 
		}
	  found =  str.find(no_working_head_node);
	  if((int)found != -1)
		{
		  //std::cout<<"ERROR>>"<<str<<std::endl;
		  completed = 1;
		  converged = 0;
		  status = 0;
		  return status;
		  //break;
		}
	   found =  str.find(mpi_wrapper_error);
	    if((int)found != -1)
		{
		  std::cout<<"ERROR>>"<<str<<std::endl;
		  completed = 1;
		  converged = 0;
		  status = 0;
		  return status;
		  //break;
		}
		found =  str.find(result_error);
	    if((int)found != -1)
		{
		  std::cout<<"ERROR>>"<<str<<std::endl;
		  completed = 1;
		  converged = 0;
		  status = 0;
		  return status;
		  //break;
		}
	   
	}


  if(completed && converged)
	status  =1;
 if(completed == 1 && converged == 1)
   status = 1;
 else if(completed == 1 && converged == 0)
   status = 2;
 else if(completed == 0)
   status = 0;
 
   //std::cout<<"stat "<<status<<std::endl;
   return status;
}


bool CheckBatchCompletion(std::vector<Population>& pVec)
{
  bool BatchCompleted = false;
  std::cout<<"Reading log file for run status.."<<std::endl;
  while(!BatchCompleted)
	{  
	  for(std::vector<Population>::size_type i = 0; i != pVec.size(); i++)
		{
		  //std::cout<<"log: "<<pVec[i].Logger<<std::endl;
		  int status = CheckRunStatus(pVec[i].Logger);
		  if(status==1)
			{
			  pVec[i].Finished = true;
			  pVec[i].Converged = true;
			}
		  else if(status==0)
			{
			  pVec[i].Finished = false;
			  pVec[i].Converged = false;
			  
			}
		   else if(status==2)
			{
			  pVec[i].Finished = true;
			  pVec[i].Converged = false;
			  
			}
		  
		}

	  unsigned int finished=0;
	  for(std::vector<Population>::size_type i = 0; i != pVec.size(); i++)
		{
		  if( pVec[i].Finished)
			{
			  //std::cout<<"FINISHED running >> "<<i<<std::endl;
			  finished++;
			}
		}
	  if(finished==pVec.size())
		BatchCompleted = true;	  
	}
 return BatchCompleted;
}


std::vector<double> EvaluateCauldronBatch(std::vector<Population>& pVec, std::vector<CalibrationData>& calVec,Cauldron cauldron)							
{

  // Write cauldron projects for each population member based on the random heat_prod  
  std::string inputProject3d = cauldron.Project3d;
  //std::cout<<"inputProject3d "<<inputProject3d<<std::endl;
  for(std::vector<Population>::size_type i = 0; i != pVec.size(); i++)
	{
	  assert(pVec[i].VariableVector.size()==1);// for heat prod, only one variable
	  std::string outputProject3d = cauldron.Identifier+"-batch-"+ToString(pVec[i].BatchId)+"-out-"+ToString(i)+".project3d";
		  pVec[i].Project3d = outputProject3d;
		  //std::cout<<outputProject3d<<std::endl;
	  for(std::vector<Variable>::size_type j = 0; j != pVec[i].VariableVector.size(); j++) 
		{
		  double wellHeatProd = pVec[i].VariableVector[j].Value();
		  //std::cout<<"h: "<<wellHeatProd <<std::endl;
		  WriteProject3d(inputProject3d,outputProject3d,wellHeatProd);
		}
	}  

 // 2. Launch Cauldron project3d
  std::cout<<"Launching runs"<<std::endl;
	std::string launcher = "gempis";
	std::string simulator = "fastcauldron";
	std::string cluster= cauldron.Cluster;
	std::string processors = ToString(cauldron.Processors);
	std::string version = cauldron.CauldronVersion;
	std::string options = cauldron.CommandOptions;
	std::string mode=cauldron.CauldronMode;

	for(std::vector<Population>::size_type i = 0; i != pVec.size(); i++)
	  {
		assert(pVec[i].VariableVector.size()==1);// for heat prod, only one variable
		std::string logger = cauldron.Identifier+ToString(i)+"_logfile";
		std::string touchcommand = "touch "+logger;
		system(touchcommand.c_str());
		pVec[i].Logger = logger;
		std::string logging = "> "+logger+" 2>&1 </dev/null &";
		std::string command = launcher+" "+cluster+" "+processors+" "+version+" "+simulator+" "+"-project"+" "+pVec[i].Project3d+" "+options+" "+mode+" "+logging;
		std::cout<<command<<std::endl;
		system(command.c_str());
	  } 

	//TODO wait until fastcauldron starts running on each project
			  
// 3. Check all runs in the Batch for Completion
// checkdebatchforcompletion(batch_id,ActiveBatch,S_struct);

	bool completed = CheckBatchCompletion(pVec);
	
	if(completed)
	  std::cout<<"Batch completed "<<std::endl;
	else
	 std::cout<<"ERROR Batch not completed "<<std::endl;

// 4. Call the DataMining application several times
// [DataMiningIoTbl]     = dataminedebatch(ActiveBatch,S_struct);
	std::string datadriller = "datadriller";
	for(std::vector<Population>::size_type i = 0; i != pVec.size(); i++)
	  {
		std::string logger = cauldron.Identifier+ToString(i)+"_drillerlogfile";
		std::string touchcommand = "touch "+logger;
		system(touchcommand.c_str());
		pVec[i].DrillerLogger = logger;
		std::string logging = "> "+logger+" 2>&1 </dev/null ";
		std::string command = datadriller+" -input"+" "+pVec[i].Project3d+" -output"+" "+pVec[i].Project3d+" "+logging;
		std::cout<<"DrillerCommand= "<<command<<std::endl;
		system(command.c_str());
	  }
	
	std::vector<double> objfnVec(0);
	for(std::vector<Population>::size_type i = 0; i != pVec.size(); i++)
	  {
		std::vector<DataMiningIoTable> tableVec = GetDataMiningIoTable(pVec[i].Project3d);
		double objfn = EvaluateObjectiveFunction(calVec,tableVec);
		pVec[i].Cost = objfn;
		std::cout<<"objfn: "<<objfn<<std::endl;
		objfnVec.push_back(objfn);
	  }

	return objfnVec;


// // 5. Calculate errors related to evaluated Cauldron projects in the batch
// //fprintf('Calling evaluateobjectivefunction from evaluatecauldrondebatch \n');

// [Misfit S_struct]     = evaluateobjectivefunction(DataMiningIoTbl,S_struct,Calibrationdata,Input,batch_id);
// %[Misfit S_struct]     = evaluateobjectivefunction(DataMiningIoTbl,S_struct,Calibrationdata, NCalib,batch_id);

// //% 6. Genarate a voxet file of the best calibrated model sofar

// //%Export_and_Draw_GocadVoxet_File(S_struct, Input)

// //fprintf('Going to remove Cauldron temp files \n');


// //% 7. Remove the Cauldron Output Directories
// //if(batch_id > 1)
// //   RemoveCauldronOutputDirectories(batch_id-1,S_struct, Input);
// //   RemoveProject3Dfiles(batch_id-1,S_struct, Input);
// //else
// //    RemoveCauldronOutputDirectories(batch_id,S_struct, Input);

}


#endif
