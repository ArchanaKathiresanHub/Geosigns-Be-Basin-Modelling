%function rundeoptcauldron(Inputfile)
function rundeoptcauldron(Input)
%********************************************************************
% Script file for the initialization and run of the differential 
% evolution optimizer.
%********************************************************************


% Open the Inputfile and read data to a structure Input
%Input = readinput(Inputfile);


% F_VTR		"Value To Reach" (stop when ofunc < F_VTR)
		%F_VTR = -10; 
        F_VTR = Input.Tolerance;

% I_D		number of parameters of the objective function 
		I_D = Input.NVar; 

% FVr_minbound,FVr_maxbound   vector of lower and bounds of initial population
% the algorithm seems to work especially well if [FVr_minbound,FVr_maxbound] 
% covers the region where the global minimum is expected
% *** note: these are no bound constraints!! ***
     % FVr_minbound = -3*ones(1,I_D); 
     % FVr_maxbound = 3*ones(1,I_D); 
      I_bnd_constr = 1;  
      
% Uncertainty parameters to invert for...
% 
%     1) Timing of cementation / mobile layering
%     2) K@5% HTTP_Shale
%     3) Compaction coefficient Chlk
%
%

% Here is min and max bound for each variable
% array
for(n=1:Input.NVar)
      FVr_minbound(1,n) =  Input.InversionParameters(n).Min; 
      FVr_maxbound(1,n) = Input.InversionParameters(n).Max;    
end

%1: use bounds as bound constraints, 0: no bound constraints      
            
% I_NP            number of population members
		I_NP     = Input.Population;  %pretty high number - needed for demo purposes only
        Tot_I_NP = 0;

% I_itermax       maximum number of iterations (generations)
		I_itermax = Input.Generation; 
       
% F_weight        DE-stepsize F_weight ex [0, 2]
		F_weight = Input.Fweight; 

% F_CR            crossover probabililty constant ex [0, 1]
		F_CR = Input.Fcr; 

% I_strategy     1 --> DE/rand/1:
%                      the classical version of DE.
%                2 --> DE/local-to-best/1:
%                      a version which has been used by quite a number
%                      of scientists. Attempts a balance between robustness
%                      and fast convergence.
%                3 --> DE/best/1 with jitter:
%                      taylored for small population sizes and fast convergence.
%                      Dimensionality should not be too high.
%                4 --> DE/rand/1 with per-vector-dither:
%                      Classical DE with dither to become even more robust.
%                5 --> DE/rand/1 with per-generation-dither:
%                      Classical DE with dither to become even more robust.
%                      Choosing F_weight = 0.3 is a good start here.
%                6 --> DE/rand/1 either-or-algorithm:
%                      Alternates between differential mutation and three-point-
%                      recombination.           

		I_strategy = Input.Istrategy;

% I_refresh     intermediate output will be produced after "I_refresh"
%               iterations. No intermediate output will be produced
%               if I_refresh is < 1
      I_refresh = 1; 
      
% I_plotting    Will use plotting if set to 1. Will skip plotting otherwise.
      I_plotting = 1;
      
%-----Problem dependent constant values for plotting----------------

if (I_plotting == 1)
   FVc_xx = [-3:0.25:3]'; %% >>>>>>>>>>>>>>>>>hard coded range????
   FVc_yy = [-3:0.25:3]';

   [FVr_x,FM_y]=meshgrid(FVc_xx',FVc_yy') ;
   FM_meshd = peaks(FVr_x,FM_y); 

   % creating a struct
   S_struct.FVc_xx    = FVc_xx;
   S_struct.FVc_yy    = FVc_yy;
   S_struct.FM_meshd  = FM_meshd; 
end
% differential evolution parameters
S_struct.I_NP                = I_NP;               % Number of runs in one batch
S_struct.Tot_I_NP            = Tot_I_NP;           % Total number of runs genertaed
S_struct.F_weight            = F_weight;
S_struct.F_CR                = F_CR;
S_struct.I_D                 = I_D;                % Number of variables to invert for
S_struct.FVr_minbound        = FVr_minbound;
S_struct.FVr_maxbound        = FVr_maxbound;
S_struct.I_bnd_constr        = I_bnd_constr;
S_struct.I_itermax           = I_itermax;
S_struct.F_VTR               = F_VTR;
S_struct.I_strategy          = I_strategy;
S_struct.I_refresh           = I_refresh;
S_struct.I_plotting          = I_plotting;
S_struct.batch_winner        = 0;                  % Batch that provided the best run
S_struct.best_fit_k          = 0;                  % best run index
S_struct.BaseName = Input.BaseName;
S_struct.ConvergenceHistory.Bestcost(1) = 1.0; 

% Variable names
for(n=1:Input.NVar)
    S_struct.FVr(n).name = Input.InversionParameters(n).Name;
    S_struct.Variables(n).Initial = Input.InversionParameters(n).Initial;
end



%********************************************************************
% Start of optimization
%********************************************************************

%[FVr_x,S_y,I_nf] = deopt('objfun',S_struct)

%[FVr_x,S_y,I_nf] = deoptcauldronbatch(S_struct,Input);
deoptcauldronbatch(S_struct,Input);
