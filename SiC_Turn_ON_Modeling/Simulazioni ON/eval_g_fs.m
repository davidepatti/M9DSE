g=0;
for i=2:length(V_gs)
    g(i)=(I_d(i)-I_d(i-1))/(V_gs(i)-V_gs(i-1));
end