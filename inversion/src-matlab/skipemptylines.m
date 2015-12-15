function [line] = skipemptylines(fid,line)
if isempty(line) || ~ischar(line),
    while isempty(line) % if empty line skip lines until nonempty
        line = fgetl(fid);
        cellArray = tokenize(line, '" "');
        if( strcmp(cellArray{1},'#'))
            continue;
        end
    end
end