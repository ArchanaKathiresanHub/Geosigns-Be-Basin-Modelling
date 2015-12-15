function [S_struct] = returnmisfitstructure(Misfit,k)


%----strategy to put everything into a cost function------------
S_struct.I_nc      = 0;%no constraints
S_struct.FVr_ca    = 0;%no constraint array
S_struct.I_no      = 1;%number of objectives (costs)
S_struct.FVr_oa(1) = Misfit(k);