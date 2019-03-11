%% Initialization of the parameters

% Tempo di campionamento (al massimo 1e-10 altrimenti perde troppo tempo)
Ts=1e-10;

% Parameters
V_dd=800; % DC Voltage
V_gg_H=18; % Tensione alta per V_Gate
V_gg_L=-5; % Tensione bassa per V_Gate
V_th=4.5; % Tensione di soglia
Rise_time=52e-9; % Tempo di salite della V_Gate per passare da V_gg_L a V_gg_H
S=3.4; % Snappiness Factor

% Istante di tempo iniziale: "t" sarà il vettore contenente l'asse del tempo
t=0;

% Vettore contenente gli istanti temporali di passaggio da uno stage ad un altro
time=0;

% Errore a regime per la derivata: questo parametro permette di stabilire il valore della derivata di una variabile per il quale si assume che quella variabile sia arrivata già a regime
err_reg=0.01;

% Initial conditions
v_gsH=V_gg_L;
v_dsH=V_dd;
i_gH=0;
i_dH=0;
v_gsL=V_gg_L;
v_dsL=0;
i_gL=0;
i_dL=-I_L;

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

% Dichiarazione della varibile per la corrente di recupero
i_f=[0];
% Dichiarazione della variabile per la V_Gate
V_gg=[V_gg_L];
% Dichiarazione della variabile per eventuale Cross-Turn-ON
Flag_CrossTurnON=[];

%% Stage 1: v_gs1 increases

while (X(1,end)<V_th) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Rise_time,t)];
    % Aggiornamento delle matrici per la risoluzione dello stage
    A1=[0   0   1/(C_gsH+C_gdH)     0   0   0   0   0;
        0   0   0                   0   0   0   0   0;
        -1/L_A  0   -R_g_H/L_A      0   0   0   0   0;
        0   0   0   0   0   0   0   0;
        0   0   0   0   0   0   0   0;
        0   0   0   0   0   0   0   0;
        0   0   0   0   0   0   0   0;
        0   0   0   0   0   0   0   0];
    
    B1=[0;
        0;
        V_gg(end)/L_A;
        0;
        0;
        0;
        0;
        0];
    % Risoluzione dello satge
    [t,X]=solve_stage(t,X,A1,B1,Ts);
    % Aggiornamento delle derivate
    Xd=[Xd A1*X(:,end)+B1];
    % Aggiornamento della corrente di recupero
    i_f=[i_f 0];
end

% Aggiornamento del vettore "time" con l'istante temporale di uscita dallo
% Stage 1
time=[time t(end)];


%% Stage 2: i_d1 rise time I

while(X(4, end)<I_L) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Rise_time,t)];
    % Aggiornamento del g_fs
    g_fsH=read_gfs(X(1,end));
    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A2=[-m1*C_gdH*g_fsH  0   m1*(C_dsH+C_gdH)    m1*C_gdH    0   0   0   0;
        -m1*(C_gdH+C_gsH)*g_fsH  0   m1*C_gdH    m1*(C_gdH+C_gsH)    0   0   0   0; 
        -m2*(L_eq*L_B-L_sL_int^2)   m2*L_sH_int*L_B     -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B  -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B)  m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 1/(C_gsL+C_gdL) 0;
        0 0 0 0 0 0 0 0;
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B)  m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];

    B2=[m1*C_gdH*g_fsH*V_th;
        m1*(C_gdH+C_gsH)*g_fsH*V_th;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg(end)+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end));
        0;
        0;
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg(end)-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end))];
    
    % Risoluzione dello satge
    [t, X]=solve_stage(t, X, A2, B2, Ts);
    % Aggiornamento delle derivate
    Xd=[Xd A2*X(:,end)+B2];
    % Aggiornamento della corrente di recupero
    i_f=[i_f 0];
end

% Calcolo di d(i_d1)/dt e qundi della I_RR in corrispondenza di I_d1=I_L 
di_dt=Xd(4,end);
% I_RR=sqrt((2*Q_RR*di_dt)/(S+1));
I_RR=0; % provvisorio: Questo valore è stato cambiato manualmente per inserire il recupero del diodo (se 0 non c'è recupero)

% Aggiorno il vettore time con l'istante temporale di uscita dello stage 2
time=[time t(end)];


%% Stage 2.1
% Questo stage viene eseguito solo se I_RR è diverso da zero, quindi se
% viene inserito il recupero di corrente
while(X(4, end)<I_L+I_RR)  % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Rise_time,t)];
    % Aggiornamento del g_fs
    g_fsH=read_gfs(X(1,end));
    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A2=[-m1*C_gdH*g_fsH  0   m1*(C_dsH+C_gdH)    m1*C_gdH    0   0   0   0;
        -m1*(C_gdH+C_gsH)*g_fsH  0   m1*C_gdH    m1*(C_gdH+C_gsH)    0   0   0   0; 
        -m2*(L_eq*L_B-L_sL_int^2)   m2*L_sH_int*L_B     -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B  -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B)  m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 1/(C_gsL+C_gdL) 0;
        0 0 0 0 0 0 0 0;
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B)  m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];

    B2=[m1*C_gdH*g_fsH*V_th;
        m1*(C_gdH+C_gsH)*g_fsH*V_th;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg(end)+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end));
        0;
        0;
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg(end)-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end))];

    % Risoluzione dello satge
    [t, X]=solve_stage(t, X, A2, B2, Ts);
    % Aggiornamento delle derivate
    Xd=[Xd A2*X(:,end)+B2];
    % Aggiornamento della corrente di recupero
    i_f=[i_f X(8,end)];
