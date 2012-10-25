function result = parallel_probe( project, adjustParamString, ranges, probes )
% In parallel (on the AMSGDC cluster) it adjust a project, runs cauldron on it, and probes it.
% project: project file name
% adjustParamString:  parameter format string for 'adjust'
% ranges: values with which to call 'adjust' and the adjustParamString
% probes: List of 2 element cell, where each cell is a variable anem (like
%   VrVec2 or Temperature) and a 3D coordinate of the location of the probe


% paths
inversionRootDir='/nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/src/c++/cauldron/applications/inversion_workbench'
environScript = [ inversionRootDir '/src/setenv.sh' ];
adjust = [ inversionRootDir '/obj.Linux64/adjust' ];
probe = [ inversionRootDir '/obj.Linux64/probe' ];
cauldron = '/nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion/src/c++/cauldron/applications/fastcauldron/obj.Linux64/fastcauldron';

% some parameters
mode='itcoupled';
lsfProject='test';
runtimePerLSFJob=2; % minute
parallelism=100;

% Create job name and temporary directory
lsfJobName = sprintf('caldinv.%d', floor( rand() * 10000) );
mkdir( lsfJobName );
cd( lsfJobName );

NTasks = size(ranges, 1); % There are as many tasks as there are rows in 'ranges'
NParams = size(ranges, 2); % The 'adjustParamString' is parameterized with 
NProbes = numel(probes); % Number of variables to measure
NTasksPerLSFJob=floor(NTasks/parallelism);

% Shape the task scripts in the same way as 'ranges'
taskScriptNumber=0;
for i = 1 : NTasks
    
  if mod(i, NTasksPerLSFJob) == 1
    taskScriptNumber = taskScriptNumber + 1;
    
    taskScriptName = sprintf('%s-%d.sh', lsfJobName, taskScriptNumber);
    taskScript = fopen(taskScriptName, 'w');
    taskLog=sprintf('%s-%d.log', lsfJobName, taskScriptNumber);

    % Write script header
    fprintf(taskScript, '#!/bin/bash\n');
    fprintf(taskScript, 'source %s\n', environScript);
  end

  intermProject = sprintf('%s-%d.project3d', lsfJobName, i);
  outputName = sprintf('%s-%d.out', lsfJobName, i);

 
  % Write adjust line
  fprintf(taskScript, '%s --project %s --output %s ', adjust, project, intermProject );
  fprintf(taskScript, '%s\n', sprintf(adjustParamString, ranges(i,:)) );
  
  % write cauldron line
  fprintf(taskScript, '%s -project %s -%s -onlyat 0\n', cauldron,intermProject,mode );
  
  % write write probes
  for j = 1 : NProbes
      probeVariableName = probes{j}{1};
      probeCoordinate = probes{j}{2};
      
      if ( numel(probeCoordinate) ~= 3 )
          error('One of the probe coordinates is not a 3D coordinate\n');
      end
      
      fprintf(taskScript, '%s -project %s -property %s -x %d -y %d -z %d >> %s\n', probe, intermProject, probeVariableName, probeCoordinate, outputName);
  end
  
  
  if mod(i, NTasksPerLSFJob) == 0 || i == NTasks
      fclose( taskScript );

      % submit task
      cmd = sprintf('bsub -P %s -W %d -J ''%s'' -o %s bash %s', lsfProject, runtimePerLSFJob, lsfJobName, taskLog, taskScriptName);
      fprintf('Submitting task %d of %d: %s\n', i, NTasks, cmd);

      if i > 1
          [s,r] = system( [cmd ' &' ]); % Fire and forget.
      else
          [s, r] = system(cmd); % We have to wait for at least one task to be submitted, or else submission of synchronisation job (See below) will fail.
      end

      if s ~= 0 
          fprintf('WARNING: unexecpted exit status %d. Output = %s', s, r);
      end

  end
end

% SYNCHRONIZE
% Phony job submission to wait for all jobs
cmd = sprintf('bsub -P %s -W %d -K -w ''ended(%s)'' -o /dev/null echo DONE', lsfProject, runtimePerLSFJob, lsfJobName);
fprintf('SYNCHRONIZING ... (by executing ''%s'' ...', cmd);
[s,r] = system(cmd);
fprintf('DONE\n');
if s ~= 0 
  fprintf('WARNING: unexecpted exit status %d. Output = %s', s, r);
end


% Read the output from probes into the result matrix
fprintf('READING PROBED DATA\n');
result=zeros(NTasks, NProbes);
for i = 1 : NTasks
    outputName = sprintf('%s-%d.out', lsfJobName, i);
    output=load(outputName);
    
    if size(output, 2) > 1
        fprintf('WARNING: unexpected output format in %s\n', outputName);
    end
    
    result(i, :)=output';
end

cd ..
rmdir(lsfJobName, 's');

end
