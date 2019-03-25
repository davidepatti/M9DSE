#include "explorer.h"
#include "common.h"

#ifdef M9DSE_MPI
#include "mpi.h"
#endif

#include <ctime>
//#include <values.h>

////////////////////////////////////////////////////////////////////////////


vector<Simulation> Explorer::simulate_loop(const vector<Configuration>& space)
{
	Simulation current_sim;
	vector<Simulation> simulations;

	static int n_simulate_space_call = 0;
	static int n_exploration_executed = 0;

	string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
	string matlab_params_file = get_base_dir()+string(MATLAB_WORKSPACE)+string(MATLAB_PARAMS_FILE);

	int myid = get_mpi_rank();

	write_to_log(myid,logfile,"Starting simulate_loop (space size: "+to_string(space.size())+")");

	for (unsigned int i = 0;i< space.size();i++)
	{
		cout << endl;
		cout << "\n -------> M9DSE   [ simulation n." << i+1 << " / " << space.size() << " ]";
		cout << endl;

		this->model_inverter.set_config(space[i]);
		current_sim.config = space[i];

        matlabInterface->save_model_config(model_inverter,matlab_params_file);
        matlabInterface->execute_sim(); //db

		dyn_stats = matlabInterface->get_dynamic_stats();

		estimate = estimator.get_estimate(dyn_stats);
		current_sim.avg_err_VGS = estimate.avg_err_VGS;
		current_sim.avg_err_VDS = estimate.avg_err_VDS;
		current_sim.avg_err_ID = estimate.avg_err_ID;
		current_sim.simulated = true;//do_simulation;

        simulations.push_back(current_sim);
	}

	// -------------------------------------------------------------------
	//  when doing simulation some interesting info can be optionally saved
	if (Options.save_spaces)
	{
		n_simulate_space_call++;
		if (get_sim_counter()==0)
		{
			n_simulate_space_call=1;
			n_exploration_executed++;
		}
		// M9_space_EXP_2.12, 12th explorated space of 2nd exploration algorithm
		char name[40];
		sprintf(name,"_simulatedspace_%d",n_simulate_space_call);
		string filename = current_algo+"_"+string(name);
		saveConfigurations(space, filename);
	}

	if (Options.save_estimation) // detailed and verbose estimator report
	{
		char temp[10];
		string filename= current_algo+"_"+"."+string(temp)+".est";
		save_estimation_file(dyn_stats,estimate, filename);	//db
	}

	if (Options.save_objectives_details) //
	{
		string filename= current_algo+"_"+".details";
		saveObjectivesDetails(dyn_stats, current_sim.config, filename);
	}
	// -------------------------------------------------------------------

	write_to_log(myid,logfile,"Finished simulate_loop (space size: "+to_string(space.size())+")");
	return simulations;
}


