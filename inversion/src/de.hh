#ifndef DE_HH
#define DE_HH

#include <iostream>      
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "decontrols.hh"
#include "calibrationdata.hh"
#include "cauldron.hh"
#include "variable.hh"
#include "datamining.hh"
#include "population.hh"
#include "evaluatecauldronbatch.hh"
#include "random.hh"
#include "random.h"  //for genrand
#include "mersennetw.hh"
#include <assert.h>

class DifferentialEvolution
{
 
  
public:
  DifferentialEvolution(){}

  void Run(Cauldron& cauldron, DEControls& dc, std::vector<Variable>& varVec,std::vector<CalibrationData>& calVec, FILE *Fp_out);
    
  ~DifferentialEvolution(){}
};




bool TrialWinsOverTarget(Population trial, Population target)
{

  if(trial.Cost < 0 || target.Cost < 0 )
	{
	  std::cout<<"Eror! COST is negative!"<<std::endl;
	}
  if(trial.Cost <= target.Cost)
	return true;

  return false;
  
}


void permute(int ia_urn2[], int i_urn2_depth, int i_NP, int i_avoid, int ia_urn1[] )
/**C*F****************************************************************
**                                                                  
** Function       :void permute(int ia_urn2[], int i_urn2_depth)                                        
**                                                                  
** Author         :Rainer Storn                                     
**                                                                  
** Description    :Generates i_urn2_depth random indices ex [0, i_NP-1]
**                 which are all distinct. This is done by using a 
**                 permutation algorithm called the "urn algorithm"
**                 which goes back to C.L.Robinson.                
**                                                                  
** Functions      :-                                                
**                                                                  
** Globals        :-
**                                               
** Parameters     :ia_urn2       (O)    array containing the random indices
**                 i_urn2_depth  (I)    number of random indices (avoided index included)
**                 i_NP          (I)    range of indices is [0, i_NP-1]
**                 i_avoid       (I)    is the index to avoid and is located in
**                                      ia_urn2[0].   
**                                                                  
** Preconditions  :# Make sure that ia_urn2[] has a length of i_urn2_depth.
**                 # i_urn2_depth must be smaller than i_NP.                     
**                                                                  
** Postconditions :# the index to be avoided is in ia_urn2[0], so fetch the
**                   indices from ia_urn2[i], i = 1, 2, 3, ..., i_urn2_depth. 
**
** Return Value   :-                                            
**                                                                  
***C*F*E*************************************************************/

{
	int  i, k, i_urn1, i_urn2;
	//int  ia_urn1[MAXPOP] = {0};      //urn holding all indices

	k      = i_NP;
	i_urn1 = 0; 
	i_urn2 = 0;
	for (i=0; i<i_NP; i++) ia_urn1[i] = i; //initialize urn1

	i_urn1 = i_avoid;                  //get rid of the index to be avoided and place it in position 0.
	while (k >= i_NP-i_urn2_depth)     //i_urn2_depth is the amount of indices wanted (must be <= NP) 
	{
	   ia_urn2[i_urn2] = ia_urn1[i_urn1];      //move it into urn2
	   ia_urn1[i_urn1] = ia_urn1[k-1]; //move highest index to fill gap
	   k = k-1;                        //reduce number of accessible indices
	   i_urn2 = i_urn2 + 1;            //next position in urn2
       i_urn1 = static_cast<int>(genrand()*k);    //choose a random index
	}
}


// int left_vector_wins(t_pop t_trial, t_pop t_target)
// /**C*F****************************************************************
// **                                                                  
// ** Function       :int left_vector_wins(t_pop t_trial, t_pop t_target)                                        
// **                                                                  
// ** Author         :Rainer Storn                                     
// **                                                                  
// ** Description    :Selection criterion of DE. Decides when the trial
// **                 vector wins over the target vector.                 
// **                                                                  
// ** Functions      :-                                                
// **                                                                  
// ** Globals        :-                                                
// **                                                                  
// ** Parameters     :t_trial    (I)   trial vector
// **                 t_target   (I)   target vector   
// **                                                                  
// ** Preconditions  :-                     
// **                                                                  
// ** Postconditions :- 
// **
// ** Return Value   :TRUE if trial vector wins, FALSE otherwise.                                            
// **                                                                  
// ***C*F*E*************************************************************/
// {
// 	//---trial wins against target even when cost is equal.-----
// 	if (t_trial.fa_cost[0] <= t_target.fa_cost[0]) return(TRUE);
// 	else return(FALSE);
// }




