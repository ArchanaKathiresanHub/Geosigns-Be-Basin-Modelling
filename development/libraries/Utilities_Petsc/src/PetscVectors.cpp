//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifdef SEEK_SET
#undef SEEK_SET
#endif
#include "mpi.h"
#include "PetscVectors.h"
#include <iostream>
using namespace std;

//#include "ConstantsFastcauldron.h"

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

PETSC_3D_Array::PETSC_3D_Array () 
: Petsc_Array ()
{

  Global_Distributed_Array  = nullptr;
  Global_Distributed_Vector = nullptr;
  Local_Distributed_Vector  = nullptr;
  Distributed_Data          = nullptr;
  Data_Not_Restored         = false;
  iIt = jIt = kIt = minI = minJ = minK = maxI = maxJ = maxK = numK = numJ = 0;

} // end PETSC_3D_Array::constructor


//------------------------------------------------------------//

PETSC_3D_Array::PETSC_3D_Array ( const DM         Global_Array, 
                                 const Vec        Global_Vector, 
                                 const InsertMode addv,
                                 const bool       Include_Ghost_Values ) { 

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
      //
      //    DMGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
      //    can not be used with new PETSc Because of the way PETSc handle reference counting you cannot put 
      //    a DMRestoreLocalVector() or DMRestoreGlobalVector() into an object destructor. From PETSc developers
	 /* ========= Solution =============================================================================================
            1) locate all uses of DMGetLocalVector() or DMGetGlobalVector() where you keep the vector beyond the subroutine where the original DMGetLocalVector() or DMGetGlobalVector() is called
		    2) replace them with DMCreateLocalVector() or DMCreateGlobalVector().
		    3) Then on those vectors in your destructor call VecDestroy() instead of DMRestoreLocalVector() or DMRestoreGlobalVector()   
      */

    DMCreateLocalVector(Global_Distributed_Array, &Local_Distributed_Vector);

    DMGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DMGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DMDAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

  // get local coordinates
  DMDALocalInfo localVecInfo;
  DMDAGetLocalInfo (Global_Array, &localVecInfo);
  
  maxI = localVecInfo.xs + localVecInfo.xm;
  maxJ = localVecInfo.ys + localVecInfo.ym;
  maxK = localVecInfo.zs + localVecInfo.zm;
  minI = localVecInfo.xs;
  minJ = localVecInfo.ys;
  minK = localVecInfo.zs;
  numK = localVecInfo.zm;
  numJ = localVecInfo.ym;
 
  // init iterators;
  begin();

} // end PETSC_3D_Array::constructor


//------------------------------------------------------------//


PETSC_3D_Array::~PETSC_3D_Array () {

  if ( Data_Not_Restored ) 
  {
    DMDAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                          &Distributed_Data );
    if ( Global_Distributed_Vector != Local_Distributed_Vector) {
      //DMRestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
      Destroy_Petsc_Vector(Local_Distributed_Vector);
    }
  } // end if

} // end PETSC_3D_Array::destructor


//------------------------------------------------------------//


void PETSC_3D_Array::Set_Global_Array ( const DM         Global_Array,
					const Vec        Global_Vector,
					const InsertMode addv,
					const bool       Include_Ghost_Values) {

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    //DMGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DMCreateLocalVector(Global_Distributed_Array, &Local_Distributed_Vector);

    DMGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DMGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DMDAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                    &Distributed_Data);
  Data_Not_Restored = true;

} // end PETSC_3D_Array::Set_Global_Array


//------------------------------------------------------------//


void PETSC_3D_Array::Restore_Global_Array ( const Update_Mode Update_Method ) {

  DMDAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                        &Distributed_Data );

  switch ( Update_Method ) {

    case No_Update :

      break;

    case Update_Excluding_Ghosts :

      // DALocalToGlobal( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
      //   	       Global_Distributed_Vector );
      DMLocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
                            Global_Distributed_Vector );
      DMLocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
                          Global_Distributed_Vector );
      break;

    case Update_Including_Ghosts :

       //  Petsc 3.3: Should the ghosted locations be zero in global vector????
      // DALocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector,
      //   		    Global_Distributed_Vector );
      // DALocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector,
      //   		  Global_Distributed_Vector );      
       DMLocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector, ADD_VALUES,
                             Global_Distributed_Vector );
       DMLocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector, ADD_VALUES,
                           Global_Distributed_Vector );      
       break;

    default :
      
      cout << "Basin_Warning: Update Method not recognized in PETSC_3D_Array::Restore_Global_Array" << endl;
      cout << "         Using No_Update by default" << endl;
      break;

  } // end switch

  if ( Global_Distributed_Vector != Local_Distributed_Vector ) 
  {
   // DMRestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
    Destroy_Petsc_Vector(Local_Distributed_Vector);
  }

  Data_Not_Restored = false;

} // end PETSC_3D_Array::Restore_Global_Array

