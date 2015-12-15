function   checkproject3dfile(project3dfilename)

fid = fopen(project3dfilename,'r');

if(fid == -1)
    fprintf('ReadCauldronDataMiningIoTblReadCauldronDataMiningIoTbl: project3d file %s doesnt exist \n', project3dfilename);
end


keywordHeader  = '[DataMiningIoTbl]';
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
    fprintf('project3d file has [DataMiningIoTbl] entry\n');
else
    fprintf('ERROR..project3d file does not have [DataMiningIoTbl] entry in it. For inversion, it should have [DataMiningIoTbl]!! \n');
    error('Error, project3d file does not have [DataMiningIoTbl] entry in it. For inversion, it should have [DataMiningIoTbl]!');
end