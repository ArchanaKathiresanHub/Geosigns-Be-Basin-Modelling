function [cumMisfit S_struct] = evaluateobjectivefunction(DataMiningIoTbl,S_struct, Calibrationdata,Input,batch_id)

%---------------------------------------
% Evaluate_Objective_Function
%
%
%                       Quintijn Clevis
%                       November 2009
%---------------------------------------

Nruns       = S_struct.I_NP;

for(r=1:Nruns)
    for param = 1:Input.NCalibParam
           Input.CalibrationParameters(param).Misfit(r)=0.0;
           %Input.CalibrationParameters(param).MisfitDiff(r)=0.0;
    end
    cumMisfit(r) = 0.0;
end

filename = [Input.OutputDir Input.BaseName '_Error_comparison_' int2str(batch_id)];
fid = fopen(filename,'w');
host = getenv('HOSTNAME');
fprintf(fid,'hostname = %s \n',host);
for(r=1:Nruns)
    count = 0;
    for(param=1:Input.NCalibParam)
        pointsPerParam = Input.CalibrationParameters(param).Points;
        for(d=1:pointsPerParam)
            %fprintf('Comparing properties...>>>> %s , %s \n',Calibrationdata(param).Values(d).PropertyName, Input.CalibrationParameters(param).CalibrationData(d).PropertyName );
            if( strcmp(Calibrationdata(param).Values(d).PropertyName,Input.CalibrationParameters(param).CalibrationData(d).PropertyName) == 1)
                calibPoint = count + d;
                computedValue = DataMiningIoTbl(r,calibPoint).Value;
                valueStr = num2str(computedValue);
                evalOper = Input.CalibrationParameters(param).EvaluateOperator;
                evalStr = [ evalOper '(' valueStr ')'];
                computedValue = eval(evalStr);
                observedValue = Calibrationdata(param).Values(d).Observed;
                valueStr = num2str(observedValue);
                evalOper = Input.CalibrationParameters(param).EvaluateOperator;
                evalStr = [ evalOper '(' valueStr ')'];
                observedValue = eval(evalStr);
                %fprintf('Param %d , Point %d, value %f \n',param, calibPoint, value)
                if(computedValue > -9999)
                     Mf(r) =  evaluatemisfit(observedValue, computedValue, Input.Norm);
                     %Mf(r) =  abs(observedValue- computedValue); 
                     
                    Input.CalibrationParameters(param).Misfit(r)  = Input.CalibrationParameters(param).Misfit(r) + evaluatemisfit(observedValue, computedValue, Input.Norm);
                    %Input.CalibrationParameters(param).Misfit(r)  = Input.CalibrationParameters(param).Misfit(r) + abs(observedValue- computedValue);  % difference
                    %Input.CalibrationParameters(param).Misfit(r)  = Input.CalibrationParameters(param).Misfit(r) + ((observedValue- computedValue)/observedValue)^2;  
                    %Input.CalibrationParameters(param).MisfitDiff(r)  = Input.CalibrationParameters(param).MisfitDiff(r) + (observedValue- computedValue);
                else
                   Mf(r) = -9999;
                    Input.CalibrationParameters(param).Misfit(r) = -9999;
                   
                end
                %Input.CalibrationParameters(param).Misfit(r)  = Input.CalibrationParameters(param).Misfit(r) + abs(observedValue- computedValue);
                fprintf(fid,'Run = %d, PropertyName = %s, CalibPoint = %d, Observed = %g, Computed = %g , Misfit = %g\n',r,DataMiningIoTbl(r,calibPoint).PropertyName,d,observedValue , computedValue, Mf(r));
            end
        end
        count = count+pointsPerParam;
        fprintf(fid,'Run = %d, PropertyName = %s,  TotalMisfit = %g\n',r,DataMiningIoTbl(r,calibPoint).PropertyName, Input.CalibrationParameters(param).Misfit(r));
        %divide by Number of points
        Input.CalibrationParameters(param).Misfit(r) = Input.CalibrationParameters(param).Misfit(r)/d;
        
        fprintf(fid,'Run = %d, PropertyName = %s,  TotalMisfit/n = %g\n',r,DataMiningIoTbl(r,calibPoint).PropertyName, Input.CalibrationParameters(param).Misfit(r));
        
        if(batch_id < 2)
            % Remember to poorest match ever. This is used as scaling
            fprintf(fid,'param %d, S_struct.worstmisft= %g \n',param, S_struct.WorstMisFit(param));
            if( Input.CalibrationParameters(param).Misfit(r) > S_struct.WorstMisFit(param)  &&  Input.CalibrationParameters(param).Misfit(r) > 0.0)
                S_struct.WorstMisFit(param)   = Input.CalibrationParameters(param).Misfit(r);
                fprintf(fid,'param %d,S_struct.worstmisft= %g \n',param,S_struct.WorstMisFit(param));
            end
        end
    end
end %% end of r loop

fprintf('Finished Evaluating the objective function \n');
lowest_starter_misfit = cumMisfit(1);
S_struct.best_initialisation_run = 1;
for(r=1:Nruns)
    for(param=1:Input.NCalibParam)
        fprintf(fid,'run=%d, param=%d,S_struct.worstmisft= %g \n',r, param,S_struct.WorstMisFit(param));
        if(Input.CalibrationParameters(param).Misfit(r) > 0.0)
            Input.CalibrationParameters(param).MisfitScaled(r) =  Input.CalibrationParameters(param).Misfit(r) /  S_struct.WorstMisFit(param) ;
            fprintf(fid,'run=%d, param=%d, misfitscaled= %g \n',r, param,Input.CalibrationParameters(param).MisfitScaled(r));
        else
            Input.CalibrationParameters(param).MisfitScaled(r) = S_struct.WorstMisFit(param) + r;
            fprintf(fid,'run=%d, param=%d, misfitscaled= %g \n',r, param,Input.CalibrationParameters(param).MisfitScaled(r));
        end
        % cimulative misfit
        %cumMisfit(r)       =  cumMisfit(r) + Input.CalibrationParameters(param).Weightage*Input.CalibrationParameters(param).MisfitScaled(r);
         cumMisfit(r)       =  cumMisfit(r) + Input.CalibrationParameters(param).Weightage*Input.CalibrationParameters(param).Misfit(r);
        %fprintf('global cumMisfit>>>>>> %g \n',cumMisfit(r));
        fprintf(fid,'Worst Misfit ever for parameter (%d) is %g \n',param, S_struct.WorstMisFit(param));
        fprintf(fid,'Misfit for run %d for parameter (%d)  is %g \n',r, param, Input.CalibrationParameters(param).Misfit(r));
        fprintf(fid,'Misfit scaled for run %d,for parameter (%d) is %g, MisfitCum = %g \n',r, param,  Input.CalibrationParameters(param).MisfitScaled(r), cumMisfit(r) );
        if(batch_id ==1)
            if(cumMisfit(r) < lowest_starter_misfit)
                lowest_starter_misfit = cumMisfit(r);
                S_struct.best_initialisation_run = r;
            end
        end
    end
end
fclose(fid);
