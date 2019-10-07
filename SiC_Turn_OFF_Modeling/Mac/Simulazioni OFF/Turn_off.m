%% Initialization of the parameters

% Tempo di campionamento (al massimo 1e-10 altrimenti perde troppo tempo)
Ts=1e-10;

% Parameters
V_dd=800; % DC Voltage
V_gg_H=18; % Tensione alta per V_Gate
V_gg_L=-5; % Tensione bassa per V_Gate
V_th=4.5; % Tensione di soglia
Fall_time=20e-9;%72e-9; % Tempo di salite della V_Gate per passare da V_gg_L a V_gg_H

% Istante di tempo iniziale: "t" sarà il vettore contenente l'asse del tempo
t=0;

% Vettore contenente gli istanti temporali di passaggio da uno stage ad un altro
time=0;

% Errore a regime per la derivata: questo parametro permette di stabilire il valore della derivata di una variabile per il quale si assume che quella variabile sia arrivata già a regime
err_reg=0.01;

% Initial conditions
v_gsH=V_gg_H;
v_dsH=0;
i_gH=0;
i_dH=I_L;
v_gsL=V_gg_L;
v_dsL=V_dd;
i_gL=0;
i_dL=0;

% Inizializzazione del vettore di stato "X" contenente tensioni e correnti dei due dispositivi
X=[v_gsH;
   v_dsH;
   i_gH;
   i_dH;
   v_gsL;
   v_dsL;
   i_gL;
   i_dL];

% Inizializzazione del vettore "Xd" contenente le derivate delle tensioni e delle correnti dei due dispositivi
Xd=[0;
    0;
    0;
    0;
    0;
    0;
    0;
    0];

% Dichiarazione della variabile per la V_Gate
V_gg=[V_gg_H];
% % Dichiarazione della variabile per eventuale Cross-Turn-ON
% Flag_CrossTurnON=[];

%% Stage 6: v_gs1 decreases

% Inizializzazione g_fs
g_fs=read_gfs(X(1,end));
% Inizializzazione V_miller
V_miller=X(4,end)/g_fs+V_th;
while (X(1,end)>V_miller && t(end)<5e-7) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento g_fs
    g_fs=read_gfs(X(1,end));
    % Aggiornamento V_miller
    V_miller=X(4,end)/g_fs+V_th;
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Fall_time,t)];
    % Definizione delle matrici di stato per lo Stage 6
    A6=[0 0 1/(C_gsH+C_gdH)  0 0 0 0 0;                                          
        0 0 0 0 0 0 0 0;
        -1/(L_A) 0 -R_g_H/(L_A) 0 0 0 0 0;
        zeros(5, 8)];

    B6=[0 0 V_gg(end)/(L_A) 0 0 0 0 0]';
    
    % Risoluzione dello satge
    [t,X]=solve_stage(t,X,A6,B6,Ts);
    % Aggiornamento delle derivate
    Xd=[Xd A6*X(:,end)+B6];
end

% Aggiornamento del vettore "time" con l'istante temporale di uscita dallo
% Stage 6
time=[time t(end)];


%% Stage 7: v_ds1 rise time I

while(X(2, end)<V_miller-V_th && t(end)<5e-7) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento g_fs
    g_fs=read_gfs(X(1,end));
    % Aggiornamento V_miller
    V_miller=X(4,end)/g_fs+V_th;
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Fall_time,t)];
    % Definizione delle matrici di stato per lo Stage 7
    A7=[0 0 0 0 0 0 0 0;
        0 0 -1/(C_gdH) 0 0 0 0 0;
        -1/(L_A) 0 -R_g_H/(L_A) 0 0 0 0 0;
        0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0];

    B7=[0;
        0;
        V_gg(end)/(L_A);
        0;
        0;
        0;
        0;
        0];
    
    % Risoluzione dello satge
    [t, X]=solve_stage(t, X, A7, B7, Ts);
    % Aggiornamento delle derivate
    Xd=[Xd A7*X(:,end)+B7];
end

% Aggiorno il vettore time con l'istante temporale di uscita dello stage 7
time=[time t(end)];


%% Stage 8
while(X(6, end)>0 && t(end)<5e-7)  % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento g_fs
    g_fs=read_gfs(X(1,end));
    % Aggiornamento V_miller
    V_miller=X(4,end)/g_fs+V_th;
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Fall_time,t)];
    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A8=[-m1*C_gdH*g_fs 0 m1*(C_dsH+C_gdH) m1*C_gdH 0 0 0 0;
        -m1*(C_gdH+C_gsH)*g_fs 0 m1*C_gdH m1*(C_gdH+C_gsH) 0 0 0 0; 
        -m2*(L_eq*L_B-L_sL_int^2) m2*L_sH_int*L_B -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 m3*(C_dsL+C_gdL) m3*C_gdL;
        0 0 0 0 0 0 m3*C_gdL m3*(C_gsL+C_gdL);
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];

    B8=[m1*C_gdH*g_fs*V_th;
        m1*(C_gdH+C_gsH)*g_fs*V_th;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg_L+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg_L);
        0;
        0;
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg_L-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg_L)];

    % Risoluzione dello satge
    [t, X]=solve_stage(t, X, A8, B8, Ts);
    % Aggiornamento delle derivate
    Xd=[Xd A8*X(:,end)+B8];