end

% Salvo l'istante temporale di uscita dallo stage 2 che mi serve per il
% calcolo della i_f
t2=t(end);

% Aggiorno il vettore time con l'istante temporale di uscita dello stage 2
time=[time t(end)];

%% Stage 3:  v_ds2 rise time I

while(X(2, end)>0) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Rise_time,t)];
    % Aggiornamento del g_fs
    g_fsH=read_gfs(X(1,end));
    
    % Cross turn on monitoring
    if (X(5,end)>V_th)
        Flag_CrossTurnON=[Flag_CrossTurnON; t(end) X(5,end)];
    end

    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A3=[-m1*C_gdH*g_fsH  0   m1*(C_dsH+C_gdH)    m1*C_gdH    0   0   0   0;
        -m1*(C_gdH+C_gsH)*g_fsH  0   m1*C_gdH    m1*(C_gdH+C_gsH)    0   0   0   0; 
        -m2*(L_eq*L_B-L_sL_int^2)   m2*L_sH_int*L_B     -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B  -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B)  m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 m3*(C_dsL+C_gdL) m3*C_gdL;
        0 0 0 0 0 0 m3*C_gdL m3*(C_gsL+C_gdL);
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B)  m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];

    B3=[m1*C_gdH*g_fsH*V_th;
        m1*(C_gdH+C_gsH)*g_fsH*V_th;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg(end)+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end));
        -m3*C_gdL*i_f(end);
        -m3*(C_gsL+C_gdL)*i_f(end);
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg(end)-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end))];

        % Risoluzione dello satge
        [t, X]=solve_stage(t, X, A3, B3, Ts);
        % Aggiornamento delle derivate
        Xd=[Xd A3*X(:,end)+B3];
        % Aggiornamento della corrente di recupero
        if (I_RR-1/S*di_dt*(t(end)-t2)>0)
            i_f=[i_f I_RR-1/S*di_dt*(t(end)-t2)];
        else
            i_f=[i_f 0];
        end
end

% Aggiornamento del vettore con istante di uscita dallo stage 3
if (t(end)==time(end))
else
    time=[time t(end)];
end
   
%% Stage 4: v_ds2 rise time 2
while(((abs(X(6,end)-V_dd)>err_reg || abs(X(4,end)-I_L)>err_reg) || t(end)==time(end) )&& t(end)<3.5e-7) % Condizione di uscita dallo stage
    % Aggiornamento delle capacità parassite
    [C_gsH, C_dsH, C_gdH, C_gsL, C_dsL, C_gdL]=read_cap2(X(2,end), X(6,end));
    % Aggiornamento della V_Gate
    V_gg=[V_gg read_Vgg(V_gg_H, V_gg_L, Rise_time,t)];
    % Aggiornamento del g_fs
    g_fsH=read_gfs(X(1,end));
    
    % Cross turn on monitoring
    if (X(5,end)>V_th)
        Flag_CrossTurnON=[Flag_CrossTurnON; t(end) X(5,end)];
    end

    % Aggiornamento dei parametri per il calcolo delle matrici
    m1=1/((C_gdH+C_gsH)*(C_dsH+C_gdH)-C_gdH^2);
    m2=1/((L_A)*(L_B)*(L_eq)-(L_sL_int^2)*(L_A)-(L_sH_int^2)*(L_B));
    m3=1/((C_dsL+C_gdL)*(C_gsL+C_gdL)-C_gdL^2);
    % Aggiornamento delle matrici per la risoluzione dello stage
    A4=[0 0 1/(C_gsH+C_gdH) 0 0 0 0 0;
        0 0 0 0 0 0 0 0; 
        -m2*(L_eq*L_B-L_sL_int^2) m2*L_sH_int*L_B -m2*(L_B*L_eq-L_sL_int^2)*R_g_H    0   -m2*L_sL_int*L_sH_int   m2*L_sH_int*L_B -m2*L_sL_int*L_sH_int*R_g_L     0; 
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0;
        0 0 0 0 0 0 m3*(C_dsL+C_gdL) m3*C_gdL;
        0 0 0 0 0 0 m3*C_gdL m3*(C_gsL+C_gdL);
        -m2*L_sH_int*L_sL_int  m2*(L_A)*L_sL_int     -m2*L_sH_int*L_sL_int*R_g_H     0   -m2*((L_A)*(L_eq)-L_sH_int^2) m2*(L_A)*L_sL_int  -m2*((L_A)*(L_eq)-L_sH_int^2)*R_g_L    0;
        m2*L_sH_int*(L_B)  -m2*(L_A)*(L_B) m2*L_sH_int*(L_B)*R_g_H   0   m2*L_sL_int*(L_A) -m2*(L_A)*(L_B) m2*L_sL_int*(L_A)*R_g_L   0];

    B4=[0;
        0;
        m2*((L_B*L_eq-L_sL_int^2)*V_gg(end)+L_sL_int*L_sH_int*V_gg_L-L_sH_int*L_B*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end));
        -m3*C_gdL*i_f(end);
        -m3*(C_gsL+C_gdL)*i_f(end);
        m2*(((L_A)*(L_eq)-L_sH_int^2)*V_gg_L+L_sH_int*L_sL_int*V_gg(end)-(L_A)*L_sL_int*V_dd);
        m2*((L_A)*(L_B)*V_dd-L_sL_int*(L_A)*V_gg_L-L_sH_int*(L_B)*V_gg(end))];

        % Risoluzione dello satge
        [t, X]=solve_stage(t, X, A4, B4, Ts);
        % Aggiornamento delle derivate
        Xd=[Xd A4*X(:,end)+B4];
        % Aggiornamento della corrente di recupero
        if (I_RR-1/S*di_dt*(t(end)+Ts-t2)>0)
            i_f=[i_f I_RR-1/S*di_dt*(t(end)+Ts-t2)];
        else
            i_f=[i_f 0];
        end
