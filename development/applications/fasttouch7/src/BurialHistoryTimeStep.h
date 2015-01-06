#ifndef FASTTOUCH7_BURIALHISTORYTIMESTEP_H
#define FASTTOUCH7_BURIALHISTORYTIMESTEP_H


struct BurialHistoryTimeStep
{
   double  time;          //Snapshot time                 [Ma]          
   double  temperature;   //Temperature at snapshot       [C]
   double  depth;         //Depth at snapshot             [m]
   double  effStress;     //Effective stress at snapshot  [MPa]
   double  waterSat;      //Water saturation at snapshot  [MPa] // added in 3.0.0
   double  overPressure;
};


#endif
