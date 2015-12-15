
function [ActiveBatch S_struct] = writecauldronproject3dfile(FM_pop,S_struct,batch_id,Input)





ProjectBasename   = S_struct.BaseName;
%fprintf('WritingCauldronProject3dfiles Test2 - only 6 variables \n')

% Applicable to the Pcal_1D_29_10_2 model of Gareth

% Variables are:
% 1) Onset of mobile layer increase to 10m
% 2) HPHT_shale1 K@5% porosity
% 3) HPHTchalk1 compaction coefficient




Tot_I_NP    = S_struct.Tot_I_NP;   % Total number of experiments generated in the optimisation procedure
Nruns       = S_struct.I_NP;       % Number of in this batch 
Nparameters = S_struct.I_D;        % Number of parameters to tweak/optimize for
%fprintf('Writing Batch = %d, number of runs in this batch = %d, number of parameters to change = %d \n', batch_id ,Nruns, Nparameters);


delimiter = ' '; % space delimiter in cauldron .project3d


for(r=1:Nruns)
    clone_projectfile = Input.MotherProjectfile;
    experimentname    = [Input.LaunchDir ProjectBasename '_' int2str(batch_id) '_' int2str(r) '.project3d'];
    fprintf('New projectfile = %s \n', experimentname); %>>>>>>>>>>>>>change this to SPEED UP
    
    
    if strcmp(Input.CalibrationType,'Pressure')
    %initialise forMulti
    %forMulti = '" ';
    for(v=1:Nparameters)
        type = Input.InversionParameters(v).SingleOrMulti;
        %if(strcmp(type,'Single'))
            Inputp3d(r,v) = FM_pop(r,v);
            fprintf('batch %d, Run %d, setting parameter %d to %f \n',batch_id,r,v,Inputp3d(r,v));
            %ref_projectfile_p = fopen(ref_projectfile,'r');
            varType = Input.InversionParameters(v).MainVariableType;
            multiPoints=  Input.InversionParameters(v).MultiPoints;
            multiPoint2change=  Input.InversionParameters(v).MultiPointToChange;
            varCauldronName = Input.InversionParameters(v).CauldronName;
            varNameInProject3d = Input.InversionParameters(v).LithologyName;
            %new_projectfile_p = fopen(experimentname,'w');%>>>>>>>>>>>>>change this to SPEED UP
            createproject3d(clone_projectfile,experimentname,varType ,varCauldronName, varNameInProject3d, delimiter, type, num2str(Inputp3d(r,v)), multiPoints,multiPoint2change);
            %experimentname_old  =  [ProjectBasename '_' int2str(batch_id) '_' int2str(r) '_' int2str(v) '.project3d'];
            %experimentname_old  =  [ProjectBasename '_' int2str(batch_id) '_' int2str(r) '.project3d' '.OLD'];
            experimentname_old = [ProjectBasename '_.project3d.TMP' ];
            cpcommand = ['cp ' experimentname ' ' experimentname_old];
            system(cpcommand);
            clone_projectfile=experimentname_old;% for first parameter u change the motherfile, then for subsequent parameters u change the previously created experiment file until u fill all parameters.
%         elseif strcmp(type,'Multi')
%              Inputp3d(r,v) = FM_pop(r,v);
%             forMulti = [forMulti  num2str(FM_pop(r,v)) ' '];
%         end
    end
    elseif strcmp(Input.CalibrationType,'Thermal')
        for(v=1:Nparameters)
        
            Inputp3d(r,v) = FM_pop(r,v);
            fprintf('batch %d, Run %d, setting parameter %d to %f \n',batch_id,r,v,Inputp3d(r,v));
            %ref_projectfile_p = fopen(ref_projectfile,'r');
            varType = Input.InversionParameters(v).MainVariableType;
            varCauldronName = Input.InversionParameters(v).CauldronName;
            modifybasementiotbl(clone_projectfile,experimentname, varType,varCauldronName,  Input.ThermalModel, delimiter, num2str(Inputp3d(r,v)));
            experimentname_old = [ProjectBasename '_.project3d.TMP' ];
            cpcommand = ['cp ' experimentname ' ' experimentname_old];
            system(cpcommand);
            clone_projectfile=experimentname_old;% for first parameter u change the motherfile, then for subsequent parameters u change the previously created experiment file until u fill all parameters.
    end
        
        
    end
    
    Tot_I_NP = Tot_I_NP + 1;                  % Increment the counter for total number of runs made
     S_struct.CauldronSample(Tot_I_NP).name =  experimentname;
     ActiveBatch(r).Name                    =  experimentname;
     ActiveBatch(r).JobId                   =  -9999;
     ActiveBatch(r).JobId_string            =  'NoString';
     ActiveBatch(r).TimeElapsed             =  0.0;
     ActiveBatch(r).StartTime                      =  clock;
end



fprintf('Finished WriteCauldronProject3dfiles \n');    
%pause %%>>>>>>>>>>>>>>>>>>>>>>>>>>>>
