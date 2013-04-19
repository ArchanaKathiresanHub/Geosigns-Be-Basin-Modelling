//------------------------------------------------------------//

#include "preevaluated_basis_functions.h"
#include "array.h"
#include "quadrature.h"

//------------------------------------------------------------//

#include "BasisFunction.h"

//------------------------------------------------------------//


Basin_Modelling::Preevaluated_Basis_Functions::Preevaluated_Basis_Functions () {

  Preevaluated_Bases = 0;
  Preevaluated_Grad_Bases = 0;
  Quadrature_Degree_Plane = 0;
  Quadrature_Degree_Normal = 0;
  Size = 0;

  Memory_For_Vectors = 0;
  Vector_Memory_Position = 0;
  Vector_Count = 0;

  Memory_For_Grad_Vectors = 0;
  Memory_For_Grad_Vectors_Ptrs = 0;
  Grad_Vector_Memory_Position = 0;
  Grad_Vector_Memory_Ptr_Position = 0;
  Grad_Vector_Count = 0;

}


//------------------------------------------------------------//


Basin_Modelling::Preevaluated_Basis_Functions::~Preevaluated_Basis_Functions () {

  if ( Preevaluated_Bases != 0 ) {
    Array<double>::delete2d ( Preevaluated_Bases );
  }

  if ( Preevaluated_Grad_Bases != 0 ) {
    Array<double>::delete3d ( Preevaluated_Grad_Bases );
  }

  Quadrature_Degree_Plane = 0;
  Quadrature_Degree_Normal = 0;
  Size = 0;

  if ( Memory_For_Vectors != 0 ) {
    delete [] Memory_For_Vectors;
  }

  if ( Memory_For_Grad_Vectors != 0 ) {
    delete [] Memory_For_Grad_Vectors;
    delete [] Memory_For_Grad_Vectors_Ptrs;
  }

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Preevaluate ( const int Degree_Plane,
                                                                  const int Degree_Normal ) {

//    using namespace Quadrature;

  Quadrature::Quadrature_Array Plane_Points;
  Quadrature::Quadrature_Array Plane_Weights;
  Quadrature::Quadrature_Array Normal_Points;
  Quadrature::Quadrature_Array Normal_Weights;

  FiniteElementMethod::BasisFunction basisFunction;
  ElementVector      Basis;
  GradElementVector Grad_Basis;

  double X;
  double Y;
  double Z;

  int I;
  int J;
  int K;
  int L;
  int Count = 0;

  if ( Preevaluated_Bases != 0 ) {
    Array<double>::delete2d ( Preevaluated_Bases );
  }

  if ( Preevaluated_Grad_Bases != 0 ) {
    Array<double>::delete3d ( Preevaluated_Grad_Bases );
  }

  Quadrature_Degree_Plane  = Degree_Plane;
  Quadrature_Degree_Normal = Degree_Normal;
  Size = Degree_Plane * Degree_Plane * Degree_Normal;

  Preevaluated_Bases      = Array<double>::create2d ( Size, 8 );
  Preevaluated_Grad_Bases = Array<double>::create3d ( Size, 3, 8 );

  Quadrature::Get_Gauss_Legendre_Quadrature ( Degree_Plane,  Plane_Points,  Plane_Weights );
  Quadrature::Get_Gauss_Legendre_Quadrature ( Degree_Normal, Normal_Points, Normal_Weights );


  for ( I = 0; I < Degree_Plane; I++ ) {
    X = Plane_Points [ I ];

    for ( J = 0; J < Degree_Plane; J++ ) {
      Y = Plane_Points [ J ];

      for ( K = 0; K < Degree_Normal; K++ ) {
        Z = Normal_Points [ K ];
        basisFunction ( X, Y, Z, Basis, Grad_Basis );

        for ( L = 1; L <= 8; L++ ) {
          Preevaluated_Bases [ Count ][ L - 1 ] = Basis ( L );

          Preevaluated_Grad_Bases [ Count ][ 0 ][ L - 1 ] = Grad_Basis ( L, 1 );
          Preevaluated_Grad_Bases [ Count ][ 1 ][ L - 1 ] = Grad_Basis ( L, 2 );
          Preevaluated_Grad_Bases [ Count ][ 2 ][ L - 1 ] = Grad_Basis ( L, 3 );
        }

        Count = Count + 1;
      }

    }

  }


  Memory_For_Vectors = new double [ Maximum_Number_Of_Vectors * Size ];
  Vector_Memory_Position = 0;
  Vector_Count = 0;

  Memory_For_Grad_Vectors = new double [ Maximum_Number_Of_Grad_Vectors * Size * 3 ];
  Memory_For_Grad_Vectors_Ptrs = new double* [ Maximum_Number_Of_Grad_Vectors * Size ];
  Grad_Vector_Memory_Position = 0;
  Grad_Vector_Memory_Ptr_Position = 0;
  Grad_Vector_Count = 0;

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Set_Jacobian ( const int                    Quadrature_Position,
                                                                   const ElementGeometryMatrix& Geometry_Matrix,
                                                                         Matrix3x3&             Jacobian ) const {

  int I;
  int J;
  int K;
  double Inner_Product;

  for ( I = 1; I <= 3; ++I ) {

    for ( J = 1; J <= 3; ++J ) {
      Inner_Product = 0.0;

      for ( K = 1; K <= 8; ++K ) {
        Inner_Product = Inner_Product + Geometry_Matrix ( I, K ) * Preevaluated_Grad_Bases [ Quadrature_Position ][ J - 1 ][ K - 1 ];
      }

      Jacobian ( I, J ) = Inner_Product;
    }

  }
    

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Evaluate_Coefficients ( const ElementVector& Coefficients,
                                                                                  Vector&         Evaluations ) {

  int    I;
  int    J;
  double Inner_Product;

  for ( I = 0; I < Size; ++I ) {
    Inner_Product = 0.0;

    for ( J = 0; J < 8; ++J ) {
      Inner_Product = Inner_Product + Preevaluated_Bases [ I ][ J ] * Coefficients ( J + 1 );
    }

    Evaluations ( I ) = Inner_Product;
  }

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Evaluate_Coefficients ( const ElementVector& Coefficients,
                                                                                  Grad_Vector&    Evaluations ) {

  int    I;
  int    J;
//    int    K;
  double Inner_Product;

  for ( I = 0; I < Size; ++I ) {

//      for ( K = 0; K < 3; ++K ) {
//        Inner_Product = 0.0;

//        for ( J = 0; J < 8; ++J ) {
//          Inner_Product = Inner_Product + Preevaluated_Grad_Bases [ I ][ K ][ J ] * Coefficients ( J + 1 );
//        }

//        Evaluations ( I, K ) = Inner_Product;
//      }


    Inner_Product = 0.0;

    for ( J = 0; J < 8; ++J ) {
      Inner_Product = Inner_Product + Preevaluated_Grad_Bases [ I ][ 0 ][ J ] * Coefficients ( J + 1 );
    }

    Evaluations ( I, 0 ) = Inner_Product;

    Inner_Product = 0.0;

    for ( J = 0; J < 8; ++J ) {
      Inner_Product = Inner_Product + Preevaluated_Grad_Bases [ I ][ 1 ][ J ] * Coefficients ( J + 1 );
    }

    Evaluations ( I, 1 ) = Inner_Product;

    Inner_Product = 0.0;

    for ( J = 0; J < 8; ++J ) {
      Inner_Product = Inner_Product + Preevaluated_Grad_Bases [ I ][ 2 ][ J ] * Coefficients ( J + 1 );
    }

    Evaluations ( I, 2 ) = Inner_Product;

  }

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Sum_Basis_Vectors ( const Vector&         Coefficients,
                                                                              ElementVector& Residual ) const {

  int I;
  int J;

  for ( I = 0; I < Size; ++I ) {

    for ( J = 0; J < 8; ++J ) {
      Residual ( J + 1 ) = Residual ( J + 1 ) + Preevaluated_Bases [ I ][ J ] * Coefficients ( I );
    }

  }

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Sum_Grad_Basis_Vectors ( const Grad_Vector&    Coefficients,
                                                                                   ElementVector& Residual ) const {

  int I;
  int J;
//    int K;
//    double Inner_Product;
  double C1;
  double C2;
  double C3;

  for ( I = 0; I < Size; ++I ) {
    C1 =  Coefficients ( I, 0 );
    C2 =  Coefficients ( I, 1 );
    C3 =  Coefficients ( I, 2 );

    for ( J = 0; J < 8; ++J ) {
      Residual ( J + 1 ) = Residual ( J + 1 ) + Preevaluated_Grad_Bases [ I ][ 0 ][ J ] * C1;
    }

    for ( J = 0; J < 8; ++J ) {
      Residual ( J + 1 ) = Residual ( J + 1 ) + Preevaluated_Grad_Bases [ I ][ 1 ][ J ] * C2;
    }

    for ( J = 0; J < 8; ++J ) {
      Residual ( J + 1 ) = Residual ( J + 1 ) + Preevaluated_Grad_Bases [ I ][ 2 ][ J ] * C3;
    }

  }

//    for ( I = 0; I < Size; ++I ) {
//      C1 =  Coefficients ( I, 0 );
//      C2 =  Coefficients ( I, 1 );
//      C3 =  Coefficients ( I, 2 );

//      for ( J = 0; J < 8; ++J ) {
//  //        Inner_Product = Preevaluated_Grad_Bases [ I ][ 0 ][ J ] * Coefficients ( I, 0 ) +
//  //                        Preevaluated_Grad_Bases [ I ][ 1 ][ J ] * Coefficients ( I, 1 ) +
//  //                        Preevaluated_Grad_Bases [ I ][ 2 ][ J ] * Coefficients ( I, 2 );

//        Inner_Product = Preevaluated_Grad_Bases [ I ][ 0 ][ J ] * C1 +
//                        Preevaluated_Grad_Bases [ I ][ 1 ][ J ] * C2 +
//                        Preevaluated_Grad_Bases [ I ][ 2 ][ J ] * C3;

//        Residual ( J + 1 ) = Residual ( J + 1 ) + Inner_Product;
//      }

//    }

}


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Next_Vector_Memory ( double*& Memory_Chunk ) {

//    double* Memory_Chunk;

  if ( Vector_Count == Maximum_Number_Of_Vectors ) {
    // Error
  }

  Memory_Chunk = &Memory_For_Vectors [ Vector_Memory_Position ];
  Vector_Count = Vector_Count + 1;
  Vector_Memory_Position = Vector_Memory_Position + Size;

//    return Memory_Chunk;
} // Next_Vector_Memory


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Release_Vector_Memory ( const double* Memory_To_Release ) {

  Vector_Count = Vector_Count - 1;

  if ( Vector_Count == 0 ) {
    Vector_Memory_Position = 0;
  }

} // Release_Vector_Memory


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Get_Grad_Vector_Memory ( double**& Memory ) {

  int I;

  Memory = &Memory_For_Grad_Vectors_Ptrs [ Grad_Vector_Memory_Ptr_Position ];

  Grad_Vector_Count = Grad_Vector_Count + 1;
  Grad_Vector_Memory_Ptr_Position = Grad_Vector_Memory_Ptr_Position + Size;

  for ( I = 0; I < Size; ++I ) {
    Memory [ I ] = &Memory_For_Grad_Vectors [ Grad_Vector_Memory_Position ];
    Grad_Vector_Memory_Position = Grad_Vector_Memory_Position + 3;
  }

} // Get_Grad_Vector_Memory


//------------------------------------------------------------//


void Basin_Modelling::Preevaluated_Basis_Functions::Free_Grad_Vector_Memory ( double**& Memory_To_Release ) {

  Grad_Vector_Count = Grad_Vector_Count - 1;

  if ( Grad_Vector_Count == 0 ) {
    Grad_Vector_Memory_Position = 0;
    Grad_Vector_Memory_Ptr_Position = 0;
  }

} // Free_Grad_Vector_Memory


//------------------------------------------------------------//
