#include "ElementAssembly.h"


int ElementAssembly::Number_Of_X_Points = 2;
int ElementAssembly::Number_Of_Y_Points = 2;
int ElementAssembly::Number_Of_Z_Points = 3;

double ElementAssembly::X_Quadrature_Weights[] = {1.0, 1.0};
double ElementAssembly::Y_Quadrature_Weights[] = {1.0, 1.0};
double ElementAssembly::Z_Quadrature_Weights[] = {0.55555556, 0.8888889, 0.55555556};

double ElementAssembly::X_Quadrature_Points[] = {-5.7735026918962573e-01, 5.7735026918962573e-01};
double ElementAssembly::Y_Quadrature_Points[] = {-5.7735026918962573e-01, 5.7735026918962573e-01};
double ElementAssembly::Z_Quadrature_Points[] = {-7.7459666924148340e-01, 0.0, 7.7459666924148340e-01};
