function [C_gs1, C_ds1, C_gd1, C_gs2, C_ds2, C_gd2]=read_cap2(V_x1, V_x2)
load('Capacitance_tables_r.mat');
% V_x1=100;
% V_x2=200;

    i=1;
    if (V_x1<=V_ds(i))
        V_x1=V_ds(1);
        C_gs1=C_gs(1);
        C_ds1=C_ds(1);
        C_gd1=C_gd(1);
    else
        while (V_x1>V_ds(i))
            C_gs1=C_gs(i);
            C_ds1=C_ds(i);
            C_gd1=C_gd(i);
            i=i+1;
            if (i>length(V_ds)) break;
            end
        end
    end;
    
    i=1;
    if (V_x2<=V_ds(i))
        V_x2=V_ds(1);
        C_gs2=C_gs(1);
        C_ds2=C_ds(1);
        C_gd2=C_gd(1);
    else
        while (V_x2>V_ds(i))
            C_gs2=C_gs(i);
            C_ds2=C_ds(i);
            C_gd2=C_gd(i);
            i=i+1;
            if (i>length(V_ds)) break;
            end
        end
    end;
clear i;

% C_gs1=C_gs1*1.2;
% C_gs2=C_gs2*1.2;
% C_ds1=C_ds1*1.2;
% C_ds2=C_ds2*1.2;
% C_gd1=C_gd1*1.2;
% C_gd2=C_gd2*1.2;