end

% Aggiorno il vettore time con l'istante temporale di uscita dello stage 8
time=[time t(end)];

%% Stage 9

while(X(1,end)>V_th && t(end)<5e-7) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento g_fs
    g_fs=read_gfs(X(1,end));
    % Aggiornamento V_miller
    V_miller=X(4,end)/g_fs+V_th;
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Fall_time,t)];
    
%     % Cross turn on monitoring
%     if (X(5,end)>V_th)
%         Flag_CrossTurnON=[Flag_CrossTurnON; t(end) X(5,end)];
%     end

    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A9=[-m1*C_gdH*g_fs 0 m1*(C_dsH+C_gdH) m1*C_gdH 0 0 0 0;
        -m1*(C_gdH+C_gsH)*g_fs 0 m1*C_gdH m1*(C_gdH+C_gsH) 0 0 0 0; 
        -m2*(L_eq*L_B-L_sL_int^2) m2*L_sH_int*L_B -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 1/(C_dsL+C_gdL) 0;
        0 0 0 0 0 0 0 0;
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];
    
    B9=[m1*C_gdH*g_fs*V_th;
        m1*(C_gdH+C_gsH)*g_fs*V_th;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg_L+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg_L);
        0;
        0;
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg_L-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg_L)];

        % Risoluzione dello stage
        [t, X]=solve_stage(t, X, A9, B9, Ts);
        % Aggiornamento delle derivate
        Xd=[Xd A9*X(:,end)+B9];
end

% Aggiornamento del vettore con istante di uscita dallo stage 9
if (t(end)==time(end))
else
    time=[time t(end)];
end
   
%% Stage 10
while ((abs((X(1,end)-X(1, end-1))/Ts)>err_reg || t(end)== time(end)) && t(end)<5e-7) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Blocco g_fs in quanto il dispositivo sarà spento
    g_fs=0;
    % Aggiornamento V_miller
    V_miller=X(4,end)/g_fs+V_th;
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Fall_time,t)];
    
%     % Cross turn on monitoring
%     if (X(5,end)>V_th)
%         Flag_CrossTurnON=[Flag_CrossTurnON; t(end) X(5,end)];
%     end

    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A10=[-m1*C_gdH*g_fs 0 m1*(C_dsH+C_gdH) m1*C_gdH 0 0 0 0;
        -m1*(C_gdH+C_gsH)*g_fs 0 m1*C_gdH m1*(C_gdH+C_gsH) 0 0 0 0; 
        -m2*(L_eq*L_B-L_sL_int^2) m2*L_sH_int*L_B -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 1/(C_dsL+C_gdL) 0;
        0 0 0 0 0 0 0 0;
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];

    B10=[m1*C_gdH*g_fs*V_th;
        m1*(C_gdH+C_gsH)*g_fs*V_th;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg_L+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg_L);
        0;
        0;
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg_L-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg_L)];

        % Risoluzione dello satge
        [t, X]=solve_stage(t, X, A10, B10, Ts);
        % Aggiornamento delle derivate
        Xd=[Xd A10*X(:,end)+B10];
end

% Aggiornamento del vettore con istante di uscita dallo stage 4
if (t(end)==time(end))
else
    time=[time t(end)];
end
% %% Cross Turn ON Occurance
% % Se il vettore Flag_CrossTurnON è vuoto vuol dire che non si sono
% % verificate le condizioni per cui si dovrebbe riaccendere il dipsositivo
% % passivo; in caso contrario questo modello potrebbe essere invalido 
% if (size(Flag_CrossTurnON) ==[0 0])
% else Flag_CrossTurnON
% end
%%  Evaluation of external signals
% Aggiornamento per adattare le dimensioni dei vettori
V_gg=[V_gg V_gg_L];
X=[X(:,1),X];
Xd=[Xd(:,1),Xd];

% Calcolo delle forme d'onda esterne
X_R=[V_gg-R_g_H_ext*X(3,:)-(L_gH_ext+L_Hwire)*Xd(3,:);
    X(2,:)+(L_dH_int+L_d_pin+L_sH_int)*Xd(4,:)+(L_sH_int)*Xd(3,:);
    X(3,:);
    X(4,:);
    X(5,:)+R_g_L_int*X(7,:)+(L_gL_int+L_sL_int)*Xd(7,:)+L_sL_int*Xd(8,:);
    X(6,:)+(L_dL_int+L_d_pin+L_sL_int)*Xd(8,:)+(L_sL_int)*Xd(7,:);
    X(7,:);
    X(8,:)];

