%clc
%clear
%close all force

if exist('pareto_exp_filename','var') == 0
    disp("Please set the 'pareto_exp_filename' variable ...");
    pareto_folder = "..";
    tools_folder = pwd;

    d = dir(pareto_folder + filesep + "*pareto.exp");
    [~, index] = max([d.datenum]);
    pareto_exp_filename = fullfile(d(index).folder, d(index).name);
    disp("Trying with '" + pareto_exp_filename + "'");
end

t = readtable(pareto_exp_filename,'FileType','text');
t.Properties.VariableNames{1} = 'VGS';

while (isnan(t{1,1})) t(1,:)=[]; end

f_xyz = t{:,1:3};

%%%%%%%%%%%
% OPTION 1
%
paretoFig = figure('Name','Pareto Front');
set(0,'DefaultFigureWindowStyle','docked');

buttonH = uicontrol(                ...
    'Parent',   paretoFig,          ...
    'Style',    'pushbutton',       ...
    'String',   'View Data',        ...
    'Units',    'normalized',       ...
    'Position', [0.0 0.0 0.2 0.075],...
    'Visible',  'on',               ...
    'CallBack', {@show_data, pareto_exp_filename} ...
);

s = scatter3(f_xyz(:,1),f_xyz(:,2),f_xyz(:,3),'b.');
s.DataTipTemplate.DataTipRows(1).Label = "VGS error";
s.DataTipTemplate.DataTipRows(2).Label = "VDS error";
s.DataTipTemplate.DataTipRows(3).Label = "ID error";

s.DataTipTemplate.DataTipRows(4) = dataTipTextRow("Index",1:size(f_xyz,1));

title('Pareto Front');
xlabel('VGS error');
ylabel('VDS error');
zlabel('ID error');

% %%%%%%%%%%%
% % OPTION 2
% %
% % To see the Pareto front as a surface, create a scattered interpolant.
% F = scatteredInterpolant(f(:,1),f(:,2),f(:,3),'linear','none');
% 
% % To plot the resulting surface, create a mesh in x-y space from the smallest to the largest values. Then plot the interpolated surface.
% sgr = linspace(min(f(:,1)),max(f(:,1)));
% ygr = linspace(min(f(:,2)),max(f(:,2)));
% [XX,YY] = meshgrid(sgr,ygr);
% ZZ = F(XX,YY);
% 
% paretoSurfFig = figure('Name','Pareto Front - Surf');
% surf(XX,YY,ZZ,'LineStyle','none');
% 
% hold on
% scatter3(f(:,1),f(:,2),f(:,3),'k.');
% 
% title('Pareto Front - Surf');
% xlabel('VGS error');
% ylabel('VDS error');
% zlabel('ID error');

paramsValuesHistogram(t, '../space.conf');

function show_data(PushButton, EventData, pareto_exp_filename)
    openParetoFig(pareto_exp_filename, ancestor(PushButton, 'figure'))
end

function paramsValuesHistogram(paretoTable, dseConfig)
    %% Open DSE configuration file
    space_conf = readtable(dseConfig,'FileType','text');

    %% Remove 'DEFAULTS'
    idxNaN = find(isnan(space_conf{:,3}));
    space_conf(idxNaN,:) = [];

    %% Remove last column (zeros)
    space_conf(:, size(space_conf,2)) = [];

    figure('Name','Frequency of params values');

    plotsXrow = 3;
    for i = 1:size(space_conf, 1)
        paramName = char(space_conf{i,1});
        subplot(ceil(size(space_conf, 1)/plotsXrow), plotsXrow, i);
        param_minMax = [min(space_conf{i,2:end}'), max(space_conf{i,2:end}')];
        histogram(extractfield(table2struct(paretoTable), paramName), 'BinWidth', (param_minMax(2)-param_minMax(1))/size(space_conf, 2)/2)
        xlim(param_minMax);
        xlabel(paramName, 'FontSize', 10, 'Interpreter', 'none');
        ylabel('#', 'FontSize', 10);
    end
end
