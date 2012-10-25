function  [XCoord YCoord NumberX NumberY DeltaX DeltaY ScaleX ScaleY OffsetX OffsetY WindowXMin WindowXMax WindowYMin WindowYMax] = findgriddetails(project3dfile,delimiter)
   
    finput = fopen(project3dfile,'r');

    if(finput == -1)
        fprintf('cannot read .project3dfile \n');
    end
    keywordHeader  = '[ProjectIoTbl]';

    FoundHeader = 0;
    
   % fields to read
   
    
    %------------------------------------------
    % Find the header of the datamining io tbl
    %------------------------------------------
    while ( feof(finput) == 0 && FoundHeader == 0)
        line = fgetl(finput);
        if( strfind( line, keywordHeader) )
            FoundHeader = 1;
        end
    end
    if(FoundHeader == 1)
        line = fgetl(finput);% skip ;
        line = fgetl(finput);% read field names
        position=tokenposition(line,'XCoord',delimiter);  %space as delimter
       line = fgetl(finput);% skip unit row
       line = fgetl(finput);
       lineArray=tokenize(line, delimiter);
       
       XCoord = str2double(lineArray{position}); % origin x coord
       YCoord = str2double(lineArray{position+1}); % origin y coord
       NumberX = str2double(lineArray{position+2}); % no of points in x
       NumberY = str2double(lineArray{position+3}); % no of points in y
       DeltaX = str2double(lineArray{position+4}); % delta x
       DeltaY = str2double(lineArray{position+5}); % 
       ScaleX  = str2double(lineArray{position+6}); %                  
       ScaleY   = str2double(lineArray{position+7}); %                
       OffsetX   = str2double(lineArray{position+8}); %                
       OffsetY    = str2double(lineArray{position+9}); %            
       WindowXMin  = str2double(lineArray{position+10}); %              
       WindowXMax  = str2double(lineArray{position+11}); %              
       WindowYMin   = str2double(lineArray{position+12}); %             
       WindowYMax = str2double(lineArray{position+13}); % 
       
    end
fclose(finput);
end