end

% Aggiornamento del vettore con istante di uscita dallo stage 4
if (t(end)==time(end))
else
    time=[time t(end)];
end
%% Cross Turn ON Occurance
% Se il vettore Flag_CrossTurnON è vuoto vuol dire che non si sono
% verificate le condizioni per cui si dovrebbe riaccendere il dipsositivo
% passivo; in caso contrario questo modello potrebbe essere invalido 
if (size(Flag_CrossTurnON) ==[0 0])
else Flag_CrossTurnON
end
%%  Evaluation of external signals
% Aggiornamento per adattare le dimensioni dei vettori
V_gg=[V_gg V_gg_H];
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

%% Time allignment
% Allineamento degli assi temporali per simulate e sperimentali 
Time_Allignment;
% Ricostruzione vettore temporale di High Side
t_exp_HS=t_exp_LS+Dt_HS_LS;

% Ritardo totale per allineare le simulate 
T_delay=Dt_exp_sim+Dt_HS_LS;

% Ricostruzione dei vettori t e time per le forme d'onda simulate tenendo conto
% del ritardo

t_sim=[0,t+T_delay];
time_sim=time+T_delay;

% Definizione dell'istante massimo per i plot
T_max=t_exp_LS(end);
close all;

%% Saving
% Salvataggio delle forme d'onda
VgsH_sim=X_R(1,:);
VgsH_exp=wfm2read (['../test/on/Rgh=Rgl=',num2str(R_g_H_ext),'/LSatHSpas/wave/800V_ID=',num2str(I_L),'Aon_Ch3.wfm']);

VdsH_sim=X_R(2,:);
VdsH_exp=wfm2read (['../test/on/Rgh=Rgl=',num2str(R_g_H_ext),'/LSatHSpas/wave/800V_ID=',num2str(I_L),'Aon_Ch2.wfm']);

IdH_sim=X_R(4,:);
IdH_exp=wfm2read (['../test/on/Rgh=Rgl=',num2str(R_g_H_ext),'/LSatHSpas/wave/800V_ID=',num2str(I_L),'Aon_Ch4.wfm']);

VgsL_sim=X_R(5,:);
VgsL_exp=1+wfm2read (['../test/on/Rgh=Rgl=',num2str(R_g_H_ext),'/LSpasHSat/wave/800V_ID=',num2str(I_L),'Aon_Ch3.wfm']);

VdsL_sim=X_R(6,:);
VdsL_exp=wfm2read (['../test/on/Rgh=Rgl=',num2str(R_g_H_ext),'/LSpasHSat/wave/800V_ID=',num2str(I_L),'Aon_Ch2.wfm']);

IdL_sim=X_R(8,:);
IdL_exp=wfm2read (['../test/on/Rgh=Rgl=',num2str(R_g_H_ext),'/LSpasHSat/wave/800V_ID=',num2str(I_L),'Aon_Ch4.wfm']);

evalin('base',(['save([''../data_on_'',num2str(I_L),''A_'',num2str(R_g_H_ext),''Ohm.mat''], ', ...
    '''t_sim'', ''t_exp_HS'', ''t_exp_LS'', ',...
    '''VgsH_sim'', ''VgsH_exp'', ',...
    ' ''VdsH_sim'',''VdsH_exp'', ',...
    ' ''IdH_sim'',''IdH_exp'', ',...
    ' ''VgsL_sim'', ''VgsL_exp'', ',...
    ' ''VdsL_sim'', ''VdsL_exp'', ',...
    ' ''IdL_sim'', ''IdL_exp'', ',...
    ' ''I_L'', ''R_g'')']));
clearvars -except I_L R_g