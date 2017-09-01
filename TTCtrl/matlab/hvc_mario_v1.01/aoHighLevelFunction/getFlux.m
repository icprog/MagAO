%BCUs location
% BCU_IP_00 = '192.168.1.70';
% BCU_IP_01 = '192.168.1.71';
% BCU_IP_10 = '192.168.1.72';
% BCU_IP_11 = '192.168.1.73';
% BCU_IP_20 = '192.168.1.74';
% BCU_IP_21 = '192.168.1.75';
% AOConnect(BCU_IP_00,BCU_IP_01,BCU_IP_10,BCU_IP_11,BCU_IP_20,BCU_IP_21);

clear tmp;
tmp=aoGetBCUStatus(2);

coolingFlux=tmp.ext_humidity;

fprintf(1,'Cooling flux   %5.2f l/min\n',coolingFlux);
clear tmp;
