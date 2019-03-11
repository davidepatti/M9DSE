function V_gg=read_Vgg(V_ggH,V_ggL, Rise_time, t)

V_gg=(V_ggH-V_ggL)/Rise_time*t(end)+V_ggL;

if (V_gg>V_ggH)
    V_gg=V_ggH;
end

end
