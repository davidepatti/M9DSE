
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

