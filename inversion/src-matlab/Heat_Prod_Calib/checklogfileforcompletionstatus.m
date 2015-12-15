function [status ActiveBatch] = checklogfileforcompletionstatus(runname,ActiveBatch,r)

% Check run for completion by reading the log file
% searching for strings

status    = 0;
completed = 0;
converged = 1;


logfile = [runname '_logfile'];
    
fid = fopen(logfile,'r');
if(fid == -1)
   fprintf('Could not open log file %s \n',logfile);
   completed = 0;
   
end

completed_string        = ['STATE COMPLETE'];
non_convergence_string  = ['MeSsAgE WARNING Maximum number of geometric iterations, 10, exceeded and geometry has not converged'];
Job_Id_string           = ['submitted to default queue <untouchable>'];


while ( feof(fid) == 0 && completed == 0) 
    activeline = fgetl(fid);
    cellArray = tokenize(activeline, '" "');
     if( isempty(cellArray)==false && strcmp(cellArray{1},'Could'))
          error('Error, gempis error.. Could not detect a working head node in ');
     end
    if( strfind( activeline, non_convergence_string) )
       converged = 0; 
    end
    
    if( strfind( activeline, completed_string) )
       completed = 1; 
    end
    
    if( strfind( activeline, Job_Id_string) )
        Job_Id = findjobid(activeline);
       fprintf('Job Id string for this run %s is %d \n',runname, Job_Id);
       if(ActiveBatch(r).Job_Id == -9999)
           ActiveBatch(r).Job_Id = Job_Id;
           fprintf('Assigning run %s - Job ID %d \n', ActiveBatch(r).Name, Job_Id);
           t1 = ActiveBatch(r).StartTime;
           timeelapsed  = etime(clock,t1);
           fprintf('Timelapsed between launch and rename = %f \n',timeelapsed);
       end 
       if(Job_Id == ActiveBatch(r).Job_Id)
           t1 = ActiveBatch(r).StartTime;
           timeelapsed  = etime(clock,t1);
           if(timeelapsed > 1000)       % It took more than 2 hours to run, kill
              converged = 0;
              fprintf('Run %s took %f seconds, killing run \n',ActiveBatch(r).Name,timeelapsed);
              %%%>>>>>>>>>>>>> change here
              LaunchLine = ['gempis res Darwin -v2009.06 kill ' num2str(ActiveBatch(r).Job_Id) ' > ' ActiveBatch(r).Name '_JOBKILL_logfile 2>&1 </dev/null &'];
              system(LaunchLine);
           end
       end
       
       
      
    end
    
end

% Return what the status is, completed and converged?
if(completed == 1 && converged == 1)
    status = 1;
elseif(completed == 1 && converged == 0)
    status = 2;
elseif(completed == 0)
    status = 0;
end


fclose(fid);
