function [line] = skipemptyorcommentlines(fid,line)
if isempty(line) || ~ischar(line) || line(1)=='#'
    while isempty(line) || ~ischar(line) || line(1)=='#' % if empty line skip lines until nonempty
        line = fgetl(fid);
    end
end