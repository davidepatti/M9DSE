
%% Condizioni operative
% Cambiando le condizioni operative si fa riferimento a diversi casi
% sperimentali
% I casi presenti sono 4 e sono dati dalle seguenti condizioni:
    %I_L=[10 50] [Ampere]
    %R_g=[2.2 10] [Ohm]

% Corrente di carico    
I_L=10;

% Resistenza di gate
R_g=10;

%% Resistenze
% Resistenze interne al package (uguali se si utilizzano gli stessi
% dispositivi): NON DOVREBBE SERVIRE CAMBIARLE
R_g_H_int=1;
R_g_L_int=1;

% Resistenze esterne (imposte dalle condizioni operative)
R_g_H_ext=R_g;
R_g_L_ext=R_g;

% Calcolo delle resistenze utilizzate nella fase di simulazione 
R_g_H=R_g_H_int+R_g_H_ext;
R_g_L=R_g_L_int+R_g_L_ext;


params

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
cd 'Simulazioni ON'
Turn_on;
cd ..
%% Plot
load(['data_on_',num2str(I_L),'A_',num2str(R_g),'Ohm.mat']);
f1=figure;

subplot(2,3,1)
hold on
plot(t_sim, VgsH_sim, 'b');
plot(t_exp_HS, VgsH_exp, 'r.');

% interpolation
y_new = interp1(t_sim,VgsH_sim,t_exp_HS);
%plot(t_exp_HS,y_new,'g.');

error = sum(abs(VgsH_exp-y_new'))/length(t_exp_HS);

save("error.txt","error","-ascii");

grid on;
title(['V_G_S del dispositivo di High Side  [V]']);
xlabel(['t [s]']);
hold off

subplot(2,3,2)
hold on
plot(t_sim, VdsH_sim, 'b');
plot(t_exp_HS, VdsH_exp, 'r');
grid on;
title(['V_D_S del dispositivo di High Side  [V]']);
xlabel(['t [s]']);
hold off

subplot(2,3,3)
hold on
plot(t_sim, IdH_sim, 'b');
plot(t_exp_HS, IdH_exp, 'r');
grid on;
title(['I_D del dispositivo di High Side  [A]']);
xlabel(['t [s]']);
hold off

subplot(2,3,4)
hold on
plot(t_sim, VgsL_sim, 'b');
plot(t_exp_LS, VgsL_exp, 'r');
grid on;
title(['V_G_S del dispositivo di Low Side  [V]']);
xlabel(['t [s]']);
hold off

subplot(2,3,5)
hold on
plot(t_sim, VdsL_sim, 'b');
plot(t_exp_LS, VdsL_exp, 'r');
grid on;
title(['V_D_S del dispositivo di Low Side  [V]']);
xlabel(['t [s]']);
hold off

subplot(2,3,6)
hold on
plot(t_sim, IdL_sim, 'b');
plot(t_exp_LS, IdL_exp, 'r');
grid on;
title(['I_D del dispositivo di Low Side  [A]']);
xlabel(['t [s]']);
hold off

set(f1,'Position',get(0,'ScreenSize'));
