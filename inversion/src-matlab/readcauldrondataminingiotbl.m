function [DataMiningIoTbl_run] = readcauldrondataminingiotbl(projectname)

fid = fopen(projectname,'r');

if(fid == -1)
    fprintf('Project3d file not found \n');
end


keywordHeader  = '[DataMiningIoTbl]';
keywordFooter  = '[End]';
FoundHeader = 0;


%------------------------------------------
% Find the header of the datamining io tbl
%------------------------------------------
while ( feof(fid) == 0 && FoundHeader == 0) 
    activeline = fgetl(fid);
    if( strfind( activeline, keywordHeader) )
       FoundHeader = 1; 
    end  
end

if(FoundHeader == 1)
    line = fgetl(fid);  %skip the semicolon line after header 
    line = fgetl(fid);  %skip the property names
    line = fgetl(fid);  %skip the property units
    lineArray=tokenize(line, ' ');%space as delimiter
    %read data from line until we reach the footer keyword
    calibPoints=1;
    while ( ~strcmp(lineArray{1},keywordFooter) )
        line = fgetl(fid); 
        lineArray=tokenize(line, ' ');
        if (strcmp(lineArray{1},keywordFooter))
            break;
        end
        % properties in the line for a given calibration points/entries
        DataMiningIoTbl_run(calibPoints).BPAKey          = lineArray{1}; 
        DataMiningIoTbl_run(calibPoints).Time            = str2double(lineArray{2});
        DataMiningIoTbl_run(calibPoints).XCoord          = str2double(lineArray{3});
        DataMiningIoTbl_run(calibPoints).YCoord          = str2double(lineArray{4});
        DataMiningIoTbl_run(calibPoints).ZCoord          = str2double(lineArray{5});
        DataMiningIoTbl_run(calibPoints).ReservoirName   = lineArray{6};
        DataMiningIoTbl_run(calibPoints).FormationName   = lineArray{7};
        DataMiningIoTbl_run(calibPoints).SurfaceName     = lineArray{8};
        DataMiningIoTbl_run(calibPoints).PropertyName    = lineArray{9};
        DataMiningIoTbl_run(calibPoints).PropertyUnit    = lineArray{10};
        DataMiningIoTbl_run(calibPoints).Value           = str2double(lineArray{11});
        calibPoints=calibPoints+1;
    end
else
   fprintf('Error in ReadCauldronDataMiningIoTbl - Could not find table!! \n'); 
