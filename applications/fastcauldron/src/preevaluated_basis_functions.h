//------------------------------------------------------------//

#ifndef __Preevaluated_Basis_Functions_HH__
#define __Preevaluated_Basis_Functions_HH__

//------------------------------------------------------------//

#include "FiniteElementTypes.h"

using namespace FiniteElementMethod;

//------------------------------------------------------------//


namespace Basin_Modelling {



  class Preevaluated_Basis_Functions {

  public :

    static Preevaluated_Basis_Functions* Get_Instance () { static Preevaluated_Basis_Functions Bases; return &Bases;}

    //------------------------------------------------------------//

    class Vector {

    public :

      Vector () { Preevaluated_Basis_Functions::Get_Instance () -> Next_Vector_Memory    ( Values );}
     ~Vector () { Preevaluated_Basis_Functions::Get_Instance () -> Release_Vector_Memory ( Values );}

      double operator ()( const int Position ) const {
        return Values [ Position ];
      }

      double& operator ()( const int Position ) {
        return Values [ Position ];
      }

      int Dimension () const { return Preevaluated_Basis_Functions::Get_Instance () -> Number_Of_Quadrature_Points ();}

    private :

      double* Values;

    };

    //------------------------------------------------------------//

    class Grad_Vector {

    public :

      Grad_Vector () { Preevaluated_Basis_Functions::Get_Instance () -> Get_Grad_Vector_Memory  ( Values );}
     ~Grad_Vector () { Preevaluated_Basis_Functions::Get_Instance () -> Free_Grad_Vector_Memory ( Values );}

      double operator ()( const int Row, const int Col ) const {
        return Values [ Row ][ Col ];
      }

      double& operator ()( const int Row, const int Col ) {
        return Values [ Row ][ Col ];
      }

      int Dimension () const { return Preevaluated_Basis_Functions::Get_Instance () -> Number_Of_Quadrature_Points ();}

    private :

      double** Values;

    };

    //------------------------------------------------------------//


    int Number_Of_Quadrature_Points () { return Size; }

    void Preevaluate ( const int Degree_Plane,
                       const int Degree_Normal );


    void Set_Jacobian ( const int                      Quadrature_Position,
                        const ElementGeometryMatrix& Geometry_Matrix,
                              Matrix3x3&             Jacobian ) const;

    void Evaluate_Coefficients ( const ElementVector& Coefficients,
                                       Vector&        Evaluations );

    void Evaluate_Coefficients ( const ElementVector& Coefficients,
                                       Grad_Vector&   Evaluations );


    void Sum_Basis_Vectors ( const Vector&        Coefficients,
                                   ElementVector& Residual ) const;

    void Sum_Grad_Basis_Vectors ( const Grad_Vector&   Coefficients,
                                        ElementVector& Residual ) const;


  private :


    Preevaluated_Basis_Functions ();
   ~Preevaluated_Basis_Functions ();


    double **Preevaluated_Bases;
    double ***Preevaluated_Grad_Bases;

    int Quadrature_Degree_Plane;
    int Quadrature_Degree_Normal;
    int Size;

    //------------------------------------------------------------//

    friend class Vector;

    ///
    ///
    /// A VERY SIMPLE memory management scheme.
    ///
    /// 
    /// Memory_For_Vectors:     an array containing ALL the memory that the vectors will use. So if more than
    ///                         'Maximum_Number_Of_Vectors' vectors are declared then this will be a problem.
    ///
    /// Vector_Memory_Position: The position in the above array that the memory for the next vector will come from.
    ///
    /// Vector_Count:           A count of the number of vectors that has been allocated. If this reaches zero then
    ///                         Memory_Position will also be reset to zero.
    ///
    ///
    ///
    /// Whenever a vector is declared it gets given the next chunk of memory at position in the Memory_For_Vectors array and the counter (Vector_Count) is incremented. 
    /// On destruction the memory itself is not changed, but a counter (Vector_Count) is decremented. When this counter
    /// reaches zero the Memory_Position is reset to zero.
    ///
    void Next_Vector_Memory ( double*& Memory_Chunk );
    void Release_Vector_Memory ( const double* Memory_To_Release );

    static const int Maximum_Number_Of_Vectors = 30;

    double* Memory_For_Vectors;
    int     Vector_Memory_Position;
    int     Vector_Count;

    //------------------------------------------------------------//

    friend class Grad_Vector;
    ///
    ///
    /// The memory management is exactly the same as for the Vectors, except this time there is 
    /// the extra dimension (Nx8x3)
    ///
    void Get_Grad_Vector_Memory  ( double**& Memory );
    void Free_Grad_Vector_Memory ( double**& Memory_To_Release );

    static const int Maximum_Number_Of_Grad_Vectors = 5;

    double*  Memory_For_Grad_Vectors;
    double** Memory_For_Grad_Vectors_Ptrs;
    int     Grad_Vector_Memory_Position;
    int     Grad_Vector_Memory_Ptr_Position;
    int     Grad_Vector_Count;


  };



//    class Preevaluated_Basis_Functions {

//    public :

//      static Preevaluated_Basis_Functions* Get_Instance () { static Preevaluated_Basis_Functions Bases; return &Bases;}

//      //------------------------------------------------------------//

//      class Vector {

//      public :

//        Vector () { Values = new double [ Preevaluated_Basis_Functions::Get_Instance () -> Number_Of_Quadrature_Points ()];}
//       ~Vector () { delete [] Values;}

//        double operator ()( const int Position ) const {
//          return Values [ Position ];
//        }

//        double& operator ()( const int Position ) {
//          return Values [ Position ];
//        }

//        int Dimension () const { return Preevaluated_Basis_Functions::Get_Instance () -> Number_Of_Quadrature_Points ();}

//      private :

//        double* Values;

//      };

//      //------------------------------------------------------------//


//      int Number_Of_Quadrature_Points () { return Size; }

//      void Preevaluate ( const int Degree_Plane,
//                         const int Degree_Normal );


//      void Evaluate_Coefficients ( const Element_Vector& Coefficients,
//                                         Vector&         Evaluations );


//    private :

//      Preevaluated_Basis_Functions ();
//     ~Preevaluated_Basis_Functions ();


//      double **Preevaluated_Bases;

//      int Quadrature_Degree_Plane;
//      int Quadrature_Degree_Normal;
//      int Size;

//    };



  typedef Preevaluated_Basis_Functions::Vector Evaluations_Vector;

  typedef Preevaluated_Basis_Functions::Grad_Vector Grad_Evaluations_Vector;


}


//------------------------------------------------------------//

#endif // __Preevaluated_Basis_Functions_HH__

//------------------------------------------------------------//
