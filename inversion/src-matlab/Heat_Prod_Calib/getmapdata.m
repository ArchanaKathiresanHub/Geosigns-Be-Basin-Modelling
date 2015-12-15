
function getmapdata(project3dfile)

[MapName MapFileName MapSeqNbr] = findmapdetails(project3dfile,' ');

Input.MapName = MapName;
Input.MapFileName=MapFileName;
Input.MapSeqNbr = str2double(MapSeqNbr);


%Read HDF data to get map data

finfo = hdf5info(Input.MapFileName);
%hdf5 data for map is stored in layer = Input.MapSeqNbr+1
%check for 

hdf5layer=['/Layer=' MapSeqNbr];
%since matlabindex starts from 1, needs to add 1 to mapseqnr
layername=finfo.GroupHierarchy(1).Datasets(Input.MapSeqNbr+1).Name;
if(strcmp(layername,hdf5layer)==0 )
    error('ERROR.. something wrong due to map hdf layer mismatch');
end

% for hdf data  somehow transposed... ie, nx -> ny and ny -> nx
data=hdf5read(finfo.GroupHierarchy(1).Datasets(Input.MapSeqNbr+1));

[XCoord YCoord NumberX NumberY DeltaX DeltaY ScaleX ScaleY OffsetX OffsetY WindowXMin WindowXMax WindowYMin WindowYMax] = findgriddetails(project3dfile,' ');

Input.GridOriginXCoord = XCoord;
Input.GridOriginYCoord = YCoord;
Input.GridNumberX = NumberX;
Input.GridNumberY = NumberY;
Input.GridDeltaX = DeltaX;
Input.GridDeltaY = DeltaY;

Input.GridXMax = Input.GridOriginXCoord+Input.GridNumberX*Input.GridDeltaX;
Input.GridYMax = Input.GridOriginYCoord+Input.GridNumberY*Input.GridDeltaY;


[x y z ] = readwelllocation('welllocation.dat');
nx=(x-Input.GridOriginXCoord)/Input.GridDeltaX + 1;
ny=(y-Input.GridOriginYCoord)/Input.GridDeltaY + 1;

% for hdf data this is somehow transposed... ie, nx -> ny and ny -> nx
% if need to speed up, do for loop on data
welllocationmapdata = diag(data(ny,nx));

%modify this data after optimisation and write new map file

% create heat data for whole grid points
fid= fopen('heat_map.dat','w');
[r,c]=size(data);
for i = 1:r
    for j = 1:c
        xc = Input.GridOriginXCoord + i*Input.GridDeltaX;
        yc= Input.GridOriginYCoord + j*Input.GridDeltaY;
        fprintf(fid,'%d %d %f\n',xc,yc,data(j,i));
    end
end
fclose(fid);
%then modify

command=['grid2grid -input ' ' xyz ' ' -output ' ' hdf5 ' ' -origin ' ' -resolution ' ' -delta '];
%grid2grid -input xyz.dat xyz -output heatflow.HDF hdf5 -origin 0 0 -resolution 3 3 -delta 10 50

end

