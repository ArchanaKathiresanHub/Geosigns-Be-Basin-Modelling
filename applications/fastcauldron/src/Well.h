#ifndef _WELL_H_
#define _WELL_H_

#include "propinterface.h"
#include <string>

struct Location {
  double X_Coord;
  double Y_Coord;
  int    X_Position;
  int    X_Step;
  int    Y_Position;
  int    Y_Step;
  int    Z_Position;
  bool   Exists;
  bool   In_Processor_Range;
  double Fractions [4];

};

class Well
{
public:

  Well ( AppCtx* Application_Context );

  ~Well ();

  void   Save_Present_Day_Data ();
  double Get_Well_Interpolated_Value ( const Location& Well, PETSC_3D_Array& property ) const;
  void   Locate_Well( Location& Well );
  void   Save_Well_Data( Location& Well, const string& Related_Project_Name );

private:

  static const int width = 21;

  PETSC_3D_Array* getPropertyVector ( DA  layerDA,
				      Vec propertyVector ) const;


  double interpolateValue ( const Location& needle,
			    const double*   values ) const;

  double surfaceDepth ( const Location& needle ) const;


  AppCtx* Basin_Model;
  std::string calculationModeFileNameExtension;

};

#endif // _WELL_H_
