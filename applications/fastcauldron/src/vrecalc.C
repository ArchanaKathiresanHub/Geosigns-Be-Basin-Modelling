#include "vrecalc.h"
#include "thermalindicator.h"
#include "globaldefs.h"
#include "petscvec.h"
#include "PetscVectors.h"
#include "layer_iterators.h"
#include "property_manager.h"
#include "FastcauldronSimulator.h"

#include "Interface/RunParameters.h"

#include "NumericFunctions.h"

using namespace Basin_Modelling;

VreCalc::VreCalc( const AppCtx* Application_Context ) {

  Basin_Model = const_cast<AppCtx*>(Application_Context);

  ALGORITHMTYPE = LOPATINMETHOD;
  FFACTOR       = MODIFIEDLOPATINFACTOR;
  REFTEMP       = LOPATINREFTEMP;
  TEMPINC       = LOPATINTEMPINC;
  P             = MODIFIEDLOPATINP;
  Q             = MODIFIEDLOPATINQ;
  NAES          = 0;
  ARR           = 0.000000;
  FE            = 0;
  EN            = 0;

  bool ret = SetAlgorithm ( FastcauldronSimulator::getInstance ().getRunParameters ()->getVreAlgorithm ());
  if (!ret) {
    cout << "Error Setting Vitrinite Calculation Algorithm" << endl;
  }

  ret = InitializeGINT();

};

VreCalc::~VreCalc()
{
  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Ascending, Sediments_Only, 
			       Active_And_Inactive_Layers );

  while ( ! Layers.Iteration_Is_Done () ) {

    Layers.Current_Layer () -> destroyGINT();
    Layers++;

  }

};

#undef __FUNCT__  
#define __FUNCT__ "VreCalc::CalcSnaptimeVr"

bool VreCalc::CalcSnaptimeVr ( const double    time,
                               const Boolean2DArray& dataPresent )
{
  int xs, ys, zs, xm, ym, zm;
  int i, j, k;

  double ***vreintvalue, Vr;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, 
			       Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    DAGetCorners(Current_Layer ->layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

    DAVecGetArray(Current_Layer ->layerDA, 
		  Current_Layer ->VreIntValue,  &vreintvalue);

    DACreateGlobalVector(Current_Layer ->layerDA,
			 &Current_Layer ->Vre);
    VecSet(Current_Layer ->Vre, CAULDRONIBSNULLVALUE);
    double ***vre;
    DAVecGetArray(Current_Layer ->layerDA, Current_Layer ->Vre, 
		   &vre);
    double **presentdayvre;
    if ( (time == Present_Day) && Current_Layer -> isSourceRock() ) {
      DAVecGetArray(*Basin_Model -> mapDA, Current_Layer->Present_Day_VRE, 
		     &presentdayvre); 
    }
    
    for (i = xs; i < xs+xm; i++) {
      for (j = ys; j < ys+ym; j++) {
	if ( !dataPresent(i,j) ) continue;
	for (k = zs; k < zs+zm; k++) {

	  if ( ALGORITHMTYPE == SWEENEYMETHOD ) {
	    Vr = VreSweeney (vreintvalue[k][j][i]);
	  } else if (ALGORITHMTYPE == LARTERMETHOD) {
	    Vr = VreLarter (vreintvalue[k][j][i]);
	  } else {
	    Vr = TtiToVr (vreintvalue[k][j][i],P,Q,INITIALVRE);
	  }
	  vre[k][j][i] = Vr;

	  if ( (time == Present_Day) && Current_Layer -> isSourceRock() && (k == zs+zm-1) ) {
	    presentdayvre[j][i] = Vr;
	  }
	}
      }
    }

    DAVecRestoreArray(Current_Layer ->layerDA, 
		      Current_Layer ->VreIntValue,  &vreintvalue);
    DAVecRestoreArray(Current_Layer ->layerDA, Current_Layer ->Vre, 
		       &vre);

    if ( (time == Present_Day) && Current_Layer -> isSourceRock() ) {
      DAVecRestoreArray(*Basin_Model -> mapDA, Current_Layer->Present_Day_VRE, 
		     &presentdayvre);      
    }

    Layers++;
  }
 
  return true;
}

