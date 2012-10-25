function RemoveProject3Dfiles(batch_id,S_struct, Input)


Nruns       = S_struct.I_NP;
Nparameters = S_struct.I_D;
ProjectBasename   = S_struct.BaseName;
fprintf('Removing project3d files of Batch with ID = %d \n', batch_id);

for(r=1:Nruns)
    
    projectname    = [Input.LaunchDir ProjectBasename '_' int2str(batch_id) '_' int2str(r) '.project3d'];
    logfile        = [Input.LaunchDir ProjectBasename '_' int2str(batch_id) '_' int2str(r) '.project3d_drillerlogfile'];
    drillerlogfile = [Input.LaunchDir ProjectBasename '_' int2str(batch_id) '_' int2str(r) '.project3d_logfile'];
    
    fprintf('Removing project3d file %s \n', projectname);
    ProjectRemove = ['rm -r -f  '  projectname];
    system (ProjectRemove);
    drillerlogfileRemove = ['rm -r -f  '  drillerlogfile];
    system (drillerlogfileRemove);
    logfileRemove = ['rm -r -f  '  logfile];
    system (logfileRemove);
    
   
    fprintf('Done...removing project3d and related log files of %s \n', projectname);
    fprintf('   \n');
    pause(2);
end

fprintf('Finished Removing project3d files\n');