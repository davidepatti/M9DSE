% if exist('t','var') == 0
%     show_pareto
%     close all force
% end

if exist('index','var') == 0
    index = 1;
end

disp("Using Pareto Set element with index " + index)

for paramName = t.Properties.VariableNames
    command = paramName{:} + " = t{" + index + ", '" + paramName{:} + "'}";
    eval(command);
end