#undef __FUNCT__  
#define __FUNCT__ "VreCalc::CalcStep"

bool VreCalc::CalcStep (double time, double timeStep)
{

  bool Node_Is_Active;
  int xs, ys, zs, xm, ym, zm;
  int i, j, k;

  double ***vreintvalue, ***starttemperature, ***endtemperature;
  double *GINT;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, 
			       Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    DAGetCorners(Current_Layer ->layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

    DAVecGetArray(Current_Layer ->layerDA, 
		  Current_Layer ->VreIntValue,  &vreintvalue);

    DAVecGetArray(Current_Layer ->layerDA, 
                  Current_Layer -> Current_Properties ( Basin_Modelling::Temperature ),
                   &endtemperature);

    DAVecGetArray(Current_Layer ->layerDA, 
                  Current_Layer -> Previous_Properties ( Basin_Modelling::Temperature ),
		   &starttemperature);

    double***depth;
    DAVecGetArray(Current_Layer -> layerDA, 
		  Current_Layer -> Current_Properties ( Basin_Modelling::Depth ), 
		   &depth);

    for (i = xs; i < xs+xm; i++) {
      for (j = ys; j < ys+ym; j++) {
	for (k = zs; k < zs+zm; k++) {

          // If, during deposition, the node is at the surface then do not start the vre calculation.
          if ( time >= Current_Layer->depoage &&
               NumericFunctions::isEqual ( depth[k][j][i], FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, time ), 1.0e-6 )) {
            continue;
          }
	  
	  if (k>zs && (depth[k][j][i] == FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, time ))) {
	    Node_Is_Active = Current_Layer->getSolidThickness ( i, j, k - 1, time) > 0.0;
// 	    Node_Is_Active = Current_Layer ->FCT( i,j,k-1 ).F(time) > 0.0 ? true : false;
	    if (!Node_Is_Active) continue;
	  }

	  if ( (ALGORITHMTYPE == SWEENEYMETHOD) || 
	       (ALGORITHMTYPE == LARTERMETHOD) ) {
	    GINT = Current_Layer ->gint[i-xs][j-ys][k-zs];
	    vreintvalue[k][j][i] = 
	      VreKinetic (timeStep, starttemperature[k][j][i],
			  endtemperature[k][j][i],ARR,FE,EN,GINT,NAES);
	  } else {
	    vreintvalue[k][j][i] += 
	      Lopatin (timeStep,starttemperature[k][j][i],
		       endtemperature[k][j][i],FFACTOR,REFTEMP,TEMPINC);
	  }

	}
      }
    }

    DAVecRestoreArray(Current_Layer ->layerDA, 
		      Current_Layer ->VreIntValue,  &vreintvalue);
    DAVecRestoreArray(Current_Layer ->layerDA, 
                      Current_Layer -> Current_Properties ( Basin_Modelling::Temperature ),
                       &endtemperature);
    DAVecRestoreArray(Current_Layer ->layerDA, 
                      Current_Layer -> Previous_Properties ( Basin_Modelling::Temperature ),
		       &starttemperature);
    DAVecRestoreArray(Current_Layer ->layerDA, 
                      Current_Layer -> Current_Properties ( Basin_Modelling::Depth ), 
		       &depth);

    Layers++;
  }

  return true;

}