// void DifferentialEvolution::Run(Cauldron& cauldron, DEControls& de, std::vector<Variable>& varVec,
// 		 std::vector<CalibrationData>& calVec, FILE *Fp_out)
//  {
//    #define URN_DEPTH   5   //4 + one index to avoid

//   int gi_NP=de.NumberOfPopulations();//population size, Number of population members
//   int gi_strategy=de.IStrategy();//choice of strategy, chooses DE-strategy
//   int i_genmax=de.NumberOfGenerations(); //maximum number of generations
//   int i_refresh=1; //output refresh cycle
//   double f_weight=de.FWeight();//weight factor
//   double f_cross=de.FCr();//crossing over factor
//   int gi_D=de.NumberOfParameters();//number of parameters,  // Dimension of parameter vector
  
//   double VTR = de.VTR();
//  //population always >= 5
//   assert(gi_NP>5);

//   //-----Variable declarations-----------------------------------------
//   int ia_urn2[URN_DEPTH];
//   int ia_urn1[gi_NP];
//   int MAXDIM = varVec.size();

//   if (gi_D > MAXDIM)
// 	 {
// 	   printf("Error! too many parameters\n");
// 	   return;
// 	 }
   
//   double fa_minbound[MAXDIM];
//   double fa_maxbound[MAXDIM];
//   //int i_bs_flag=0;//if TRUE: best of parent+child selection, if FALSE: DE standard tournament selection

//   //Always with boundary constraints
//   //reading bounds
//   assert (gi_D==varVec.size());
//   for(std::vector<Variable>::size_type i = 0; i != varVec.size(); i++)
// 	{
// 	  fa_minbound[i] = varVec[i].Min();
// 	  fa_maxbound[i] = varVec[i].Max();
// 	}
  
//   //create random population based on constraints
//   //int min2 = 1;
//   //int max2 = 1000000;
//   //int seed = random(min2, max2);
//   MTRand *mtrand;//creation of a random processor
//   //mtrand = new MTRand (seed);
//   // Seed the generator with an array from /dev/urandom if available
//   // Otherwise use a hash of time() and clock() values
//   mtrand = new MTRand();

//   int BatchId = 0;
//   //create random population based on constraints
//  std::vector<Population> pVec(gi_NP);
//  for (int p=0; p<gi_NP; p++)
//    {     
// 	 for(std::vector<Variable>::size_type i = 0; i != varVec.size(); i++)
// 	   {
// 		 //double val = varVec[i].Min() + genrand()*( varVec[i].Max() - varVec[i].Min() );
// 		 //double r1 = genrand();
// 		 double r2 = mtrand->rand();
// 		 //std::cout<<"r1="<<r1<<"r2="<<r2<<std::endl;
// 		 double val = varVec[i].Min() + r2*( varVec[i].Max() - varVec[i].Min() );
// 		 varVec[i].SetValue(val);
// 		 //std::cout<<"Population "<<p<<" Var"<<i<<"="<<val<<std::endl;
// 	   }
// 	 //assign random parameters to population
// 	 pVec[p].VariableVector = varVec;
// 	 pVec[p].BatchId = BatchId;
	 
//    }

//  //for the first batch evalute objective function

//  std::vector<double> objFuncVec =  EvaluateCauldronBatch(pVec, calVec, cauldron);
//  for (int p=0; p<gi_NP; p++)
//    {
// 	 for(std::vector<Variable>::size_type i = 0; i != pVec[p].VariableVector.size(); i++)
// 	   {
// 		 std::cout<<"Var "<<i<<", value:"<<pVec[p].VariableVector[i].Value()<<std::endl;
// 	   }
//    }

