%% Condizioni operative

% Corrente di carico    
%I_L=10;
% I_L=30;
 I_L=50;
% I_L=70;

% Resistenze di gate
R_gH=2.2;
% R_gH=8.2;
% R_gH=20;

R_gL=2.2;
% R_gL=8.2;


%% Resistenze
% Resistenze interne al package (uguali se si utilizzano gli stessi
% dispositivi): NON DOVREBBE SERVIRE CAMBIARLE
R_g_H_int=1.2;
R_g_L_int=1.2;

% Resistenze esterne (imposte dalle condizioni operative)
R_g_H_ext=R_gH;
R_g_L_ext=R_gL;

% Calcolo delle resistenze utilizzate nella fase di simulazione 
R_g_H=R_g_H_int+R_g_H_ext;
R_g_L=R_g_L_int+R_g_L_ext;

params


%% Induttanze
% Induttanze parassite di bonding interne al package
L_d_int=0.38e-9;   % Riferimento Q3D 0.38e-9;   % Valore modificabile
L_s_int=3e-9;  % Riferimento Q3D 6.41e-9;   % Valore modificabile
L_g_int=2e-9;  % Riferimento Q3D 8.76e-9;   % Valore modificabile

% Induttanze parassite dei pin esterne al package (supponiamo che G e S siano uguali)
L_d_pin=3e-9;   % Riferimento Q3D 13.46e-9;   % Valore modificabile
L_s_pin=3e-9;   % Riferimento Q3D 10.95e-9;   % Valore modificabile
L_g_pin=L_s_pin;   % Riferimento Q3D 10.99e-9;

% Induttanze parassite del PCB
L_dH_ext=5e-9;  % Riferimento Q3D 50e-9;   % Valore modificabile
L_sH_ext=6e-9;  % Riferimento Q3D 12.5e-9;   % Valore modificabile
L_gH_ext=6e-9;  % Riferimento Q3D 12e-9;   % Valore modificabile
L_dL_ext=5e-9;  % Riferimento Q3D 12.5e-9;   % Valore modificabile
L_sL_ext=6e-9;  % Riferimento Q3D 48e-9;   % Valore modificabile
L_gL_ext=6e-9;  % Riferimento Q3D 12e-9;   % Valore modificabile

% Induttanze parassite dei cavi per i collegamenti di Kelvin-Source
L_Hwire=2.5e-9; % Riferimento Q3D 10.95e-9;   % Valore modificabile
L_Lwire=2.5e-9; % Riferimento Q3D 10.95e-9;   % Valore modificabile

% Le induttanze interne sono uguali per High Side(HS) e Low Side(LS) se si
% suppone di utilizzare dispositivi uguali
L_dH_int=L_d_int;
L_sH_int=L_s_int;
L_gH_int=L_g_int;

L_dL_int=L_d_int;
L_sL_int=L_s_int;
L_gL_int=L_g_int;


% Calcolo dei parametri induttivi utilizzati in fase di simulazione
L_gH=L_g_pin+L_gH_int+L_gH_ext+L_Hwire;
L_dH=L_d_pin+L_dH_int+L_dH_ext;
L_sH=L_s_pin+L_sH_int+L_sH_ext;
L_sH1=L_s_pin+L_sH_ext;

L_gL=L_g_pin+L_gL_int+L_gL_ext+L_Lwire;
L_dL=L_d_pin+L_dL_int+L_dL_ext;
L_sL=L_s_pin+L_sL_int+L_sL_ext;
L_sL1=L_s_pin+L_sL_ext;

L_A=L_gH+L_sH_int;
L_B=L_gL+L_sL_int;
L_eq=L_dH+L_sH_int+L_sH1+L_dL+L_sL_int+L_sL1;

%% Simulazioni
% In questa parte viene lanciata la simulazione con i parametri selezionati
cd 'Simulazioni OFF'
Turn_off;
cd ..

%% Plot
load(['data_off_',num2str(I_L),'A_',num2str(R_gH),'Ohm.mat']);

