#ifndef ELEMENT_ASSEMBLY_H
#define ELEMENT_ASSEMBLY_H

class ElementAssembly
{
public:
   static int  Number_Of_X_Points;
   static int  Number_Of_Y_Points;
   static int  Number_Of_Z_Points;

   static double X_Quadrature_Weights[2];
   static double Y_Quadrature_Weights[2];
   static double Z_Quadrature_Weights[3];

   static double X_Quadrature_Points[2];
   static double Y_Quadrature_Points[2];
   static double Z_Quadrature_Points[3];
};

#endif
