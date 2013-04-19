#ifdef SEEK_SET
#undef SEEK_SET
#endif
#include "mpi.h"
#include "PetscVectors.h"
#include <iostream.h>

//#include "globaldefs.h"

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

  Global_Distributed_Array  = 0;
  Global_Distributed_Vector = 0;
  Local_Distributed_Vector  = 0;
  Distributed_Data          = 0;
  Data_Not_Restored         = false;
  iIt = jIt = kIt = minI = minJ = minK = maxI = maxJ = maxK = numK = numJ = 0;

} // end PETSC_3D_Array::constructor


//------------------------------------------------------------//

PETSC_3D_Array::PETSC_3D_Array ( const DA         Global_Array, 
                                 const Vec        Global_Vector, 
                                 const InsertMode addv,
                                 const bool       Include_Ghost_Values ) { 

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    DAGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DAGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DAGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

  // get local coordinates
  DALocalInfo localVecInfo;
  DAGetLocalInfo (Global_Array, &localVecInfo);
  
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
    DAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
			&Distributed_Data );
    if ( Global_Distributed_Vector != Local_Distributed_Vector) {
      DARestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
    }
  } // end if

} // end PETSC_3D_Array::destructor


//------------------------------------------------------------//


void PETSC_3D_Array::Set_Global_Array ( const DA         Global_Array,
					const Vec        Global_Vector,
					const InsertMode addv,
					const bool       Include_Ghost_Values) {

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    DAGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DAGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DAGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

} // end PETSC_3D_Array::Set_Global_Array


//------------------------------------------------------------//


void PETSC_3D_Array::Restore_Global_Array ( const Update_Mode Update_Method ) {

  DAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		      &Distributed_Data );

  switch ( Update_Method ) {

    case No_Update :

      break;

    case Update_Excluding_Ghosts :

      DALocalToGlobal( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
		       Global_Distributed_Vector );
      break;

    case Update_Including_Ghosts :

      DALocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector,
			    Global_Distributed_Vector );
      DALocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector,
			  Global_Distributed_Vector );      
      break;

    default :
      
      cout << "WARNING: Update Method not recognized in PETSC_3D_Array::Restore_Global_Array" << endl;
      cout << "         Using No_Update by default" << endl;
      break;

  } // end switch

  if ( Global_Distributed_Vector != Local_Distributed_Vector ) 
  {
    DARestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
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
  Global_Distributed_Array  = 0;
  Global_Distributed_Vector = 0;
  Local_Distributed_Vector  = 0;
  Distributed_Data          = 0;
  Data_Not_Restored         = false;

  iIt = jIt = minI = minJ = maxI = maxJ = numJ = 0;
  
} // end PETSC_2D_Array::constructor


//------------------------------------------------------------//


PETSC_2D_Array::PETSC_2D_Array ( const DA         Global_Array, 
                                 const Vec        Global_Vector, 
                                 const InsertMode addv,
                                 const bool       Include_Ghost_Values ) { 

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    DAGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DAGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DAGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

  // get local coordinates
  DALocalInfo localVecInfo;
  DAGetLocalInfo (Global_Array, &localVecInfo);
  
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
    DAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
			&Distributed_Data );
    if ( Global_Distributed_Vector != Local_Distributed_Vector) {
      DARestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
    }
  } // end if

} // end PETSC_2D_Array::destructor


//------------------------------------------------------------//


void PETSC_2D_Array::Set_Global_Array ( const DA         Global_Array,
					const Vec        Global_Vector,
					const InsertMode addv,
					const bool       Include_Ghost_Values) {

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) {
    DAGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DAGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DAGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } else {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

} // end PETSC_2D_Array::Set_Global_Array


//------------------------------------------------------------//


void PETSC_2D_Array::Restore_Global_Array ( const Update_Mode Update_Method ) {

  DAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		      &Distributed_Data );

  switch ( Update_Method ) {

    case No_Update :

      break;

    case Update_Excluding_Ghosts :

      DALocalToGlobal( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
		       Global_Distributed_Vector );
      break;

    case Update_Including_Ghosts :

      DALocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector,
			    Global_Distributed_Vector );
      DALocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector,
			  Global_Distributed_Vector );      
      break;

    default :
      
      cout << "WARNING: Update Method not recognized in PETSC_2D_Array::Restore_Global_Array" << endl;
      cout << "         Using No_Update by default" << endl;
      break;
      
  } // end switch

  if ( Global_Distributed_Vector != Local_Distributed_Vector ) 
  {
    DARestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
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
  Global_Distributed_Array  = 0;
  Global_Distributed_Vector = 0;
  Local_Distributed_Vector  = 0;
  Distributed_Data          = 0;
  Data_Not_Restored         = false;

  iIt = minI = maxI = 0;
  
} // end PETSC_1D_Array::constructor

