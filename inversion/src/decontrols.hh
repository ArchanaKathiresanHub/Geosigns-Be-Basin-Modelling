#ifndef DECONTROLS_HH
#define DECONTROLS_HH
#include <iostream>



class DEControls
{


  int _p,_g,_is,_params;
  double _fw,_fc;
  double _vtr;//value to reach
  
public:

  DEControls() {}
  DEControls(int p1=10, int params1=1, int g1=10, double fw1=0.85, double fc1=0.9, int is1=3, double vtr1=1e-5):
	_p(p1), _params(params1),_g(g1), _fw(fw1), _fc(fc1), _is(is1), _vtr(vtr1)
  {}
  
  int NumberOfPopulations() const
  {
	return _p;
  }
  int NumberOfParameters() const // no of parameters
  {
	return _params;
  }
  int NumberOfGenerations() const
  {
	return _g;
  }
  double FWeight() const
  {
	return _fw;
  };
  double FCr() const
  {
	return _fc;
  };
  int IStrategy() const
  {
	return _is;
  };
  double VTR() const
  {
	return _vtr;
  }

  ~DEControls(){}
  
};

#endif
