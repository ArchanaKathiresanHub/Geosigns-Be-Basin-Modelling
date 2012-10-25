function [ActiveBatch] = launchcauldrondebatch(Input,batch_id,ActiveBatch,S_struct)

S_struct; %% >>>>>comment this line


Nruns       = S_struct.I_NP;
Nparameters = S_struct.I_D;
fprintf('Launching Batch with = %d, number of runs = %d, number of parameters = %d \n', batch_id ,Nruns,Nparameters);

for(r=1:Nruns)
    
    %gempis LOCAL 1 fastcauldron -project <project-name.project1d> -itcoupled 
    fprintf('Launching run %s \n', ActiveBatch(r).Name);
    %LaunchLine = ['gempis Darwin 1 fastcauldron -project '  ActiveBatch(r).Name  ' -itcoupled  > ' ActiveBatch(r).Name '_logfile 2>&1 </dev/null &'];
    LaunchLine = [Input.CauldronLauncher ' ' Input.Cluster ' ' num2str(Input.Processors) ' ' Input.CauldronVersion ' ' 'fastcauldron' ' ' '-project' '  ' ActiveBatch(r).Name ' ' Input.CauldronOptions ' ' '  > ' ActiveBatch(r).Name '_logfile 2>&1 </dev/null &'];
    LaunchLine
    
    system (LaunchLine);
    ActiveBatch(r).StartTime = clock;
    ActiveBatch(r).Job_Id = -9999;
    fprintf('Done...launching run %s \n', ActiveBatch(r).Name);
    fprintf('   \n');
    pause(5);
end

fprintf('Finished launching Batch with = %d, number of runs = %d, number of parameters = %d \n', batch_id ,Nruns,Nparameters);