%% Added for M9DSE 

% interpolation
VgsH_sim_new = interp1(t_sim,VgsH_sim,t_exp_HS, 'previous','extrap');
error_VgsH = sum(abs(VgsH_exp-VgsH_sim_new'))/length(t_exp_HS);

save("error_VgsH.txt","error_VgsH","-ascii");

% interpolation
VdsH_sim_new = interp1(t_sim,VdsH_sim,t_exp_HS, 'previous','extrap');
error_VdsH = sum(abs(VdsH_exp-VdsH_sim_new'))/length(t_exp_HS);
save("error_VdsH.txt","error_VdsH","-ascii");

% interpolation
IdH_sim_new = interp1(t_sim,IdH_sim,t_exp_HS, 'previous','extrap');
error_IdH = sum(abs(IdH_exp-IdH_sim_new'))/length(t_exp_HS);
save("error_IdH.txt","error_IdH","-ascii");

% interpolation
VgsL_sim_new = interp1(t_sim,VgsL_sim,t_exp_LS, 'previous','extrap');
error_VgsL = sum(abs(VgsL_exp-VgsL_sim_new'))/length(t_exp_LS);
save("error_VgsL.txt","error_VgsL","-ascii");

% interpolation
VdsL_sim_new = interp1(t_sim,VdsL_sim,t_exp_LS, 'previous','extrap');
error_VdsL = sum(abs(VdsL_exp-VdsL_sim_new'))/length(t_exp_LS);
save("error_VdsL.txt","error_VdsL","-ascii");

% interpolation
IdL_sim_new = interp1(t_sim,IdL_sim,t_exp_LS, 'previous','extrap');
error_IdL = sum(abs(IdL_exp-IdL_sim_new'))/length(t_exp_LS);
save("error_IdL.txt","error_IdL","-ascii");

error_HL = [error_VgsH error_VdsH error_IdH error_VgsL error_VdsL error_IdL ]

save("error_HL.txt","error_HL","-ascii");

disp("CIAO HO FINITO - Ora plotto!");
%------

f1=figure;

subplot(2,3,1)
hold on
plot(t_sim, VgsH_sim, 'b');
plot(t_exp_HS, VgsH_exp, 'r');
grid on;
title(['V_G_S del dispositivo di High Side  [V]']);
xlabel(['t [s]']);
xlim([0 5e-7])
hold off

subplot(2,3,2)
hold on
plot(t_sim, VdsH_sim, 'b');
plot(t_exp_HS, VdsH_exp, 'r');
grid on;
title(['V_D_S del dispositivo di High Side  [V]']);
xlabel(['t [s]']);
xlim([0 5e-7])
hold off

subplot(2,3,3)
hold on
plot(t_sim, IdH_sim, 'b');
plot(t_exp_HS, IdH_exp, 'r');
grid on;
title(['I_D del dispositivo di High Side  [A]']);
xlabel(['t [s]']);
xlim([0 5e-7])
hold off

subplot(2,3,4)
hold on
plot(t_sim, VgsL_sim, 'b');
plot(t_exp_LS, VgsL_exp, 'r');
grid on;
title(['V_G_S del dispositivo di Low Side  [V]']);
xlabel(['t [s]']);
xlim([0 5e-7])
hold off

subplot(2,3,5)
hold on
plot(t_sim, VdsL_sim, 'b');
plot(t_exp_LS, VdsL_exp, 'r');
grid on;
title(['V_D_S del dispositivo di Low Side  [V]']);
xlabel(['t [s]']);
xlim([0 5e-7])
hold off

subplot(2,3,6)
hold on
plot(t_sim, IdL_sim, 'b');
plot(t_exp_LS, IdL_exp, 'r');
grid on;
title(['I_D del dispositivo di Low Side  [A]']);
xlabel(['t [s]']);
xlim([0 5e-7])
hold off

set(f1,'Position',get(0,'ScreenSize'));
