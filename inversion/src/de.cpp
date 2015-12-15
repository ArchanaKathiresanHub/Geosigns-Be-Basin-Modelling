#include "de.hh"

void DifferentialEvolution::Run(Cauldron& cauldron, DEControls& de, std::vector<Variable>& varVec,
		 std::vector<CalibrationData>& calVec, FILE *Fp_out)
 {
   #define URN_DEPTH   5   //4 + one index to avoid

  int gi_NP=de.NumberOfPopulations();//population size, Number of population members
  int gi_strategy=de.IStrategy();//choice of strategy, chooses DE-strategy
  int i_genmax=de.NumberOfGenerations(); //maximum number of generations
  int i_refresh=1; //output refresh cycle
  double f_weight=de.FWeight();//weight factor
  double f_cross=de.FCr();//crossing over factor
  int gi_D=de.NumberOfParameters();//number of parameters,  // Dimension of parameter vector
  
  double VTR = de.VTR();
 //population always >= 5
  assert(gi_NP>5);

  //-----Variable declarations-----------------------------------------
  int ia_urn2[URN_DEPTH];
  int ia_urn1[gi_NP];
  int MAXDIM = varVec.size();

  if (gi_D > MAXDIM)
	 {
	   printf("Error! too many parameters\n");
	   return;
	 }
   
  double fa_minbound[MAXDIM];
  double fa_maxbound[MAXDIM];
  //int i_bs_flag=0;//if TRUE: best of parent+child selection, if FALSE: DE standard tournament selection

  //Always with boundary constraints
  //reading bounds
  assert (gi_D==varVec.size());
  for(std::vector<Variable>::size_type i = 0; i != varVec.size(); i++)
	{
	  fa_minbound[i] = varVec[i].Min();
	  fa_maxbound[i] = varVec[i].Max();
	}
  
  //create random population based on constraints
  //int min2 = 1;
  //int max2 = 1000000;
  //int seed = random(min2, max2);
  MTRand *mtrand;//creation of a random processor
  //mtrand = new MTRand (seed);
  // Seed the generator with an array from /dev/urandom if available
  // Otherwise use a hash of time() and clock() values
  mtrand = new MTRand();

  int BatchId = 0;
  //create random population based on constraints
 std::vector<Population> pVec(gi_NP);
 for (int p=0; p<gi_NP; p++)
   {     
	 for(std::vector<Variable>::size_type i = 0; i != varVec.size(); i++)
	   {
		 //double val = varVec[i].Min() + genrand()*( varVec[i].Max() - varVec[i].Min() );
		 //double r1 = genrand();
		 double r2 = mtrand->rand();
		 //std::cout<<"r1="<<r1<<"r2="<<r2<<std::endl;
		 double val = varVec[i].Min() + r2*( varVec[i].Max() - varVec[i].Min() );
		 varVec[i].SetValue(val);
		 //std::cout<<"Population "<<p<<" Var"<<i<<"="<<val<<std::endl;
	   }
	 //assign random parameters to population
	 pVec[p].VariableVector = varVec;
	 pVec[p].BatchId = BatchId;
	 
   }

 //for the first batch evalute objective function

 std::vector<double> objFuncVec =  EvaluateCauldronBatch(pVec, calVec, cauldron);
 for (int p=0; p<gi_NP; p++)
   {
	 for(std::vector<Variable>::size_type i = 0; i != pVec[p].VariableVector.size(); i++)
	   {
		 std::cout<<"Var "<<i<<", value:"<<pVec[p].VariableVector[i].Value()<<std::endl;
	   }
   }

 for(std::vector<double>::size_type i = 0; i != objFuncVec.size(); i++)
   {
	 std::cout<<"Population "<<i<<", Objfn:"<<objFuncVec[i]<<std::endl;
  }


 //mark first one as best
 Population p_best = pVec[0];
 Population p_orig= Population();
 //check for other population to find the best
 for(int i = 1; i<gi_NP; i++)
   {
	 std::cout<<"best cost="<<pVec[i].Cost<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;	 
	 if(TrialWinsOverTarget(pVec[i],p_best))
	   p_best = pVec[i];
   }

 //store current parameter vectors
 std::vector<Population> pCurrentVec(gi_NP);
  for(int i = 0; i<gi_NP; i++)
   {
	   pCurrentVec[i] = pVec[i];
   }

 Population t_best = p_best;
 Population p_tmp = Population();
 std::vector<Population> pNewVec(gi_NP);
 std::vector<Population> pSwapVec(gi_NP);
 
//------Iteration loop------------
 int gi_gen = 0;// generation counter
 assert(gi_strategy == 1 || gi_strategy == 3);
 while ((gi_gen < i_genmax) && (p_best.Cost > VTR))
	 {
	   gi_gen++;
	   BatchId++;
	   //std::cout<<"Generation: "<<gi_gen<<" Batch: "<<BatchId<<std::endl;
	   //----start of loop through ensemble------------------------
	   for (int p=0; p<gi_NP; p++)           
		 {
		   permute(ia_urn2,URN_DEPTH,gi_NP,p,ia_urn1); //Pick 4 random and distinct
		   int i_r1 = ia_urn2[1];                 //population members
		   int i_r2 = ia_urn2[2];
		   int i_r3 = ia_urn2[3];
		   //int i_r4 = ia_urn2[4];

		   // ===Choice of strategy=======================================================
	    switch (gi_strategy)
		  {
		  case 1:
			{
			  // ---classical strategy DE/rand/1/bin---------------------------------			  
			  p_tmp.VariableVector = pCurrentVec[p].VariableVector;
			  int randj = static_cast<int>( mtrand->rand()*gi_D); // random parameter         
			  int kk = 0;
			  do
				{				  
				  // add fluctuation to random target
				  //randomly choose 3 parameter/variable vectors
				  //add difference vector weighted with F to third randmly chosen vector
				  double tmp_val = pVec[i_r1].VariableVector[randj].Value() +
					f_weight*(pVec[i_r2].VariableVector[randj].Value() -pVec[i_r3].VariableVector[randj].Value());

				  //set the noisy vector parameter value
				  p_tmp.VariableVector[randj].SetValue(tmp_val);				  
				  randj = (randj+1)%gi_D;
				  kk++;
				}while(( mtrand->rand() < f_cross) && (kk < gi_D));
			  //when boundary constraint
			  p_orig.VariableVector = pVec[i_r1].VariableVector; 
			  break;
			}
		  case 2:
			{
			 //  // ---DE/local-to-best/1/bin-------------------------------------------
// 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// 			  do {				// add fluctuation to random target 
// 				t_tmpP[j] = t_tmpP[j] + f_weight * (t_bestitP[j] - t_tmpP[j]) + f_weight * 
// 				  (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// 				j = (j + 1) % i_D;
// 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
			  break;
			}
		  case 3:
			{
			  // ---DE/best/1/bin with jitter---------------------------------------
			   p_tmp.VariableVector = pCurrentVec[p].VariableVector;
			   int randj = static_cast<int>( mtrand->rand()*gi_D); // random parameter         
			   int kk = 0;
			   do
				 {
				   
				   // add fluctuation to random target
				   double f_jitter = (0.0001* mtrand->rand()+f_weight);
				   //add difference vector weighted with f to third randomly chosen vector and store in a p_tmp
				   p_tmp.VariableVector[randj].SetValue(t_best.VariableVector[randj].Value() +
														f_jitter*(pVec[i_r1].VariableVector[randj].Value() -pVec[i_r2].VariableVector[randj].Value()));
				   randj = (randj+1)%gi_D;
				   kk++;
				 }while(( mtrand->rand() < f_cross) && (kk < gi_D));
			   //boundary constraint
			   p_orig.VariableVector = p_tmp.VariableVector;
			   break;
		  }
		  case 4:
			{				// // ---DE/rand/1/bin with per-vector-dither----------------------------
// 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// 			  do {				// add fluctuation to random target *
// 				t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + (f_weight + ::unif_rand()*(1.0 - f_weight)) 
// 				  * (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// 				j = (j + 1) % i_D;
// 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
			  break;
			}
		  case 5:
			{
			  // // ---DE/rand/1/bin with per-generation-dither------------------------
// 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// 			  do {				// add fluctuation to random target 
// 				t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + f_dither 
// 				  * (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// 				j = (j + 1) % i_D;
// 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
// 			  
			  break;
			}
		  case 6:
			{				// // ---DE/current-to-p-best/1 (JADE)-----------------------------------
// 			  int i_pbest = sortIndex[static_cast<int>(::unif_rand() * p_NP)]; // select from [0, 1, 2, ..., (pNP-1)] 
// 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// 			  do {				// add fluctuation to random target 
// 				t_tmpP[j] = ta_oldP.at(j,i) + f_weight * (ta_oldP.at(j,i_pbest) - ta_oldP.at(j,i)) + 
// 				  f_weight * (ta_oldP.at(j,ia_urn2[1]) - ta_oldP.at(j,ia_urn2[2]));
// 				j = (j + 1) % i_D;
// 			  } while ((::unif_rand() < f_cross) && (++k < i_D));
			  break;
			}
		  default:
			{
			  // // ---variation to DE/rand/1/bin: either-or-algorithm------------------
// 			  int j = static_cast<int>(::unif_rand() * i_D); 	// random parameter 
// 			  if (::unif_rand() < 0.5) { 	// differential mutation, Pmu = 0.5 
// 				do {			// add fluctuation to random target */
// 				  t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + f_weight * (ta_oldP.at(j,ia_urn2[2]) - ta_oldP.at(j,ia_urn2[3]));
// 				  j = (j + 1) % i_D;
// 				} while ((::unif_rand() < f_cross) && (++k < i_D));
				
// 			  } else
// 				{ 			// recombination with K = 0.5*(F+1) -. F-K-Rule 
// 				  do {			// add fluctuation to random target */
// 					t_tmpP[j] = ta_oldP.at(j,ia_urn2[1]) + 0.5 * (f_weight + 1.0) * 
// 					  (ta_oldP.at(j,ia_urn2[2]) + ta_oldP.at(j,ia_urn2[3]) - 2 * ta_oldP.at(j,ia_urn2[1]));
// 					j = (j + 1) % i_D;
// 				  } while ((::unif_rand() < f_cross) && (++k < i_D));
// 				}
			  break;
			}
		  } // end switch (i_strategy) ...
		//Applying boundary constraints
		//----boundary constraints via random reinitialization-------
		for (int j=0; j<gi_D; j++) 
		  {                  
			//std::cout<<"min and max="<<fa_minbound[j]<<" "<<fa_maxbound[j]<<std::endl;
			if (p_tmp.VariableVector[j].Value() < fa_minbound[j])
			  {
				p_tmp.VariableVector[j].SetValue(fa_minbound[j]+ mtrand->rand()*(p_orig.VariableVector[j].Value() - fa_minbound[j]));
				//std::cout<<"j1: "<<p_tmp.VariableVector[j].Value()<<" - "<<p_orig.VariableVector[j].Value()<<std::endl;
			  }
			if (p_tmp.VariableVector[j].Value() > fa_maxbound[j])
			  {
				p_tmp.VariableVector[j].SetValue(fa_maxbound[j]+ mtrand->rand()*(p_orig.VariableVector[j].Value() - fa_maxbound[j]));
				//std::cout<<"j2: "<<p_tmp.VariableVector[j].Value()<<" - "<<p_orig.VariableVector[j].Value()<<std::endl;
			  }
		  }	   
		//------Trial mutation now in t_tmp-----------------
		p_tmp.BatchId = BatchId;
		pVec[p] = p_tmp;				   
		 }//end for (int p=0; p<gi_NP; p++) // End mutation loop through pop.
	   	   
	   for(int i = 0; i<gi_NP; i++)
		 {
		   std::cout<<"gen="<<gi_gen<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;	 
		 }
	   
	   std::vector<double> tmpObjFuncVec =  EvaluateCauldronBatch(pVec, calVec, cauldron);
	   for(int i = 0; i<gi_NP; i++)
		 {
		   std::cout<<"best cost generation="<<pVec[i].Cost<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;
		 }
	   for(int i = 0; i<gi_NP; i++)
		 {
		   std::cout<<"BatchId="<<pVec[i].BatchId<<", Cost="<<pVec[i].Cost<<std::endl;
		   if (TrialWinsOverTarget(pVec[i],pCurrentVec[i]))
			 {
			   std::cout<<"replace target with mutant"<<std::endl;
			   pNewVec[i]=pVec[i];              // replace target with mutant 
			   if (TrialWinsOverTarget(pVec[i],p_best))// Was this a new minimum? 
				 {                               // if so...
				   std::cout<<"new p_best, cost="<<p_best.Cost<<std::endl;
				   p_best = pVec[i];             // store best member so far  
				 }                               // If mutant fails the test...
			 }                                  // go to next the configuration 
		   else
			 {
			   pNewVec[i]=pCurrentVec[i];              // replace target with old value 
			 }

		   pSwapVec[i] = pCurrentVec[i];
		   pCurrentVec[i] = pNewVec[i];
		   pNewVec[i] = pSwapVec[i];	   
		   t_best = p_best;
		 }
	  

	   //======Output Part=====================================================
	   if (gi_gen%i_refresh == 0) //refresh control
		 {
		   printf("generation=%d, bestcost= %12.6f\n",gi_gen, p_best.Cost);	  
		   //printf("%6d   %12.6f   %12.6f\n",gl_nfeval, p_best.Cost, gt_best.fa_constraint[0]);
		   fprintf(Fp_out,"generation=%d, %12.6f\n",gi_gen, p_best.Cost);
		   std::cout<<"best cost="<<p_best.Cost<<" var="<<p_best.VariableVector[0].Value()<<std::endl;	 
		   fprintf(Fp_out,"generation=%d, cost=%12.6f  var=%12.6f\n",gi_gen, p_best.Cost,p_best.VariableVector[0].Value() );
		 }
	 }//end of while
   
   //fprintf(Fp_out,"\n******** best vector ********\n", i, gt_best.fa_vector[i]);
   for (int i=0; i<gi_D; i++)
   {
	 printf("Variable value= %12.6f\n", p_best.VariableVector[i].Value());	  
	 fprintf(Fp_out,"best_vector[%d]=%12.6f\n", i, p_best.VariableVector[i].Value());
   }
  
   delete mtrand;

 }