void PETSC_3D_Array::inc (void)
{ 
  if ( ++iIt == maxI )
  {
     if ( ++jIt == maxJ )
     {
        if ( ++kIt != maxK )
        {
           iIt = minI; 
           jIt = minJ;
        }
     }
     else
     {
        iIt = minI;
     }
  }
}

//------------------------------------------------------------//

PETSC_2D_Array::PETSC_2D_Array ()  
: Petsc_Array ()
{
  Global_Distributed_Array  = nullptr;
  Global_Distributed_Vector = nullptr;
  Local_Distributed_Vector  = nullptr;
  Distributed_Data          = nullptr;
  Data_Not_Restored         = false;

  iIt = jIt = minI = minJ = maxI = maxJ = numJ = 0;
  
} // end PETSC_2D_Array::constructor


//------------------------------------------------------------//


PETSC_2D_Array::PETSC_2D_Array ( const DM         Global_Array, 
                                 const Vec        Global_Vector, 
                                 const InsertMode addv,
                                 const bool       Include_Ghost_Values ) { 

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    //DMGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DMCreateLocalVector(Global_Distributed_Array, &Local_Distributed_Vector);

    DMGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DMGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DMDAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

  // get local coordinates
  DMDALocalInfo localVecInfo;
  DMDAGetLocalInfo (Global_Array, &localVecInfo);
  
  maxJ = localVecInfo.ys + localVecInfo.ym;
  maxI = localVecInfo.xs + localVecInfo.xm;
  minJ = localVecInfo.ys;
  minI = localVecInfo.xs;
  numJ = localVecInfo.ym;
  
  // init iterators
  begin();

} // end PETSC_2D_Array::constructor


//------------------------------------------------------------//


PETSC_2D_Array::~PETSC_2D_Array () {

  if ( Data_Not_Restored ) {
    DMDAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
			&Distributed_Data );
    if ( Global_Distributed_Vector != Local_Distributed_Vector) {
      //DMRestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
      Destroy_Petsc_Vector(Local_Distributed_Vector);
    }
  } // end if

} // end PETSC_2D_Array::destructor


//------------------------------------------------------------//


void PETSC_2D_Array::Set_Global_Array ( const DM         Global_Array,
					const Vec        Global_Vector,
					const InsertMode addv,
					const bool       Include_Ghost_Values) {

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    //DMGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DMCreateLocalVector(Global_Distributed_Array, &Local_Distributed_Vector);

    DMGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DMGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DMDAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

} // end PETSC_2D_Array::Set_Global_Array


//------------------------------------------------------------//


void PETSC_2D_Array::Restore_Global_Array ( const Update_Mode Update_Method ) {

  DMDAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                        &Distributed_Data );

  switch ( Update_Method ) {

    case No_Update :

      break;

    case Update_Excluding_Ghosts :

      // DALocalToGlobal( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
      //   	       Global_Distributed_Vector );
      DMLocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
                            Global_Distributed_Vector );
      DMLocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
                          Global_Distributed_Vector );
      break;

    case Update_Including_Ghosts :

       DMLocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector, ADD_VALUES,
			    Global_Distributed_Vector );
       DMLocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector, ADD_VALUES,
                           Global_Distributed_Vector );      
      break;

    default :
      
      cout << "Basin_warning: Update Method not recognized in PETSC_2D_Array::Restore_Global_Array" << endl;
      cout << "         Using No_Update by default" << endl;
      break;
      
  } // end switch

  if ( Global_Distributed_Vector != Local_Distributed_Vector ) 
  {
    //DMRestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
    Destroy_Petsc_Vector(Local_Distributed_Vector);
  }

  Data_Not_Restored = false;

} // end PETSC_2D_Array::Restore_Global_Array

//------------------------------------------------------------//

