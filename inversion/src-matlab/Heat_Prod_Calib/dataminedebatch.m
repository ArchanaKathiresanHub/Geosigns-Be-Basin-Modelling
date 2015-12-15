function [DataMiningIoTbl] = dataminedebatch(ActiveBatch,S_struct)

Nruns       = S_struct.I_NP;
%Nparameters = S_struct.I_D;


% for each run do the datadriller      
for(r=1:Nruns)
    %pause(5); %%%% >>>>>>>>>>>>>>>>>>>>>remove this
    fprintf('Launching Datamine application on run %s \n', ActiveBatch(r).Name);
    % reading and writing to same file???
    % check datadriller to see what it does
    LaunchLine = ['datadriller -input ' ActiveBatch(r).Name ' -output ' ActiveBatch(r).Name ' > ' ActiveBatch(r).Name '_drillerlogfile 2>&1 </dev/null &'];   
    LaunchLine
    system(LaunchLine);
    fprintf('Finished datamining application using datadriller on run %s \n',ActiveBatch(r).Name);
end

% Delete the output directories here?

for(r=1:Nruns)
     pause(5); %%%% >>>>>>>>>>>>>>>>>>>>>remove this
     % each run get the table
     [DataMiningIoTbl_run]   = readcauldrondataminingiotbl(ActiveBatch(r).Name);
     N_entries = length(DataMiningIoTbl_run); % total calib points
     % Copy the data into a (r,datapoint) indexed structure
     for(d=1:N_entries)
         %rand('state', sum(100*clock));
         DataMiningIoTbl(r,d).BPAKey        = DataMiningIoTbl_run(d).BPAKey; 
         DataMiningIoTbl(r,d).Time          = DataMiningIoTbl_run(d).Time;
         DataMiningIoTbl(r,d).XCoord        = DataMiningIoTbl_run(d).XCoord;
         DataMiningIoTbl(r,d).YCoord        = DataMiningIoTbl_run(d).YCoord;
         DataMiningIoTbl(r,d).ZCoord        = DataMiningIoTbl_run(d).ZCoord;
         DataMiningIoTbl(r,d).ReservoirName = DataMiningIoTbl_run(d).ReservoirName;
         DataMiningIoTbl(r,d).FormationName = DataMiningIoTbl_run(d).FormationName;
         DataMiningIoTbl(r,d).SurfaceName   = DataMiningIoTbl_run(d).SurfaceName;
         DataMiningIoTbl(r,d).PropertyName  = DataMiningIoTbl_run(d).PropertyName;
         DataMiningIoTbl(r,d).PropertyUnit  = DataMiningIoTbl_run(d).PropertyUnit;
         DataMiningIoTbl(r,d).Value         = DataMiningIoTbl_run(d).Value; %+ (DataMiningIoTbl_run(d).Value * rand(1,1))/100;
     end
     
     
end

fprintf('Finished Datamine_DE_Batch \n');


