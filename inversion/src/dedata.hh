#ifndef DEDATA_HH
#define DEDATA_HH

struct DEdata
{
  int NP;       
  double weight;
  double CR;
  int D;
  double Vr_minbound;
  double Vr_maxbound;
  int bnd_constr;
  int itermax;
  double VTR;
  int strategy;
  int refresh;    
  int plotting;
}

#endif