//  for(std::vector<double>::size_type i = 0; i != objFuncVec.size(); i++)
//    {
// 	 std::cout<<"Population "<<i<<", Objfn:"<<objFuncVec[i]<<std::endl;
//   }


//  //mark first one as best
//  Population p_best = pVec[0];
//  Population p_orig= Population();
//  //check for other population to find the best
//  for(int i = 1; i<gi_NP; i++)
//    {
// 	 std::cout<<"best cost="<<pVec[i].Cost<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;	 
// 	 if(TrialWinsOverTarget(pVec[i],p_best))
// 	   p_best = pVec[i];
//    }

//  //store current parameter vectors
//  std::vector<Population> pCurrentVec(gi_NP);
//   for(int i = 0; i<gi_NP; i++)
//    {
// 	   pCurrentVec[i] = pVec[i];
//    }

//  Population t_best = p_best;
//  Population p_tmp = Population();
//  std::vector<Population> pNewVec(gi_NP);
//  std::vector<Population> pSwapVec(gi_NP);
 
// //------Iteration loop------------
//  int gi_gen = 0;// generation counter
//  assert(gi_strategy == 1 || gi_strategy == 3);
//  while ((gi_gen < i_genmax) && (p_best.Cost > VTR))
// 	 {
// 	   gi_gen++;
// 	   BatchId++;
// 	   //std::cout<<"Generation: "<<gi_gen<<" Batch: "<<BatchId<<std::endl;
// 	   //----start of loop through ensemble------------------------
// 	   for (int p=0; p<gi_NP; p++)           
// 		 {
// 		   permute(ia_urn2,URN_DEPTH,gi_NP,p,ia_urn1); //Pick 4 random and distinct
// 		   int i_r1 = ia_urn2[1];                 //population members
// 		   int i_r2 = ia_urn2[2];
// 		   int i_r3 = ia_urn2[3];
// 		   //int i_r4 = ia_urn2[4];

// 		   // ===Choice of strategy=======================================================
// 	    switch (gi_strategy)
// 		  {
// 		  case 1:
// 			{
// 			  // ---classical strategy DE/rand/1/bin---------------------------------			  
// 			  p_tmp.VariableVector = pCurrentVec[p].VariableVector;
// 			  int randj = static_cast<int>( mtrand->rand()*gi_D); // random parameter         
// 			  int kk = 0;
// 			  do
// 				{				  
// 				  // add fluctuation to random target
// 				  //randomly choose 3 parameter/variable vectors
// 				  //add difference vector weighted with F to third randmly chosen vector
// 				  double tmp_val = pVec[i_r1].VariableVector[randj].Value() +
// 					f_weight*(pVec[i_r2].VariableVector[randj].Value() -pVec[i_r3].VariableVector[randj].Value());

// 				  //set the noisy vector parameter value
// 				  p_tmp.VariableVector[randj].SetValue(tmp_val);				  
// 				  randj = (randj+1)%gi_D;
// 				  kk++;
// 				}while(( mtrand->rand() < f_cross) && (kk < gi_D));
// 			  //when boundary constraint
// 			  p_orig.VariableVector = pVec[i_r1].VariableVector; 
// 			  break;
// 			}
// 		  case 2:
// 			{
// 			 //  // ---DE/local-to-best/1/bin-------------------------------------------
// // 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// // 			  do {				// add fluctuation to random target 
// // 				t_tmpP[j] = t_tmpP[j] + f_weight * (t_bestitP[j] - t_tmpP[j]) + f_weight * 
// // 				  (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// // 				j = (j + 1) % i_D;
// // 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
// 			  break;
// 			}
// 		  case 3:
// 			{
// 			  // ---DE/best/1/bin with jitter---------------------------------------
// 			   p_tmp.VariableVector = pCurrentVec[p].VariableVector;
// 			   int randj = static_cast<int>( mtrand->rand()*gi_D); // random parameter         
// 			   int kk = 0;
// 			   do
// 				 {
				   
