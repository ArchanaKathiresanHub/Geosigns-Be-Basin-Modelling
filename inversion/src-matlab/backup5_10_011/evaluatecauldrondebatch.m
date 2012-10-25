function [Misfit S_struct] = evaluatecauldrondebatch(FM_pop,S_struct,batch_id,Calibrationdata,Input)


%-----------------------------------------
% EvaluateCauldron_DE_Batch
%
%
%
%                       Quintijn Clevis 
%                         November 2009
%-----------------------------------------


% 1. Generate Cauldron project3d files
[ActiveBatch S_struct] = writecauldronproject3dfile(FM_pop,S_struct,batch_id,Input);

% 2. Launch Cauldron project3d
[ActiveBatch]          = launchcauldrondebatch(Input,batch_id,ActiveBatch,S_struct);

% 3. Check all runs in the Batch for Completion
checkdebatchforcompletion(batch_id,ActiveBatch,S_struct);

% 4. Call the DataMining application several times
[DataMiningIoTbl]     = dataminedebatch(ActiveBatch,S_struct);


% 5. Calculate errors related to evaluated Cauldron projects in the batch
fprintf('Calling evaluateobjectivefunction from evaluatecauldrondebatch \n');

[Misfit S_struct]     = evaluateobjectivefunction(DataMiningIoTbl,S_struct,Calibrationdata,Input,batch_id);
%[Misfit S_struct]     = evaluateobjectivefunction(DataMiningIoTbl,S_struct,Calibrationdata, NCalib,batch_id);

% 6. Genarate a voxet file of the best calibrated model sofar

%Export_and_Draw_GocadVoxet_File(S_struct, Input)

fprintf('Going to remove Cauldron temp files \n');


% 7. Remove the Cauldron Output Directories
if(batch_id > 1)
   RemoveCauldronOutputDirectories(batch_id-1,S_struct, Input);
   RemoveProject3Dfiles(batch_id-1,S_struct, Input);
else
    RemoveCauldronOutputDirectories(batch_id,S_struct, Input);
end

% 8. Remove the Cauldron project3d files - they can be big.


fprintf('Finished EvaluateCauldron_DE_Batch %d \n',batch_id);