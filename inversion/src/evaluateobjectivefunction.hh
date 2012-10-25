#ifndef EVALUATEOBJECTIVEFUNCTION_HH
#define EVALUATEOBJECTIVEFUNCTION_HH
#include "dataminingiotable.hh"
#include <cmath>
#include <assert.h>
double EvaluateObjectiveFunction(std::vector<CalibrationData>& calVec, std::vector<DataMiningIoTable>& tableVec )
{
  assert(calVec.size()==tableVec.size());

  //weight for multiobjective
  //how to scale obj fn
  
  double weight=1.0;
  double sum=0;
  //sum for each points with same property 
for(std::vector<DataMiningIoTable>::size_type i = 0; i != tableVec.size(); i++)
  {
	//std::cout<<tableVec[i].PropertyName()<<" -- "<<calVec[i].PropertyName()<<std::endl;
	assert(tableVec[i].PropertyName()== calVec[i].PropertyName());
	//std::cout<<tableVec[i].PropertyName()<<" at ("<<tableVec[i].XCoord()<<","<<tableVec[i].YCoord()<<") ="<<tableVec[i].Value()<<std::endl;

	if(calVec[i].isMultiObjective())
	  {
		if(calVec[i].PropertyName()=="\"Temperature\"")
		  weight = 0.2;
		else
		  weight = 0.8;
	  }
	//get observed value from calibrationdata
	double observed = calVec[i].ObservedValue();
	//get computed value from dataminingiotable
	double computed = tableVec[i].Value();
	
	//double error = (observed - computed);//L1

	//double error = (observed - computed)*(observed - computed)/(no_of_points*no_of_points*) RMS
	double error = weight*(observed - computed)*(observed - computed);//L2
	sum=sum+fabs(error);
	//std::cout<<"obs="<<observed<<" comp="<<computed<<" error="<<error<<" sum="<<sum<<std::endl;

	
	
  }

 return sum;
 //return sqrt(sum);//RMSE
 
}
 
#endif
