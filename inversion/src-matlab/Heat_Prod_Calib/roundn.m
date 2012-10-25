function x = roundn(x, n)
%ROUNDN  Round to multiple of 10^n
% Copied from windows 7.11 version C:\Program Files\MATLAB\R2010b\toolbox\shared\maputils\roundn.m
%
%   ROUNDN(X,N) rounds each element of X to the nearest multiple of 10^N.
%   N must be scalar, and integer-valued. For complex X, the imaginary
%   and real parts are rounded independently. For N = 0 ROUNDN gives the
%   same result as ROUND. That is, ROUNDN(X,0) == ROUND(X).
%
%   Examples
%   --------
%   % Round pi to the nearest hundredth
%   roundn(pi, -2)
%
%   % Round the equatorial radius of the Earth, 6378137 meters,
%   % to the nearest kilometer.
%   roundn(6378137, 3)
%
%  See also ROUND.

% Copyright 1996-2009 The MathWorks, Inc.
% $Revision: 1.1.6.2 $  $Date: 2009/04/15 23:34:51 $

validateattributes(x, {'single', 'double'}, {}, 'ROUNDN', 'X')

if nargin == 1
    warning('maputils:roundn:exponentOmitted', ...
        ['%s was not specified when calling %s, so the default value %d', ...
        ' will be used. In the future, %s will no longer support a default', ...
        ' value and will error if %s is omitted. You will have', ...
        ' to supply both input arguments. If there are any instances in', ...
        ' your code of the usage %s you should replace them with %s.'], ...
        'N','ROUNDN',-2,'ROUNDN','N', 'ROUNDN(X)', 'ROUNDN(X,-2)')
    
    n = -2;
end
    
validateattributes(n, ...
    {'numeric'}, {'scalar', 'real', 'integer'}, 'ROUNDN', 'N')

% Compute the power of 10 to which input will be rounded.
p = 10 ^ n;

% Round x to the nearest multiple of p.
x = p * round(x / p);