vector<Simulation> Explorer::simulate_space(const vector<Configuration>& space)
{

	int myrank = get_mpi_rank(); // id of the current M9DSE process
	int mysize = get_mpi_size(); // # of M9DSE processes running

	string logfile = get_base_dir()+string(M9DSE_LOG_FILE);

#ifdef M9DSE_MPI
	MPI_Status status;
#endif

	vector<Simulation> simulations;
	vector<Configuration> space2;

	//  main exploration loop
	// ********************************************************

	int communicator[N_PARAMS]; //db
	int counter = 0;
	int counter2 = 0;

	int committed = 0;
	int rest = 0;

	committed = space.size()/mysize;
	rest = space.size() - (committed*mysize);
	vector<int> to_sim;

	for (int p = 0; p < mysize; p++) {

		if (p < rest)
			to_sim.push_back(committed+1);
		else
			to_sim.push_back(committed);
	}

#ifdef M9DSE_MPI
	counter2 = to_sim[0];

    for (int p = 1; p < mysize; p++) {

	counter = 0;
	for (int s = counter2; s < counter2+to_sim[p]; s++) {
	    space2.push_back(space[s]);
	}
	counter = space2.size();
	MPI_Send(&counter, 1, MPI_INT, p, 98, MPI_COMM_WORLD);

	int bench_len = Options.benchmark.length() + 1; // null character terminated string
	MPI_Send(&bench_len, 1, MPI_INT, p, 95, MPI_COMM_WORLD);
	char* bench_cstr = new char[bench_len];
	strcpy(bench_cstr, Options.benchmark.c_str());
	MPI_Send(bench_cstr, bench_len, MPI_CHAR, p, 96, MPI_COMM_WORLD);

	for (int s = 0; s < counter; s++) {
	    communicator[0] = space2[s].L1D_block;
	    communicator[1] = space2[s].L1D_size;
	    communicator[2] = space2[s].L1D_assoc;
	    communicator[3] = space2[s].L1I_block;
	    communicator[4] = space2[s].L1I_size;
	    communicator[5] = space2[s].L1I_assoc;
	    communicator[6] = space2[s].L2U_block;
	    communicator[7] = space2[s].L2U_size;
	    communicator[8] = space2[s].L2U_assoc;
	    communicator[9] = space2[s].L_s_int;
	    communicator[10] = space2[s].L_g_int;
	    communicator[11] = space2[s].L_s_pin;
	    communicator[12] = space2[s].L_d_pin;
	    communicator[13] = space2[s].L_g_pin;
	    communicator[14] = space2[s].L_dH_ext;
	    communicator[15] = space2[s].L_gH_ext;
	    communicator[16] = space2[s].L_sH_ext;
	    communicator[17] = space2[s].L_dL_ext;
	    communicator[18] = space2[s].L_d_int;
	    communicator[19] = space2[s].tcc_region;	//db
	    communicator[20] = space2[s].max_unroll_allowed; //db
	    communicator[21] = space2[s].regroup_only;	//db
	    communicator[22] = space2[s].do_classic_opti;	//db
	    communicator[23] = space2[s].do_prepass_scalar_scheduling;	//db
	    communicator[24] = space2[s].do_postpass_scalar_scheduling;	//db
	    communicator[25] = space2[s].do_modulo_scheduling;	//db
	    communicator[26] = space2[s].memvr_profiled;	//db
	    MPI_Send(communicator, N_PARAMS, MPI_INT, p, 99, MPI_COMM_WORLD);
	}
	space2.clear();
	counter2 += counter;
    }
#endif

	for (int s = 0; s < to_sim[0]; s++) {
		space2.push_back(space[s]);
	}
	counter = space2.size();

	// Main simulation loop
	simulations = simulate_loop(space2);

	space2.clear();

#ifdef M9DSE_MPI
	write_to_log(myrank,logfile,"Parallel Execution of "+to_string(counter)+" simulations completed");

    double comms[4];
    counter2 = to_sim[0];
    Simulation current_sim;

    for(int p = 1; p<mysize; p++) 
    {
	for (int s = counter2; s < counter2+to_sim[p]; s++) 
	    space2.push_back(space[s]); 

	if (space2.size() > 0) 
	    MPI_Recv(&counter, 1, MPI_INT, p, 97, MPI_COMM_WORLD, &status);
	else counter = 0;

	for(int s = 0; s < counter; s++) {
	    MPI_Recv(comms, 4, MPI_DOUBLE, p, 99, MPI_COMM_WORLD, &status); 
	    current_sim.config = space2[s];
	    current_sim.avg_err_ID = comms[0];
	    current_sim.avg_err_VDS = comms[1];
	    current_sim.avg_err_VGS = comms[2];
	    current_sim.clock_freq = comms[3]; //dovrebbe essere sempre la stessa...
	    current_sim.simulated = true;//do_simulation;

	    simulations.push_back(current_sim);

	    /* TODO: re-enable
	    if (Options.approx_settings.enabled>0) 
	    {
		model_inverter.set_config(space2[s]);
		mem_hierarchy.set_config(space2[s]); 
		compiler.set_config(space2[s]);	//db
		function_approx->Learn(space2[s],current_sim,model_inverter,mem_hierarchy);
	    }
	    */
	}
	space2.clear();
	counter2 += counter;
    }
#else
	write_to_log(myrank,logfile,"Execution of "+to_string(counter)+" simulations completed");
#endif

	sim_counter+=simulations.size();

	// update current simulated space and benchmark
	previous_simulations.clear();
	append_simulations(previous_simulations,simulations);

	return simulations;
}


