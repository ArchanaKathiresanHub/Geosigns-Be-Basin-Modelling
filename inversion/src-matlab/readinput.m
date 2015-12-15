%> @brief Brief description of the function
%>
%> More detailed description.
%>
%> @param Inputfile the keyword formatted input file
%>
%> @retval Input a matlab structure containing various fields for storing input informations


function [Input] = readinput(Inputfile)

% Open the inputfile
fid = fopen(Inputfile,'r');
if(fid == -1)
    fprintf(' Cannot read the inputfile \n');
end
format long e;
while ~feof(fid)
    line = fgetl(fid);
    if isempty(line) || ~ischar(line), continue, end
    if( strcmp(line,'STOP')), break,end %stop reading when end reached
    % skip comment lines
    cellArray = tokenize(line, '" "');
    if(  isempty(cellArray)==false && strcmp(cellArray{1},'#'))
        continue;
    end
    
    % Reading input file blocks
    
    %% Control parameters
    if( strcmp(line,'BEGIN DECONTROLS'))
        while ~strcmp(line,'END DECONTROLS')
            line  = readline(fid);
            %line = fgetl(fid);
            %line = skipemptylines(fid,line);%brefore reading, skip if any empty lines
            cellArray = tokenize(line, '" "');
            if(isempty(cellArray))
                continue;
            end
            if size(cellArray)~=2
                fprintf('Error, one of the control parameter is NaN \n');
            end
            if( strcmp(cellArray{1},'POPULATION'))
                Input.Population  = str2double(cellArray{2});
                continue;
            end
            if( strcmp(cellArray{1},'GENERATION'))
                Input.Generation = str2double(cellArray{2});
                continue;
            end
            if( strcmp(cellArray{1},'FWEIGHT'))
                Input.Fweight    = str2double(cellArray{2});
                continue;
            end
            if( strcmp(cellArray{1},'FCR'))
                Input.Fcr = str2double(cellArray{2});
                continue;
            end
            if( strcmp(cellArray{1},'ISTRATEGY'))
                Input.Istrategy = str2double(cellArray{2});
                continue;
            end
        end
        %Check the differential evolution control parameters
        fprintf('Population = %d \n', Input.Population);
        fprintf('Generation = %d \n', Input.Generation );
        fprintf('Fweight = %f \n', Input.Fweight);
        fprintf('Fcr = %f \n', Input.Fcr);
        fprintf('Istrategy = %d \n', Input.Istrategy);
    end
    
    
    %% Project3d files, path
    if( strcmp(line,'BEGIN PROJECT3D'))
        while ~strcmp(line,'END PROJECT3D')
            line  = readline(fid);
            cellArray = tokenize(line, '" "');
            if(isempty(cellArray))
                continue;
            end
             if( strcmp(cellArray{1},'BASENAME'))
                Input.BaseName  = cellArray{2};
                continue;
            elseif( strcmp(cellArray{1},'LAUNCHDIR'))
                Input.LaunchDir  = cellArray{2};
            elseif( strcmp(cellArray{1},'PATH'))
                Input.Path  = cellArray{2};
                continue;
            elseif( strcmp(cellArray{1},'REFERENCE'))
                Input.ReferenceProjectfile    = cellArray{2};
                continue;
            elseif( strcmp(cellArray{1},'MOTHER'))
                Input.MotherProjectfile    = cellArray{2};
                continue;
            elseif( strcmp(cellArray{1},'BASE'))
                Input.ProjectfileBasename    = cellArray{2};
                continue;
            elseif( strcmp(cellArray{1},'OUTPUTDIR'))
                Input.OutputDir = cellArray{2};
                diaryFile = [Input.OutputDir Input.BaseName '.MatDiaryLog'];
                diary(diaryFile);
                continue;
            end
        end
        %Check the path, projectfile etc
        fprintf('path = %s \n', Input.Path);
        fprintf('referenceProjectfile = %s \n', Input.ReferenceProjectfile );
        fprintf('motherProjectfile = %s \n', Input.MotherProjectfile);
        fprintf('ProjectfileBasename = %s \n', Input.ProjectfileBasename);
    end
    %% Cauldron run options
    if( strcmp(line,'BEGIN CAULDRON'))
        while ~strcmp(line,'END CAULDRON')
            line  = readline(fid);
            cellArray = tokenize(line, '" "');
            if(isempty(cellArray))
                continue;
            end
            if( strcmp(cellArray{1},'LAUNCHER'))
                Input.CauldronLauncher   = cellArray{2};
                continue;
            end
            if( strcmp(cellArray{1},'COMMAND'))
                Input.CauldronCommand   = cellArray{2};
                continue;
            end
            if( strcmp(cellArray{1},'OPTION'))
                Input.CauldronOption   = cellArray{2};
                continue;
            end
            if( strcmp(cellArray{1},'CLUSTER'))
                Input.Cluster   = cellArray{2};
                continue;
            end
            if( strcmp(cellArray{1},'PROCESSORS'))
                Input.Processors   = str2double(cellArray{2});
                continue;
            end
            if( strcmp(cellArray{1},'CAULDRONMODE'))
                Input.CauldronMode = cellArray{2};
                continue;
            end
        end
        fprintf('Cauldron Launcher = %s, Cauldron Command = %s, Cauldron Option = %s, Cauldron Mode = %s,  Cluster = %s, Processors per run = %d \n',Input.CauldronLauncher,Input.CauldronCommand, Input.CauldronOption, Input.CauldronMode, Input.Cluster, Input.Processors);
    end
    
    %% Variables
    if( strcmp(line,'BEGIN VARIABLES'))
        totalVar = 0;
        while ~strcmp(line,'END VARIABLES')
            line  = readline(fid);
            cellArray = tokenize(line, '" "');
            if(isempty(cellArray))
                continue;
            end
            if( strcmp(cellArray{1},'VARIABLETYPE'))
                Input.MainVariableType  = cellArray{2};
                Input.MainVariableSize = str2double(cellArray{3});
                fprintf('VariableType = %s VariableSize =%d \n',Input.MainVariableType,Input.MainVariableSize);
                for s = 1:Input.MainVariableSize
                    line  = readline(fid);
                    cellArray = tokenize(line, '" "');
                    if( strcmp(cellArray{1},'VARIABLETYPENAME'))
                        Input.VariableType(s).Name = cellArray{2};
                        Input.VariableType(s).Size =  str2double(cellArray{3});
                        fprintf('Name of Variable type = %s \n',Input.VariableType(s).Name);
                        fprintf('Number of variables per type for inversion = %d \n',Input.VariableType(s).Size);
                        for n = 1:Input.VariableType(s).Size
                            line  = readline(fid);
                            cellArray = tokenize(line, '" "');
                            %Input.Var(n).TypeName = cellArray{1}; % TypeName as in .project3d file
                            
                            Input.InversionParameters(totalVar+n).CauldronName = cellArray{1};
                            Input.InversionParameters(totalVar+n).Name = cellArray{2};
                            Input.InversionParameters(totalVar+n).MainVariableType=Input.MainVariableType;
                            Input.InversionParameters(totalVar+n).Project3dVariableName=Input.VariableType(s).Name;
                            Input.InversionParameters(totalVar+n).Min = str2double(cellArray{3});
                            Input.InversionParameters(totalVar+n).Max = str2double(cellArray{4});
                            Input.InversionParameters(totalVar+n).Initial = str2double(cellArray{5});
                            Input.InversionParameters(totalVar+n).SingleOrMulti = cellArray{6};
                            if strcmp(Input.InversionParameters(totalVar+n).SingleOrMulti,'Multi')
                                 Input.InversionParameters(totalVar+n).MultiPoints = str2double(cellArray{7});
                                 Input.InversionParameters(totalVar+n).MultiPointToChange = str2double(cellArray{8});
                            else
                                 Input.InversionParameters(totalVar+n).MultiPoints = -9999;
                                 Input.InversionParameters(totalVar+n).MultiPointToChange = -9999;
                            end
                            
                            %Input.VariableType(s).SubVariable(n).CauldronName = cellArray{1}; % TypeName as in .project3d file
                            %Input.VariableType(s).SubVariable(n).SingleOrMulti = cellArray{2}; % TypeName as in .project3d file
                            %Input.VariableType(s).SubVariable(n).Name = cellArray{3};
                            %Input.VariableType(s).SubVariable(n).Min = str2double(cellArray{4});
                            %Input.VariableType(s).SubVariable(n).Max = str2double(cellArray{5});
                            %Input.VariableType(s).SubVariable(n).Initial = str2double(cellArray{6});
                            
                            %Input.Var(n).CauldronName = cellArray{1}; % TypeName as in .project3d file
                            %Input.Var(n).Name = cellArray{3};
                            %Input.Var(n).Min = str2double(cellArray{4});
                            %Input.Var(n).Max = str2double(cellArray{5});
                            %Input.Var(n).Initial = str2double(cellArray{6});
                            %fprintf('SubVariable = %s min =%f, max = %f  cauldronName=%s \n', Input.VariableType(s).SubVariable(n).Name, Input.VariableType(s).SubVariable(n).Min, Input.VariableType(s).SubVariable(n).Max, Input.VariableType(s).SubVariable(n).CauldronName );
                            fprintf('Variable = %s min =%f, max = %f  cauldronName=%s \n',Input.InversionParameters(totalVar+n).Name,Input.InversionParameters(totalVar+n).Min,Input.InversionParameters(totalVar+n).Max, Input.InversionParameters(totalVar+n).CauldronName );
                        end
                        totalVar= totalVar + Input.VariableType(s).Size;
                    end
                end
            end
            
        end
        Input.NVar = totalVar;
        fprintf('Total number of variables for inversion = %d \n',Input.NVar);
        for v = 1:Input.NVar
            fprintf('Inversion Parameters: name= %s,  CauldName= %s, MainVarType= %s, SubVarType = %s \n', Input.InversionParameters(v).Name,Input.InversionParameters(v).CauldronName, Input.InversionParameters(v).MainVariableType,  Input.InversionParameters(v).SubVariableName  );
        end
    end
    
    
    %% Calibration parameters
    if( strcmp(line,'BEGIN CALIBRATION'))
        calibPoints = 0.0;
        param = 0;
        while ~strcmp(line,'END CALIBRATION')
            line  = readline(fid);
            cellArray = tokenize(line, '" "');
            if( strcmp(cellArray{1},'NUMBEROFPARAMETERS'))
                Input.NCalibParam  = str2double(cellArray{2});
                fprintf('Number of calibration Parameters = %d \n',Input.NCalibParam);
                continue;
            end
            if( strcmp(cellArray{1},'PARAMETER'))
                param = param+1;
                parameter = cellArray{2};
                Input.CalibrationParameters(param).PropertyName=parameter;
                parameterPoints = cellArray{3};
                Input.CalibrationParameters(param).Points = str2double(parameterPoints);
                continue;
            end
            if( strcmp(cellArray{1},'PLOTOPERATOR'))
                Input.CalibrationParameters(param).PlotOperator=cellArray{2};% plot operator in string format
                continue;
            end
            if( strcmp(cellArray{1},'EVALUATEOPERATOR'))
                Input.CalibrationParameters(param).EvaluateOperator=cellArray{2};% plot operator in string format
                continue;
            end
            
            if( strcmp(cellArray{1},'WEIGHTAGE'))
                weightage = str2double(cellArray{2});
                Input.CalibrationParameters(param).Weightage=weightage;
                fprintf('Calibration parameter= %s, Number of data points = %d, weight = %d \n', Input.CalibrationParameters(param).PropertyName,Input.CalibrationParameters(param).Points, Input.CalibrationParameters(param).Weightage );
                outputfile = [Input.OutputDir Input.BaseName '_Param_' int2str(param) '_Calib.dat'];
                fcalib = fopen(outputfile,'w');
                %dlmwrite(outputfilenm, results, 'delimiter', '\t','precision', 6);
                for n = 1:Input.CalibrationParameters(param).Points
                    line  = readline(fid);
                    cellArray = tokenize(line, '" "');
                    Input.CalibrationParameters(param).CalibrationData(n).PropertyName  = parameter; % parameter name
                    Input.CalibrationParameters(param).CalibrationData(n).X     = str2double(cellArray{1});
                    Input.CalibrationParameters(param).CalibrationData(n).Y     = str2double(cellArray{2});
                    Input.CalibrationParameters(param).CalibrationData(n).Z     = str2double(cellArray{3});
                    Input.CalibrationParameters(param).CalibrationData(n).Value = str2double(cellArray{4});
                    Input.CalibrationParameters(param).CalibrationData(n).Wellindex = str2double(cellArray{5});
                    fprintf('n=%d, typename = %s, wellindex =%d, value = %g \n',n,Input.CalibrationParameters(param).CalibrationData(n).PropertyName,Input.CalibrationParameters(param).CalibrationData(n).Wellindex, Input.CalibrationParameters(param).CalibrationData(n).Value );
                    fprintf(fcalib,'%g\t %g\t %g\t  %g\n', Input.CalibrationParameters(param).CalibrationData(n).X, Input.CalibrationParameters(param).CalibrationData(n).Y, Input.CalibrationParameters(param).CalibrationData(n).Z, Input.CalibrationParameters(param).CalibrationData(n).Value);
                end
                fclose(fcalib);
                calibPoints = calibPoints+str2double(parameterPoints);
                Input.NCalib = calibPoints;
            end
        end
    end
    
    %% Wells
    if( strcmp(line,'BEGIN WELLS'))
        while ~strcmp(line,'END WELLS')
            line  = readline(fid);
            cellArray = tokenize(line, '" "');
            if(isempty(cellArray))
                continue;
            end
            if( strcmp(cellArray{1},'NUMBEROFWELLS'))
                nWells   = str2double(cellArray{2});
                Input.NWells = nWells;
                for n = 1:nWells
                    line  = readline(fid);
                    cellArray = tokenize(line, '" "');
                    if( strcmp(cellArray{1},'WELLFILE'))
                        Input.Well(n).Name = cellArray{2};
                        fprintf('Wellname =%s \n',Input.Well(n).Name);
                    end
                end
            end
        end
    end
    
    
    
    if( strcmp(line,'BEGIN OPTIMIZATION'))
        while ~strcmp(line,'END OPTIMIZATION')
            line  = readline(fid);
            cellArray = tokenize(line, '" "');
            if(isempty(cellArray))
                continue;
            end
            if( strcmp(cellArray{1},'TOLERANCE'))
                Input.Tolerance   = str2double(cellArray{2});
            end
        end
    end
    
    
    
end


% verification
if (isnan(Input.Population) || isnan(Input.Istrategy))
    error('Error, Population is NaN');
end

%create dir if not existing
createDir = ['mkdir ' Input.LaunchDir];
system(createDir);
%cp reference and mother projects to dir
cpProjects = ['cp ' Input.ReferenceProjectfile ' ' Input.MotherProjectfile ' ' Input.LaunchDir];
system(cpProjects);
%mv to launch directory
launchDir = ['cd ' Input.LaunchDir];
system(launchDir);
hostname=getenv('HOSTNAME');
fprintf('Host machine: %s \n',hostname);
fprintf('Setting up cauldron environment, gocbm3 \n');
% a bash script with setcauldron is made
system('/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlskui/bin/setcauldron &');
fclose(fid);
