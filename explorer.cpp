
#include "explorer.h"
#include "common.h"
#ifdef EPIC_MPI
#include "mpi.h"
#endif


//********************************************************************
Explorer::Explorer(MatlabInterface * ti)
{
    this->anInterface = ti;
    // when simulation is started these values will be calculated
    // to be more realistic

    // by default objectives include avg_err_vds and average power
    Options.objective_id = false;
    Options.objective_power = true;
    Options.objective_energy = false;
    Options.objective_vds = true;

    Options.save_spaces = false;
    Options.save_estimation = false;

    Options.save_objectives_details = false;
    force_simulation = false;
    function_approx = NULL;
    base_dir = string(getenv(BASE_DIR));

    current_space = "NOT_SET";
    string space_dir = get_base_dir()+"/SUBSPACES/";
    string space_file = space_dir+"current.sub";

    char target_space_file[140];
    int l = readlink(space_file.c_str(),target_space_file,140);
    if (l!=-1)
    {
        load_space_file(space_file);
        target_space_file[l] = '\0';
        set_space_name(target_space_file);
    }
    else
    {
        cout << "\nError while setting subspace " << target_space_file << ". Check that symbolic link";
        cout << "\n'current.sub' in " << space_dir << " it's properly set to point to a valid subspace file!";
    }
}

//********************************************************************

Explorer::~Explorer()
{
    string logfile = get_base_dir()+string(EE_LOG_PATH);
    write_to_log(get_mpi_rank(),logfile,"Destroying explorer class");
}


//********************************************************************
void Explorer::set_options(const struct UserSettings& user_settings)
{
    Options = user_settings;

    // Number of objectives
    n_obj = 0;

    if (Options.objective_power) n_obj++;
    if (Options.objective_vds) n_obj++;
    if (Options.objective_id) n_obj++;
    if (Options.objective_energy) n_obj++;

}

////////////////////////////////////////////////////////////////////////////
// The following  are utility functions used by exploring functions
////////////////////////////////////////////////////////////////////////////
//********************************************************************




