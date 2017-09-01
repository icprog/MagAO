basedir='./lbt672';

for n = 1:672
    fprintf('Actuator %d\n',n)
    load( sprintf('%s/ARCH/S_%d.mat',basedir,n));
    data256 = fitsread(sprintf('%s/GRID/Stress_%d.fits',basedir,n-1));
    data128 = fitsread(sprintf('%s/GRID-128/Stress_%d.fits',basedir,n-1));
    data64  = fitsread(sprintf('%s/GRID-64/Stress_%d.fits',basedir,n-1));

    mor    = max( reshape( [Sxl Syl] ,[],1));
    mgr256 = max( reshape( data256, [],1));
    mgr128 = max( reshape( data128, [],1));
    mgr64  = max( reshape( data64, [],1));

    diff256(n) = (mgr256-mor)/mor;
    diff128(n) = (mgr128-mor)/mor;
    diff64(n)  = (mgr64-mor)/mor;
end
diffM = [diff64;diff128;diff256]';
save(sprintf('%s/TestGridding.mat',basedir), 'diffM');
TestGriddingPlot(diffM);