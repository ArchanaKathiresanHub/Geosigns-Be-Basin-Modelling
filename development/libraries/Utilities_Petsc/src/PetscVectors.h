#ifndef _PETSCVECTORS_H_
#define _PETSCVECTORS_H_

#include "petscsys.h"
#include "petscvec.h"
#include "petscda.h"

      #include<iostream>
      #include<fstream>
      using namespace std;


//
// Abstract class interface for PETSC Vectors and arrays
//
//
// The Petsc Vector class provides an interface giving access to DA global
// and local vectors and arrays. It takes care of their creation and deletion
// restoring the local vector to the global vector when it is being destroyed
// and also allows the local vector to be traversed in the same mannor whether
// it is 1D, 2D or 3D. 
// The base class is Petsc_Array and PETSC_1D_Array, PETSC_2D_Array and 
// PETSC_3D_Array inherit from it adding their own version of some methods
// and adding new methods specific to their own class
//

enum Update_Mode {
  No_Update,
  Update_Excluding_Ghosts,
  Update_Including_Ghosts };

class Petsc_Array 
{
public:
   typedef double DimType;
  
   // ctor / dtor
   Petsc_Array () {}
   virtual ~Petsc_Array () {}

   virtual void     begin                  (void) = 0;
   virtual void     inc                    (void) = 0;
   virtual DimType& currentPos             (void) = 0; 
   virtual bool     end                    (void) = 0;
   virtual int      convertCurrentToLinear (void) = 0;
   virtual int      linearSize             (void) = 0;
   virtual DimType  operator             ()(const int K, const int J=0, const int I=0 ) const = 0;
   virtual DimType& operator             ()(const int K, const int J=0, const int I=0) = 0; 
   virtual void     Restore_Global_Array   (const Update_Mode Update_Method = No_Update ) = 0;
   virtual void     Set_Global_Array       (const DA         Global_Array,
			                    const Vec        Global_Vector,
			                    const InsertMode addv = INSERT_VALUES,
                                            const bool       Include_Ghost_Values = false ) = 0;
private:
   // disallow operator= as have dynamic memory in child classes 
   Petsc_Array& operator= (const Petsc_Array &rhs);
};

class PETSC_3D_Array : public Petsc_Array
{

public :

  PETSC_3D_Array ();

  PETSC_3D_Array ( const DA         Global_Array,
                   const Vec        Global_Vector,
                   const InsertMode addv = INSERT_VALUES,
                   const bool       Include_Ghost_Values = false );

  ~PETSC_3D_Array ();

  DimType operator ()( const int K, const int J=0, const int I=0 ) const 
  {
     return Distributed_Data [ K ][ J ][ I ];
  } // end operator ()

  DimType& operator ()( const int K, const int J=0, const int I=0 ) 
  {
     return Distributed_Data [ K ][ J ][ I ];
  } // end operator ()

  void Set_Global_Array ( const DA         Global_Array,
			  const Vec        Global_Vector,
			  const InsertMode addv = INSERT_VALUES,
			  const bool       Include_Ghost_Values = false );

  void Restore_Global_Array ( const Update_Mode Update_Method = No_Update );

  // virtual iterator functions
  void begin (void) { kIt = minK; jIt = minJ; iIt = minI; }
   
  void inc (void);

  DimType& currentPos (void) { return Distributed_Data[kIt][jIt][iIt]; } 

  bool end (void) { return (kIt == maxK) && (jIt == maxJ) && (iIt == maxI); }
  
  int convertCurrentToLinear ()
  {
     return ((iIt-minI) * numJ * numK) + ((jIt-minJ) * numK) + (kIt-minK);
  }

  int linearSize () { return numK * numJ * (maxI - minI); }
  
private :

  

  DA     Global_Distributed_Array;
  Vec    Global_Distributed_Vector;
  Vec    Local_Distributed_Vector;
  double ***Distributed_Data;

  int iIt, jIt, kIt, minI, minJ, minK, maxI, maxJ, maxK, numK, numJ;
  
  bool   Data_Not_Restored;
  
  // static fstream testOut;
}; // end class PETSC_3D_Array

static fstream testOut;

class PETSC_2D_Array : public Petsc_Array 
{

public :

  PETSC_2D_Array ();

  PETSC_2D_Array ( const DA         Global_Array,
                   const Vec        Global_Vector,
                   const InsertMode addv = INSERT_VALUES,
                   const bool       Include_Ghost_Values = false );

  ~PETSC_2D_Array ();

  DimType operator ()( const int J, const int I=0, const int K=0) const 
  {
     return Distributed_Data [ J ][ I ];
  } // end operator ()

  DimType& operator ()( const int J, const int I=0, const int K=0) 
  {
     return Distributed_Data [ J ][ I ];
  } // end operator ()

  void Set_Global_Array ( const DA         Global_Array,
			  const Vec        Global_Vector,
			  const InsertMode addv = INSERT_VALUES,
			  const bool       Include_Ghost_Values = false );

  void Restore_Global_Array ( const Update_Mode Update_Method = No_Update );

  // virtual iterator functions
  void begin (void) { jIt = minJ; iIt = minI; }
   
  void inc (void); 

  DimType& currentPos (void) 
  { 
     return Distributed_Data[jIt][iIt]; 
  } 

  bool end (void) { return (jIt == maxJ) && (iIt == maxI); }
  
  int convertCurrentToLinear ()
  {
     return ((iIt-minI) * (numJ)) + (jIt-minJ);
  } 

  int linearSize () { return numJ * (maxI - minI); }    
  
private :

  DA     Global_Distributed_Array;
  Vec    Global_Distributed_Vector;
  Vec    Local_Distributed_Vector;
  double **Distributed_Data;

  bool   Data_Not_Restored;

  int iIt, jIt, minI, minJ, maxI, maxJ, numJ;

}; // end class PETSC_2D

class PETSC_1D_Array : public Petsc_Array 
{

public :

  PETSC_1D_Array ();

  PETSC_1D_Array ( const DA         Global_Array,
                   const Vec        Global_Vector,
                   const InsertMode addv = INSERT_VALUES,
                   const bool       Include_Ghost_Values = false );

  ~PETSC_1D_Array ();

  DimType operator ()(const int I, const int J=0, const int K=0) const 
  {
    return Distributed_Data [ I ];
  } // end operator ()

  DimType& operator ()(const int I, const int J=0, const int K=0) 
  {
    return Distributed_Data [ I ];
  } // end operator ()

  void Set_Global_Array ( const DA         Global_Array,
			  const Vec        Global_Vector,
			  const InsertMode addv = INSERT_VALUES,
			  const bool       Include_Ghost_Values = false );

  void Restore_Global_Array ( const Update_Mode Update_Method = No_Update );

  // virtual iterator functions
  void begin (void) { iIt = minI; }
   
  void inc (void) { ++iIt; }

  DimType& currentPos (void) 
  { 
     return Distributed_Data[iIt]; 
  } 

  bool end (void) { return (iIt == maxI); }
  
  int convertCurrentToLinear ()
  {
     return iIt - minI;
  } 

  int linearSize () { return maxI - minI; }    
  
private :

  DA     Global_Distributed_Array;
  Vec    Global_Distributed_Vector;
  Vec    Local_Distributed_Vector;
  double *Distributed_Data;

  bool   Data_Not_Restored;

  int iIt, minI, maxI;

}; // end class PETSC_1D

void Destroy_Petsc_Vector ( Vec& vector );
void View_Petsc_Vector ( Vec& vector );

//============================================================//


#endif /* _PETSCVECTORS_H_ */
