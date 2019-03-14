
while(1)
    disp('Waiting for command:   nc 127.0.0.1 30000')
    tcpip_conn = tcpip('127.0.0.1', 30000, 'NetworkRole', 'server');
    save tcpip.mat tcpip_conn
    fopen(tcpip_conn);
    % tcpip_msg = fgetl(tcpip_conn)

    run("Simula")

    load tcpip.mat tcpip_conn
    fclose(tcpip_conn);
end