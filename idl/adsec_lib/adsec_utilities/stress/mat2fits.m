% convert a .mat file from Ciro's FEAs to one or more fits file.
% eventually interpolate on a square grid

shell='p45modes';
if strcmp(shell, 'lbt672')
    basedir     = './lbt672/ARCH';
    outdir      = './lbt672/GRID-128';
    basename  = '/S_';
    offname   = -1;
    r_out_ciro=  0.4556;
    r_out     =  0.4556; % 0.120005 for p45 ;  0.4556 for lbt672;
    r_inn     =  28.34e-3;
    n_act     =  672;
    n_grid    =  128;
    I=[1:672];
    %I=[192 215 218 219 224 246 306 337 538 573:672];
elseif strcmp(shell, 'p45')
    basedir     = './p45/ARCH';
    outdir      = './p45/GRID';
    basename  = '/S_';
    offname   = -1;
    r_out_ciro=  0.122;    % ciro ha la shell rifinita larga (0.122)
    r_out     =  0.120005; % 0.120005 for p45 ;  0.4556 for lbt672;
    r_inn     =  28.34e-3;
    n_act     =  45;
    n_grid    =  256;
    I=[1:45];
elseif strcmp(shell, 'lbt672modes')
    basedir     = 'Z:\adopt\FEA\lbt672\Modes';
    outdir      = 'Z:\adopt\FEA\lbt672\Modes';
    basename  = 'P_output_';
    offname   = 0;
    r_out     =  0.4556; % 0.120005 fsor p45 ;  0.4556 for lbt672;
    r_inn     =  28.34e-3;
    n_act     =  672;
    n_grid    =  256;
    I=[0:100];
elseif strcmp(shell, 'p45modes')
    basedir     = './p45/Modes';
    outdir      = './p45/Modes';
    basename  = '/P_output_';
    offname   = 0;
    r_out_ciro=  0.122;    % ciro ha la shell rifinita larga (0.122)
    r_out     =  0.120005; % 0.122 fsor p45 ;  0.4556 for lbt672;
    r_inn     =  28.34e-3;
    n_act     =  45;
    n_grid    =  256;
    I=[0:44];
end


s=fullfile(outdir, ['Ngrid.fits']);
fitswrite(n_grid,s);

[XI,YI]=meshgrid(linspace(-r_out,r_out,n_grid),linspace(-r_out,r_out,n_grid));

if strcmp(shell, 'lbt672modes')
    RR=sqrt(XI.^2+YI.^2);
    Pupil = find ( RR < r_out &  RR > r_inn ) ;
    s=fullfile(outdir, ['Pupil.fits']);
    fitswrite(Pupil-1,s); % !!!! IDL starts counting from 0 !!!!
end

