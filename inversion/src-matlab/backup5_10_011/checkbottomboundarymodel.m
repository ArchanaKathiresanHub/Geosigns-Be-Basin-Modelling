function  [model] = checkbottomboundarymodel(project3dfilename)

finput = fopen(project3dfilename,'r');

if(finput == -1)
    fprintf('project3d file %s doesnt exist \n', project3dfilename);
end


keywordHeader  = 'BottomBoundaryModel';
FoundHeader = 0;

%------------------------------------------
% Find the header of the datamining io tbl
%------------------------------------------
while ( feof(finput) == 0 && FoundHeader == 0) 
    activeline = fgetl(finput);
    if( strfind( activeline, keywordHeader) )
       FoundHeader = 1; 
    end  
end

if(FoundHeader == 1)
    line = fgetl(finput);
    line = fgetl(finput);
    lineArray=tokenize(line,' ');
    model = lineArray{1};
    fprintf('BottomBoundaryModel is %s\n', model);
    %msg = ['Bottom boundary model is ' model]
    %h=msgbox(msg);
else
    fprintf('ERROR..project3d file does not have BottomBoundaryModel entry \n');
    errordlg('Error, project3d file does not have BottomBoundaryModel entry ');
end