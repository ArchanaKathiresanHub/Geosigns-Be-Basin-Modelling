#ifndef __rbyutilities_multidimensional_array___
#define __rbyutilities_multidimensional_array___

// Templates for the creation (with or without initialization)
// and deletion of multi-dimensional arrays
//
// Note that the functions are set up recursively,
// e.g create3d () is defined in terms of create2d () and.
// delete3d () is defined in terms of delete2d ().
//
// Captured within a class because it won't work otherwise.
// It does not appear possible to call template functions using explicit
// template arguments (Section 10.4 of the C++ primer).
//
// Calling patterns, e.g.:
//    float ** a = Array<float>::create2d (5, 10);
//    float ** a = Array<float>::create2d (5, 10, 0.0); // with initialization
//    Array<float>::delete2d(a);
//
// What I would like them to be:
//    float ** a = create2d<float> (5, 10);
//    float ** a = create2d<float> (5, 10, 0.0);
//    delete2d<float> (a);

#include <iostream>

#define CheckAlloc(myarray,type,size) \
   if (!myarray) \
   { \
      std::cerr << "Failed to allocate " << size << " elements of size " \
           << sizeof (type) << ", totalling " << size * sizeof (type) \
           << " bytes" << std::endl; \
      return 0; \
   } 

#define CheckArray(myarray) \
   if (!myarray) \
   { \
      return 0; \
   } 
namespace ibs
{
template <class Type>
class Array {
 public:
   static Type * create1d (size_t n1)
   {
      Type * array = new Type [n1];
      CheckAlloc (array, Type, n1);
      return array;
   }

   static Type * create1d (size_t n1, Type value)
   {
      Type * array = Array<Type>::create1d (n1);
      CheckArray (array);
      for (size_t j = 0; j < n1; ++j)
      {
         array[j] = value;
      }
      return array;
   }
   
   static void delete1d (Type * array)
   {
      delete [] array;
   }
   
   static Type ** create2d (size_t n1, size_t n2)
   {
      Type * array1d = Array<Type>::create1d (n1 * n2);
      CheckArray (array1d);

      Type ** array = Array<Type *>::create1d (n1);
      CheckArray (array);
      
      for (size_t j = 0; j < n1; ++j)
      {
         array[j] = array1d + j * n2;
      }

      return array;
   }
   
   static Type ** create2d (size_t n1, size_t n2, Type value)
   {
      Type ** array = Array<Type>::create2d (n1, n2);
      CheckArray (array);
      
      for (size_t j = 0; j < n1*n2; ++j)
      {
         array[0][j] = value;
      }
      return array;
   }

   static void delete2d (Type ** array)
   {
      delete [] array[0];
      Array<Type *>::delete1d (array);
   }
   
   static Type *** create3d (size_t n1, size_t n2, size_t n3)
   {
      Type * array1d = Array<Type>::create1d (n1 * n2 * n3);
      CheckArray (array1d);

      Type *** array = Array<Type *>::create2d (n1, n2);
      CheckArray (array);
      
      for (size_t j = 0; j < n1*n2; ++j)
      {
         array[0][j] = array1d + j * n3;
      }

      return array;
   }
   
   static Type *** create3d (size_t n1, size_t n2, size_t n3, Type value)
   {
      Type *** array = Array<Type>::create3d (n1, n2, n3);
      CheckArray (array);
      
      for (size_t j = 0; j < n1*n2*n3; ++j)
      {
         array[0][0][j] = value;
      }
      return array;
   }

   static void delete3d (Type *** array)
   {
      delete [] array[0][0];
      Array<Type *>::delete2d (array);
   }

   static Type **** create4d (size_t n1, size_t n2, size_t n3, size_t n4)
   {
      Type * array1d = Array<Type>::create1d (n1 * n2 * n3 * n4);
      CheckArray (array1d);

      Type **** array = Array<Type *>::create3d (n1, n2, n3);
      CheckArray (array);
      
      for (size_t j = 0; j < n1*n2*n3; ++j)
      {
         array[0][0][j] = array1d + j * n4;
      }

      return array;
   }

   static Type **** create4d (size_t n1, size_t n2,size_t n3, size_t n4, Type value)
   {
      Type **** array = Array<Type>::create4d (n1, n2, n3, n4);
      CheckArray (array);
      for (size_t j = 0; j < n1*n2*n3*n4; ++j)
      {
         array[0][0][0][j] = value;
      }
      return array;
   }
   
   static void delete4d (Type **** array)
   {
      delete [] array[0][0][0];
      Array<Type *>::delete3d (array);
   }
};


   template <class Type>
   class Array2D {

   public :

      Array2D ( const int xSize, const int ySize ) {
         Dimensions [ 0 ] = xSize;
         Dimensions [ 1 ] = ySize;

         Entries = Array<Type>::create2d ( xSize, ySize );
      }

      ~Array2D () {
         Array<Type>::delete2d ( Entries );
      }

      Type  operator ()( const int xPosition, const int yPosition ) const {
         return Entries [ xPosition ][ yPosition ];
      }

      Type& operator ()( const int xPosition, const int yPosition ) {
         return Entries [ xPosition ][ yPosition ];
      }

      int Last ( const int dimension ) const {
         return Dimensions [ dimension - 1 ];
      }

      friend std::ostream& operator << ( std::ostream& o, const Array2D& A ) {

         int I;
         int J;

         for ( I = 0; I < A.Dimensions [ 0 ]; I++ ) {

            for ( J = 0; J < A.Dimensions [ 1 ]; J++ ) {
               std::cout << A.Entries [ I ][ J ] << "  ";
            }

            std::cout << std::endl;
         }

         std::cout << std::endl;
         return o;
      }



   private :

      Type** Entries;
      int    Dimensions [ 2 ];

   };


  template <class Type>
  class Array3D {

  public :

    Array3D ( const int X_Size, const int Y_Size, const int Z_Size ) {
      Dimensions [ 0 ] = X_Size;
      Dimensions [ 1 ] = Y_Size;
      Dimensions [ 2 ] = Z_Size;

      Entries = Array<Type>::create3d ( X_Size, Y_Size, Z_Size );
    }

    ~Array3D () {
      Array<Type>::delete3d ( Entries );
    }

    Type  operator ()( const int X_Position, const int Y_Position, const int Z_Position ) const {
      return Entries [ X_Position ][ Y_Position ][ Z_Position ];
    }

    Type& operator ()( const int X_Position, const int Y_Position, const int Z_Position ) {
      return Entries [ X_Position ][ Y_Position ][ Z_Position ];
    }

    int First ( const int Dimension ) const {
      return 0;
    }

    int Last ( const int Dimension ) const {
      return Dimensions [ Dimension - 1 ];
    }

  private :

    Type*** Entries;
    int     Dimensions [ 3 ];

  }; // end class Array3D

}
using namespace ibs;

#endif // __rbyutilities_multidimensional_array___