for i = 1:length(I)
    t0_tmp=clock;
    d=I(i);
    filenome = [basename int2str(d) '.mat']; 
    s = fullfile(basedir, filenome);
    fprintf('Analyzing file %s ...',s);
    if (strcmp(shell, 'lbt672') || strcmp(shell, 'p45'))
        try        
            load(s, 'Deltan_act', 'Sxl', 'Syl');
        catch
            fprintf('Couldnt load file %s \n',s);
            continue;
        end
        s=fullfile(outdir, ['Deltan_' int2str(d+offname) '.fits']);
        fitswrite(Deltan_act,s);
        fprintf(' Deltan ...');
        [X,Y]=meshgrid(linspace(-r_out_ciro,r_out_ciro,size(Sxl,1)), ...
            linspace(-r_out_ciro,r_out_ciro,size(Sxl,1)));
        Sx_t = interp2(X,Y,Sxl(:,:,1),XI,YI);fprintf(' Sxt ...');
        Pupil = find ( isfinite(Sx_t)==1 ) ;
        Sx_b = interp2(X,Y,Sxl(:,:,2),XI,YI);fprintf(' Sxb ...');
        Pupil = intersect(Pupil, find ( isfinite(Sx_b)==1 )) ;
        Sy_t = interp2(X,Y,Syl(:,:,1),XI,YI);fprintf(' Syt ...');
        Pupil = intersect(Pupil, find ( isfinite(Sy_t)==1 )) ;
        Sy_b = interp2(X,Y,Syl(:,:,2),XI,YI);fprintf(' Syb ...');
        Pupil = intersect(Pupil, find ( isfinite(Sy_b)==1 )) ;
        s=fullfile(outdir, ['Pupil_' int2str(d+offname) '.fits']);
        fitswrite(Pupil-1,s); % !!!! IDL starts counting from 0 !!!!
        
        s=fullfile(outdir, ['Stress_' int2str(d+offname) '.fits']);
        fitswrite([Sx_t(Pupil) Sx_b(Pupil) Sy_t(Pupil) Sy_b(Pupil)],s);
        fprintf(' Max %g...', max( reshape( [Sx_t(Pupil) Sy_t(Pupil) Sx_b(Pupil) Sy_b(Pupil)] ,[],1)));
        
        clear Sx_t; clear Sx_b; clear Sy_t; clear Sy_b; clear Pupil;
        clear Deltan_act; clear Sxl; clear Syl; clear X; clear Y;
        
    elseif strcmp(shell, 'p45modes')
        try        
            load(s, 'Delta_n', 'Sxl', 'Syl');
        catch
            fprintf('Couldnt load file %s \n',s);
            continue;
        end    
        [X,Y]=meshgrid(linspace(-r_out_ciro,r_out_ciro,size(Sxl,1)), ...
            linspace(-r_out_ciro,r_out_ciro,size(Sxl,1)));
        Delt = interp2(X,Y,Delta_n,XI,YI);fprintf(' Deltan ...');
        Sx_t = interp2(X,Y,Sxl(:,:,1),XI,YI);fprintf(' Sxt ...');
        Pupil = find ( isfinite(Sx_t)==1 ) ;
        Sx_b = interp2(X,Y,Sxl(:,:,2),XI,YI);fprintf(' Sxb ...');
        Pupil = intersect(Pupil, find ( isfinite(Sx_b)==1 )) ;
        Sy_t = interp2(X,Y,Syl(:,:,1),XI,YI);fprintf(' Syt ...');
        Pupil = intersect(Pupil, find ( isfinite(Sy_t)==1 )) ;
        Sy_b = interp2(X,Y,Syl(:,:,2),XI,YI);fprintf(' Syb ...');
        Pupil = intersect(Pupil, find ( isfinite(Sy_b)==1 )) ;
        
        s=fullfile(outdir, ['Pupil_' int2str(d+offname) '.fits']);
        fitswrite(Pupil-1,s); % !!!! IDL starts counting from 0 !!!!
        s=fullfile(outdir, ['Deltan_' int2str(d+offname) '.fits']);
        fitswrite(Delt(Pupil),s);
        s=fullfile(outdir, ['ModalStress_' int2str(d+offname) '.fits']);
        fitswrite([Sx_t(Pupil) Sx_b(Pupil) Sy_t(Pupil) Sy_b(Pupil)],s);
        clear Sx_t; clear Sx_b; clear Sy_t; clear Sy_b; clear Delt;
        clear Delta_n; clear Sxl; clear Syl; clear Pupil;
        
    elseif strcmp(shell, 'lbt672modes')
        try        
            load(s, 'Delta_n', 'SSxl', 'SSyl', 'X', 'Y');
        catch
            fprintf('Couldnt load file %s \n',s);
            continue;
        end    
        Delta_n_g = griddata(X,Y,Delta_n,XI,YI);
        s=fullfile(outdir, ['Deltan_' int2str(d+offname) '.fits']);
        fitswrite(Delta_n_g(Pupil),s);
        fprintf(' Deltan ...');

        Sx_t = griddata(X,Y,SSxl(:,1),XI,YI);fprintf(' Sxt ...');
        Sx_b = griddata(X,Y,SSxl(:,2),XI,YI);fprintf(' Sxb ...');
        Sy_t = griddata(X,Y,SSyl(:,1),XI,YI);fprintf(' Syt ...');
        Sy_b = griddata(X,Y,SSyl(:,2),XI,YI);fprintf(' Syb ...');
        s=fullfile(outdir, ['ModalStress_' int2str(d+offname) '.fits']);
        fitswrite([Sx_t(Pupil) Sx_b(Pupil) Sy_t(Pupil) Sy_b(Pupil) ],s);
        clear Sx_t; clear Sx_b; clear Sy_t; clear Sy_b; clear Delta_n_g;
        clear Delta_n; clear SSxl; clear SSyl; clear X; clear Y;
    end
    fprintf(' end (%.1f'''').\n',etime(clock,t0_tmp)); clear t0_tmp;
end

