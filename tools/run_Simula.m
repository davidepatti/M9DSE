%% Clear environment
clc
clear
close all force


%% Configuration
simulation_folder = ".." + filesep + "SiC_Turn_ON_Modeling";
pareto_folder = "..";
tools_folder = pwd;

d = dir(pareto_folder + filesep + "*pareto.exp");
[~, index] = max([d.datenum]);
pareto_exp_filename = fullfile(d(index).folder, d(index).name);
pareto_exp_shortname = d(index).name;

fig_out_basedir = "." + filesep + "Simula_results" + filesep + pareto_exp_shortname;
fig_out_name = "SiC_Turn_ON_Modeling_" + pareto_exp_shortname;
diary_filename = 'run_Simula.log';


%% Setup to execute run_Simula
if (exist(simulation_folder + filesep + "Simula.m") ~= 2)
    error("Check 'simulation folder': '" + simulation_folder + filesep + "Simula.m' not found!");
end

mkdir(fig_out_basedir);
copyfile('params_rS.m', simulation_folder + filesep + "params.m");
diary(diary_filename);
 

%% Load Pareto Set
show_pareto


%% Store Configuration with Pareto set to prevent unwanted clear commands
save(simulation_folder + filesep + 'conf', 'simulation_folder', 'tools_folder', 'pareto_exp_filename', 'pareto_exp_shortname', 'fig_out_basedir', 'fig_out_name', 't');


%% Main loop
for index = 1:size(t, 1)
    save(simulation_folder + filesep + 'conf', 'index', '-append');
    close all force
    cd(simulation_folder)
    Simula
    load('conf');
    cd(tools_folder)
    index_str = num2str(index,"%0" + ceil(log10(size(t,1))) + "d")
    saveas(gcf, fig_out_basedir + filesep + fig_out_name + "_" + index_str + ".fig", 'fig');
end

diary off