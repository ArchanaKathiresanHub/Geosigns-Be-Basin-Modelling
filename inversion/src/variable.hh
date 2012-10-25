#ifndef VARIABLE_HH
#define VARIABLE_HH
#include "tokenize.hh"


//class for storing parameters of inversion
class Variable
{

  std::string name;
  double min,max,value;

public:
  void SetData(std::string n, double mi,double ma,double ini);
  void SetValue(double ini) ;
  Variable(){}
  Variable(std::string n, double mi, double ma, double val): name(n), min(mi), max(ma), value(val)
  {}

  std::vector<Variable> Read(const std::string& );
  
  double Value() const // this can be initial value for start or optimised value after 
  {
	return value;
  }
  std::string Name() 
  {
	return name;
  }
  double Min() const
  {
	return min;
  }
  double Max() const
  {
	return max;
  }
  
};

void Variable::SetValue(double ini)
{
   value=ini;
}
void Variable::SetData(std::string n, double mi,double ma,double ini)
{
  name=n;
  min=mi;
  max=ma;
  value=ini;
}
std::vector<Variable> Variable::Read(const std::string& varinputdata)
{

//check if the input file present or not
  try {
	std::ifstream file_up;
	file_up.open(varinputdata.c_str());
	if(!file_up)
	  throw(varinputdata);//If the file is not found, this calls the "catch"
  }
  catch(std::string varinputdata)
	//This catches the fileup and exit process
	{
	  std::cout << "ERROR: File not found."<<std::endl<<"Exiting.."<<std::endl;
	  exit(1);
	}
  std::ifstream inputFile(varinputdata.c_str());
    std::string str;
	std::vector<Variable> varVec(0);	
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
		  Variable var = Variable();
		  double mi,ma,va;
		  std::istringstream mis(vecStr[1]);
		  mis>>mi;
		   std::istringstream mas(vecStr[2]);
		  mas>>ma;
		   std::istringstream vas(vecStr[3]);
		  vas>>va;
		 
		  var.SetData(vecStr[0],mi,ma,va);
		  varVec.push_back(var);
		}
	}

  return varVec;
}
#endif