// 				   // add fluctuation to random target
// 				   double f_jitter = (0.0001* mtrand->rand()+f_weight);
// 				   //add difference vector weighted with f to third randomly chosen vector and store in a p_tmp
// 				   p_tmp.VariableVector[randj].SetValue(t_best.VariableVector[randj].Value() +
// 														f_jitter*(pVec[i_r1].VariableVector[randj].Value() -pVec[i_r2].VariableVector[randj].Value()));
// 				   randj = (randj+1)%gi_D;
// 				   kk++;
// 				 }while(( mtrand->rand() < f_cross) && (kk < gi_D));
// 			   //boundary constraint
// 			   p_orig.VariableVector = p_tmp.VariableVector;
// 			   break;
// 		  }
// 		  case 4:
// 			{				// // ---DE/rand/1/bin with per-vector-dither----------------------------
// // 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// // 			  do {				// add fluctuation to random target *
// // 				t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + (f_weight + ::unif_rand()*(1.0 - f_weight)) 
// // 				  * (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// // 				j = (j + 1) % i_D;
// // 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
// 			  break;
// 			}
// 		  case 5:
// 			{
// 			  // // ---DE/rand/1/bin with per-generation-dither------------------------
// // 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// // 			  do {				// add fluctuation to random target 
// // 				t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + f_dither 
// // 				  * (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// // 				j = (j + 1) % i_D;
// // 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
// // 			  
// 			  break;
// 			}
// 		  case 6:
// 			{				// // ---DE/current-to-p-best/1 (JADE)-----------------------------------
// // 			  int i_pbest = sortIndex[static_cast<int>(::unif_rand() * p_NP)]; // select from [0, 1, 2, ..., (pNP-1)] 
// // 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// // 			  do {				// add fluctuation to random target 
// // 				t_tmpP[j] = ta_oldP.at(j,i) + f_weight * (ta_oldP.at(j,i_pbest) - ta_oldP.at(j,i)) + 
// // 				  f_weight * (ta_oldP.at(j,ia_urn2[1]) - ta_oldP.at(j,ia_urn2[2]));
// // 				j = (j + 1) % i_D;
// // 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
// 			  break;
// 			}
// 		  default:
// 			{
// 			  // // ---variation to DE/rand/1/bin: either-or-algorithm------------------
// // 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// // 			  if (::unif_rand() < 0.5) { 	// differential mutation, Pmu = 0.5 
// // 				do {			// add fluctuation to random target */
// // 				  t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + f_weight * (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// // 				  j = (j + 1) % i_D;
// // 				} while ((::unif_rand() < f_cross) && (++k < i_D));
				
// // 			  } else
// // 				{ 			// recombination with K = 0.5*(F+1) -. F-K-Rule 
// // 				  do {			// add fluctuation to random target */
// // 					t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + 0.5 * (f_weight + 1.0) * 
// // 					  (ta_oldP.at(j,ia_urn2[2]) + ta_oldP.at(j,ia_urn2[3]) - 2 * ta_oldP.at(j,ia_urn2[1]));
// // 					j = (j + 1) % i_D;
// // 				  } while ((::unif_rand() < f_cross) && (++k < i_D));
// // 				}
// 			  break;
// 			}
// 		  } // end switch (i_strategy) ...
// 		//Applying boundary constraints
// 		//----boundary constraints via random reinitialization-------
// 		for (int j=0; j<gi_D; j++) 
// 		  {                  
// 			//std::cout<<"min and max="<<fa_minbound[j]<<" "<<fa_maxbound[j]<<std::endl;
// 			if (p_tmp.VariableVector[j].Value() < fa_minbound[j])
// 			  {
// 				p_tmp.VariableVector[j].SetValue(fa_minbound[j]+ mtrand->rand()*(p_orig.VariableVector[j].Value() - fa_minbound[j]));
// 				//std::cout<<"j1: "<<p_tmp.VariableVector[j].Value()<<" - "<<p_orig.VariableVector[j].Value()<<std::endl;
// 			  }
// 			if (p_tmp.VariableVector[j].Value() > fa_maxbound[j])
// 			  {
// 				p_tmp.VariableVector[j].SetValue(fa_maxbound[j]+ mtrand->rand()*(p_orig.VariableVector[j].Value() - fa_maxbound[j]));
// 				//std::cout<<"j2: "<<p_tmp.VariableVector[j].Value()<<" - "<<p_orig.VariableVector[j].Value()<<std::endl;
// 			  }
// 		  }	   
// 		//------Trial mutation now in t_tmp-----------------
// 		p_tmp.BatchId = BatchId;
// 		pVec[p] = p_tmp;				   
// 		 }//end for (int p=0; p<gi_NP; p++) // End mutation loop through pop.
	   	   