//------------------------------------------------------------//


PETSC_1D_Array::PETSC_1D_Array ( const DA         Global_Array, 
                                 const Vec        Global_Vector, 
                                 const InsertMode addv,
                                 const bool       Include_Ghost_Values ) { 

  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) 
  {
    DAGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );
    DAGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
    DAGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } 
  else 
  {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
 
  Data_Not_Restored = true;

  // get local coordinates
  DALocalInfo localVecInfo;
  DAGetLocalInfo (Global_Array, &localVecInfo);
  
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
     DAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
			&Distributed_Data );
    
     if ( Global_Distributed_Vector != Local_Distributed_Vector) 
     {
        DARestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
     }
   } 

} // end PETSC_1D_Array::destructor

//------------------------------------------------------------//

void PETSC_1D_Array::Set_Global_Array ( const DA         Global_Array,
					const Vec        Global_Vector,
					const InsertMode addv,
					const bool       Include_Ghost_Values) 
{
  Global_Distributed_Array  = Global_Array;
  Global_Distributed_Vector = Global_Vector;

  if ( Include_Ghost_Values ) 
  {
    DAGetLocalVector     ( Global_Distributed_Array, &Local_Distributed_Vector );

    DAGlobalToLocalBegin ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );

    DAGlobalToLocalEnd   ( Global_Distributed_Array, Global_Distributed_Vector, 
			   addv, Local_Distributed_Vector );
  } 
  else 
  {
    Local_Distributed_Vector = Global_Distributed_Vector;
  }// end if

  DAVecGetArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		  &Distributed_Data);
  Data_Not_Restored = true;

} // end PETSC_1D_Array::Set_Global_Array

//------------------------------------------------------------//

void PETSC_1D_Array::Restore_Global_Array ( const Update_Mode Update_Method ) 
{

  DAVecRestoreArray ( Global_Distributed_Array, Local_Distributed_Vector, 
		      &Distributed_Data );

  switch ( Update_Method ) {

    case No_Update :

      break;

    case Update_Excluding_Ghosts :

      DALocalToGlobal( Global_Distributed_Array, Local_Distributed_Vector, INSERT_VALUES,
		       Global_Distributed_Vector );
      break;

    case Update_Including_Ghosts :

      DALocalToGlobalBegin( Global_Distributed_Array, Local_Distributed_Vector,
			    Global_Distributed_Vector );
      DALocalToGlobalEnd( Global_Distributed_Array, Local_Distributed_Vector,
			  Global_Distributed_Vector );      
      break;

    default :
      
      cout << "WARNING: Update Method not recognized in PETSC_1D_Array::Restore_Global_Array" << endl;
      cout << "         Using No_Update by default" << endl;
      break;
      
  } // end switch

  if ( Global_Distributed_Vector != Local_Distributed_Vector ) 
  {
    DARestoreLocalVector( Global_Distributed_Array, &Local_Distributed_Vector);
  }

  Data_Not_Restored = false;

} // end PETSC_1D_Array::Restore_Global_Array

//------------------------------------------------------------//

void Destroy_Petsc_Vector ( Vec& vector ) {
  
  PetscTruth IsValid;

  VecValid( vector, &IsValid );

  if ( IsValid ) {

    VecDestroy ( vector );
    vector = 0;
  }

  
} // end Destroy_Petsc_Vector 

//------------------------------------------------------------//

//------------------------------------------------------------//

// void View_Petsc_Vector ( Vec& vector ) {
  
//   PetscViewer viewer;

//   PetscViewerDrawOpen(PETSC_COMM_WORLD,0,"",300,0,300,300,&viewer);

//   VecView(vector,viewer);

//   PetscViewerDestroy(viewer);
  
// } // end View_Petsc_Vector 

//------------------------------------------------------------//
