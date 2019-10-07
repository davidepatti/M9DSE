function start_server()
mlock
persistent t5
while(1)
    
    t5 = tcpip('0.0.0.0', 30000, 'NetworkRole', 'server');
    fopen(t5);
    ls %Simula
    fclose(t5);
end
end
start_server()
