function checkdebatchforcompletion(batch_id,ActiveBatch,S_struct)

Nruns       = S_struct.I_NP;
Nparameters = S_struct.I_D;

% Move to the active directory, check the content
files = dir;
grootte = size(files);
Nfiles  = grootte(1);
fprintf('Function Check_DE_Batch_for_Completion, there are %d files in this directory \n',Nfiles);
fprintf('Checking log files....  \n');
NChecks = 50;
% Initialize the structure with filesizes
for(r=1:Nruns)
    ActiveBatch(r).Filesize = 0.0;
    ActiveBatch(r).Finished = 0;
end 

% Number of check of the files - make it a 
batch_completed = 0;
while(batch_completed == 0)
    
    pause(5); %%%>>>>>>>>>>>>>>>>remove this
    for(r=1:Nruns)
       pause(5);%%%>>>>>>>>>>>>>>>>remove this
        %fprintf('Checking log file %s \n',ActiveBatch(r).Name );
       [status ActiveBatch] = checklogfileforcompletionstatus(ActiveBatch(r).Name,ActiveBatch,r);
       if(status == 0)        % Not completed yet
          ActiveBatch(r).Finished  = 0;
          ActiveBatch(r).Converged = 0;
       elseif(status == 1)    % Completed and converged
          ActiveBatch(r).Finished  = 1;
          ActiveBatch(r).Converged = 1;
       elseif(status == 2)    % Completed but did not converge, or took too long
          ActiveBatch(r).Finished  = 1;
          ActiveBatch(r).Converged = 0;
       end
    end
    
  Nfinished = 0;
  for(r=1:Nruns)
    if(ActiveBatch(r).Finished == 1)
        Nfinished = Nfinished + 1;
    end   
  end
  if(Nfinished == Nruns) 
    fprintf('All runs in batch %d are finished \n',batch_id );
    batch_completed = 1;
    break
  end
    
end


