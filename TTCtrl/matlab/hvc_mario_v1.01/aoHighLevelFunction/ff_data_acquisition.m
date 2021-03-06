function data_out=ff_data_acquisition(nmeas, amp, OPD_filename, OPD_store_dir, varargin)
%
% NAME:
%  IF_DATA_ACQUISITION
%
% PURPOSE:
%	The routine performs the data acquisition for the influence matrix estimation.
%
% USAGE:
%	data_out = if_data_acquisition(nmeas, amp, OPD_filename, OPD_store_dir, MODAL=modal, FF=ff, CL_ACT=cl_act,
%     INTEGRATOR=integ, MAX_PEAK_FORCE=max_peak_force, DM_SYSTEM=dmSystem, STEP_PAUSE=step_pause)
%
% INPUT:
%  nmeas:            number of measurements to do. (positive and negative excitation)
%  amp:              amplitude of excitation (FLOAT).
%  OPD_filename:     filename of the OPD image generated by the interferometer (including path)
%  OPD_store_dir:    path where the OPD images should be stored
%  [modal]:          use the modal estmation of the feed forward matrix (the default is zonal).
%  [ff]:             use the feed forward contribution of a precalculated matrix.
%  [cl_act]:         close loop actuators. (defaul all acts)
%  [integ]:          perfoms the use of the integrator. (don't use with FF enabled.)
%  [max_peak_force]: set a new max peak force value.
%  [dmSystem]:       set the system to use (0=DP-M4, 1=LBT/VLT)
%  [step_pause]:     prompt at every step
%  [interfer]:       enable sincronous interferometer acquisition
%
% OUTPUT:
%  data_out
%
% NOTE:
%  A) Integrator parameter iteration not explicity set. 
%     The dafault value in integrator function is four, to be tested with the experience.
%  B) Data ordering:
%     reference, +amp(ch/mode_1), -amp(ch/mode_1), +amp(ch/mode_2), ... , -amp(ch/mode_n_meas), reference
%
% HISTORY
%  01/02/10 - Modified by Mario to expand for D45 system
%

sys_data=evalin('base','sys_data');

% checks and setting on default parameters
modal='ZONAL';
ff='';
cl_act=0:sys_data.nAct-1;
integ='';
max_peak_force='';
dmSystem=0;
step_pause=0;
interfer=0;

if nargin>=5
   modal=varargin{1};
end
if nargin>=6
   ff=varargin{2};
end
if nargin>=7
   cl_act=varargin{3};
end
if nargin>=8
   integ=varargin{4};
end
if nargin>=9
   max_peak_force=varargin{5};
end
if nargin>=10
   dmSystem=varargin{6};
end
if nargin>=11
   step_pause=varargin{7};
end
if nargin>=12
   interfer=varargin{8};
end

n_act=length(cl_act);
data_proto=get_sys_status(cl_act,sys_data.mirrorActMap,dmSystem);
data_out=repmat(data_proto, n_act*2*nmeas+2, 1);

% set del flat con il nuovo guadagno
if dmSystem==0
   mirrorCmdUpdate(uint32(sys_data.flat_command*2^sys_data.scale_input),int32(sys_data.flat_ff_force*sys_data.A2BIT_gain));
   pause(.1);
   mirrorCmdUpdate(uint32(sys_data.flat_command*2^sys_data.scale_input),int32(sys_data.flat_ff_force*sys_data.A2BIT_gain));
   pause(.1);
else
   mirrorCmdUpdate(sys_data.flat_command,sys_data.flat_ff_force,1,strcmp(ff,'FF'));
   while sum(aoRead('_dm2_runningAcc',0:sys_data.nDsp-1)),pause(.05),end
   pause(.1);
end

% reading the current system status
idx = 1;
data_out(idx)=get_sys_status(cl_act,sys_data.mirrorActMap,dmSystem);
idx = idx + 1;
sign = [1 -1];
ButtonName = 'Continue';

if interfer
   if size(dir([OPD_store_dir '\*.int']),1)
      error('Error: The target directory already contains image files');
   end
   imm_cnt = 0;
   fprintf('Ready to start, launch interferometer script with all aberrations (5 sec pause - %d steps)\n',2+2*nmeas*length(cl_act));
%    pause(1);copyfile('C:\Srv\Progetti\Engineering\VLT_DSM\Design\Matlab\Test\D45\v3.04\data_interfer\image.int',OPD_filename);
   % salvataggio della prima immagine dell'interferometro (flat iniziale)
   while ~exist(OPD_filename,'file'),pause(.1),end;
   pause(.2);
   [imm,imm_struct]=readOPDImage(OPD_filename,1,0);
   ss=sprintf('%s\\imm_[%04d].mat',OPD_store_dir,imm_cnt);
   save (ss,'imm_struct');
   delete(OPD_filename);
   imm_cnt=imm_cnt+1;
end

