function [wellproject3dfile] = createprojectaroundwells(project3dfile,Input)


%read well location
%[x y z ] = readwelllocation(welllocation);
%[x y z temperature heat_min heat_max heat_init] = readwelllocation(welllocation);



%get grid details from project3d
[XCoord YCoord NumberX NumberY DeltaX DeltaY ScaleX ScaleY OffsetX OffsetY WindowXMin WindowXMax WindowYMin WindowYMax] = findgriddetails(project3dfile,' ');
%get the lower left corner index of the element containing well location
nx=floor((Input.X-XCoord)/DeltaX );
ny=floor((Input.Y-YCoord)/DeltaY);

% CHECK FOR Max and Min index and skip in that case

%foreach model, find model area
   Input.WindowXMin = nx-1;
   Input.WindowXMax = nx+2;
   Input.WindowYMin = ny-1;
   Input.WindowYMax = ny+2;


% %foreach index pair (nx, ny) create a window of 3x3 elements
% for i=1:length(nx)
%    Well(i).WindowXMin = nx(i)-1;
%    Well(i).WindowXMax = nx(i)+2;
%    Well(i).WindowYMin = ny(i)-1;
%    Well(i).WindowYMax = ny(i)+2;
% end

outputfilename=[Input.LaunchDir Input.BaseName '.project3d'];
modifygridwindow(project3dfile,outputfilename, Input.WindowXMin,  Input.WindowXMax,  Input.WindowYMin,  Input.WindowYMax);
wellproject3dfile = outputfilename;
% for i=1:length(nx)
%     outputfilename=['/scratch/nlskui/Well_' num2str(i) '.project3d'];
%     modifygridwindow(project3dfile,outputfilename, Well(i).WindowXMin,  Well(i).WindowXMax,  Well(i).WindowYMin,  Well(i).WindowYMax);
% end

end


 