%BCUs location
% BCU_IP_00 = '192.168.1.70';
% BCU_IP_01 = '192.168.1.71';
% BCU_IP_10 = '192.168.1.72';
% BCU_IP_11 = '192.168.1.73';
% BCU_IP_20 = '192.168.1.74';
% BCU_IP_21 = '192.168.1.75';
% AOConnect(BCU_IP_00,BCU_IP_01,BCU_IP_10,BCU_IP_11,BCU_IP_20,BCU_IP_21);

clear tmp;
for crateN=1:6,
    tmp(crateN)=aoGetBCUStatus(crateN);
    rearPBPlateTemp(crateN)=tmp(crateN).in0_temp;
    voltage_vccl(crateN)=tmp(crateN).voltage_vccl;
    voltage_vcca(crateN)=tmp(crateN).voltage_vcca;
    voltage_vssa(crateN)=tmp(crateN).voltage_vssa;
    voltage_vccp(crateN)=tmp(crateN).voltage_vccp;
    voltage_vssp(crateN)=tmp(crateN).voltage_vssp;
    current_vccl(crateN)=tmp(crateN).current_vccl;
    current_vcca(crateN)=tmp(crateN).current_vcca;
    current_vssa(crateN)=tmp(crateN).current_vssa;
    current_vccp(crateN)=tmp(crateN).current_vccp;
    current_vssp(crateN)=tmp(crateN).current_vssp;
    power_logic(crateN)=voltage_vccl(crateN)*current_vccl(crateN);
    power_analog(crateN)=voltage_vcca(crateN)*current_vcca(crateN)+voltage_vssa(crateN)*current_vssa(crateN);
    power_power(crateN)=voltage_vccp(crateN)*current_vccp(crateN)+voltage_vssp(crateN)*current_vssp(crateN);
    power_total(crateN)=power_logic(crateN)+power_analog(crateN)+power_power(crateN);
end;
power_total_sum=sum(power_total);


coolingInTemp=tmp(3).out0_temp;
coolingOutTemp=tmp(5).out0_temp;
coolingFlux=tmp(2).ext_humidity;
% coolingFlux=12.5;
ambient_0=tmp(3).out1_temp;
ambient_1=tmp(6).out1_temp;
ambient_ext=tmp(1).out0_temp;
humidity_0=tmp(3).ext_humidity/(1.0546-0.00216*ambient_0);
humidity_1=tmp(6).ext_humidity/(1.0546-0.00216*ambient_1);
dew_0=dew(ambient_0,humidity_0);
dew_1=dew(ambient_1,humidity_1);

% sensor7=tmp(2).out0_temp;
% sensor8=tmp(2).out1_temp;

fprintf(1,'\nCrate    #0.0    #0.1    #1.0    #1.1    #2.0    #2.1\n\n');
fprintf(1,'PBTemp'); for i=1:6, fprintf(1,' %6.2f ',rearPBPlateTemp(i)); end; fprintf('°C\n');
fprintf(1,'V_VCC '); for i=1:6, fprintf(1,' %6.2f ',voltage_vccl(i)); end; fprintf(' V\n');
fprintf(1,'V_VCCA'); for i=1:6, fprintf(1,' %6.2f ',voltage_vcca(i)); end; fprintf(' V\n');
fprintf(1,'V_VSSA'); for i=1:6, fprintf(1,' %6.2f ',voltage_vssa(i)); end; fprintf(' V\n');
fprintf(1,'V_VCCP'); for i=1:6, fprintf(1,' %6.2f ',voltage_vccp(i)); end; fprintf(' V\n');
fprintf(1,'V_VSSP'); for i=1:6, fprintf(1,' %6.2f ',voltage_vssp(i)); end; fprintf(' V\n');
fprintf(1,'I_VCC '); for i=1:6, fprintf(1,' %6.2f ',current_vccl(i)); end; fprintf(' A\n');
fprintf(1,'I_VCCA'); for i=1:6, fprintf(1,' %6.2f ',current_vcca(i)); end; fprintf(' A\n');
fprintf(1,'I_VSSA'); for i=1:6, fprintf(1,' %6.2f ',current_vssa(i)); end; fprintf(' A\n');
fprintf(1,'I_VCCP'); for i=1:6, fprintf(1,' %6.2f ',current_vccp(i)); end; fprintf(' A\n');
fprintf(1,'I_VSSP'); for i=1:6, fprintf(1,' %6.2f ',current_vssp(i)); end; fprintf(' A\n');
fprintf(1,'PowerL'); for i=1:6, fprintf(1,' %6.2f ',power_logic(i)); end; fprintf(' W\n');
fprintf(1,'PowerA'); for i=1:6, fprintf(1,' %6.2f ',power_analog(i)); end; fprintf(' W\n');
fprintf(1,'PowerP'); for i=1:6, fprintf(1,' %6.2f ',power_power(i)); end; fprintf(' W\n');
fprintf(1,'PwrTot'); for i=1:6, fprintf(1,' %6.2f ',power_total(i)); end; fprintf(' W\n');
% 
% 
% 
fprintf(1,'\nPower, total  %5.2f W\n',power_total_sum);
fprintf(1,'Cooling inlet  %5.2f °C\n',coolingInTemp);
fprintf(1,'Cooling outlet %5.2f °C\n',coolingOutTemp);
fprintf(1,'Cooling flux   %5.2f l/min\n',coolingFlux);
fprintf(1,'Removed heat %7.2f W\n',(coolingOutTemp-coolingInTemp)*coolingFlux*4186/60);
fprintf(1,'Ambient  %5.1f %5.1f °C\n',ambient_0,ambient_1);
fprintf(1,'Humidity %5.1f %5.1f %%RU\n',humidity_0,humidity_1);
fprintf(1,'DewPoint %5.1f %5.1f °C\n',dew_0,dew_1);
fprintf(1,'Ambient, ext   %5.1f °C\n',ambient_ext);

% fprintf(1,'7: %5.1f  ----   8: %5.1f\n',sensor7,sensor8);

clear tmp;
