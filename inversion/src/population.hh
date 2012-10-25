#ifndef POPULATION_HH
#define POPULATION_HH
#include "variable.hh"

struct Population
{

  int BatchId;
  std::string Project3d;
  std::string Logger;
  std::string DrillerLogger;
  bool Finished;
  bool Converged;
  std::vector<Variable> VariableVector; //parameter vector
  double Cost;
  
  
} population;

#endif