end
fclose(fid);   % CLOSE THE FILE!
fprintf('Finished function ReadCauldronDataMiningIoTbl %s \n',projectname);
% 
% 
%   % 11 properties to read
%         BPAKey          = fscanf(fid,'%s ',1);
%         Time            = fscanf(fid,'%f ',1);
%         XCoord          = fscanf(fid,'%f ',1);
%         YCoord          = fscanf(fid,'%f ',1);
%         ZCoord          = fscanf(fid,'%f ',1);
%         ReservoirName   = fscanf(fid,'%s ',1);
%         FormationName   = fscanf(fid,'%s ',1);
%         SurfaceName     = fscanf(fid,'%s ',1);
%         PropertyName    = fscanf(fid,'%s ',1);
%         PropertyUnit    = fscanf(fid,'%s ',1);
%         Value           = fscanf(fid,'%f \n',1);
%     
%         %fprintf('Reading mined data entry %d at %f %f %f type = %s, value= %f \n',d, XCoord, YCoord, ZCoord, PropertyName, Value);
%         % Copy the information to the appropriate
%         DataMiningIoTbl_run(d).BPAKey         = BPAKey; 
%         DataMiningIoTbl_run(d).Time           = Time;
%         DataMiningIoTbl_run(d).XCoord         = XCoord;
%         DataMiningIoTbl_run(d).YCoord         = YCoord;
%         DataMiningIoTbl_run(d).ZCoord         = ZCoord;
%         DataMiningIoTbl_run(d).ReservoirName  = ReservoirName;
%         DataMiningIoTbl_run(d).FormationName  = FormationName;
%         DataMiningIoTbl_run(d).SurfaceName    = SurfaceName;
%         DataMiningIoTbl_run(d).PropertyName   = PropertyName;
%         DataMiningIoTbl_run(d).PropertyUnit   = PropertyUnit;
%         DataMiningIoTbl_run(d).Value          = Value;
% 
% 
% 
% 
% 
% 
% 
% %------------------------------------------
% % Find the footer of the datamining io tbl
% %------------------------------------------
% if(FoundHeader == 1)
%     ndata = 0;
%     %fprintf('FoundHeader \n');
%     
%     activeline = fgetl(fid);   % :
%     activeline = fgetl(fid);   % BPAKey Time XCoord YCoord ZCoord ReservoirName FormationName SurfaceName PropertyName PropertyUnit Value 
%     activeline = fgetl(fid);   % () (Ma) (m) (m) (m) () () () () () ()
%     
%     
%     % How many calibration data entries do we have?
%     while( feof(fid) == 0 && FoundFooter == 0) 
%     
%            activeline = fgetl(fid);
%            ndata = ndata + 1;
%            if( strfind( activeline, keywordFooter) )
%                FoundFooter = 1; 
%            end  
%     end
%     
%     %fprintf('Found the [end] of the DataMiningIoTbl for file %s \n',projectname); 
%     %fprintf('Number of calibration data entries = %d  \n',ndata-1);
%     fclose(fid);   % CLOSE THE FILE!
% end
% 
% 
% %----------------------------------------------------------------------
% % The file is already closed. Open it again ana read the data entries
% %----------------------------------------------------------------------
% if(FoundHeader == 1 & FoundFooter == 1)
%     
%     fid = fopen(projectname,'r');
%     if(fid == -1)
%         fprintf('ReadCauldronDataMiningIoTbl: projectname file fid == -1 \n');
%     end
%     FoundHeader = 0;
%     ndata = ndata - 1;
%     
%     % Search for the header again
%     while ( feof(fid) == 0 && FoundHeader == 0) 
%        
%           activeline = fgetl(fid);
%           if( strfind( activeline, keywordHeader) )
%              FoundHeader = 1; 
%           end  
%     end
%     
%     % Read the additional header stuff
%     activeline = fgetl(fid);   % :
%     activeline = fgetl(fid);   % BPAKey Time XCoord YCoord ZCoord ReservoirName FormationName SurfaceName PropertyName PropertyUnit Value 
%     activeline = fgetl(fid);   % () (Ma) (m) (m) (m) () () () () () ()
%     for(d=1:ndata)
%     
%          % 11 properties to read
%         BPAKey          = fscanf(fid,'%s ',1);
%         Time            = fscanf(fid,'%f ',1);
%         XCoord          = fscanf(fid,'%f ',1);
%         YCoord          = fscanf(fid,'%f ',1);
%         ZCoord          = fscanf(fid,'%f ',1);
%         ReservoirName   = fscanf(fid,'%s ',1);
%         FormationName   = fscanf(fid,'%s ',1);
%         SurfaceName     = fscanf(fid,'%s ',1);
%         PropertyName    = fscanf(fid,'%s ',1);
%         PropertyUnit    = fscanf(fid,'%s ',1);
%         Value           = fscanf(fid,'%f \n',1);
%     
%         %fprintf('Reading mined data entry %d at %f %f %f type = %s, value= %f \n',d, XCoord, YCoord, ZCoord, PropertyName, Value);
%         % Copy the information to the appropriate
%         DataMiningIoTbl_run(d).BPAKey         = BPAKey; 
%         DataMiningIoTbl_run(d).Time           = Time;
%         DataMiningIoTbl_run(d).XCoord         = XCoord;
%         DataMiningIoTbl_run(d).YCoord         = YCoord;
%         DataMiningIoTbl_run(d).ZCoord         = ZCoord;
%         DataMiningIoTbl_run(d).ReservoirName  = ReservoirName;
%         DataMiningIoTbl_run(d).FormationName  = FormationName;
%         DataMiningIoTbl_run(d).SurfaceName    = SurfaceName;
%         DataMiningIoTbl_run(d).PropertyName   = PropertyName;
%         DataMiningIoTbl_run(d).PropertyUnit   = PropertyUnit;
%         DataMiningIoTbl_run(d).Value          = Value;
%         
%     end %loop over the data entries 
%     
%     fclose(fid);
% else
%    fprintf('Error in ReadCauldronDataMiningIoTbl - Could not find table!! \n'); 
% end
% 
% fprintf('Finished function ReadCauldronDataMiningIoTbl %s \n',projectname);
% 
% 
%     
