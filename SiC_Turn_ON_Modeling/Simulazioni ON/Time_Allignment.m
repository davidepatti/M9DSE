% Questo file contiene i valori per allineare gli assi temporali: NON
% MODIFICARE

if (I_L==10 && R_g==2.2)
    % Ritardo temporale delle forme d'onda del Low Side 
    Dt_exp_sim=9.7e-8;
    % Ritardo temporale tra le forme d'onda di High Side e Low Side 
    Dt_HS_LS=9.6e-9;
    % Definizione del vettore temporale di Low Side
    t_exp_LS=0:1/1.25e9:400e-9-1/1.25e9;
end

if (I_L==10 && R_g==10)
    % Ritardo temporale delle forme d'onda del Low Side 
    Dt_exp_sim=6.3e-8;
    % Ritardo temporale tra le forme d'onda di High Side e Low Side 
    Dt_HS_LS=24e-9;
    % Definizione del vettore temporale di Low Side
    t_exp_LS=0:1/1.25e9:400e-9-1/1.25e9;
end

if (I_L==50 && R_g==2.2)
    % Ritardo temporale delle forme d'onda del Low Side 
    Dt_exp_sim=7.8e-8;
    % Ritardo temporale tra le forme d'onda di High Side e Low Side 
    Dt_HS_LS=23e-9;
    % Definizione del vettore temporale di Low Side
    t_exp_LS=0:1/1.25e9:400e-9-1/1.25e9;
end

if (I_L==50 && R_g==10)
    % Ritardo temporale delle forme d'onda del Low Side 
    Dt_exp_sim=4e-8;
    % Ritardo temporale tra le forme d'onda di High Side e Low Side 
    Dt_HS_LS=31.4e-9;
    % Definizione del vettore temporale di Low Side
    t_exp_LS=0:1/1.25e9:400e-9-1/1.25e9;
end