if strcmp(ButtonName,'Continue')
   % cicle of measurements
   for act = cl_act
      if strcmp(modal,'MODAL')
         fprintf('Mode#%02d - amp: �%5.1f [um] ',find(cl_act==act),amp*1e6);
      else
         fprintf('Act#%02d - amp: �%5.1f [um] ',act,amp*1e6);
      end
      for i = 1:nmeas
         for j = 1:2
            % modal or zonal excitation
            new_amp=amp;
            dummy_command=sys_data.flat_command;
            if strcmp(modal,'MODAL')
               mod_vect=zeros(size(sys_data.ff_v,2),1);
               mod_vect(cl_act==act)=sign(j)*new_amp;
               dummy_command(cl_act+1)=dummy_command(cl_act+1)+sys_data.ff_v*mod_vect;
            else
               dummy_command(act+1)=dummy_command(act+1)+sign(j)*new_amp;
            end
            dummy_force=sys_data.flat_ff_force;
            if strcmp(ff,'FF')
               delta_command=dummy_command - sys_data.flat_command;
               delta_force=sys_data.ff_matrix*delta_command(cl_act+1);
            else
               delta_force=zeros(n_act,1);
            end
            if max(abs(delta_force))>max_peak_force
               new_amp=new_amp*max_peak_force/max(abs(delta_force));
               dummy_command=sys_data.flat_command;
               if strcmp(modal,'MODAL')
                  mod_vect=zeros(size(sys_data.ff_v,2),1);
                  mod_vect(cl_act==act)=sign(j)*new_amp;
                  dummy_command(cl_act+1)=dummy_command(cl_act+1)+sys_data.ff_v*mod_vect;
               else
                  dummy_command(act+1)=dummy_command(act+1)+sign(j)*new_amp;
               end
               dummy_force=sys_data.flat_ff_force;
               if strcmp(ff,'FF')
                  delta_command=dummy_command - sys_data.flat_command;
                  delta_force=sys_data.ff_matrix*delta_command(cl_act+1);
               else
                  delta_force=zeros(n_act,1);
               end
            end
            dummy_force(cl_act+1)=dummy_force(cl_act+1)+delta_force;
            if dmSystem==0
               mirrorCmdUpdate(uint32(dummy_command*2^sys_data.scale_input),int32(dummy_force*sys_data.A2BIT_gain));
               pause(.1);
               mirrorCmdUpdate(uint32(dummy_command*2^sys_data.scale_input),int32(dummy_force*sys_data.A2BIT_gain));
               pause(.1);
            else
               mirrorCmdUpdate(dummy_command,dummy_force,1,strcmp(ff,'FF'));
               while sum(aoRead('_dm2_runningAcc',0:sys_data.nDsp-1)),pause(.05),end
               pause(.1);
            end
            data_out(idx)=get_sys_status(cl_act,sys_data.mirrorActMap,dmSystem);
            plot_mirror(cl_act,[data_out(idx).mean_position-data_out(1).mean_position data_out(idx).mean_position data_out(idx).mean_tot_curr],1);
            idx = idx + 1;
            fprintf('.');
            if step_pause
               ButtonName = questdlg('Continue, exit ?','', 'Continue', 'Exit', 'Continue');
               switch ButtonName,
                  case 'Continue',
                     continue;
                  case 'Exit',
                     break;
               end % switch
            end
            if interfer
               % salvataggio dell'immagine dell'interferometro
%                copyfile('C:\Srv\Progetti\Engineering\VLT_DSM\Design\Matlab\Test\D45\v3.04\data_interfer\image.int',OPD_filename);
               while ~exist(OPD_filename,'file'),pause(.1),end;
               pause(.2);
               [imm,imm_struct]=readOPDImage(OPD_filename,1,0);
               ss=sprintf('%s\\imm_[%04d].mat',OPD_store_dir,imm_cnt);
               save (ss,'imm_struct');
               delete(OPD_filename);
               imm_cnt=imm_cnt+1;
            end
         end
         if strcmp(ButtonName,'Exit')
            break;
         end
      end
      if strcmp(ButtonName,'Exit')
         break;
      end
      if amp ~= new_amp
         fprintf(' feed forward force clipped to maximum value %e',new_amp+1e-7);
      end
      % dopo una serie di misure, prima di passare al modo/act successivo spiano a shell
      if dmSystem==0
         mirrorCmdUpdate(uint32(sys_data.flat_command*2^sys_data.scale_input),int32(sys_data.flat_ff_force*sys_data.A2BIT_gain));
         pause(.1);
         mirrorCmdUpdate(uint32(sys_data.flat_command*2^sys_data.scale_input),int32(sys_data.flat_ff_force*sys_data.A2BIT_gain));
         pause(.5);
      else
         mirrorCmdUpdate(sys_data.flat_command,sys_data.flat_ff_force,1,strcmp(ff,'FF'));
         while sum(aoRead('_dm2_runningAcc',0:sys_data.nDsp-1)),pause(.05),end
         pause(.5);
      end
      fprintf('\n');
   end
end

% salvataggio del flat finale dopo tutta l'acquisizione
data_out(idx)=get_sys_status(cl_act,sys_data.mirrorActMap,dmSystem);
plot_mirror(cl_act,[data_out(idx).mean_position data_out(idx).mean_tot_curr],1);

if interfer
   % salvataggio dell'ultima immagine dell'interferometro (flat finale)
%    copyfile('C:\Srv\Progetti\Engineering\VLT_DSM\Design\Matlab\Test\D45\v3.04\data_interfer\image.int',OPD_filename);
   while ~exist(OPD_filename,'file'),pause(.1),end;
   pause(.2);
   [imm,imm_struct]=readOPDImage(OPD_filename,1,0);
   ss=sprintf('%s\\imm_[%04d].mat',OPD_store_dir,imm_cnt);
   save (ss,'imm_struct');
   delete(OPD_filename);
   imm_cnt=imm_cnt+1;
end

fprintf('all done!!!\n');