// 	   for(int i = 0; i<gi_NP; i++)
// 		 {
// 		   std::cout<<"gen="<<gi_gen<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;	 
// 		 }
	   
// 	   std::vector<double> tmpObjFuncVec =  EvaluateCauldronBatch(pVec, calVec, cauldron);
// 	   for(int i = 0; i<gi_NP; i++)
// 		 {
// 		   std::cout<<"best cost generation="<<pVec[i].Cost<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;
// 		 }
// 	   for(int i = 0; i<gi_NP; i++)
// 		 {
// 		   std::cout<<"BatchId="<<pVec[i].BatchId<<", Cost="<<pVec[i].Cost<<std::endl;
// 		   if (TrialWinsOverTarget(pVec[i],pCurrentVec[i]))
// 			 {
// 			   std::cout<<"replace target with mutant"<<std::endl;
// 			   pNewVec[i]=pVec[i];              // replace target with mutant 
// 			   if (TrialWinsOverTarget(pVec[i],p_best))// Was this a new minimum? 
// 				 {                               // if so...
// 				   std::cout<<"new p_best, cost="<<p_best.Cost<<std::endl;
// 				   p_best = pVec[i];             // store best member so far  
// 				 }                               // If mutant fails the test...
// 			 }                                  // go to next the configuration 
// 		   else
// 			 {
// 			   pNewVec[i]=pCurrentVec[i];              // replace target with old value 
// 			 }

// 		   pSwapVec[i] = pCurrentVec[i];
// 		   pCurrentVec[i] = pNewVec[i];
// 		   pNewVec[i] = pSwapVec[i];	   
// 		   t_best = p_best;
// 		 }
	  

// 	   //======Output Part=====================================================
// 	   if (gi_gen%i_refresh == 0) //refresh control
// 		 {
// 		   printf("generation=%d, bestcost= %12.6f\n",gi_gen, p_best.Cost);	  
// 		   //printf("%6d   %12.6f   %12.6f\n",gl_nfeval, p_best.Cost, gt_best.fa_constraint[0]);
// 		   fprintf(Fp_out,"generation=%d, %12.6f\n",gi_gen, p_best.Cost);
// 		   std::cout<<"best cost="<<p_best.Cost<<" var="<<p_best.VariableVector[0].Value()<<std::endl;	 
// 		   fprintf(Fp_out,"generation=%d, cost=%12.6f  var=%12.6f\n",gi_gen, p_best.Cost,p_best.VariableVector[0].Value() );
// 		 }
// 	 }//end of while
   
//    //fprintf(Fp_out,"\n******** best vector ********\n", i, gt_best.fa_vector[i]);
//    for (int i=0; i<gi_D; i++)
//    {
// 	 printf("Variable value= %12.6f\n", p_best.VariableVector[i].Value());	  
// 	 fprintf(Fp_out,"best_vector[%d]=%12.6f\n", i, p_best.VariableVector[i].Value());
//    }
  
//    delete mtrand;

//  }


#include "de.cpp"


#endif
