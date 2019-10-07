function [Tdelay]=TrigAllignNEG(t1,t2,x1,x2, TrigLev)
i=1;    
while (x1(i)>TrigLev)
    i=i+1;
end
j=1;    
while (x2(j)>TrigLev)
    j=j+1;
end

Tdelay=t2(j)-t1(i);
end