% %% Time allignment
% % Allineamento degli assi temporali per simulate e sperimentali 
% Time_Allignment;
% % Ricostruzione vettore temporale di High Side
% t_exp_HS=t_exp_LS+Dt_HS_LS;
% 
% % Ritardo totale per allineare le simulate 
% T_delay=Dt_exp_sim+Dt_HS_LS;
% 
% % Ricostruzione dei vettori t e time per le forme d'onda simulate tenendo conto
% % del ritardo
% 
% t_sim=[0,t+T_delay];
% time_sim=time+T_delay;
% 
% % Definizione dell'istante massimo per i plot
% T_max=t_exp_LS(end);
% close all;




%% Saving
    VgsH_sim=X_R(1,:);
    [VgsH_exp,t_exp_HS]=wfm2read (['../test/low/off/wave/#57Vgs18-5RgL',num2str(R_g_H),'RgH',num2str(R_g_L),'Id',num2str(I_L),'_Ch1.wfm']);

    VdsH_sim=X_R(2,:);
    VdsH_exp=wfm2read (['../test/low/off/wave/#57Vgs18-5RgL',num2str(R_g_H),'RgH',num2str(R_g_L),'Id',num2str(I_L),'_Ch3.wfm']);

    IdH_sim=X_R(4,:);
    IdH_exp=wfm2read (['../test/low/off/wave/#57Vgs18-5RgL',num2str(R_g_H),'RgH',num2str(R_g_L),'Id',num2str(I_L),'_Ch4.wfm']);

    VgsL_sim=X_R(5,:);
    [VgsL_exp,t_exp_LS]=wfm2read (['../test/high/off/wave/#57Vgs18-5RgL',num2str(R_g_L),'RgH',num2str(R_g_H),'Id',num2str(I_L),'_Ch1.wfm']);

    VdsL_sim=X_R(6,:);
    VdsL_exp=wfm2read (['../test/high/off/wave/#57Vgs18-5RgL',num2str(R_g_L),'RgH',num2str(R_g_H),'Id',num2str(I_L),'_Ch3.wfm']);

    IdL_sim=X_R(8,:);
    IdL_exp=wfm2read (['../test/high/off/wave/#57Vgs18-5RgL',num2str(R_g_L),'RgH',num2str(R_g_H),'Id',num2str(I_L),'_Ch4.wfm']);

    if (length(t_exp_HS)==5000)
        TrigPosition=find(t_exp_HS==0);
        VgsH_exp=VgsH_exp(TrigPosition-length(t_exp_HS)/4:TrigPosition+length(t_exp_HS)/4);
        VdsH_exp=VdsH_exp(TrigPosition-length(t_exp_HS)/4:TrigPosition+length(t_exp_HS)/4);
        IdH_exp=IdH_exp(TrigPosition-length(t_exp_HS)/4:TrigPosition+length(t_exp_HS)/4);
        t_exp_HS=t_exp_HS(TrigPosition-length(t_exp_HS)/4:TrigPosition+length(t_exp_HS)/4);

    end
    if (length(t_exp_LS)==5000)
        TrigPosition=find(t_exp_LS==0);
        VgsL_exp=VgsL_exp(TrigPosition-length(t_exp_LS)/4:TrigPosition+length(t_exp_LS)/4);
        VdsL_exp=VdsL_exp(TrigPosition-length(t_exp_LS)/4:TrigPosition+length(t_exp_LS)/4);
        IdL_exp=IdL_exp(TrigPosition-length(t_exp_LS)/4:TrigPosition+length(t_exp_LS)/4);
        t_exp_LS=t_exp_LS(TrigPosition-length(t_exp_LS)/4:TrigPosition+length(t_exp_LS)/4);

    end
    t_exp_HS=t_exp_HS-t_exp_HS(1);
    t_exp_LS=t_exp_LS-t_exp_LS(1);
 
    Dt_HS_LS=TrigAllignNEG(t_exp_HS,t_exp_LS, IdH_exp,IdL_exp+I_L, I_L/2);
    t_exp_LS=t_exp_LS-Dt_HS_LS;

    [Tdelay]=TrigAllignPOS(t, t_exp_HS,VdsH_sim, VdsH_exp, V_dd/2);
    t_sim=[0, t+Tdelay];

evalin('base',(['save([''../data_off_'',num2str(I_L),''A_'',num2str(R_g_H_ext),''Ohm.mat''], ', ...
    '''t_sim'', ''t_exp_HS'', ''t_exp_LS'', ',...
    '''VgsH_sim'', ''VgsH_exp'', ',...
    ' ''VdsH_sim'',''VdsH_exp'', ',...
    ' ''IdH_sim'',''IdH_exp'', ',...
    ' ''VgsL_sim'', ''VgsL_exp'', ',...
    ' ''VdsL_sim'', ''VdsL_exp'', ',...
    ' ''IdL_sim'', ''IdL_exp'', ',...
    ' ''I_L'', ''R_gH'', ''R_gL'');']));
clearvars -except I_L R_gH R_gL