// void devol(Cauldron cauldron, DEControls decontrols,
// 		   std::vector<CalibrationData>& calVec,  std::vector<Variable>& varVec,FILE *Fp_out)
// {

// #define URN_DEPTH   5   //4 + one index to avoid



// //-----Initialization of annealing parameters-------------------------

//   double VTR = 1e-2;


  // int gi_NP=decontrols.Population();//population size, Number of population members
//   int gi_strategy=decontrols.IStrategy();//choice of strategy, chooses DE-strategy
//   int i_genmax=decontrols.Generation(); //maximum number of generations
//   int i_refresh=1; //output refresh cycle
//   double f_weight=decontrols.FWeight();//weight factor
//   double f_cross=decontrols.FCr();//crossing over factor
//   int gi_D=decontrols.Parameters();//number of parameters,  // Dimension of parameter vector
//   int gi_genmax = i_genmax;// Maximum number of generations
   


 
  

// //-----Initialize random number generator-----------------------------


//  long gl_nfeval    =  0;              // reset number of function evaluations 
//  int BatchId = 0;
 
//  //int min2 = 1;
//  //int max2 = 1000000;
//  //int seed = random(min2, max2);
//  MTRand *mtrand;//creation of a random processor
//  //mtrand = new MTRand (seed);

//  // Seed the generator with an array from /dev/urandom if available
//  // Otherwise use a hash of time() and clock() values
//  mtrand = new MTRand();
 
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
// 		 std::cout<<"Population "<<p<<" Var"<<i<<"="<<val<<std::endl;
// 	   }
// 	 pVec[p].VariableVector = varVec;
// 	 pVec[p].BatchId = BatchId;
// 	 //pVec[p] = population;
//    }
 
