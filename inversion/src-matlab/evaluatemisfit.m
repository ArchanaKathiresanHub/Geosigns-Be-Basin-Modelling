function [ error ] = evaluatemisfit( observed, computed, type )

% implement different objective functions here

if strcmp(type,'L1')
% Absolute error
error = abs(observed - computed);
end
if strcmp(type,'L2')
% Absolute error
error = (observed - computed).^2;
end

end