Configuration Explorer::create_configuration(const ModelInverter& p,const Mem_hierarchy& mem, const Compiler& comp) //db
{
    Configuration conf;

    // TODO M9DSE - add correct parameters of config
    conf.gpr_static_size = processor.gpr_static_size.get_val();
    conf.fpr_static_size = processor.fpr_static_size.get_val();
    conf.pr_static_size = processor.pr_static_size.get_val();
    conf.cr_static_size = processor.cr_static_size.get_val();
    conf.btr_static_size = processor.btr_static_size.get_val();

    return conf;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration() const
{

    Configuration default_config;

    // TODO M9DSE - add correct parameters of config
    default_config.gpr_static_size = processor.gpr_static_size.get_default();
    default_config.fpr_static_size = processor.fpr_static_size.get_default();
    default_config.pr_static_size = processor.pr_static_size.get_default();

    return default_config;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration(const Space_mask& mask,const Configuration& base) const
{
    Configuration config = create_configuration();

    // TODO M9DSE - add correct parameters of config

    if (mask.L1D_size) config.L1D_size=base.L1D_size;
    if (mask.L1D_block) config.L1D_block=base.L1D_block;
    if (mask.L1D_assoc) config.L1D_assoc=base.L1D_assoc;
    return config;
}
////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::negate_mask(Space_mask mask)
{
    vector<bool> boolean_mask = get_boolean_mask(mask);

    for (unsigned int i=0;i<boolean_mask.size();i++) boolean_mask[i]=!(boolean_mask[i]);

    return create_space_mask(boolean_mask);
}

//********************************************************************
// Remove dominated simulations using the appropriate get_pareto(...)
// function, depending on the objectives selected in Options
void Explorer::remove_dominated_simulations(vector<Simulation>& sims)
{
    vector<Simulation> pareto_set = get_pareto(sims);
    sims.clear();
    append_simulations(sims,pareto_set);
}


// ********************************************************************
//  Euclidean distance between objective function values
// ********************************************************************
double Explorer::distance(const Simulation& s1, const Simulation& s2)
{
    if ( (Options.objective_id) && (Options.objective_power) && (Options.objective_vds) )
        return sqrt(pow(s1.avg_err_id-s2.avg_err_id,2)+ pow(s1.avg_err_vgs-s2.avg_err_vgs,2) + pow(double(s1.avg_err_vds-s2.avg_err_vds),2) );

    if ( (Options.objective_power) && (Options.objective_vds) )
        return sqrt(pow(s1.avg_err_vgs-s2.avg_err_vgs,2) + pow(double(s1.avg_err_vds-s2.avg_err_vds),2) );

    if ( (Options.objective_energy) && (Options.objective_vds) )
        return sqrt(pow(s1.avg_err_vgs-s2.avg_err_vgs,2) + pow(double(s1.avg_err_vds-s2.avg_err_vds),2) );

    if ( (Options.objective_id) && (Options.objective_power) )
        return sqrt(pow(s1.avg_err_vgs-s2.avg_err_vgs,2) + pow(double(s1.avg_err_id-s2.avg_err_id),2) );

    if ( (Options.objective_id) && (Options.objective_vds) )
        return sqrt(pow(s1.avg_err_id-s2.avg_err_id,2) + pow(double(s1.avg_err_vds-s2.avg_err_vds),2) );

    cout << "\n\n The selected combination of objectives is not supported from";
    cout << "\n get_distance(...) function in explorer.cpp ";
    exit(0);

    return 0;
}


//*********************************************************************
//    Ehaustive exploration of all parameters 
// ********************************************************************
void Explorer::start_EXHA()
{
    current_algo = "EXHA";
    Exploration_stats stats;
    reset_sim_counter();
    stats.space_size = get_space_size();
    stats.start_time=time(NULL);

    Space_mask mask = get_space_mask(SET_ALL);

    vector<Configuration> space = build_space(mask);

    // all parameters are explored in all their values 
    vector<Simulation> simulations = simulate_space(space);
    vector<Simulation> pareto_set = get_pareto(simulations);

    string filename = Options.benchmark+"_EXHA_"+current_space;
    save_simulations(simulations,filename+".exp");
    save_simulations(pareto_set,filename+".pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");
}


//**************************************************************

vector<Simulation> Explorer::normalize(const vector<Simulation>& sims)
{
    vector<Simulation> sorted1 = sort_by_vds(sims);
    vector<Simulation> sorted2 = sort_by_vgs(sims);
    vector<Simulation> sorted3 = sort_by_area(sims);

    double min_exec_time = sorted1[0].avg_err_vds;
    double max_exec_time = sorted1[sorted1.size()-1].avg_err_vds;

    double min_energy = sorted2[0].avg_err_vgs;
    double max_energy = sorted2[sorted2.size()-1].avg_err_vgs;

    double min_area = sorted3[0].avg_err_id;
    double max_area = sorted3[sorted3.size()-1].avg_err_id;

    vector<Simulation> normalized;

    for (unsigned int i=0;i<sorted1.size();i++)
    {
        Simulation temp_sim = sims[i];

        temp_sim.avg_err_vgs = (temp_sim.avg_err_vgs)/(max_energy);
        temp_sim.avg_err_vds = (temp_sim.avg_err_vds)/(max_exec_time);
        temp_sim.avg_err_id   = (temp_sim.avg_err_id)/(max_area);

        normalized.push_back(temp_sim);
    }
    return normalized;
}

//**************************************************************
// from min product to max product
vector<Simulation> Explorer::sort_by_vgsvds_product(vector<Simulation> sims)
{
    int min_index;
    double min_product;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_product = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if ( (sims[i].avg_err_vds)*(sims[i].avg_err_vgs)  <= min_product)
            {
                //cout << "\nDEBUG:(i="<<i<< ") " << sims[i].avg_err_vds << " <= " << min_exec_time;
                min_product= sims[i].avg_err_vds * sims[i].avg_err_vgs;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);
        sims.erase(sims.begin()+min_index);
    }
    return temp;
}


vector<Simulation> Explorer::sort_by_vds(vector<Simulation> sims)
{
    int min_index;
    double min_exec_time;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_exec_time = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if (sims[i].avg_err_vds<=min_exec_time)
            {
                min_exec_time=sims[i].avg_err_vds;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);
        sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << EE_TAG << "DEBUG temp(ordinato): " << temp[i].avg_err_vds;
#endif

    return temp;
}

//********************************************************************
vector<Simulation> Explorer::sort_by_vgs(vector<Simulation> sims)
{
    int min_index;
    double min_energy;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_energy = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if (sims[i].avg_err_vgs<=min_energy)
            {
                min_energy=sims[i].avg_err_vgs;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);

        sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << EE_TAG << "DEBUG (orderer E): " << temp[i].avg_err_vgs;
#endif
    return temp;
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::sort_by_area(vector<Simulation> sims)
{
    int min_index;
    double min_area;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_area = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if (sims[i].avg_err_id<=min_area)
            {
                min_area=sims[i].avg_err_id;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);
        sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << EE_TAG << "DEBUG (orderer Area): " << temp[i].avg_err_id;
#endif

    return temp;
}

/////////////////////////////////////////////////////////////////////////////
bool isDominated(Simulation sim, const vector<Simulation>& simulations)
{

    for(int i=0;i<simulations.size();++i)
    {
        if ((sim.avg_err_vgs>=simulations[i].avg_err_vgs) && (sim.avg_err_vds>=simulations[i].avg_err_vds))
            return (true);
    }

    return (false);

}
// wrap function which determines the right pareto function to be 
// called 
vector<Simulation> Explorer::get_pareto(const vector<Simulation>& simulations)
{
    if ( (Options.objective_id) && (Options.objective_power) && (Options.objective_vds) )
        return get_pareto3d(simulations);

    if ( (Options.objective_power) && (Options.objective_vds) )
        return get_pareto_CyclesPower(simulations);

    if ( (Options.objective_energy) && (Options.objective_vds) )
        return get_pareto_CyclesPower(simulations);

    if ( (Options.objective_id) && (Options.objective_power) )
        return get_pareto_AreaPower(simulations);

    if ( (Options.objective_id) && (Options.objective_vds) )
        return get_pareto_AreaCycles(simulations);
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_CyclesPower(const vector<Simulation>& simulations)
{
    double min_e = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_vds(simulations);

    while (sorted.size()>0)
    {
        if (sorted[0].avg_err_vgs<=min_e)
        {
            if ( (pareto_set.size()>0) && (pareto_set.back().avg_err_vds==sorted[0].avg_err_vds))
                pareto_set.pop_back();

            min_e = sorted[0].avg_err_vgs;
            pareto_set.push_back(sorted[0]);
        }
        sorted.erase(sorted.begin());
    }
    return pareto_set;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_AreaCycles(const vector<Simulation>& simulations)
{
    double min_area = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_vds(simulations);

    while (sorted.size()>0)
    {
        if (sorted[0].avg_err_id<=min_area)
        {
            if ( (pareto_set.size()>0) && (pareto_set.back().avg_err_vds==sorted[0].avg_err_vds))
                pareto_set.pop_back();

            min_area = sorted[0].avg_err_id;
            pareto_set.push_back(sorted[0]);
        }
        sorted.erase(sorted.begin());
    }
    return pareto_set;
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_AreaPower(const vector<Simulation>& simulations)
{
    double min_area = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_vgs(simulations);

    while (sorted.size()>0)
    {
        if (sorted[0].avg_err_id<=min_area)
        {
            if ( (pareto_set.size()>0) && (pareto_set.back().avg_err_vgs==sorted[0].avg_err_vgs))
                pareto_set.pop_back();

            min_area = sorted[0].avg_err_id;
            pareto_set.push_back(sorted[0]);
        }
        sorted.erase(sorted.begin());
    }
    return pareto_set;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto3d(const vector<Simulation>& simulations)
{
    vector<Simulation> pareto_set;

    for (int i=0; i<simulations.size(); i++)
    {
        bool dominated = false;

        for (int j=0; j<simulations.size() && !dominated; j++)

            if (   // if it is dominated...
                    (simulations[j].avg_err_vgs <= simulations[i].avg_err_vgs &&
                     simulations[j].avg_err_id <= simulations[i].avg_err_id &&
                     simulations[j].avg_err_vds <= simulations[i].avg_err_vds)
                    &&
                    ( // ...but not from an identical sim
                            simulations[j].avg_err_vgs != simulations[i].avg_err_vgs ||
                            simulations[j].avg_err_id != simulations[i].avg_err_id ||
                            simulations[j].avg_err_vds != simulations[i].avg_err_vds)
                    )
                dominated = true;

        // avoid repeated pareto configs
        if ( (!dominated) && (simulation_present(simulations[i],pareto_set) == -1) )
            pareto_set.push_back(simulations[i]);
    }

    return pareto_set;
}
// power/avg_err_id pareto set



int Explorer::count_needed_recompilations(const vector<Configuration>& space)
{
    bool processor_changed;
    int recompilations = 0;

    Configuration c;
    c.invalidate();

    Space_mask mask = get_space_mask(SET_PROCESSOR);

    for (unsigned int i=0;i<space.size();i++)
    {
        processor_changed = c.check_difference(space[i],mask);

        if (processor_changed)  recompilations++;

        c = space[i];
    }
    return recompilations;
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_configurations(const vector<Configuration>& space, const string& filename)
{
    vector<Simulation> pseudo_sims;
    Simulation pseudo_sim;

    for (unsigned int i = 0;i< space.size();i++)
    {
        pseudo_sim.config = space[i];
        pseudo_sim.avg_err_id = 0.0;
        pseudo_sim.avg_err_vds = 0.0;
        pseudo_sim.avg_err_vgs = 0.0;
        pseudo_sim.simulated = false;
        pseudo_sims.push_back(pseudo_sim);
    }
    save_simulations(pseudo_sims,filename);
}
////////////////////////////////////////////////////////////////////////////
void Explorer::save_simulations(const vector<Simulation>& simulations, const string& filename)
{
    double area,energy;
    double energydelay;
    double exec_time;

    FILE * fp;

    string path = get_base_dir()+"/trimaran-workspace/epic-explorer/";

    string file_path = path+filename;

    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    string pretitle = "\n%% Epic Explorer simulation file - created on " + uora;
    if(current_algo=="REP")
        pretitle += "\n%% REP source file: " + REP_source_file;

    string pretitle2 ="\n%% Objectives: ";
    //G

    if (Options.objective_id) pretitle2+="Area, ";
    if (Options.objective_energy) pretitle2+="Energy, ";
    if (Options.objective_power) pretitle2+= "Average Power, ";
    if (Options.objective_vds) pretitle2+="Execution Time";

    string pretitle3 ="\n%% Benchmark: ";
    if(Options.multibench)
        for(int i=0; i<benchmarks.size(); i++)
            pretitle3 += benchmarks.at(i) + " ";
    else
        pretitle3 += Options.benchmark;

    string pretitle4 = "\n%% Compiler: tcc_region, max_unroll_allowed, regroup_only, do_classic_opti, do_prepass_scalar_scheduling, do_postpass_scalar_scheduling, do_modulo_scheduling, memvr_profiled";

    string title = "\n\n%% ";

    // currently, avg_err_vgs and power are mutually exclusive objectives
    if (Options.objective_energy) title+="Area(cm^2)\tEnergy(J)\tTime(ms)";
    else
        title+="Area(cm^2)  Power(W)     Time(ms)   / MHz";

    title+=" / cl / I F B M / gpr fpr pr cr btr / L1D(CBA) / L1I(CBA) / L2U(CBA) / Compiler";

    fp=fopen(file_path.c_str(),"w");

    fprintf(fp,"\n%% ----------------------------------------------");
    fprintf(fp,pretitle.c_str());
    fprintf(fp,pretitle2.c_str());
    fprintf(fp,pretitle3.c_str());
    fprintf(fp,pretitle4.c_str());
    fprintf(fp,title.c_str());
    fprintf(fp,"\n%% ----------------------------------------------");

    for (unsigned int i =0;i<simulations.size();i++)
    {
        area = simulations[i].avg_err_id;
        energy = simulations[i].avg_err_vgs;
        exec_time = simulations[i].avg_err_vds*1000;

        string conf_string = simulations[i].config.get_header();

        int mhz = int(simulations[i].clock_freq/1e6);

        char ch = ' ';
        if (!simulations[i].simulated) ch = '*';

        fprintf(fp,"\n%.9f  %.9f  %.9f %%/ %d / %s %c",area,energy,exec_time,mhz,conf_string.c_str(),ch);
    }
    fclose(fp);
}

void Explorer::save_objectives_details(const Dynamic_stats& dyn,const Configuration& config, const string filename) const
{
    FILE * fp;
    fp=fopen(filename.c_str(),"a");

    string c = config.get_header();

    fprintf(fp,"\n %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %s",
            dyn.compute_cycles,
            dyn.ialu,
            dyn.falu,
            dyn.branch,
            dyn.load,
            dyn.store,
            dyn.total_dynamic_operations,
            dyn.L1D_r_hit,
            dyn.L1D_w_hit,
            dyn.L1D_r_miss,
            dyn.L1D_w_miss,
            dyn.L1I_hit,
            dyn.L1I_miss,
            dyn.L2U_hit,
            dyn.L2U_miss,
            c.c_str());

    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_stats(const Exploration_stats& stats,const string& file)
{
    FILE * fp;
    string file_path = get_base_dir()+"/trimaran-workspace/epic-explorer/"+file;

    fp = fopen(file_path.c_str(),"w");

    // we want the total size of the space of all possible
    // configurations

    int elapsed_time = (int)difftime(stats.end_time,stats.start_time)/60;

    fprintf(fp,"\n Space size: %g",stats.space_size);
    fprintf(fp,"\n simulations: %d ",stats.n_sim);
    fprintf(fp,"\n unique simulations: %d ",get_unique_configs());
    fprintf(fp,"\n total exploration time: %d minutes ",elapsed_time);
    fprintf(fp,"\n simulation start: %s ",asctime(localtime(&stats.start_time)));
    fprintf(fp,"\n simulation end: %s ",asctime(localtime(&stats.end_time)));

    fclose(fp);
}

/////////////////////////////////////////////////////////////////
void Explorer::prepare_explorer(const string& application, const Configuration& config)
{
    // Note that this fuction does NOT create any directory, but
    // simply sets explorer class members according to the app and configuration
}


////////////////////////////////////////////////////////////////////////////
void Explorer::save_estimation_file( const Dynamic_stats& dynamic_stats,
                                     const Estimate& estimate,
                                     ModelInverter& processor,
                                     Mem_hierarchy& mem,
                                     Compiler& comp,
                                     string& filename) const
{
    string file_path;
    file_path = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    file_path += filename;

    std::ofstream output_file(file_path.c_str());

    if (!output_file)
    {
        string logfile = get_base_dir()+string(EE_LOG_PATH);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"WARNING: Error while saving " + file_path);
    }
    else
    {
        output_file << "\n >>>>>>>> EPIC Explorer estimation file: " << filename;
        output_file << "\n";
        output_file << "\n **************************************************";
        output_file << "\n # of clusters: " <<  processor.L_d_int.get_val();
        output_file << "\n GPR,FPR,PR,CR,BTR = " <<processor.L_g_pin.get_val();
        output_file << "," <<  processor.L_dH_ext.get_val();
        output_file << "," <<  processor.L_sH_ext.get_val();
        output_file << "," <<  processor.L_gH_ext.get_val();
        output_file << "," <<  processor.L_dL_ext.get_val();

        output_file << "\n Per-cluster IU,FPR,MU,BU  =  " << processor.L_s_int.get_val();
        output_file << "," <<   processor.L_g_int.get_val();
        output_file << "," <<  processor.L_s_pin.get_val();
        output_file << "," <<  processor.L_d_pin.get_val();

        output_file <<"\n";
        output_file <<"\n L2U Size,BSize,Assoc = " << mem.L2U.size.get_val() << "," << mem.L2U.block_size.get_val() << "," << mem.L2U.associativity.get_val();
        output_file <<"\n L1I Size,BSize,Assoc = " << mem.L1I.size.get_val() << "," << mem.L1I.block_size.get_val() << "," << mem.L1I.associativity.get_val();
        output_file <<"\n L1D Size,BSize,Assoc = " << mem.L1D.size.get_val() << "," << mem.L1D.block_size.get_val() << "," << mem.L1D.associativity.get_val();
        output_file << "\n " << comp.tcc_region.get_label() << ": "<< comp.tcc_region.get_val();
        output_file << "\n " << comp.max_unroll_allowed.get_label()<< ": " << comp.max_unroll_allowed.get_val();
        output_file << "\n " << comp.do_classic_opti.get_label()<< ": " << comp.do_classic_opti.get_val();
        output_file << "\n " << comp.do_prepass_scalar_scheduling.get_label()<< ": " << comp.do_prepass_scalar_scheduling.get_val();
        output_file << "\n " << comp.do_postpass_scalar_scheduling.get_label()<< ": " << comp.do_postpass_scalar_scheduling.get_val();
        output_file << "\n " << comp.do_modulo_scheduling.get_label()<< ": " << comp.do_modulo_scheduling.get_val();
        output_file << "\n " << comp.memvr_profiled.get_label()<< ": " << comp.memvr_profiled.get_val(); //db
        output_file << "\n ****************************************************";
        output_file << "\n";
        output_file << "\n  P e r f o r m a n c e ";
        output_file << "\n ----------------------------------------------------";
        output_file << "\n     Total cycles: " << estimate.execution_cycles;
        output_file << "\n   Compute cycles: " << estimate.compute_cycles;
        output_file << "\n     Stall cycles: " << estimate.stall_cycles;
        output_file << "\n   Execution time (ms): " << estimate.avg_err_vgs*1000 << " (clock: " << estimate.clock_freq/1e6 << "MHz)";
        if (Options.auto_clock)
        {
            output_file << "\n   L1D access time (ns): " << estimate.L1D_access_time*1e9;
            output_file << "\n   L1I access time (ns): " << estimate.L1I_access_time*1e9;
        }

        output_file << "\n   IPC: " << estimate.IPC << " ops/cycle ";
        output_file << "\n   IPC (no stalls): " << dynamic_stats.average_issued_ops_compute_cycles << " ops/cycle";
        output_file << "\n";
        output_file << "\n  E n e r g y  &  P o w e r ";
        output_file << "\n ----------------------------------------------------";
        output_file << "\n   L1D cache avg_err_vgs (mJ): " << estimate.L1D_energy*1000;
        output_file << "\n   L1I cache avg_err_vgs (mJ): " << estimate.L1I_energy*1000;
        output_file << "\n   L2U cache avg_err_vgs (mJ): " << estimate.L2U_energy*1000;
        output_file << "\n-->  Total Cache avg_err_vgs (mJ) " << estimate.total_cache_energy*1000;
        output_file << "\n   Main Memory avg_err_vgs (mJ): " << estimate.main_memory_energy*1000;
        output_file << "\n   ModelInverter avg_err_vgs (mJ): " << estimate.total_processor_energy*1000;
        output_file << "\n\n--> Total System Energy (mJ): "<< estimate.total_system_energy*1000;
        output_file << "\n--> Average System Power (W):"<< estimate.total_average_power;
        output_file << "\n\n   Total System Energy (no stalls) (mJ): "<< estimate.NO_MEM_system_energy*1000;
        output_file << "\n   Average System Power (no stalls) (W):"<< estimate.NO_MEM_system_energy/(estimate.clock_T*dynamic_stats.total_cycles);

        output_file << "\n";
        output_file << "\n I n s t r u c t i o n  M i x";
        output_file << "\n ----------------------------------------------------";

        output_file << "\n                Branches: " << dynamic_stats.branch;
        output_file << "\n                    Load: " << dynamic_stats.load;
        output_file << "\n                   Store: " << dynamic_stats.store;
        output_file << "\n             Integer alu: " << dynamic_stats.ialu;
        output_file << "\n               Float alu: " << dynamic_stats.falu;
        output_file << "\n    Compare to predicate: " << dynamic_stats.cmp;
        output_file << "\n       Prepare to branch: " << dynamic_stats.pbr;
        output_file << "\n          Spill Restores: " << dynamic_stats.spills_restores;
        output_file << "\n";
        output_file << "\n  M e m o r y ";
        output_file << "\n ----------------------------------------------------";
        output_file << "\n L1I total fetches: " << dynamic_stats.L1I_fetches;
        output_file << "\n L1I (total hit/miss): ("<< dynamic_stats.L1I_hit <<"/"<< dynamic_stats.L1I_miss<<")";
        output_file << "\n";
        output_file << "\n L1D (read hit/miss): ("<< dynamic_stats.L1D_r_hit<<"/"<<dynamic_stats.L1D_r_miss<<")";
        output_file << "\n L1D (write hit/miss): ("<< dynamic_stats.L1D_w_hit<<"/"<<dynamic_stats.L1D_w_miss<<")";

        output_file << "\n";
        output_file << "\n L2U (hit/miss): ("<< dynamic_stats.L2U_hit<<"/" << dynamic_stats.L2U_miss<<")";
        output_file << "\n";
        output_file << "\n L1D bus transition prob: " << estimate.L1D_transition_p;
        output_file << "\n L1I bus transition prob: " << estimate.L1I_transition_p;
    }
}

////////////////////////////////////////////////////////////////////////////

bool Explorer::equivalent_spaces(const vector<Configuration>& s1,const vector<Configuration>& s2) const
{
    if (s1.size()!=s2.size()) return false;

    for (int i=0;i<s1.size();i++)
    {
        if (!configuration_present(s1[i],s2)) return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////
// return one of the commonly used configuration masks available if user
// don't need to create a particular one

Space_mask Explorer::get_space_mask(Mask_type mask_type) const
{
    Space_mask mask;

    // sets all values false, like an none_free mask
    mask.gpr_static_size = false;
    mask.fpr_static_size = false;
    mask.pr_static_size = false;
    mask.cr_static_size = false;
    mask.btr_static_size = false;

    mask.num_clusters = false;
    mask.integer_units = false;
    mask.float_units = false;
    mask.memory_units = false;
    mask.branch_units = false;

    mask.L1D_size = false;
    mask.L1D_block = false;
    mask.L1D_assoc = false;

    mask.L1I_size = false;
    mask.L1I_block = false;
    mask.L1I_assoc = false;

    mask.L2U_size = false;
    mask.L2U_block = false;
    mask.L2U_assoc = false;


    mask.tcc_region = false;	//db
    mask.max_unroll_allowed = false;  //db
    mask.regroup_only = false;	//db
    mask.do_classic_opti = false;	//db
    mask.do_prepass_scalar_scheduling = false;	//db
    mask.do_postpass_scalar_scheduling = false;	//db
    mask.do_modulo_scheduling = false;	//db
    mask.memvr_profiled = false; 	//db


    switch (mask_type)
    {
        case SET_ALL:
            mask.gpr_static_size = true;
            mask.fpr_static_size = true;
            mask.pr_static_size = true;
            mask.cr_static_size = true;
            mask.btr_static_size = true;

            mask.num_clusters = true;
            mask.integer_units = true;
            mask.float_units = true;
            mask.memory_units = true;
            mask.branch_units = true;

            mask.L1D_size = true;
            mask.L1D_block = true;
            mask.L1D_assoc = true;

            mask.L1I_size = true;
            mask.L1I_block = true;
            mask.L1I_assoc = true;

            mask.L2U_size = true;
            mask.L2U_block = true;
            mask.L2U_assoc = true;

            mask.tcc_region = true;  //db
            mask.max_unroll_allowed = true;  //db
            mask.regroup_only = true;	//db
            mask.do_classic_opti = true;	//db
            mask.do_prepass_scalar_scheduling = true;	//db
            mask.do_postpass_scalar_scheduling = true;	//db
            mask.do_modulo_scheduling = true;	//db
            mask.memvr_profiled = true; 	//db
            break;

        case UNSET_ALL:
            // do nothing, mask is already set!
            break;

        case SET_L1D:
            mask.L1D_size = true;
            mask.L1D_block = true;
            mask.L1D_assoc = true;
            break;

        case SET_L1I:
            mask.L1I_size = true;
            mask.L1I_block = true;
            mask.L1I_assoc = true;
            break;

        case SET_L2U:
            mask.L2U_size = true;
            mask.L2U_block = true;
            mask.L2U_assoc = true;
            break;

        case SET_PROCESSOR_UNITS:
            mask.num_clusters = true;
            mask.integer_units = true;
            mask.float_units = true;
            mask.memory_units = true;
            mask.branch_units = true;
            break;

        case SET_PROCESSOR:

            mask.num_clusters = true;
            mask.gpr_static_size = true;
            mask.fpr_static_size = true;
            mask.pr_static_size = true;
            mask.cr_static_size = true;
            mask.btr_static_size = true;

            mask.integer_units = true;
            mask.float_units = true;
            mask.memory_units = true;
            mask.branch_units = true;
            break;

        case SET_COMPILER:

            mask.tcc_region = true;  //db
            mask.max_unroll_allowed = true;  //db
            mask.regroup_only = true;	//db
            mask.do_classic_opti = true;	//db
            mask.do_prepass_scalar_scheduling = true;	//db
            mask.do_postpass_scalar_scheduling = true;	//db
            mask.do_modulo_scheduling = true;	//db
            mask.memvr_profiled = true; 	//db
            break;
        default:
            assert(false);
    }

    return mask;
}

////////////////////////////////////////////////////////////////////////////
vector<bool> Explorer::get_boolean_mask(const Space_mask& mask)
{
    vector<bool> b;

    b.push_back(mask.L1D_size);
    b.push_back(mask.L1D_block);
    b.push_back(mask.L1D_assoc);
    b.push_back(mask.L1I_size);
    b.push_back(mask.L1I_block);
    b.push_back(mask.L1I_assoc);
    b.push_back(mask.L2U_size);
    b.push_back(mask.L2U_block);
    b.push_back(mask.L2U_assoc);
    b.push_back(mask.integer_units);
    b.push_back(mask.float_units);
    b.push_back(mask.memory_units);
    b.push_back(mask.branch_units);
    b.push_back(mask.gpr_static_size);
    b.push_back(mask.fpr_static_size);
    b.push_back(mask.pr_static_size);
    b.push_back(mask.cr_static_size);
    b.push_back(mask.btr_static_size);
    b.push_back(mask.num_clusters);

    b.push_back(mask.tcc_region);  //db
    b.push_back(mask.max_unroll_allowed);  //db
    b.push_back(mask.regroup_only);	//db
    b.push_back(mask.do_classic_opti);	//db
    b.push_back(mask.do_prepass_scalar_scheduling);	//db
    b.push_back(mask.do_postpass_scalar_scheduling);	//db
    b.push_back(mask.do_modulo_scheduling);	//db
    b.push_back(mask.memvr_profiled); 	//db


    return b;
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::create_space_mask(const vector<bool>& boolean_mask)
{
    Space_mask mask;

    mask.L1D_size = boolean_mask[0];
    mask.L1D_block = boolean_mask[1];
    mask.L1D_assoc = boolean_mask[2];

    mask.L1I_size = boolean_mask[3];
    mask.L1I_block = boolean_mask[4];
    mask.L1I_assoc = boolean_mask[5];

    mask.L2U_size = boolean_mask[6];
    mask.L2U_block = boolean_mask[7];
    mask.L2U_assoc = boolean_mask[8];

    mask.integer_units = boolean_mask[9];
    mask.float_units = boolean_mask[10];
    mask.memory_units = boolean_mask[11];
    mask.branch_units = boolean_mask[12];

    mask.gpr_static_size = boolean_mask[13];
    mask.fpr_static_size = boolean_mask[14];
    mask.pr_static_size = boolean_mask[15];
    mask.cr_static_size = boolean_mask[16];
    mask.btr_static_size = boolean_mask[17];

    mask.num_clusters = boolean_mask[18];

    mask.tcc_region = boolean_mask[19];  //db
    mask.max_unroll_allowed = boolean_mask[20];  //db
    mask.regroup_only = boolean_mask[21];	//db
    mask.do_classic_opti = boolean_mask[22];	//db
    mask.do_prepass_scalar_scheduling = boolean_mask[23];	//db
    mask.do_postpass_scalar_scheduling = boolean_mask[24];	//db
    mask.do_modulo_scheduling = boolean_mask[25];	//db
    mask.memvr_profiled = boolean_mask[26]; 	//db
    return mask;
}

///////////////////////////////////////////////////////////////////////////////
//
// Build space functions 
//----------------------------------------------------------------------------
//
//  These function help Explorer user to create vector<Configuration>
//  spaces needed by 
//  vector<Simulation> simulate_space(vector<Configuration>& custom_space);
//

//----------------------------------------------------------------------------
// build a configuration space from a vector of simulations .
// The return value is the configuration space where simulation were
// performed
//----------------------------------------------------------------------------
vector<Configuration> Explorer::extract_space(const vector<Simulation>& sims) const
{
    vector<Configuration> temp;

    for (unsigned int i=0;i<sims.size();i++) temp.push_back(sims[i].config);

    return temp;
}

//----------------------------------------------------------------------------
// build a configuration space where parameters are explored
// exhaustively .
// Parameters with Space_mask value set to false are
// considered fixed to the default config passed as argument
//----------------------------------------------------------------------------

vector<Configuration> Explorer::build_space(const Space_mask& mask,Configuration base_conf, Space_opt opt)
{
    vector<Configuration> space;

    processor.num_clusters.set_to_first();
    do {
        if (mask.num_clusters) base_conf.num_clusters=processor.num_clusters.get_val();

        processor.integer_units.set_to_first();
        do {
            if (mask.integer_units) base_conf.integer_units=processor.integer_units.get_val();

            processor.float_units.set_to_first();
            do {
                if (mask.float_units) base_conf.float_units=processor.float_units.get_val();

                processor.memory_units.set_to_first();
                do {
                    if (mask.memory_units) base_conf.memory_units=processor.memory_units.get_val();

                    processor.branch_units.set_to_first();
                    do {
                        if (mask.branch_units) base_conf.branch_units=processor.branch_units.get_val();

                        processor.gpr_static_size.set_to_first();
                        do {
                            if (mask.gpr_static_size) base_conf.gpr_static_size=processor.gpr_static_size.get_val();

                            processor.fpr_static_size.set_to_first();
                            do {
                                if (mask.fpr_static_size) base_conf.fpr_static_size=processor.fpr_static_size.get_val();

                                processor.pr_static_size.set_to_first();
                                do {
                                    if (mask.pr_static_size) base_conf.pr_static_size=processor.pr_static_size.get_val();

                                    processor.cr_static_size.set_to_first();
                                    do {
                                        if (mask.cr_static_size) base_conf.cr_static_size=processor.cr_static_size.get_val();

                                        processor.btr_static_size.set_to_first();
                                        do {
                                            if (mask.btr_static_size) base_conf.btr_static_size=processor.btr_static_size.get_val();

                                            compiler.tcc_region.set_to_first();	//db
                                            do{	//db
                                                if (mask.tcc_region) base_conf.tcc_region=compiler.tcc_region.get_val();	//db
                                                compiler.max_unroll_allowed.set_to_first();	//db
                                                do{	//db
                                                    if (mask.max_unroll_allowed) base_conf.max_unroll_allowed=compiler.max_unroll_allowed.get_val();	//db
                                                    compiler.regroup_only.set_to_first();	//db
                                                    do{	//db
                                                        if (mask.regroup_only) base_conf.regroup_only=compiler.regroup_only.get_val();	//db
                                                        compiler.do_classic_opti.set_to_first();	//db
                                                        do{	//db
                                                            if (mask.do_classic_opti) base_conf.do_classic_opti=compiler.do_classic_opti.get_val();	//db
                                                            compiler.do_prepass_scalar_scheduling.set_to_first();	//db
                                                            do{	//db
                                                                if (mask.do_prepass_scalar_scheduling) base_conf.do_prepass_scalar_scheduling=compiler.do_prepass_scalar_scheduling.get_val();	//db
                                                                compiler.do_postpass_scalar_scheduling.set_to_first();	//db
                                                                do{	//db
                                                                    if (mask.do_postpass_scalar_scheduling) base_conf.do_postpass_scalar_scheduling=compiler.do_postpass_scalar_scheduling.get_val();	//db
                                                                    compiler.do_modulo_scheduling.set_to_first();	//db
                                                                    do{	//db
                                                                        if (mask.do_modulo_scheduling) base_conf.do_modulo_scheduling=compiler.do_modulo_scheduling.get_val();	//db
                                                                        compiler.memvr_profiled.set_to_first();	//db
                                                                        do{	//db
                                                                            if (mask.memvr_profiled) base_conf.memvr_profiled=compiler.memvr_profiled.get_val();	//db

                                                                            mem_hierarchy.L1D.size.set_to_first();
                                                                            do {
                                                                                if (mask.L1D_size) base_conf.L1D_size=mem_hierarchy.L1D.size.get_val();

                                                                                mem_hierarchy.L1D.block_size.set_to_first();
                                                                                do {
                                                                                    if (mask.L1D_block) base_conf.L1D_block=mem_hierarchy.L1D.block_size.get_val();

                                                                                    mem_hierarchy.L1D.associativity.set_to_first();
                                                                                    do {
                                                                                        if (mask.L1D_assoc) base_conf.L1D_assoc=mem_hierarchy.L1D.associativity.get_val();

                                                                                        mem_hierarchy.L1I.size.set_to_first();
                                                                                        do {
                                                                                            if (mask.L1I_size) base_conf.L1I_size=mem_hierarchy.L1I.size.get_val();

                                                                                            mem_hierarchy.L1I.block_size.set_to_first();
                                                                                            do {
                                                                                                if (mask.L1I_block) base_conf.L1I_block=mem_hierarchy.L1I.block_size.get_val();

                                                                                                mem_hierarchy.L1I.associativity.set_to_first();
                                                                                                do {
                                                                                                    if (mask.L1I_assoc) base_conf.L1I_assoc=mem_hierarchy.L1I.associativity.get_val();

                                                                                                    mem_hierarchy.L2U.size.set_to_first();
                                                                                                    do {
                                                                                                        if (mask.L2U_size) base_conf.L2U_size=mem_hierarchy.L2U.size.get_val();

                                                                                                        mem_hierarchy.L2U.block_size.set_to_first();
                                                                                                        do {
                                                                                                            if (mask.L2U_block) base_conf.L2U_block=mem_hierarchy.L2U.block_size.get_val();

                                                                                                            mem_hierarchy.L2U.associativity.set_to_first();
                                                                                                            do {
                                                                                                                if (mask.L2U_assoc) base_conf.L2U_assoc=mem_hierarchy.L2U.associativity.get_val();

                                                                                                                //////////////////////////////////////////////////////////////////////////////////
                                                                                                                //  inner loop

                                                                                                                // if all cache parameters are valid
                                                                                                                if (base_conf.is_feasible()) space.push_back(base_conf);

                                                                                                                //////////////////////////////////////////////////////////////////////////////////
                                                                                                            }while ( (mask.L2U_assoc)&&(mem_hierarchy.L2U.associativity.increase()));
                                                                                                        }while ( (mask.L2U_block)&&(mem_hierarchy.L2U.block_size.increase() ));
                                                                                                    }while ( (mask.L2U_size)&&(mem_hierarchy.L2U.size.increase()));
                                                                                                }while ( (mask.L1I_assoc)&&(mem_hierarchy.L1I.associativity.increase()));
                                                                                            }while ( (mask.L1I_block)&&(mem_hierarchy.L1I.block_size.increase() ));
                                                                                        }while ( (mask.L1I_size)&&(mem_hierarchy.L1I.size.increase()));
                                                                                    } while ( (mask.L1D_assoc)&&(mem_hierarchy.L1D.associativity.increase()));
                                                                                } while ( (mask.L1D_block)&&(mem_hierarchy.L1D.block_size.increase() ));
                                                                            } while ( (mask.L1D_size)&&(mem_hierarchy.L1D.size.increase()));
                                                                        }while ( (mask.memvr_profiled)&&(compiler.memvr_profiled.increase()));	//db
                                                                    }while ( (mask.do_modulo_scheduling)&&(compiler.do_modulo_scheduling.increase()));	//db
                                                                }while ( (mask.do_postpass_scalar_scheduling)&&(compiler.do_postpass_scalar_scheduling.increase()));	//db
                                                            }while ( (mask.do_prepass_scalar_scheduling)&&(compiler.do_prepass_scalar_scheduling.increase()));	//db
                                                        }while ( (mask.do_classic_opti)&&(compiler.do_classic_opti.increase()));	//db
                                                    }while ( (mask.regroup_only)&&(compiler.regroup_only.increase()));	//db
                                                }while ( (mask.max_unroll_allowed)&&(compiler.max_unroll_allowed.increase()));	//db
                                            }while ( (mask.tcc_region)&&(compiler.tcc_region.increase()));	//db
                                        }while ( (mask.btr_static_size) && (processor.btr_static_size.increase() ) );
                                    }while ( (mask.cr_static_size) && (processor.cr_static_size.increase() ) );
                                }while ( (mask.pr_static_size) && (processor.pr_static_size.increase() ) );
                            } while ( (mask.fpr_static_size) && (processor.fpr_static_size.increase() ) );
                        } while ( (mask.gpr_static_size) && (processor.gpr_static_size.increase() ) );
                    } while ( (mask.branch_units) && (processor.branch_units.increase()) );
                } while ( (mask.memory_units) && (processor.memory_units.increase()) ) ;
            } while( (mask.float_units) && (processor.float_units.increase()) );
        } while ( (mask.integer_units) && (processor.integer_units.increase() ));
    } while ( (mask.num_clusters) && (processor.num_clusters.increase() ));

    return space;

}


//----------------------------------------------------------------------------
// build a configuration space where parameters are explored
// exhaustively .
// Parameters with Space_mask value set to false are
// considered fixed to their default value and will not be explored.
//----------------------------------------------------------------------------

vector<Configuration> Explorer::build_space(const Space_mask& mask)
{
    Configuration default_conf = create_configuration();

    return build_space(mask,default_conf,STANDARD);
}

////////////////////////////////////////////////////////////////////////////
vector<Configuration> Explorer::build_space(const Space_mask& mask,Configuration base_conf)
{
    return build_space(mask,base_conf,STANDARD);
}

////////////////////////////////////////////////////////////////////////////
vector<Configuration> Explorer::build_space(const Space_mask& mask,Space_opt opt)
{
    Configuration default_conf = create_configuration();

    return build_space(mask,default_conf,opt);
}

////////////////////////////////////////////////////////////////////////////////
// build a new space cross merging two pre-existing spaces .
// Each configuration of s1 space will be merged with each of the
// configurations of s2 . 
// Space_masks specifies which parameters of each space must be
// considered . Note that a parameter mask value cannot be set true in
// both mask1 and mask2.
// The parameters that are unset in both mask1 and mask2 will be
// considered to their default value.
// Example:
// s1 = { a1, b1, c1 }     s2 { a3, b3, c3 }
//      { a2, b2, c2 }        { a4, b4, c4 }
//
//      mask1 = { 1,0,0}   mask2 = {0,0,1}
//
// s12 = { a1,def,c3 }
//       { a1,def,c4 }
//       { a2,def,c3 }
//       { a2,def,c4 }
//
vector<Configuration> Explorer::build_space_cross_merge(const vector<Configuration>& s1,
                                                        const vector<Configuration>& s2,
                                                        const Space_mask& mask1,
                                                        const Space_mask& mask2 ) const
{
    Configuration base_conf = create_configuration();

    vector<Configuration> merged_space;

    for (unsigned int n1=0;n1<s1.size();n1++)
    {
        if (mask1.gpr_static_size) base_conf.gpr_static_size=s1[n1].gpr_static_size;
        if (mask1.fpr_static_size) base_conf.fpr_static_size=s1[n1].fpr_static_size;
        if (mask1.pr_static_size) base_conf.pr_static_size=s1[n1].pr_static_size;
        if (mask1.cr_static_size) base_conf.cr_static_size=s1[n1].cr_static_size;
        if (mask1.btr_static_size) base_conf.btr_static_size=s1[n1].btr_static_size;

        if (mask1.num_clusters) base_conf.num_clusters=s1[n1].num_clusters;
        if (mask1.integer_units) base_conf.integer_units=s1[n1].integer_units;
        if (mask1.float_units) base_conf.float_units=s1[n1].float_units;
        if (mask1.memory_units) base_conf.memory_units=s1[n1].memory_units;
        if (mask1.branch_units) base_conf.branch_units=s1[n1].branch_units;

        if (mask1.L1D_size) base_conf.L1D_size = s1[n1].L1D_size;
        if (mask1.L1D_block) base_conf.L1D_block = s1[n1].L1D_block;
        if (mask1.L1D_assoc) base_conf.L1D_assoc = s1[n1].L1D_assoc;

        if (mask1.L1I_size) base_conf.L1I_size = s1[n1].L1I_size;
        if (mask1.L1I_block) base_conf.L1I_block = s1[n1].L1I_block;
        if (mask1.L1I_assoc) base_conf.L1I_assoc = s1[n1].L1I_assoc;

        if (mask1.L2U_size) base_conf.L2U_size = s1[n1].L2U_size;
        if (mask1.L2U_block) base_conf.L2U_block = s1[n1].L2U_block;
        if (mask1.L2U_assoc) base_conf.L2U_assoc = s1[n1].L2U_assoc;

        if (mask1.tcc_region) base_conf.tcc_region = s1[n1].tcc_region;	//db
        if (mask1.max_unroll_allowed) base_conf.max_unroll_allowed = s1[n1].max_unroll_allowed;	//db
        if (mask1.regroup_only) base_conf.regroup_only = s1[n1].regroup_only;	//db
        if (mask1.do_classic_opti) base_conf.do_classic_opti = s1[n1].do_classic_opti;	//db
        if (mask1.do_prepass_scalar_scheduling) base_conf.do_prepass_scalar_scheduling = s1[n1].do_prepass_scalar_scheduling;	//db
        if (mask1.do_postpass_scalar_scheduling) base_conf.do_postpass_scalar_scheduling = s1[n1].do_postpass_scalar_scheduling;	//db
        if (mask1.do_modulo_scheduling) base_conf.do_modulo_scheduling = s1[n1].do_modulo_scheduling;	//db
        if (mask1.memvr_profiled) base_conf.memvr_profiled = s1[n1].memvr_profiled;	//db

        for(unsigned int n2=0;n2<s2.size();n2++)
        {
            if (mask2.gpr_static_size) base_conf.gpr_static_size=s2[n2].gpr_static_size;
            if (mask2.fpr_static_size) base_conf.fpr_static_size=s2[n2].fpr_static_size;
            if (mask2.pr_static_size) base_conf.pr_static_size=s2[n2].pr_static_size;
            if (mask2.cr_static_size) base_conf.cr_static_size=s2[n2].cr_static_size;
            if (mask2.btr_static_size) base_conf.btr_static_size=s2[n2].btr_static_size;

            if (mask2.num_clusters) base_conf.num_clusters=s2[n2].num_clusters;
            if (mask2.integer_units) base_conf.integer_units=s2[n2].integer_units;
            if (mask2.float_units) base_conf.float_units=s2[n2].float_units;
            if (mask2.memory_units) base_conf.memory_units=s2[n2].memory_units;
            if (mask2.branch_units) base_conf.branch_units=s2[n2].branch_units;

            if (mask2.L1D_size) base_conf.L1D_size = s2[n2].L1D_size;
            if (mask2.L1D_block) base_conf.L1D_block = s2[n2].L1D_block;
            if (mask2.L1D_assoc) base_conf.L1D_assoc = s2[n2].L1D_assoc;

            if (mask2.L1I_size) base_conf.L1I_size = s2[n2].L1I_size;
            if (mask2.L1I_block) base_conf.L1I_block = s2[n2].L1I_block;
            if (mask2.L1I_assoc) base_conf.L1I_assoc = s2[n2].L1I_assoc;

            if (mask2.L2U_size) base_conf.L2U_size = s2[n2].L2U_size;
            if (mask2.L2U_block) base_conf.L2U_block = s2[n2].L2U_block;
            if (mask2.L2U_assoc) base_conf.L2U_assoc = s2[n2].L2U_assoc;

            if (mask2.tcc_region) base_conf.tcc_region = s2[n2].tcc_region;	//db
            if (mask2.max_unroll_allowed) base_conf.max_unroll_allowed = s2[n2].max_unroll_allowed;	//db
            if (mask2.regroup_only) base_conf.regroup_only = s2[n2].regroup_only;	//db
            if (mask2.do_classic_opti) base_conf.do_classic_opti = s2[n2].do_classic_opti;	//db
            if (mask2.do_prepass_scalar_scheduling) base_conf.do_prepass_scalar_scheduling = s2[n2].do_prepass_scalar_scheduling;	//db
            if (mask2.do_postpass_scalar_scheduling) base_conf.do_postpass_scalar_scheduling = s2[n2].do_postpass_scalar_scheduling;	//db
            if (mask2.do_modulo_scheduling) base_conf.do_modulo_scheduling = s2[n2].do_modulo_scheduling;	//db
            if (mask2.memvr_profiled) base_conf.memvr_profiled = s2[n2].memvr_profiled;	//db
            // resulting combination of two configurations may result
            // in not feasible config

            if ( base_conf.is_feasible() && !configuration_present(base_conf,merged_space) )
                merged_space.push_back(base_conf);

        }
    }
    return merged_space;
}

////////////////////////////////////////////////////////////////////////////
bool Explorer::configuration_present(const Configuration& conf, const vector<Configuration>& space) const
{

    if (space.size()==0) return false;

    for(unsigned int i = 0;i<space.size();i++)
    {
        if(   (conf.gpr_static_size==space[i].gpr_static_size)
              &&(conf.fpr_static_size==space[i].fpr_static_size)
              &&(conf.pr_static_size==space[i].pr_static_size)
              &&(conf.cr_static_size==space[i].cr_static_size)
              &&(conf.btr_static_size==space[i].btr_static_size)
              &&(conf.num_clusters == space[i].num_clusters)
              &&(conf.integer_units == space[i].integer_units)
              &&(conf.float_units == space[i].float_units)
              &&(conf.memory_units == space[i].memory_units)
              &&(conf.branch_units == space[i].branch_units)
              &&(conf.L1D_size == space[i].L1D_size)
              &&(conf.L1D_block == space[i].L1D_block)
              &&(conf.L1D_assoc == space[i].L1D_assoc)
              &&(conf.L1I_size == space[i].L1I_size)
              &&(conf.L1I_block == space[i].L1I_block)
              &&(conf.L1I_assoc == space[i].L1I_assoc)
              &&(conf.L2U_size == space[i].L2U_size)
              &&(conf.L2U_block == space[i].L2U_block)
              &&(conf.L2U_assoc == space[i].L2U_assoc)
              &&(conf.tcc_region == space[i].tcc_region) 		//db
              &&(conf.max_unroll_allowed == space[i].max_unroll_allowed)	 	//db
              &&(conf.regroup_only == space[i].regroup_only)	 	//db
              &&(conf.do_classic_opti == space[i].do_classic_opti)	 	//db
              &&(conf.do_prepass_scalar_scheduling == space[i].do_prepass_scalar_scheduling)	 	//db
              &&(conf.do_postpass_scalar_scheduling == space[i].do_postpass_scalar_scheduling)	 	//db
              &&(conf.do_modulo_scheduling == space[i].do_modulo_scheduling)	 	//db
              &&(conf.memvr_profiled == space[i].memvr_profiled))	 	//db
            return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
int Explorer::simulation_present(const Simulation& sim,const vector<Simulation>& simulations) const
{
    if (simulations.size()==0) return (-1);

    for (int i=0;i<simulations.size();++i)
    {
        if (
                (simulations[i].avg_err_id == sim.avg_err_id) &&
                (simulations[i].avg_err_vgs == sim.avg_err_vgs) &&
                (simulations[i].avg_err_vds == sim.avg_err_vds)
                )
            return (i);
    }
    return (-1);
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::mask_union(Space_mask& m1,Space_mask& m2) const
{
    Space_mask mask;
    mask.gpr_static_size = (m1.gpr_static_size) || (m2.gpr_static_size);
    mask.fpr_static_size = (m1.fpr_static_size) || (m2.fpr_static_size);
    mask.pr_static_size = (m1.pr_static_size) || (m2.pr_static_size);
    mask.cr_static_size = (m1.cr_static_size) || (m2.cr_static_size);
    mask.btr_static_size = (m1.btr_static_size) || (m2.btr_static_size);

    mask.num_clusters = (m1.num_clusters) || (m2.num_clusters);
    mask.integer_units = (m1.integer_units) || (m2.integer_units);
    mask.float_units = (m1.float_units) || (m2.float_units);
    mask.memory_units = (m1.memory_units) || (m2.memory_units);
    mask.branch_units = (m1.branch_units) || (m2.branch_units);

    mask.L1D_size = (m1.L1D_size) || (m2.L1D_size);
    mask.L1D_block = (m1.L1D_block) || (m2.L1D_block);
    mask.L1D_assoc =(m1.L1D_assoc) || (m2.L1D_assoc);

    mask.L1I_size = (m1.L1I_size) || (m2.L1I_size);
    mask.L1I_block = (m1.L1I_block) || (m2.L1I_block);
    mask.L1I_assoc =(m1.L1I_assoc) || (m2.L1I_assoc);

    mask.L2U_size = (m1.L2U_size) || (m2.L2U_size);
    mask.L2U_block = (m1.L2U_block) || (m2.L2U_block);
    mask.L2U_assoc =(m1.L2U_assoc) || (m2.L2U_assoc);

    mask.tcc_region = (m1.tcc_region)||(m2.tcc_region);  //db
    mask.max_unroll_allowed = (m1.max_unroll_allowed)||(m2.max_unroll_allowed);  //db
    mask.regroup_only = (m1.regroup_only)||(m2.regroup_only);	//db
    mask.do_classic_opti = (m1.do_classic_opti)||(m2.do_classic_opti);	//db
    mask.do_prepass_scalar_scheduling = (m1.do_prepass_scalar_scheduling)||(m2.do_prepass_scalar_scheduling);	//db
    mask.do_postpass_scalar_scheduling = (m1.do_postpass_scalar_scheduling)||(m2.do_postpass_scalar_scheduling);	//db
    mask.do_modulo_scheduling = (m1.do_modulo_scheduling)||(m2.do_modulo_scheduling);	//db
    mask.memvr_profiled = (m1.memvr_profiled)||(m2.memvr_profiled); 	//db
    return mask;

}

////////////////////////////////////////////////////////////////////////////
// compute the total number of configurations considering all
// possible values of paremeters whose mask boolean is set true

long double Explorer::get_space_size(const Space_mask& mask) const
{
    double size = 1;


    if (mask.gpr_static_size) size = size*(processor.gpr_static_size.get_size());
    if (mask.fpr_static_size) size = size*(processor.fpr_static_size.get_size());
    if (mask.pr_static_size) size = size*(processor.pr_static_size.get_size());
    if (mask.cr_static_size) size = size*(processor.cr_static_size.get_size());
    if (mask.btr_static_size) size = size*(processor.btr_static_size.get_size());

    if (mask.num_clusters) size = size*(processor.num_clusters.get_size());
    if (mask.integer_units) size = size*(processor.integer_units.get_size());
    if (mask.float_units) size = size*(processor.float_units.get_size());
    if (mask.memory_units) size = size*(processor.memory_units.get_size());
    if (mask.branch_units) size = size*(processor.branch_units.get_size());

    if (mask.L1D_size) size = size*(mem_hierarchy.L1D.size.get_size());
    if (mask.L1D_block) size = size*(mem_hierarchy.L1D.block_size.get_size());
    if (mask.L1D_assoc) size = size*(mem_hierarchy.L1D.associativity.get_size());

    if (mask.L1I_size) size = size*(mem_hierarchy.L1I.size.get_size());
    if (mask.L1I_block) size = size*(mem_hierarchy.L1I.block_size.get_size());
    if (mask.L1I_assoc) size = size*(mem_hierarchy.L1I.associativity.get_size());

    if (mask.L2U_size) size = size*(mem_hierarchy.L2U.size.get_size());
    if (mask.L2U_block) size = size*(mem_hierarchy.L2U.block_size.get_size());
    if (mask.L2U_assoc) size = size*(mem_hierarchy.L2U.associativity.get_size());

    if (mask.tcc_region) size = size*(compiler.tcc_region.get_size());	//db
    if (mask.max_unroll_allowed) size = size*(compiler.max_unroll_allowed.get_size());	//db
    if (mask.regroup_only) size = size*(compiler.regroup_only.get_size());	//db
    if (mask.do_classic_opti) size = size*(compiler.do_classic_opti.get_size());	//db
    if (mask.do_prepass_scalar_scheduling) size = size*(compiler.do_prepass_scalar_scheduling.get_size());	//db
    if (mask.do_postpass_scalar_scheduling) size = size*(compiler.do_postpass_scalar_scheduling.get_size());	//db
    if (mask.do_modulo_scheduling) size = size*(compiler.do_modulo_scheduling.get_size());	//db
    if (mask.memvr_profiled) size = size*(compiler.memvr_profiled.get_size());	//db

    return size;
}

////////////////////////////////////////////////////////////////////////////
// if no mask is specified, total configuration space size is returned
long double Explorer::get_space_size() const
{
    Space_mask all_parm = get_space_mask(SET_ALL);
    return get_space_size(all_parm);

}


///////////////////////////////////////////////////////////////////
//  Misc Functions 
// ---------------------------------------------------------------------------
// functions used by explorer methods
//
void Explorer::append_simulations(vector<Simulation>& dest,const vector<Simulation>& new_sims)
{
    for (unsigned int i=0;i<new_sims.size();i++)
    {
        if (simulation_present(new_sims[i],dest) < 0) dest.push_back(new_sims[i]);
    }
}

////////////////////////////////////////////////////////////////////////////
int Explorer::get_sim_counter() const
{
    return sim_counter;
}
////////////////////////////////////////////////////////////////////////////
int Explorer::get_unique_configs() const
{
    return unique_configs.size();
}

////////////////////////////////////////////////////////////////////////////
void Explorer::reset_sim_counter()
{
    sim_counter=0;
    unique_configs.clear();
}

////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParametersNumber()
{

    vector<pair<int,int> > v;

    v.push_back(pair<int,int>(1, processor.integer_units.get_size()) );
    v.push_back(pair<int,int>(1, processor.float_units.get_size()));
    v.push_back(pair<int,int>(1, processor.branch_units.get_size()));
    v.push_back(pair<int,int>(1, processor.memory_units.get_size()));
    v.push_back(pair<int,int>(1, processor.gpr_static_size.get_size()));
    v.push_back(pair<int,int>(1, processor.fpr_static_size.get_size()));
    v.push_back(pair<int,int>(1, processor.pr_static_size.get_size()));
    v.push_back(pair<int,int>(1, processor.cr_static_size.get_size()));
    v.push_back(pair<int,int>(1, processor.btr_static_size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L1D.size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L1D.block_size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L1D.associativity.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L1I.size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L1I.block_size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L1I.associativity.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L2U.size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L2U.block_size.get_size()));
    v.push_back(pair<int,int>(1, mem_hierarchy.L2U.associativity.get_size()));
    v.push_back(pair<int,int>(1, processor.num_clusters.get_size()));
    v.push_back(pair<int,int>(1,compiler.tcc_region.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.max_unroll_allowed.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.regroup_only.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.do_classic_opti.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.do_prepass_scalar_scheduling.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.do_postpass_scalar_scheduling.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.do_modulo_scheduling.get_size()));	//db
    v.push_back(pair<int,int>(1,compiler.memvr_profiled.get_size()));	//db
    return v;
}

////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParameterRanges()
{
    vector<pair<int,int> > v;

    v.push_back(pair<int,int>(processor.integer_units.get_first(), processor.integer_units.get_last()));
    v.push_back(pair<int,int>(processor.float_units.get_first(), processor.float_units.get_last()));
    v.push_back(pair<int,int>(processor.branch_units.get_first(), processor.branch_units.get_last()));
    v.push_back(pair<int,int>(processor.memory_units.get_first(), processor.memory_units.get_last()));
    v.push_back(pair<int,int>(processor.gpr_static_size.get_first(), processor.gpr_static_size.get_last()));
    v.push_back(pair<int,int>(processor.fpr_static_size.get_first(), processor.fpr_static_size.get_last()));
    v.push_back(pair<int,int>(processor.pr_static_size.get_first(), processor.pr_static_size.get_last()));
    v.push_back(pair<int,int>(processor.cr_static_size.get_first(), processor.cr_static_size.get_last()));
    v.push_back(pair<int,int>(processor.btr_static_size.get_first(), processor.btr_static_size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L1D.size.get_first(), mem_hierarchy.L1D.size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L1D.block_size.get_first(), mem_hierarchy.L1D.block_size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L1D.associativity.get_first(), mem_hierarchy.L1D.associativity.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L1I.size.get_first(), mem_hierarchy.L1I.size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L1I.block_size.get_first(), mem_hierarchy.L1I.block_size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L1I.associativity.get_first(), mem_hierarchy.L1I.associativity.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L2U.size.get_first(), mem_hierarchy.L2U.size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L2U.block_size.get_first(), mem_hierarchy.L2U.block_size.get_last()));
    v.push_back(pair<int,int>(mem_hierarchy.L2U.associativity.get_first(), mem_hierarchy.L2U.associativity.get_last()));
    v.push_back(pair<int,int>(processor.num_clusters.get_first(), processor.num_clusters.get_last()));
    v.push_back(pair<int,int>(compiler.tcc_region.get_first(),compiler.tcc_region.get_last()));	//db
    v.push_back(pair<int,int>(compiler.max_unroll_allowed.get_first(),compiler.max_unroll_allowed.get_last()));	//db
    v.push_back(pair<int,int>(compiler.regroup_only.get_first(),compiler.regroup_only.get_last()));	//db
    v.push_back(pair<int,int>(compiler.do_classic_opti.get_first(),compiler.do_classic_opti.get_last()));	//db
    v.push_back(pair<int,int>(compiler.do_prepass_scalar_scheduling.get_first(),compiler.do_prepass_scalar_scheduling.get_last()));	//db
    v.push_back(pair<int,int>(compiler.do_postpass_scalar_scheduling.get_first(),compiler.do_postpass_scalar_scheduling.get_last()));	//db
    v.push_back(pair<int,int>(compiler.do_modulo_scheduling.get_first(),compiler.do_modulo_scheduling.get_last()));	//db
    v.push_back(pair<int,int>(compiler.memvr_profiled.get_first(),compiler.memvr_profiled.get_last()));	//db

    return v;
}


// added by andrea.araldo@gmail.com
Parameter Explorer::getParameter(EParameterType pt)
{
    switch (pt)
    {
        case ke_gpr_static_size:	return processor.gpr_static_size;
        case ke_fpr_static_size:	return processor.fpr_static_size;
        case ke_pr_static_size:		return processor.pr_static_size;
        case ke_cr_static_size:		return processor.cr_static_size;
        case ke_btr_static_size:	return processor.btr_static_size;
        case ke_num_clusters:		return processor.num_clusters;
        case ke_integer_units:		return processor.integer_units;
        case ke_float_units:		return processor.float_units;
        case ke_branch_units:		return processor.branch_units;
        case ke_memory_units:		return processor.memory_units;

        case ke_L1D_size:			return mem_hierarchy.L1D.size;
        case ke_L1D_block_size:		return mem_hierarchy.L1D.block_size;
        case ke_L1D_associativity:	return mem_hierarchy.L1D.associativity;
        case ke_L1I_size:			return mem_hierarchy.L1I.size;
        case ke_L1I_block_size:		return mem_hierarchy.L1I.block_size;
        case ke_L1I_associativity:	return mem_hierarchy.L1I.associativity;
        case ke_L2U_size:			return mem_hierarchy.L2U.size;
        case ke_L2U_block_size:		return mem_hierarchy.L2U.block_size;
        case ke_L2U_associativity:	return mem_hierarchy.L2U.associativity;

        case ke_tcc_region:			return compiler.tcc_region;
        case ke_max_unroll_allowed:	return compiler.max_unroll_allowed;
        case ke_regroup_only:		return compiler.regroup_only;
        case ke_do_classic_opti:	return compiler.do_classic_opti;
        case ke_do_prepass_scalar_scheduling:
            return compiler.do_prepass_scalar_scheduling;
        case ke_do_postpass_scalar_scheduling:
            return compiler.do_postpass_scalar_scheduling;
        case ke_do_modulo_scheduling:
            return compiler.do_modulo_scheduling;
        case ke_memvr_profiled:		return compiler.memvr_profiled;



        default:
            string message = "explorer.cpp: ERROR: parameter "+to_string(pt)+
                             " is not valid";
            exit(-122);

    }
}


////////////////////////////////////////////////////////////////////////////

void Explorer::set_force_simulation(bool s)
{
    force_simulation = s;
}

void Explorer::set_space_name(const string& spacename)
{
    current_space = spacename;
}

string Explorer::get_space_name() const
{
    return current_space;
}
void Explorer::load_space_file(const string& filename)
{
    std::ifstream input_file (filename.c_str());
    string word;

    if (!input_file)
    {
        string logfile = get_base_dir()+string(EE_LOG_PATH);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"ERROR: cannot load " + filename);
        cout << "\n ERROR: cannot load " << filename << "Try to run post_install.sh again to fix the problem\n";

        //<aa>
#ifdef SEVERE_DEBUG
        exit(EXIT_FAILURE);
#endif
        //</aa>
        sleep(2);
    }
    else
    {

        go_until("[BEGIN_SPACE]",input_file);

        int val;
        vector<int> values;

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1D.size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1D.block_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1D.associativity.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1I.size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1I.block_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1I.associativity.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L2U.size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L2U.block_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L2U.associativity.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.integer_units.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.float_units.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.memory_units.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.branch_units.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.gpr_static_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.fpr_static_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.pr_static_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.cr_static_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.btr_static_size.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        processor.num_clusters.set_values(values,val);

        processor.set_to_default();
        //anInterface->save_model_config(processor,hmdes_filename);

//add db	
        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.tcc_region.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.max_unroll_allowed.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.regroup_only.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.do_classic_opti.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.do_prepass_scalar_scheduling.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.do_postpass_scalar_scheduling.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.do_modulo_scheduling.set_values(values,val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        compiler.memvr_profiled.set_values(values,val);

//end add db
    }

}

void Explorer::save_space_file(const string& filename)
{
    std::ofstream output_file(filename.c_str());
    if (!output_file)
    {
        cout << "\n Error while saving " << filename ;
        string logfile = get_base_dir()+string(EE_LOG_PATH);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"ERROR: cannot save " + filename);
        sleep(2);
    }
    else
    {
        output_file << "\n\n [BEGIN_SPACE]";

        processor.integer_units.set_to_first();
        processor.float_units.set_to_first();
        processor.branch_units.set_to_first();
        processor.memory_units.set_to_first();
        processor.gpr_static_size.set_to_first();
        processor.fpr_static_size.set_to_first();
        processor.pr_static_size.set_to_first();
        processor.cr_static_size.set_to_first();
        processor.btr_static_size.set_to_first();
        processor.num_clusters.set_to_first();

        mem_hierarchy.L1D.size.set_to_first();
        mem_hierarchy.L1D.block_size.set_to_first();
        mem_hierarchy.L1D.associativity.set_to_first();

        mem_hierarchy.L1I.size.set_to_first();
        mem_hierarchy.L1I.block_size.set_to_first();
        mem_hierarchy.L1I.associativity.set_to_first();

        mem_hierarchy.L2U.size.set_to_first();
        mem_hierarchy.L2U.block_size.set_to_first();
        mem_hierarchy.L2U.associativity.set_to_first();

        compiler.tcc_region.set_to_first();	//db
        compiler.max_unroll_allowed.set_to_first();	//db
        compiler.regroup_only.set_to_first();	//db
        compiler.do_classic_opti.set_to_first();	//db
        compiler.do_prepass_scalar_scheduling.set_to_first();	//db
        compiler.do_postpass_scalar_scheduling.set_to_first();	//db
        compiler.do_modulo_scheduling.set_to_first();	//db
        compiler.memvr_profiled.set_to_first();	//db

        output_file << "\nL1D_size " ;
        do { output_file << mem_hierarchy.L1D.size.get_val() << " "; } while (mem_hierarchy.L1D.size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L1D.size.get_default();

        output_file << "\nL1D_block_size " ;
        do { output_file << mem_hierarchy.L1D.block_size.get_val() << " "; } while (mem_hierarchy.L1D.block_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L1D.block_size.get_default();

        output_file << "\nL1D_associativity " ;
        do { output_file << mem_hierarchy.L1D.associativity.get_val() << " "; } while (mem_hierarchy.L1D.associativity.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L1D.associativity.get_default();

        output_file << "\nL1I_size " ;
        do { output_file << mem_hierarchy.L1I.size.get_val() << " "; } while (mem_hierarchy.L1I.size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L1I.size.get_default();

        output_file << "\nL1I_block_size " ;
        do { output_file << mem_hierarchy.L1I.block_size.get_val() << " "; } while (mem_hierarchy.L1I.block_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L1I.block_size.get_default();

        output_file << "\nL1I_associativity " ;
        do { output_file << mem_hierarchy.L1I.associativity.get_val() << " "; } while (mem_hierarchy.L1I.associativity.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L1I.associativity.get_default();

        output_file << "\nL2U_size " ;
        do { output_file << mem_hierarchy.L2U.size.get_val() << " "; } while (mem_hierarchy.L2U.size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L2U.size.get_default();

        output_file << "\nL2U_block_size " ;
        do { output_file << mem_hierarchy.L2U.block_size.get_val() << " "; } while (mem_hierarchy.L2U.block_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L2U.block_size.get_default();

        output_file << "\nL2U_associativity " ;
        do { output_file << mem_hierarchy.L2U.associativity.get_val() << " "; } while (mem_hierarchy.L2U.associativity.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << mem_hierarchy.L2U.associativity.get_default();

        output_file << "\ninteger_units ";
        do { output_file << processor.integer_units.get_val() << " "; } while (processor.integer_units.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.integer_units.get_default();

        output_file << "\nfloat_units ";
        do { output_file << processor.float_units.get_val() << " "; } while (processor.float_units.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.float_units.get_default();

        output_file << "\nmemory_units ";
        do { output_file << processor.memory_units.get_val() << " "; } while (processor.memory_units.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.memory_units.get_default();

        output_file << "\nbranch_units ";
        do { output_file << processor.branch_units.get_val() << " "; } while (processor.branch_units.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.branch_units.get_default();

        output_file << "\ngpr_static_size ";
        do { output_file << processor.gpr_static_size.get_val() << " "; } while (processor.gpr_static_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.gpr_static_size.get_default();

        output_file << "\nfpr_static_size ";
        do { output_file << processor.fpr_static_size.get_val() << " "; } while (processor.fpr_static_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.fpr_static_size.get_default();

        output_file << "\npr_static_size ";
        do { output_file << processor.pr_static_size.get_val() << " "; } while (processor.pr_static_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.pr_static_size.get_default();

        output_file << "\ncr_static_size ";
        do { output_file << processor.cr_static_size.get_val() << " "; } while (processor.cr_static_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.cr_static_size.get_default();

        output_file << "\nbtr_static_size ";
        do { output_file << processor.btr_static_size.get_val() << " "; } while (processor.btr_static_size.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.btr_static_size.get_default();

        output_file << "\nnum_clusters ";
        do { output_file << processor.num_clusters.get_val() << " "; } while (processor.num_clusters.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << processor.num_clusters.get_default();

        output_file <<"\ntcc_region "; 	//db
        do { output_file << compiler.tcc_region.get_val() << " "; } while (compiler.tcc_region.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.tcc_region.get_default();	//db

        output_file <<"\nmax_unroll_allowed "; 	//db
        do { output_file << compiler.max_unroll_allowed.get_val() << " "; } while (compiler.max_unroll_allowed.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.max_unroll_allowed.get_default();	//db

        output_file <<"\nregroup_only "; 	//db
        do { output_file << compiler.regroup_only.get_val() << " "; } while (compiler.regroup_only.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.regroup_only.get_default();	//db

        output_file <<"\ndo_classic_opti "; 	//db
        do { output_file << compiler.do_classic_opti.get_val() << " "; } while (compiler.do_classic_opti.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.do_classic_opti.get_default();	//db

        output_file <<"\ndo_prepass_scalar_scheduling "; 	//db
        do { output_file << compiler.do_prepass_scalar_scheduling.get_val() << " "; } while (compiler.do_prepass_scalar_scheduling.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.do_prepass_scalar_scheduling.get_default();	//db

        output_file <<"\ndo_postpass_scalar_scheduling "; 	//db
        do { output_file << compiler.do_postpass_scalar_scheduling.get_val() << " "; } while (compiler.do_postpass_scalar_scheduling.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.do_postpass_scalar_scheduling.get_default();	//db

        output_file <<"\ndo_modulo_scheduling "; 	//db
        do { output_file << compiler.do_modulo_scheduling.get_val() << " "; } while (compiler.do_modulo_scheduling.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.do_modulo_scheduling.get_default();	//db

        output_file <<"\nmemvr_profiled "; 	//db
        do { output_file << compiler.memvr_profiled.get_val() << " "; } while (compiler.memvr_profiled.increase()); //db
        output_file << "0";	//db
        output_file << "\n DEFAULT " << compiler.memvr_profiled.get_default();	//db

        output_file << "\n\n [END_SPACE]";

    }

}

void Explorer::set_base_dir(const string& dir)
{
    base_dir = dir;
    trimaran_interface->set_environment(dir);
}

// TODO: shouldn't get it from trimaran interface ?
string Explorer::get_base_dir() const
{
    return base_dir;
}
////////////////////////////////////////////////////////////////////////////
void Explorer::test()
{
    // PLACE CODE HERE TO QUICKLY TEST YOUR OWN ALGORITHM

    /* Example 1 ******************************************
     * using a predefined mask to make an exhaustive
     * exploration of a subset of parameters
     * ****************************************************
    Space_mask mask = get_space_mask(SET_L1D);
    vector<Configuration> space = build_space(mask);
    vector<Simulation> res = simulate_space(space);

    string filename = "L1D_EXHA";

    save_simulations(res,filename+".exp");
    vector<Simulation> pareto_set = get_pareto(res);

    save_simulations(pareto_set,filename+".pareto.exp");
    */

}

void Explorer::init_approximation()
{
    if (Options.approx_settings.enabled==1)
    {
        cout << "\nFuzzy Approximation Enabled\n";
        if (function_approx != NULL) free(function_approx);
        function_approx = new CFuzzyFunctionApproximation();
        function_approx->FuzzySetsInit(getParametersNumber());
        function_approx->Init(Options.approx_settings.threshold,Options.approx_settings.min, Options.approx_settings.max,n_objectives());
    }
    else if (Options.approx_settings.enabled==2)
    {

        cout << "\nArtificial Neural Network Approximation is not available in this release\n";
        cout << "\nFuzzy Approximation will be enabled instead\n";
        Options.approx_settings.enabled = 1;
        init_approximation();

        /*

        cout << "\nArtificial Neural Network Approximation Enabled\n";
        if (function_approx != NULL) free(function_approx);
        function_approx = new CAnnFunctionApproximation();
        function_approx->Init(Options.approx_settings.threshold,Options.approx_settings.min, Options.approx_settings.max,n_objectives());
        */
    }

}