bool VreCalc::SetAlgorithm ( const std::string& algorithmName )
{
  bool result = true;

   if (algorithmName == VRALGBURNHAMSWEENEY)
   {
      (ALGORITHMTYPE) = SWEENEYMETHOD;
      GetSweeneyData (&(NAES),
                      &(ARR),
                      &(FE),
                      &(EN));

   }
   else if (algorithmName == VRALGLARTER)
   {
      (ALGORITHMTYPE) = LARTERMETHOD;
      GetLarterData (&(NAES),
                     &(ARR),
                     &(FE),
                     &(EN));
   }
   else if (algorithmName == VRALGLOPATINMODIFIED)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = MODIFIEDLOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = MODIFIEDLOPATINP;
      (Q) = MODIFIEDLOPATINQ;
   }
   else if (algorithmName == VRALGLOPATINDYKSTRA)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = LOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = LOPATINDYKSTRAP;
      (Q) = LOPATINDYKSTRAQ;
   }
   else if (algorithmName == VRALGLOPATINWAPPLES)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = LOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = LOPATINWAPPLESP;
      (Q) = LOPATINWAPPLESQ;
   }
   else if (algorithmName == VRALGLOPATINROYDEN)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = LOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = LOPATINROYDENP;
      (Q) = LOPATINROYDENQ;
   }
   else if (algorithmName == VRALGLOPATINISSLER)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = LOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = LOPATINISSLERP;
      (Q) = LOPATINISSLERQ;
   }
   else if (algorithmName == VRALGLOPATINHOOD)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = LOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = LOPATINHOODP;
      (Q) = LOPATINHOODQ;
   }
   else if (algorithmName == VRALGLOPATINGOFF)
   {
      (ALGORITHMTYPE) = LOPATINMETHOD;
      (FFACTOR) = LOPATINFACTOR;
      (REFTEMP) = LOPATINREFTEMP;
      (TEMPINC) = LOPATINTEMPINC;
      (P) = LOPATINGOFFP;
      (Q) = LOPATINGOFFQ;
   }
   else
   {
     cout << "Invalid Vitrinite calculation algorithm. Previous used" << endl;
      result = false;
   }
   return result;

}


bool VreCalc::InitializeGINT ()
{
  bool result = true;
  
  if ( (ALGORITHMTYPE == SWEENEYMETHOD) || 
       (ALGORITHMTYPE == LARTERMETHOD) ) {
    int NNodeX, NNodeY,NNodeZ;
    DAGetCorners ( *Basin_Model -> mapDA, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                   &NNodeX, &NNodeY, PETSC_NULL); 
    
    Layer_Iterator Layers;
    Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, 
				 Active_And_Inactive_Layers );

    while ( ! Layers.Iteration_Is_Done () ) {
      
      LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

      NNodeZ = Current_Layer -> getMaximumNumberOfElements();

      if ( NNodeZ > 0 ) {
	Current_Layer -> gint = Array<double>::create4d(NNodeX,NNodeY,NNodeZ+1,NAES,0.0);
      }

      Layers++;
    }
    
  }
  
  return result;
}


void VreCalc::initialiseVectors () {

  if ( (ALGORITHMTYPE == SWEENEYMETHOD) || 
       (ALGORITHMTYPE == LARTERMETHOD) ) {

    int numberOfXNodes;
    int numberOfYNodes;
    int numberOfZNodes;
    int size;
    int i;

    DAGetCorners(*Basin_Model -> mapDA, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                 &numberOfXNodes, &numberOfYNodes, PETSC_NULL); 
    
    Layer_Iterator Layers;
    Layers.Initialise_Iterator ( Basin_Model -> layers, Descending, Sediments_Only, 
				 Active_And_Inactive_Layers );

    while ( ! Layers.Iteration_Is_Done () ) {
      
      LayerProps_Ptr currentLayer = Layers.Current_Layer ();

      numberOfZNodes = currentLayer -> getMaximumNumberOfElements();

      if ( numberOfZNodes > 0 ) {
        size = numberOfXNodes * numberOfYNodes * ( numberOfZNodes + 1 ) * NAES;

        for ( i = 0; i < size; ++i ) {
          currentLayer -> gint [ 0 ][ 0 ][ 0 ][ i ] = 0.0;
        }


      }

      Layers++;
    }
    
  }

}
