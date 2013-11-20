results

hold off

cmpl = { 'Intel', 'gcc412', 'gcc481' };

libn = {'ArmadilloNew', 'EigenNew', 'MklNew', 'CauldronOld', 'ArmadilloOld', 'EigenOld' };
liblegn = {'Armadillo (New)', 'Eigen (New)', 'Mkl (New)', 'Cauldron (Old)', 'Armadillo (Old)', 'Eigen (Old)' };

stn = {'-xr', '-xb', '-xg', '-ok', '-or', '-ob' };


for l = 1: length( cmpl )

   hold off
   for n = 1:length(libn)
      if ( exist( [ libn{n} cmpl{l} ] ) ) 
         eval( [ 'x = ' libn{n} cmpl{l} '(:,1);' ] );
         eval( [ 'y = ' libn{n} cmpl{l} '(:,2);' ] );

         h(n) = plot( x, y, stn{n}, 'linewidth', 3 );
         hold on
      end
   end

   legend( h, liblegn );
   xlabel( 'Nxy * Nxy * Nz' );
   ylabel( 'Time [s]' );
   title( cmpl{l} );
   grid on
   eval( ['print ' cmpl{l} '.jpg;'] );
end