//  //evalute objective function
//  //find obj for each population member
//  std::vector<double> objFuncVec =  EvaluateCauldronBatch(cauldron, decontrols, calVec,  varVec, pVec);

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
//  std::cout<<"first best cost="<<p_best.Cost<<" var="<<p_best.VariableVector[0].Value()<<std::endl;
//  //std::cout<<"first best cost="<<pVec[0].Cost<<" var="<<pVec[0].VariableVector[0].Value()<<std::endl;
//  Population p_orig= Population();
//  for(int i = 1; i<gi_NP; i++)
//    {
// 	 std::cout<<"best cost="<<pVec[i].Cost<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;	 
// 	 if(TrialWinsOverTarget(pVec[i],p_best))
// 	   p_best = pVec[i];
//    }
 
  
 
//  Population t_best = p_best;
//  Population p_old = pVec[0];
//  Population p_new = Population(); 
//  Population p_tmp = Population();
//  Population p_swap = Population();
// //------Iteration loop--------------------------------------------
//    int gi_gen = 0;// generation counter
//    assert(gi_strategy == 1 || gi_strategy == 3);
//    while ((gi_gen < i_genmax) && (p_best.Cost > VTR))
// 	 {
// 	   gi_gen++;
// 	   BatchId++;
// 	   std::cout<<"Generation: "<<gi_gen<<" Batch: "<<BatchId<<std::endl;
// 	   //----computer dithering factor (if needed)-----------------
// 	   //std::cout<<"floatrand="<<genrand()<<std::endl;
// 	   //std::cout<<"mttrand="<<mtrand->rand()<<std::endl;
// 	   f_dither = f_weight +  mtrand->rand()*(1.0 - f_weight);

