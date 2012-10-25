function RemoveCauldronOutputDirectories(batch_id,S_struct,Input)


Nruns            = S_struct.I_NP;
%Nparameters       = S_struct.I_D;
ProjectBasename   = S_struct.BaseName;
fprintf('Emptying output directores of Batch with ID = %d \n', batch_id);

for(r=1:Nruns)
    experimentname    = [Input.LaunchDir ProjectBasename '_' int2str(batch_id) '_' int2str(r)];
    removeh5  = ['rm -r -f  ' experimentname '_CauldronOutputDir/*.h5'];
    removeHDF = ['rm -r -f  ' experimentname '_CauldronOutputDir/*.HDF'];
    
    fprintf('Removing h5 files from %s \n', experimentname);
    system (removeh5);
    pause(5);
    fprintf('Removing HDF files from %s \n', experimentname);
    system(removeHDF);
    pause(5);
    fprintf('   \n');
   
end

fprintf('Finished emptying outputdirectories of Batch with ID = %d\n', batch_id);