#ifdef M9DSE_MPI
int Explorer::simulate_space()
{
    // questo medoto viene lanciato sugli altri processori e si interfaccia con la simulate space classica 

    vector<Simulation> simulations;
    vector<Configuration> space;
    
    int communicator[N_PARAMS];  //db
    Configuration tmp;
    MPI_Status status;
    int counter = 0;
    int myrank = get_mpi_rank();
    string logfile = get_base_dir()+string(M9DSE_LOG_FILE);

    MPI_Recv(&counter, 1, MPI_INT, 0, 98, MPI_COMM_WORLD, &status);
    
    if (counter == 0) 
    	return (0);

    int bench_len;
    MPI_Recv(&bench_len,1,MPI_INT,0,95,MPI_COMM_WORLD,&status);
    char bench_cstr[bench_len];
    MPI_Recv(bench_cstr,bench_len,MPI_CHAR,0,96,MPI_COMM_WORLD,&status);
    string bench(bench_cstr);
    Options.benchmark = bench;
    matlabInterface->set_benchmark(Options.benchmark);

    for(int i = 0; i<counter; i++) {
    	MPI_Recv(communicator,N_PARAMS,MPI_INT,0,99,MPI_COMM_WORLD,&status); //db
        tmp.L1D_block = communicator[0];
	tmp.L1D_size = communicator[1];
	tmp.L1D_assoc = communicator[2];
	tmp.L1I_block = communicator[3];
	tmp.L1I_size = communicator[4];
	tmp.L1I_assoc = communicator[5];
	tmp.L2U_block = communicator[6];
	tmp.L2U_size = communicator[7];
	tmp.L2U_assoc = communicator[8];
	tmp.L_s_int = communicator[9];
	tmp.L_g_int = communicator[10];
	tmp.L_s_pin = communicator[11];
	tmp.L_d_pin = communicator[12];
  	tmp.L_g_pin = communicator[13];
        tmp.L_dH_ext = communicator[14];
        tmp.L_gH_ext = communicator[15];
        tmp.L_sH_ext = communicator[16];
        tmp.L_dL_ext = communicator[17];
        tmp.L_d_int = communicator[18];
	tmp.tcc_region = communicator[19];	//db
        tmp.max_unroll_allowed = communicator[20]; //db
	tmp.regroup_only = communicator[21];	//db
	tmp.do_classic_opti = communicator[22];	//db
	tmp.do_prepass_scalar_scheduling = communicator[23];	//db
	tmp.do_postpass_scalar_scheduling = communicator[24];	//db
	tmp.do_modulo_scheduling = communicator[25];	//db
	tmp.memvr_profiled = communicator[26];	//db
 	space.push_back(tmp);
    }

    //  main exploration loop
    // *********************************************************

    simulations = simulate_loop(space);

    write_to_log(myrank,logfile,"Parallel Execution of "+to_string(counter)+" simulations completed");
	
    //MPI_Finalize();
    double comms[4];
    MPI_Send(&counter,1,MPI_INT,0,97, MPI_COMM_WORLD);
    for (int i=0; i<counter; i++) {
    	comms[0] = simulations[i].avg_err_ID;
	comms[1] = simulations[i].avg_err_VDS;
	comms[2] = simulations[i].avg_err_VGS;
    	comms[3] = simulations[i].clock_freq;
	MPI_Send(comms,4,MPI_DOUBLE,0,99, MPI_COMM_WORLD);
    }
    
    return (1);
}
#endif