// 	   //----start of loop through ensemble------------------------
// 	   for (int p=0; p<gi_NP; p++)           
// 		 {
// 		   permute(ia_urn2,URN_DEPTH,gi_NP,p,ia_urn1); //Pick 4 random and distinct
// 		   int i_r1 = ia_urn2[1];                 //population members
// 		   int i_r2 = ia_urn2[2];
// 		   int i_r3 = ia_urn2[3];
// 		   int i_r4 = ia_urn2[4];

// 		   //std::cout<<"r1="<<i_r1<<" r2="<<i_r2<<" r3="<<i_r3<<" r4="<<i_r4<<std::endl;

// 		//---classical strategy DE/rand/1/bin-----------------------------------------
// 		if (gi_strategy == 1)
// 		{
// 		  //assigna2b(gi_D,gpta_old[i].fa_vector,t_tmp.fa_vector);
// 		    p_tmp.VariableVector = p_old.VariableVector;
// 	       int randj = static_cast<int>( mtrand->rand()*gi_D); // random parameter         
// 	       int kk = 0;
// 	       do
// 		   {

// 			 // add fluctuation to random target 
// 			 double tmp_val = pVec[i_r1].VariableVector[randj].Value() + f_weight*(pVec[i_r2].VariableVector[randj].Value() -pVec[i_r3].VariableVector[randj].Value());
// 			  p_tmp.VariableVector[randj].SetValue(tmp_val);

