function [t_final, X]=solve_stage(t_start, X, A, B, Ts)
 X=[X inv(eye(length(A))-Ts*A)*(X(:,end)+Ts*B)];
 t_final=[t_start t_start(end)+Ts];