function V_gg=read_Vgg(V_ggH,V_ggL, Fall_time, t)

V_gg=(V_ggL-V_ggH)/Fall_time*t(end)+V_ggH;

if (V_gg<V_ggL)
    V_gg=V_ggL;
end

end
