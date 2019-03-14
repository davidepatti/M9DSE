#include "explorer.h"
#include "common.h"
//********************************************************************
// N Random  explorations of configuration space
//*******************************************************************
void Explorer::start_RAND(int random_size)
{

	current_algo = "RAND";
	vector<Configuration> random_space;

	int valid = 0;

	reset_sim_counter();
	Exploration_stats stats;

	int my_ID = get_mpi_rank();

	string header = "["+current_algo+"] ";
	string filename = "RAND_";
	string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
	string message = header+"Building random space for " + to_string(random_size) + " simulations...";
	write_to_log(my_ID,logfile,message);

	for(int i=0;i<random_size;i++)
	{
		//model_inverter.num_clusters.set_random();

		model_inverter.L_d_int.set_random();
		model_inverter.L_s_int.set_random();
		model_inverter.L_g_int.set_random();
		model_inverter.L_d_pin.set_random();
		model_inverter.L_s_pin.set_random();
		model_inverter.L_g_pin.set_random();
		model_inverter.L_dH_ext.set_random();
		model_inverter.L_sH_ext.set_random();
		model_inverter.L_gH_ext.set_random();
		model_inverter.L_dL_ext.set_random();
		model_inverter.L_sL_ext.set_random();
		model_inverter.L_gL_ext.set_random();
		model_inverter.L_Hwire.set_random();
		model_inverter.L_Lwire.set_random();

		Configuration temp_conf = create_configuration(model_inverter);

		if (temp_conf.is_feasible())
		{
			valid++;
			random_space.push_back(temp_conf);
		}
	}

	stats.space_size = get_space_size();
	stats.start_time = time(NULL);

	message = header+ "Valid configurations:" + to_string(valid) + " of "+to_string(random_size)+" requested";
	write_to_log(my_ID,logfile,message);

	vector<Simulation> rand_sims = simulate_space(random_space);
	vector<Simulation> pareto_set = get_pareto(rand_sims);

	save_simulations(rand_sims,filename+".exp");
	save_simulations(pareto_set,filename+".pareto.exp");

	stats.end_time = time(NULL);
	stats.n_sim = get_sim_counter();
	save_stats(stats,filename+".stat");

	write_to_log(my_ID,logfile,"End of RANDOM simulation");
}
