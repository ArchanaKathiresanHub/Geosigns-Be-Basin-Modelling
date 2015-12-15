function position = tokenposition(string,substring, delimiter)
    tokens = {};
    %stringArray = textscan(string,'%s','Delimiter',delimiter)
    while not(isempty(string))        
        [tokens{end+1},string] = strtok(string,delimiter);
    end

 position = find(strcmp(tokens,substring));
   
end

% C = textscan(line,'%s','Delimiter',' ')