// 	          randj = (randj+1)%gi_D;
// 	          kk++;
// 		   }while(( mtrand->rand() < f_cross) && (kk < gi_D));

		   
// 		   //when boundary constraint
// 		   p_orig.VariableVector = pVec[i_r1].VariableVector; 

// 		}
// 		//---DE/best/1/bin with jitter------------------------------------------------
// 		   if (gi_strategy == 3)
// 			 {
// 			   // assigna2b(gi_D,gpta_old[i].fa_vector,t_tmp.fa_vector);
// 			   p_tmp.VariableVector = p_old.VariableVector;
// 			   int randj = static_cast<int>( mtrand->rand()*gi_D); // random parameter         
// 			   int kk = 0;
// 			   do
// 				 {
// 				   //std::cout<<"randj="<<randj<<" i_r1="<<i_r1<<" i_r2="<<i_r2<<std::endl;
// 				   // add fluctuation to random target
// 				   //wight
// 				   f_jitter = (0.0001* mtrand->rand()+f_weight);
// 				   //add difference vector weighted with f to third randomly chosen vector and store in a p_tmp
// 				   //t_tmp.fa_vector[randj] = t_bestit.fa_vector[randj] + f_jitter*(gpta_old[i_r1].fa_vector[randj]-gpta_old[i_r2].fa_vector[randj]);
// 				   p_tmp.VariableVector[randj].SetValue(t_best.VariableVector[randj].Value() + f_jitter*(pVec[i_r1].VariableVector[randj].Value() -pVec[i_r2].VariableVector[randj].Value()));
// 				   randj = (randj+1)%gi_D;
// 				   kk++;
// 				 }while(( mtrand->rand() < f_cross) && (kk < gi_D));


// 			   //boundary constraint
// 			   //p_orig.VariableVector = t_best.VariableVector;
// 			   p_orig.VariableVector = p_tmp.VariableVector;

// 			 }

// 		   //Applying boundary constraints		   

