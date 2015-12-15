
% this adds [DataMiningIoTbl] entry into the _mother project3d file
% it reads the calibration data from input file and add dummy values(-9999)
% initially
function [Calibrationdata] = filldataminingtable(Input)

NCalib = Input.NCalib;
fprintf('NCalib = %d \n',NCalib);

% NCalibParam = length(Input.CalibrationParameters);
% nn = 1;
% for(m=1:NCalibParam)
%     NCalibParamData = length(Input.CalibrationParameters(m).CalibrationData)
%     for(n=1:NCalibParamData)
%         Calibrationdata(nn).X        = Input.CalibrationParameters(m).CalibrationData(n).X;
%         Calibrationdata(nn).Y        = Input.CalibrationParameters(m).CalibrationData(n).Y;
%         Calibrationdata(nn).Z        = Input.CalibrationParameters(m).CalibrationData(n).Z;
%         Calibrationdata(nn).PropertyName     = Input.CalibrationParameters(m).CalibrationData(n).PropertyName;
%         Calibrationdata(nn).Observed = Input.CalibrationParameters(m).CalibrationData(n).Value;
%         Calibrationdata(nn).Wellname = 'unknown';
%         Calibrationdata(nn).Wellindex= Input.CalibrationData(n).Wellindex;
%         nn = nn+1;
%     end
% end


for(param=1:Input.NCalibParam)
    for(d=1:Input.CalibrationParameters(param).Points)
        Calibrationdata(param).Values(d).X        = Input.CalibrationParameters(param).CalibrationData(d).X;
        Calibrationdata(param).Values(d).Y        = Input.CalibrationParameters(param).CalibrationData(d).Y;
        Calibrationdata(param).Values(d).Z        = Input.CalibrationParameters(param).CalibrationData(d).Z;
        Calibrationdata(param).Values(d).PropertyName     = Input.CalibrationParameters(param).CalibrationData(d).PropertyName;
        Calibrationdata(param).Values(d).Observed = Input.CalibrationParameters(param).CalibrationData(d).Value;
        Calibrationdata(param).Values(d).Wellname = 'unknown';
        Calibrationdata(param).Values(d).WellIndex= Input.CalibrationParameters(param).CalibrationData(d).WellIndex;
    end
end

% for(n=1:NCalib)
%     Calibrationdata(n).X        = Input.CalibrationData(n).X;
%     Calibrationdata(n).Y        = Input.CalibrationData(n).Y;
%     Calibrationdata(n).Z        = Input.CalibrationData(n).Z;
%     Calibrationdata(n).PropertyName     = Input.CalibrationData(n).PropertyName;
%     Calibrationdata(n).Observed = Input.CalibrationData(n).Value;
%     Calibrationdata(n).Wellname = 'unknown';
%     Calibrationdata(n).Wellindex= Input.CalibrationData(n).Wellindex;
%     
% end


ref_projectfile = Input.ReferenceProjectfile;
mothername      = Input.MotherProjectfile;

ref_projectfile_p = fopen(ref_projectfile,'r');
new_projectfile_p = fopen(mothername,'w');

if( ref_projectfile_p== -1)
    fprintf('File missing %s \n', ref_projectfile);
    
end

if( new_projectfile_p == -1)
    fprintf('File missing %s \n', mothername);
end

%stringSearch = ['  () (Ma)  (m)  (m)  (m)   ()   ()   ()   ()   ()   ()'];    % String in the [DataMiningIoTbl] 

%[DataMiningIoTbl]
%;
%BPAKey Time XCoord YCoord ZCoord ReservoirName FormationName SurfaceName PropertyName PropertyUnit Value 
%() (Ma) (m) (m) (m) () () () () () () 

%stringSearch = ['() (Ma) (m) (m) (m) () () () () () ()'];  %%% >>>>>>>>>>>>>>>>>>need to change
%stringSearch = '[DataMiningIoTbl]';
%search for keyword '[DataMiningIoTbl] 
%fcopylinesuntilkeyword(stringSearch, ref_projectfile_p,new_projectfile_p );





insertdataminingiotable( ref_projectfile_p,new_projectfile_p, Input, Calibrationdata);
fclose(ref_projectfile_p);
fclose(new_projectfile_p);


% NCalibParam = length(Input.CalibrationParameters);
% 
% nn = 1;
% for(m=1:NCalibParam)
%     NCalibParamData = length(Input.CalibrationParameters(m).CalibrationData)
%     for(n=1:NCalibParamData)
%             x = Calibrationdata(nn).X;
%             y = Calibrationdata(nn).Y;
%             z = Calibrationdata(nn).Z;
%      datatype = Calibrationdata(nn).PropertyName;
%         value = Calibrationdata(nn).Observed;   
%     calibrationline = [' ""   0 ' num2str(x) ' '  num2str(y) ' ' num2str(z)  ' ""   ""   "" "' datatype '" "" -99999'];
%     %calibrationline;
%     fprintf(new_projectfile_p,'%s \n', calibrationline); 
%     nn = nn+1;
%     end
% end



% for(param=1:Input.NCalibParam)
%     for(d=1:Input.CalibrationParameters(param).Points)
%         x = Calibrationdata(param).Values(d).X ;
%         y = Calibrationdata(param).Values(d).Y  ;
%         z = Calibrationdata(param).Values(d).Z  ;
%         datatype = Calibrationdata(param).Values(d).PropertyName ;
%         %value = Calibrationdata(param).Values(d).Observed;
%         calibrationline = [' ""   0 ' num2str(x) ' '  num2str(y) ' ' num2str(z)  ' ""   ""   "" "' datatype '" "" -99999'];
%          fprintf(new_projectfile_p,'%s \n', calibrationline); 
%     end
% end


% 
% for(n=1:NCalib)
%     x         = Calibrationdata(n).X;
%     y         = Calibrationdata(n).Y;
%     z         = Calibrationdata(n).Z;
%     datatype  = Calibrationdata(n).PropertyName;
%     value = Calibrationdata(n).Observed;   
%     calibrationline = [' ""   0 ' num2str(x) ' '  num2str(y) ' ' num2str(z)  ' ""   ""   "" "' datatype '" "" -99999'];
%     %calibrationline;
%     fprintf(new_projectfile_p,'%s \n', calibrationline);   
% end

%fcopylinesuntilkeyword('TheUltimateEnd', ref_projectfile_p, new_projectfile_p);

fprintf('Finished adding calibration data to the data file \n');

        
