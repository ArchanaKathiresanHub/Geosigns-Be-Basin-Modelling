function  [MapName MapFileName MapSeqNbr] = findmapdetails(project3dfile,delimiter)

   
    finput = fopen(project3dfile,'r');

    if(finput == -1)
        fprintf('cannot read .project3dfile \n');
    end
    keywordHeader  = '[GridMapIoTbl]';
    mapLineStarter = '"BasementIoTbl"';
    FoundHeader = 0;
    FoundMapLine = 0;
    
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
        position=tokenposition(line,'MapName',delimiter);  %space as delimter
       line = fgetl(finput);% skip unit row
       line = fgetl(finput);
       lineArray=tokenize(line, delimiter);
       %mapName in quotes
       
       nameInQuotes = lineArray{position}; % origin x coord 
       MapName = tokenize(nameInQuotes,'"');
        while( feof(finput) == 0 && FoundMapLine == 0) %% look for variable line           
            line = fgetl(finput);
            lineArray=tokenize(line, delimiter);  
            if(strcmp(lineArray{1},mapLineStarter))
                FoundMapLine = 1;
                %map seq nr is 5th column after the mapname column
                MapFileName = tokenize(lineArray{position+2},'"'); % 
                MapSeqNbr= lineArray{position+5}; % 
            end
        end      
    end
fclose(finput);
end