void PETSC_2D_Array::inc (void) 
{ 
  if ( ++iIt == maxI )
  {
     if ( ++jIt != maxJ )
     {
        iIt = minI;
     }
  }
}

//------------------------------------------------------------//

PETSC_1D_Array::PETSC_1D_Array ()  
: Petsc_Array ()
{
  Global_Distributed_Array  = nullptr;
  Global_Distributed_Vector = nullptr;
  Local_Distributed_Vector  = nullptr;
  Distributed_Data          = nullptr;
  Data_Not_Restored         = false;

  iIt = minI = maxI = 0;
  
} // end PETSC_1D_Array::constructor

//------------------------------------------------------------//


PETSC_1D_Array::PETSC_1D_Array ( const DM         Global_Array, 
                                 const Vec        Global_Vector, 
                                 const InsertMode addv,
                                 const bool       Include_Ghost_Values ) { 

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) 
  {
    //DMGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DMCreateLocalVector(Global_Distributed_Array, &Local_Distributed_Vector);

    DMGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DMGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } 
  else 
  {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DMDAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                    &Distributed_Data);
 
  Data_Not_Restored = true;

  // get local coordinates
  DMDALocalInfo localVecInfo;
  DMDAGetLocalInfo (Global_Array, &localVecInfo);
  
  maxI = localVecInfo.xs + localVecInfo.xm;
  minI = localVecInfo.xs;
  
  // init iterators
  begin();

} // end PETSC_1D_Array::constructor

//------------------------------------------------------------//

PETSC_1D_Array::~PETSC_1D_Array () 
{
  if ( Data_Not_Restored ) 
  {
     DMDAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                           &Distributed_Data );
    
     if ( Global_Distributed_Vector != Local_Distributed_Vector) 
     {
        //DMRestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
        Destroy_Petsc_Vector(Local_Distributed_Vector);
     }
   } 

} // end PETSC_1D_Array::destructor

//------------------------------------------------------------//

void PETSC_1D_Array::Set_Global_Array ( const DM         Global_Array,
					const Vec        Global_Vector,
					const InsertMode addv,
					const bool       Include_Ghost_Values) 
{
  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) 
  {
    //DMGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DMCreateLocalVector(Global_Distributed_Array, &Local_Distributed_Vector);

    DMGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );

    DMGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } 
  else 
  {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DMDAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                    &Distributed_Data);
  Data_Not_Restored = true;

} // end PETSC_1D_Array::Set_Global_Array

//------------------------------------------------------------//

void PETSC_1D_Array::Restore_Global_Array ( const Update_Mode Update_Method ) 
{

  DMDAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
                        &Distributed_Data );

  switch ( Update_Method ) {

    case No_Update :

      break;

    case Update_Excluding_Ghosts :

      // DALocalToGlobal( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
      //   	       Global_Distributed_Vector );
      DMLocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
                            Global_Distributed_Vector );
      DMLocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
                          Global_Distributed_Vector );
      break;

    case Update_Including_Ghosts :

       //  Petsc 3.3: Should the ghosted locations be zero in global vector????
       DMLocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector, ADD_VALUES,
                             Global_Distributed_Vector );
       DMLocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector, ADD_VALUES,
                           Global_Distributed_Vector );      
      break;

    default :
      
      cout << "Basin_Warning: Update Method not recognized in PETSC_1D_Array::Restore_Global_Array" << endl;
      cout << "         Using No_Update by default" << endl;
      break;
      
  } // end switch

  if ( Global_Distributed_Vector != Local_Distributed_Vector ) 
  {
    //DMRestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
    Destroy_Petsc_Vector(Local_Distributed_Vector);
  }

  Data_Not_Restored = false;

} // end PETSC_1D_Array::Restore_Global_Array

//------------------------------------------------------------//

void Destroy_Petsc_Vector ( Vec& vector ) {
 
   PetscBool IsValid;
   
   VecValid( vector, &IsValid );
   
   if ( IsValid ) {
      
      VecDestroy ( &vector );
      vector = nullptr;
   }
   
} // end Destroy_Petsc_Vector 

//------------------------------------------------------------//
void VecValid( Vec& vector, PetscBool * isValid ) {
   
   * isValid = ( vector != nullptr ? PETSC_TRUE : PETSC_FALSE );

}
//------------------------------------------------------------//

