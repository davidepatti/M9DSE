function [my_g_fs]=read_gfs(my_V_gs)
i=1;
load 'g_fs_tables.mat'

    if (my_V_gs<=V_gs(i))
        my_g_fs=g_fs(i);
    else
       while (my_V_gs>V_gs(i))
            my_g_fs=g_fs(i);
            i=i+1;
            if (i>length(V_gs)) break;
            end
        end
    end;
 clear i;   

    