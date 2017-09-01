
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% smooth down della rampa in ampiezza time history A %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for i=linspace(sys_data.syncA_gain,0,50)
   aoWrite('hvc_TT1CmdHistGain',i,sys_data.hvc_board);
   fprintf('.');
   pause(.005);
end
fprintf('\n');
aoWrite('hvc_CmdHistEna',0,sys_data.hvc_board);