// 		   for (int j=0; j<gi_D; j++) //----boundary constraints via random reinitialization-------
// 			 {                      //----and bounce back----------------------------------------
// 			   //std::cout<<"min and max="<<fa_minbound[j]<<" "<<fa_maxbound[j]<<std::endl;
// 			   if (p_tmp.VariableVector[j].Value() < fa_minbound[j])
// 				 {
// 				   p_tmp.VariableVector[j].SetValue(fa_minbound[j]+ mtrand->rand()*(p_orig.VariableVector[j].Value() - fa_minbound[j]));
// 				   //std::cout<<"j1: "<<p_tmp.VariableVector[j].Value()<<" - "<<p_orig.VariableVector[j].Value()<<std::endl;
// 				 }
// 			   if (p_tmp.VariableVector[j].Value() > fa_maxbound[j])
// 				 {
// 				   p_tmp.VariableVector[j].SetValue(fa_maxbound[j]+ mtrand->rand()*(p_orig.VariableVector[j].Value() - fa_maxbound[j]));
// 				   //std::cout<<"j2: "<<p_tmp.VariableVector[j].Value()<<" - "<<p_orig.VariableVector[j].Value()<<std::endl;
// 				 }
// 			 }

		   
// 		   //------Trial mutation now in t_tmp-----------------
		   
// 		   //t_tmp = evaluate(gi_D,t_tmp,&gl_nfeval,&gpta_old[0],gi_NP);  // Evaluate mutant in t_tmp[] 		   
// 		   p_tmp.BatchId = BatchId;
// 		   pVec[p] = p_tmp;
		   		   
// 		 }//end for (i=0; i<gi_NP; i++)
// 	   // End mutation loop through pop.

// 	   for(int i = 0; i<gi_NP; i++)
// 		 {
// 		   std::cout<<"gen="<<gi_gen<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;	 
// 		 }

// 	   std::vector<double> tmpObjFuncVec =  EvaluateCauldronBatch(cauldron, decontrols, calVec,  varVec, pVec);
// 	   // if (i_bs_flag == TRUE)
// 		   // 		  {
// 		   // 			gpta_new[i]=t_tmp; //save new vector, selection will come later
// 		   // 		  }
// 		   // 		else
// 		   //{
// 	   for(int i = 0; i<gi_NP; i++)
// 		 {
// 		   std::cout<<"best cost generation="<<pVec[i].Cost<<" var="<<pVec[i].VariableVector[0].Value()<<std::endl;
// 		 }
// 	   for(int i = 0; i<gi_NP; i++)
// 		 {
// 		   std::cout<<"BatchId="<<pVec[i].BatchId<<", Cost="<<pVec[i].Cost<<std::endl;
// 		   if (TrialWinsOverTarget(pVec[i],p_old))
// 			 {
// 			   std::cout<<"replace target with mutant"<<std::endl;
// 			   p_new=pVec[i];              // replace target with mutant 
			   
// 			   if (TrialWinsOverTarget(pVec[i],p_best))// Was this a new minimum? 
// 				 {                               // if so...
// 				   std::cout<<"new p_best, cost="<<p_best.Cost<<std::endl;
// 				   p_best = pVec[i];             // store best member so far  
// 				 }                               // If mutant fails the test...
// 			 }                                  // go to next the configuration 
// 		   else
// 			 {
// 			   p_new=p_old;              // replace target with old value 
// 			 }
		   
// 		 }
// 	   //}//if (i_bs_flag == TRUE)		   
// 	   //  if (i_bs_flag == TRUE)
// 	   // 		 {
// 	   // 		   sort (gpta_old, 2*gi_NP); //sort array of parents + children
// 	   // 		   p_best = p_old;
// 	   // 		 }
// 	   // 	   else
	   
// 	   p_swap = p_old;
// 	   p_old = p_new;
// 	   p_new = p_swap;	   
// 	   t_best = p_best;

// //======Output Part=====================================================

// 	if (gi_gen%i_refresh == 0) //refresh control
// 	{
// 	  printf("generation=%d, bestcost= %12.6f\n",gi_gen, p_best.Cost);	  
// 	  //printf("%6d   %12.6f   %12.6f\n",gl_nfeval, p_best.Cost, gt_best.fa_constraint[0]);
// 	  fprintf(Fp_out,"generation=%d, %12.6f\n",gi_gen, p_best.Cost);
	  
// 	  std::cout<<"best cost="<<p_best.Cost<<" var="<<p_best.VariableVector[0].Value()<<std::endl;	 
// 	  fprintf(Fp_out,"generation=%d, cost=%12.6f  var=%12.6f\n",gi_gen, p_best.Cost,p_best.VariableVector[0].Value() );
	  
	  
// 	}//end if (gi_gen%10 == 0)

//    }//end while ((gi_gen < i_genmax) && (gf_emin > MINI))


//}
 

