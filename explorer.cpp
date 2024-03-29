
#include "explorer.h"
#include "common.h"
#ifdef M9_MPI
#include "mpi.h"
#endif



Space_mask Explorer::get_space_mask(Mask_type mask_type) const {
    Space_mask mask;

    // sets all values false, like an none_free mask
    //mask.gpr_static_size = false;
    switch (mask_type) {
        case SET_ALL:
            mask.L_d_int = true;
            mask.L_s_int = true;
            mask.L_g_int = true;
            mask.L_d_pin = true;
            mask.L_s_pin = true;
            mask.L_g_pin = true;
            mask.L_dH_ext = true;
            mask.L_sH_ext = true;
            mask.L_gH_ext = true;
            mask.L_dL_ext = true;
            mask.L_sL_ext = true;
            mask.L_gL_ext = true;
            mask.L_Hwire = true;
            mask.L_Lwire = true;
        case UNSET_ALL:
            mask.L_d_int =  false;
            mask.L_s_int =  false;
            mask.L_g_int =  false;
            mask.L_d_pin =  false;
            mask.L_s_pin =  false;
            mask.L_g_pin =  false;
            mask.L_dH_ext = false;
            mask.L_sH_ext = false;
            mask.L_gH_ext = false;
            mask.L_dL_ext = false;
            mask.L_sL_ext = false;
            mask.L_gL_ext = false;
            mask.L_Hwire = false;
            mask.L_Lwire = false;
    }

    return mask;
}
//********************************************************************
Explorer::Explorer(MatlabInterface * pInterface)
{
    this->matlabInterface = pInterface;
    // when simulation is started these values will be calculated
    // to be more realistic

    this->n_obj =N_OBJ;
    // by default objectives include avg_err_VDS and average power
    if (n_obj==3){
        Options.objective_avg_errID = true;
        Options.objective_avg_errVDS = true;
        Options.objective_avg_errVGS = true;
    }
    else if (n_obj==6){
        Options.objective_errID_H = true;
        Options.objective_errVDS_H = true;
        Options.objective_errVGS_H = true;
        Options.objective_errID_L = true;
        Options.objective_errVDS_L = true;
        Options.objective_errVGS_L = true;
    } else
        assert(false);


    Options.save_spaces = false;
    Options.save_estimation = false;

    Options.save_objectives_details = false;
    force_simulation = false;

    string space_file = getenv(BASE_DIR)+string(M9DSE_PATH)+string(CONFIG_SPACE_FILE);

    load_space_file(space_file);
}

//********************************************************************

Explorer::~Explorer()
{
    string logfile;
    logfile = get_base_dir() + string(M9DSE_LOG_FILE);
    write_to_log(get_mpi_rank(),logfile,"Destroying explorer class");
}


////////////////////////////////////////////////////////////////////////////
// The following  are utility functions used by exploring functions
////////////////////////////////////////////////////////////////////////////
//********************************************************************




Configuration Explorer::create_configuration(const ModelInverter &p) const //db
{
    Configuration conf;

    conf.L_d_int= model_inverter.L_d_int.get_val();
    conf.L_s_int = model_inverter.L_s_int.get_val();
    conf.L_g_int = model_inverter.L_g_int.get_val();
    conf.L_d_pin = model_inverter.L_d_pin.get_val();
    conf.L_s_pin = model_inverter.L_s_pin.get_val();
    conf.L_g_pin = model_inverter.L_g_pin.get_val();
    conf.L_dH_ext = model_inverter.L_dH_ext.get_val();
    conf.L_sH_ext = model_inverter.L_sH_ext.get_val();
    conf.L_gH_ext = model_inverter.L_gH_ext.get_val();
    conf.L_dL_ext = model_inverter.L_dL_ext.get_val();
    conf.L_sL_ext = model_inverter.L_sL_ext.get_val();
    conf.L_gL_ext = model_inverter.L_gL_ext.get_val();
    conf.L_Hwire = model_inverter.L_Hwire.get_val();
    conf.L_Lwire = model_inverter.L_Lwire.get_val();
    return conf;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration() const
{
    Configuration default_conf;

    default_conf.L_d_int  = this->model_inverter.L_d_int.get_default();
    default_conf.L_s_int  = this->model_inverter.L_s_int.get_default();
    default_conf.L_g_int  = this->model_inverter.L_g_int.get_default();
    default_conf.L_d_pin  = this->model_inverter.L_d_pin.get_default();
    default_conf.L_s_pin  = this->model_inverter.L_s_pin.get_default();
    default_conf.L_g_pin  = this->model_inverter.L_g_pin.get_default();
    default_conf.L_dH_ext = this->model_inverter.L_dH_ext.get_default();
    default_conf.L_sH_ext = this->model_inverter.L_sH_ext.get_default();
    default_conf.L_gH_ext = this->model_inverter.L_gH_ext.get_default();
    default_conf.L_dL_ext = this->model_inverter.L_dL_ext.get_default();
    default_conf.L_sL_ext = this->model_inverter.L_sL_ext.get_default();
    default_conf.L_gL_ext = this->model_inverter.L_gL_ext.get_default();
    default_conf.L_Hwire  = this->model_inverter.L_Hwire.get_default();
    default_conf.L_Lwire  = this->model_inverter.L_Lwire.get_default();

    return default_conf;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration(const Space_mask& mask,const Configuration& base) const
{
    Configuration conf = create_configuration();


    if (mask.L_d_int ) conf.L_d_int = base.L_d_int;
    if (mask.L_s_int ) conf.L_s_int = base.L_s_int;
    if (mask.L_g_int ) conf.L_g_int = base.L_g_int;
    if (mask.L_d_pin ) conf.L_d_pin = base.L_d_pin;
    if (mask.L_s_pin ) conf.L_s_pin = base.L_s_pin;
    if (mask.L_g_pin ) conf.L_g_pin = base.L_g_pin;
    if (mask.L_dH_ext) conf.L_dH_ext = base.L_dH_ext;
    if (mask.L_sH_ext) conf.L_sH_ext = base.L_sH_ext;
    if (mask.L_gH_ext) conf.L_gH_ext = base.L_gH_ext;
    if (mask.L_dL_ext) conf.L_dL_ext = base.L_dL_ext;
    if (mask.L_sL_ext) conf.L_sL_ext = base.L_sL_ext;
    if (mask.L_gL_ext) conf.L_gL_ext = base.L_gL_ext;
    if (mask.L_Hwire ) conf.L_Hwire = base.L_Hwire;
    if (mask.L_Lwire ) conf.L_Lwire = base.L_Lwire;
    return conf;
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
    if ( (Options.objective_avg_errID) && (Options.objective_avg_errVGS) && (Options.objective_avg_errVDS) )
        return sqrt(pow(s1.avg_err_ID-s2.avg_err_ID,2)+ pow(s1.avg_err_VGS-s2.avg_err_VGS,2) + pow(double(s1.avg_err_VDS-s2.avg_err_VDS),2) );

    if ( (Options.objective_avg_errVGS) && (Options.objective_avg_errVDS) )
        return sqrt(pow(s1.avg_err_VGS-s2.avg_err_VGS,2) + pow(double(s1.avg_err_VDS-s2.avg_err_VDS),2) );

    if ( (Options.objective_avg_errID) && (Options.objective_avg_errVDS) )
        return sqrt(pow(s1.avg_err_ID-s2.avg_err_ID,2) + pow(double(s1.avg_err_VDS-s2.avg_err_VDS),2) );

    assert(n_obj==6);
    return sqrt(pow(s1.err_ID_H-s2.err_ID_H,2)+ pow(s1.err_VGS_H-s2.err_VGS_H,2) + pow((s1.err_VDS_H-s2.err_VDS_H),2) +
                pow(s1.err_ID_L-s2.err_ID_L,2)+ pow(s1.err_VGS_L-s2.err_VGS_L,2)+ pow((s1.err_VDS_L-s2.err_VDS_L),2));



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

    string filename = "EXHA";
    saveSimulations(simulations, filename + ".exp");
    saveSimulations(pareto_set, filename + ".pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    saveStats(stats, filename + ".stat");
}


//**************************************************************

vector<Simulation> Explorer::normalize(const vector<Simulation>& sims)
{
    assert(false);
    vector<Simulation> sorted1 = sort_by_VDS(sims);
    vector<Simulation> sorted2 = sort_by_VGS(sims);
    vector<Simulation> sorted3 = sort_by_ID(sims);

    double min_VDS = sorted1[0].avg_err_VDS;
    double max_VDS = sorted1[sorted1.size()-1].avg_err_VDS;

    double min_VGS = sorted2[0].avg_err_VGS;
    double max_VGS = sorted2[sorted2.size()-1].avg_err_VGS;

    double min_ID = sorted3[0].avg_err_ID;
    double max_ID = sorted3[sorted3.size()-1].avg_err_ID;

    vector<Simulation> normalized;

    for (unsigned int i=0;i<sorted1.size();i++)
    {
        Simulation temp_sim;
        temp_sim = sims[i];

        temp_sim.avg_err_VGS = (temp_sim.avg_err_VGS)/(max_VGS);
        temp_sim.avg_err_VDS = (temp_sim.avg_err_VDS)/(max_VDS);
        temp_sim.avg_err_ID   = (temp_sim.avg_err_ID) / max_ID;

        normalized.push_back(temp_sim);
    }
    return normalized;
}

//**************************************************************
// from min product to max product
vector<Simulation> Explorer::sort_by_VGSVDS_product(vector<Simulation> sims)
{
    int min_index;
    double min_product;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_product = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if ( (sims[i].avg_err_VDS)*(sims[i].avg_err_VGS)  <= min_product)
            {
                //cout << "\nDEBUG:(i="<<i<< ") " << sims[i].avg_err_VDS << " <= " << min_exec_time;
                min_product= sims[i].avg_err_VDS * sims[i].avg_err_VGS;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);
        sims.erase(sims.begin()+min_index);
    }
    return temp;
}


vector<Simulation> Explorer::sort_by_VDS(vector<Simulation> sims)
{
    int min_index;
    double min_exec_time;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_exec_time = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if (sims[i].avg_err_VDS<=min_exec_time)
            {
                min_exec_time=sims[i].avg_err_VDS;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);
        sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << M9DSE_TAG << "DEBUG temp(ordinato): " << temp[i].avg_err_VDS;
#endif

    return temp;
}

//********************************************************************
vector<Simulation> Explorer::sort_by_VGS(vector<Simulation> sims)
{
    int min_index;
    double min_VGS;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_VGS = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if (sims[i].avg_err_VGS<=min_VGS)
            {
                min_VGS=sims[i].avg_err_VGS;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);

        sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << M9DSE_TAG << "DEBUG (orderer E): " << temp[i].avg_err_VGS;
#endif
    return temp;
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::sort_by_ID(vector<Simulation> sims)
{
    int min_index;
    double min_ID;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
        min_ID = 1000000000000000.0;

        for (unsigned int i=0;i<sims.size();i++)
        {
            if (sims[i].avg_err_ID<=min_ID)
            {
                min_ID=sims[i].avg_err_ID;
                min_index = i;
            }
        }
        temp.push_back(sims[min_index]);
        sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << M9DSE_TAG << "DEBUG (orderer ID): " << temp[i].avg_err_ID;
#endif

    return temp;
}

/////////////////////////////////////////////////////////////////////////////
bool isDominated(Simulation sim, const vector<Simulation>& simulations)
{
    assert(false);

    for(int i=0;i<simulations.size();++i)
    {
        if ((sim.avg_err_VGS>=simulations[i].avg_err_VGS) && (sim.avg_err_VDS>=simulations[i].avg_err_VDS))
            return (true);
    }

    return (false);

}
// wrap function which determines the right pareto function to be 
// called 
vector<Simulation> Explorer::get_pareto(const vector<Simulation>& simulations)
{
    if ( (Options.objective_avg_errID) && (Options.objective_avg_errVGS) && (Options.objective_avg_errVDS) )
        return getPareto3d(simulations);

    if ( (Options.objective_avg_errVGS) && (Options.objective_avg_errVDS) )
        return get_pareto_VDSVGS(simulations);

    if ( (Options.objective_avg_errID) && (Options.objective_avg_errVDS) )
        return get_pareto_IDVDS(simulations);

    assert(n_obj==6);

    return getPareto6d(simulations);

}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_VDSVGS(const vector<Simulation> &simulations)
{
    double min_e = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_VDS(simulations);

    while (sorted.size()>0)
    {
        if (sorted[0].avg_err_VGS<=min_e)
        {
            if ( (pareto_set.size()>0) && (pareto_set.back().avg_err_VDS==sorted[0].avg_err_VDS))
                pareto_set.pop_back();

            min_e = sorted[0].avg_err_VGS;
            pareto_set.push_back(sorted[0]);
        }
        sorted.erase(sorted.begin());
    }
    return pareto_set;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_IDVDS(const vector<Simulation> &simulations)
{
    double min_ID = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_VDS(simulations);

    while (sorted.size()>0)
    {
        if (sorted[0].avg_err_ID<=min_ID)
        {
            if ( (pareto_set.size()>0) && (pareto_set.back().avg_err_VDS==sorted[0].avg_err_VDS))
                pareto_set.pop_back();

            min_ID = sorted[0].avg_err_ID;
            pareto_set.push_back(sorted[0]);
        }
        sorted.erase(sorted.begin());
    }
    return pareto_set;
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::getPareto6d(const vector<Simulation> &simulations)
{
    vector<Simulation> pareto_set;

    assert(n_obj==6);

    for (int i=0; i<simulations.size(); i++)
    {
        bool dominated = false;

        for (int j=0; j<simulations.size() && !dominated; j++)

            // if it is dominated...
            if (   (simulations[j].err_VGS_H <= simulations[i].err_VGS_H &&
                    simulations[j].err_ID_H <= simulations[i].err_ID_H &&
                    simulations[j].err_VDS_H <= simulations[i].err_VDS_H &&
                    simulations[j].err_VGS_L <= simulations[i].err_VGS_L &&
                    simulations[j].err_ID_L <= simulations[i].err_ID_L &&
                    simulations[j].err_VDS_L <= simulations[i].err_VDS_L)

                   &&
                   // ...but not from an identical sim
                   (simulations[j].err_VGS_H != simulations[i].err_VGS_H &&
                    simulations[j].err_ID_H != simulations[i].err_ID_H &&
                    simulations[j].err_VDS_H != simulations[i].err_VDS_H &&
                    simulations[j].err_VGS_L != simulations[i].err_VGS_L &&
                    simulations[j].err_ID_L != simulations[i].err_ID_L &&
                    simulations[j].err_VDS_L != simulations[i].err_VDS_L))

                dominated = true;

        // avoid repeated pareto configs
        if ( (!dominated) && (simulation_present(simulations[i],pareto_set) == -1) )
            pareto_set.push_back(simulations[i]);
    }

    return pareto_set;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::getPareto3d(const vector<Simulation> &simulations)
{
    vector<Simulation> pareto_set;

    assert(n_obj==3);

    for (int i=0; i<simulations.size(); i++)
    {
        bool dominated = false;

        for (int j=0; j<simulations.size() && !dominated; j++)

            if (   // if it is dominated...
                    (simulations[j].avg_err_VGS <= simulations[i].avg_err_VGS &&
                     simulations[j].avg_err_ID <= simulations[i].avg_err_ID &&
                     simulations[j].avg_err_VDS <= simulations[i].avg_err_VDS)
                    &&
                    ( // ...but not from an identical sim
                            simulations[j].avg_err_VGS != simulations[i].avg_err_VGS ||
                            simulations[j].avg_err_ID != simulations[i].avg_err_ID ||
                            simulations[j].avg_err_VDS != simulations[i].avg_err_VDS)
                    )
                dominated = true;

        // avoid repeated pareto configs
        if ( (!dominated) && (simulation_present(simulations[i],pareto_set) == -1) )
            pareto_set.push_back(simulations[i]);
    }

    return pareto_set;
}
// power/avg_err_ID pareto set



////////////////////////////////////////////////////////////////////////////
void Explorer::saveConfigurations(const vector<Configuration> &space, const string &filename)
{
    vector<Simulation> pseudo_sims;
    Simulation pseudo_sim;

    for (unsigned int i = 0;i< space.size();i++)
    {
        pseudo_sim.config = space[i];
        pseudo_sim.avg_err_ID = 0.0;
        pseudo_sim.avg_err_VDS = 0.0;
        pseudo_sim.avg_err_VGS = 0.0;
        pseudo_sim.simulated = false;
        pseudo_sims.push_back(pseudo_sim);
    }
    saveSimulations(pseudo_sims, filename);
}
////////////////////////////////////////////////////////////////////////////
void Explorer::saveSimulations(const vector<Simulation> &simulations, const string &filename)
{
    double ID,VGS,VDS;
    double ID_H,VGS_H,VDS_H;
    double ID_L,VGS_L,VDS_L;

    FILE * fp;

    string path = get_base_dir();

    string file = path+string(M9DSE_PATH)+filename;
    fp=fopen(file.c_str(),"w");

    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    string pretitle = "\n%% M9DSE Explorer simulation file - created on " + uora;

    string title = "\n\n%% ";

    fprintf(fp,"\n%% ----------------------------------------------");
    // currently, avg_err_VGS and power are mutually exclusive objectives
    if (n_obj==3)
        title+="VGS\tVDS\tID\tL_d_int\tL_s_int\tL_g_int\tL_d_pin\tL_s_pin\tL_g_pin\tL_dH_ext\tL_sH_ext\tL_gH_ext\tL_dL_ext\tL_sL_ext\tL_gL_ext\tL_Hwire\tL_Lwire";
    else if (n_obj==6)
        title+="VGS_H\tVGS_L\tVDS_H\tVDS_L\tID_H\tID_L\tL_d_int\tL_s_int\tL_g_int\tL_d_pin\tL_s_pin\tL_g_pin\tL_dH_ext\tL_sH_ext\tL_gH_ext\tL_dL_ext\tL_sL_ext\tL_gL_ext\tL_Hwire\tL_Lwire";
    else assert(false);

    fprintf(fp,"\n%% %s",title.c_str());
    fprintf(fp,"\n%% ----------------------------------------------");

    for (unsigned int i =0;i<simulations.size();i++)
    {
        string conf_string = simulations[i].config.get_string();

        if (n_obj==3){
            VGS = simulations[i].avg_err_VGS;
            VDS = simulations[i].avg_err_VDS;
            ID = simulations[i].avg_err_ID;
            fprintf(fp,"\n%.9f\t%.9f\t%.9f\t%s",VGS,VDS,ID,conf_string.c_str());
        }
        else if (n_obj==6){
            VGS_H = simulations[i].err_VGS_H;
            VDS_H = simulations[i].err_VDS_H;
            ID_H = simulations[i].err_ID_H;
            VGS_L = simulations[i].err_VGS_L;
            VDS_L = simulations[i].err_VDS_L;
            ID_L = simulations[i].err_ID_L;
            fprintf(fp,"\n%.9f\t%.9f\t%.9f\t%.9f\t%.9f\t%.9f\t%s",VGS_H,VGS_L,VDS_H,VDS_L,ID_H,ID_L,conf_string.c_str());
        }
        else assert(false);
    }
    fclose(fp);
}

void Explorer::saveObjectivesDetails(const Dynamic_stats &dyn, const Configuration &config, const string filename) const
{
    FILE * fp;
    fp=fopen(filename.c_str(),"a");

    string c = config.get_string();
    //
    fprintf(fp,"\n %g %g %g %g %g %g %s", dyn.err_VGS_L, dyn.err_VDS_L,
            dyn.err_ID_L, dyn.err_VGS_H, dyn.err_VDS_H, dyn.err_ID_H, c.c_str());

    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
void Explorer::saveStats(const Exploration_stats &stats, const string &file)
{
    FILE * fp;
    string file_path = getenv(BASE_DIR)+string(M9DSE_PATH)+file;

    fp = fopen(file_path.c_str(),"w");

    // we want the total size of the space of all possible
    // configurations

    int elapsed_time = (int)difftime(stats.end_time,stats.start_time)/60;

    fprintf(fp,"\n Space size: %g",stats.space_size);
    fprintf(fp,"\n simulations: %d ",stats.n_sim);
    fprintf(fp,"\n total exploration time: %d minutes ",elapsed_time);
    fprintf(fp,"\n simulation start: %s ",asctime(localtime(&stats.start_time)));
    fprintf(fp,"\n simulation end: %s ",asctime(localtime(&stats.end_time)));

    fclose(fp);
}


////////////////////////////////////////////////////////////////////////////
void Explorer::save_estimation_file(const Dynamic_stats &dynamic_stats, const Estimate &estimate, string &filename) const
{
    string file_path =  getenv(BASE_DIR)+string(M9DSE_PATH)+ filename;

    std::ofstream output_file(file_path.c_str());

    if (!output_file)
    {
        string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"WARNING: Error while saving " + file_path);
    }
    else {
        output_file << "\n >>>>>>>> M9DSE Explorer estimation file: " << filename;
        output_file << "\n";
        output_file << "\n **************************************************";
        output_file << "\n" << model_inverter.L_d_int.get_val();
        output_file << ","  << model_inverter.L_s_int.get_val();
        output_file << ","  << model_inverter.L_g_int.get_val();
        output_file << ","  << model_inverter.L_d_pin.get_val();
        output_file << ","  << model_inverter.L_s_pin.get_val();
        output_file << ","  << model_inverter.L_g_pin.get_val();
        output_file << ","  << model_inverter.L_dH_ext.get_val();
        output_file << ","  << model_inverter.L_sH_ext.get_val();
        output_file << ","  << model_inverter.L_gH_ext.get_val();
        output_file << ","  << model_inverter.L_dL_ext.get_val();
        output_file << ","  << model_inverter.L_sL_ext.get_val();
        output_file << ","  << model_inverter.L_gL_ext.get_val();
        output_file << ","  << model_inverter.L_Hwire.get_val();
        output_file << ","  << model_inverter.L_Lwire.get_val();
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

////////////////////////////////////////////////////////////////////////////
vector<bool> Explorer::get_boolean_mask(const Space_mask& mask)
{
    vector<bool> b;

    b.push_back(mask.L_d_int);
    b.push_back(mask.L_s_int);
    b.push_back(mask.L_g_int);
    b.push_back(mask.L_d_pin);
    b.push_back(mask.L_s_pin);
    b.push_back(mask.L_g_pin);
    b.push_back(mask.L_dH_ext);
    b.push_back(mask.L_sH_ext);
    b.push_back(mask.L_gH_ext);
    b.push_back(mask.L_dL_ext);
    b.push_back(mask.L_sL_ext);
    b.push_back(mask.L_gL_ext);
    b.push_back(mask.L_Hwire);
    b.push_back(mask.L_Lwire);

    return b;
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::create_space_mask(const vector<bool>& boolean_mask)
{
    Space_mask mask;

    mask.L_d_int = boolean_mask[0];
    mask.L_s_int = boolean_mask[1];
    mask.L_g_int = boolean_mask[2];
    mask.L_d_pin = boolean_mask[3];
    mask.L_s_pin = boolean_mask[4];
    mask.L_g_pin = boolean_mask[5];
    mask.L_dH_ext = boolean_mask[6];
    mask.L_sH_ext = boolean_mask[7];
    mask.L_gH_ext = boolean_mask[8];
    mask.L_dL_ext = boolean_mask[9];
    mask.L_sL_ext = boolean_mask[10];
    mask.L_gL_ext = boolean_mask[11];
    mask.L_Hwire = boolean_mask[12];
    mask.L_Lwire = boolean_mask[13];

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

vector<Configuration> Explorer::build_space(const Space_mask& mask,Configuration base_conf)
{
    vector<Configuration> space;

    model_inverter.L_d_int.set_to_first();
    do {
        if (mask.L_d_int) base_conf.L_d_int = model_inverter.L_d_int.get_val();
        model_inverter.L_s_int.set_to_first();
        do {
            if (mask.L_s_int) base_conf.L_s_int = model_inverter.L_s_int.get_val();
            model_inverter.L_g_int.set_to_first();
            do {
                if (mask.L_g_int) base_conf.L_g_int = model_inverter.L_g_int.get_val();
                model_inverter.L_d_pin.set_to_first();
                do {
                    if (mask.L_d_pin) base_conf.L_d_pin = model_inverter.L_d_pin.get_val();
                    model_inverter.L_s_pin.set_to_first();
                    do {
                        if (mask.L_s_pin) base_conf.L_s_pin = model_inverter.L_s_pin.get_val();
                        model_inverter.L_g_pin.set_to_first();
                        do {
                            if (mask.L_g_pin) base_conf.L_g_pin = model_inverter.L_g_pin.get_val();
                            model_inverter.L_dH_ext.set_to_first();
                            do {
                                if (mask.L_dH_ext) base_conf.L_dH_ext = model_inverter.L_dH_ext.get_val();
                                model_inverter.L_sH_ext.set_to_first();
                                do {
                                    if (mask.L_sH_ext) base_conf.L_sH_ext = model_inverter.L_sH_ext.get_val();
                                    model_inverter.L_gH_ext.set_to_first();
                                    do {
                                        if (mask.L_gH_ext) base_conf.L_gH_ext = model_inverter.L_gH_ext.get_val();
                                        model_inverter.L_dL_ext.set_to_first();
                                        do {
                                            if (mask.L_dL_ext) base_conf.L_dL_ext = model_inverter.L_dL_ext.get_val();
                                            model_inverter.L_sL_ext.set_to_first();
                                            do {
                                                if (mask.L_sL_ext) base_conf.L_sL_ext = model_inverter.L_sL_ext.get_val();
                                                model_inverter.L_gL_ext.set_to_first();
                                                do {
                                                    if (mask.L_gL_ext) base_conf.L_gL_ext = model_inverter.L_gL_ext.get_val();
                                                    model_inverter.L_Hwire.set_to_first();
                                                    do {
                                                        if (mask.L_Hwire) base_conf.L_Hwire = model_inverter.L_Hwire.get_val();
                                                        model_inverter.L_Lwire.set_to_first();
                                                        do {
                                                            if (mask.L_Lwire) base_conf.L_Lwire = model_inverter.L_Lwire.get_val();
                                                            ///////////////////////////////////////////////////////////
                                                            // Inner loop                                            //
                                                            // If all cache parameters are valid                     //
                                                            if (base_conf.is_feasible()) space.push_back(base_conf); //
                                                            ///////////////////////////////////////////////////////////
                                                        } while ( (mask.L_Lwire)  && (model_inverter.L_Lwire.increase()));
                                                    } while ( (mask.L_Hwire)  && (model_inverter.L_Hwire.increase()));
                                                } while ( (mask.L_gL_ext) && (model_inverter.L_gL_ext.increase()));
                                            } while ( (mask.L_sL_ext) && (model_inverter.L_sL_ext.increase()));
                                        } while ( (mask.L_dL_ext) && (model_inverter.L_dL_ext.increase()));
                                    } while ( (mask.L_gH_ext) && (model_inverter.L_gH_ext.increase()));
                                } while ( (mask.L_sH_ext) && (model_inverter.L_sH_ext.increase()));
                            } while ( (mask.L_dH_ext) && (model_inverter.L_dH_ext.increase()));
                        } while ( (mask.L_g_pin)  && (model_inverter.L_g_pin.increase()));
                    } while ( (mask.L_s_pin)  && (model_inverter.L_s_pin.increase()));
                } while ( (mask.L_d_pin)  && (model_inverter.L_d_pin.increase()));
            } while ( (mask.L_g_int)  && (model_inverter.L_g_int.increase()));
        } while ( (mask.L_s_int)  && (model_inverter.L_s_int.increase()));
    } while ( (mask.L_d_int)  && (model_inverter.L_d_int.increase()));

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

    return build_space(mask,default_conf);
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
        if (mask1.L_d_int ) base_conf.L_d_int  = s1[n1].L_d_int ;
        if (mask1.L_s_int ) base_conf.L_s_int  = s1[n1].L_s_int ;
        if (mask1.L_g_int ) base_conf.L_g_int  = s1[n1].L_g_int ;
        if (mask1.L_d_pin ) base_conf.L_d_pin  = s1[n1].L_d_pin ;
        if (mask1.L_s_pin ) base_conf.L_s_pin  = s1[n1].L_s_pin ;
        if (mask1.L_g_pin ) base_conf.L_g_pin  = s1[n1].L_g_pin ;
        if (mask1.L_dH_ext) base_conf.L_dH_ext = s1[n1].L_dH_ext;
        if (mask1.L_sH_ext) base_conf.L_sH_ext = s1[n1].L_sH_ext;
        if (mask1.L_gH_ext) base_conf.L_gH_ext = s1[n1].L_gH_ext;
        if (mask1.L_dL_ext) base_conf.L_dL_ext = s1[n1].L_dL_ext;
        if (mask1.L_sL_ext) base_conf.L_sL_ext = s1[n1].L_sL_ext;
        if (mask1.L_gL_ext) base_conf.L_gL_ext = s1[n1].L_gL_ext;
        if (mask1.L_Hwire ) base_conf.L_Hwire  = s1[n1].L_Hwire ;
        if (mask1.L_Lwire ) base_conf.L_Lwire  = s1[n1].L_Lwire ;

        for(unsigned int n2=0;n2<s2.size();n2++)
        {
            if (mask2.L_d_int ) base_conf.L_d_int  = s2[n2].L_d_int ;
            if (mask2.L_s_int ) base_conf.L_s_int  = s2[n2].L_s_int ;
            if (mask2.L_g_int ) base_conf.L_g_int  = s2[n2].L_g_int ;
            if (mask2.L_d_pin ) base_conf.L_d_pin  = s2[n2].L_d_pin ;
            if (mask2.L_s_pin ) base_conf.L_s_pin  = s2[n2].L_s_pin ;
            if (mask2.L_g_pin ) base_conf.L_g_pin  = s2[n2].L_g_pin ;
            if (mask2.L_dH_ext) base_conf.L_dH_ext = s2[n2].L_dH_ext;
            if (mask2.L_sH_ext) base_conf.L_sH_ext = s2[n2].L_sH_ext;
            if (mask2.L_gH_ext) base_conf.L_gH_ext = s2[n2].L_gH_ext;
            if (mask2.L_dL_ext) base_conf.L_dL_ext = s2[n2].L_dL_ext;
            if (mask2.L_sL_ext) base_conf.L_sL_ext = s2[n2].L_sL_ext;
            if (mask2.L_gL_ext) base_conf.L_gL_ext = s2[n2].L_gL_ext;
            if (mask2.L_Hwire ) base_conf.L_Hwire  = s2[n2].L_Hwire ;
            if (mask2.L_Lwire ) base_conf.L_Lwire  = s2[n2].L_Lwire ;

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
        if ((conf.L_d_int  == space[i].L_d_int )
        &&  (conf.L_s_int  == space[i].L_s_int )
        &&  (conf.L_g_int  == space[i].L_g_int )
        &&  (conf.L_d_pin  == space[i].L_d_pin )
        &&  (conf.L_s_pin  == space[i].L_s_pin )
        &&  (conf.L_g_pin  == space[i].L_g_pin )
        &&  (conf.L_dH_ext == space[i].L_dH_ext)
        &&  (conf.L_sH_ext == space[i].L_sH_ext)
        &&  (conf.L_gH_ext == space[i].L_gH_ext)
        &&  (conf.L_dL_ext == space[i].L_dL_ext)
        &&  (conf.L_sL_ext == space[i].L_sL_ext)
        &&  (conf.L_gL_ext == space[i].L_gL_ext)
        &&  (conf.L_Hwire  == space[i].L_Hwire )
        &&  (conf.L_Lwire  == space[i].L_Lwire )
        )   return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
int Explorer::simulation_present(const Simulation& sim,const vector<Simulation>& simulations) const
{
    if (simulations.size()==0) return (-1);

    for (int i=0;i<simulations.size();++i)
    {
        if (n_obj==3){
            if ( (simulations[i].avg_err_ID == sim.avg_err_ID) &&
                 (simulations[i].avg_err_VGS == sim.avg_err_VGS) &&
                 (simulations[i].avg_err_VDS == sim.avg_err_VDS) )
                return (i);
        }
        else if (n_obj==6){
            if (
                    (simulations[i].err_ID_H == sim.err_ID_H) &&
                    (simulations[i].err_ID_L == sim.err_ID_L) &&
                    (simulations[i].err_VGS_H == sim.err_VGS_H) &&
                    (simulations[i].err_VGS_L == sim.err_VGS_L) &&
                    (simulations[i].err_VDS_H == sim.err_VDS_H) &&
                    (simulations[i].err_VDS_L == sim.err_VDS_L) )
                return (i);
        }
        else
            assert(false);
    }
    return (-1);
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::mask_union(Space_mask& m1,Space_mask& m2) const
{
    Space_mask mask;

    mask.L_d_int  = (m1.L_d_int ) || (m2.L_d_int );
    mask.L_s_int  = (m1.L_s_int ) || (m2.L_s_int );
    mask.L_g_int  = (m1.L_g_int ) || (m2.L_g_int );
    mask.L_d_pin  = (m1.L_d_pin ) || (m2.L_d_pin );
    mask.L_s_pin  = (m1.L_s_pin ) || (m2.L_s_pin );
    mask.L_g_pin  = (m1.L_g_pin ) || (m2.L_g_pin );
    mask.L_dH_ext = (m1.L_dH_ext) || (m2.L_dH_ext);
    mask.L_sH_ext = (m1.L_sH_ext) || (m2.L_sH_ext);
    mask.L_gH_ext = (m1.L_gH_ext) || (m2.L_gH_ext);
    mask.L_dL_ext = (m1.L_dL_ext) || (m2.L_dL_ext);
    mask.L_sL_ext = (m1.L_sL_ext) || (m2.L_sL_ext);
    mask.L_gL_ext = (m1.L_gL_ext) || (m2.L_gL_ext);
    mask.L_Hwire  = (m1.L_Hwire ) || (m2.L_Hwire );
    mask.L_Lwire  = (m1.L_Lwire ) || (m2.L_Lwire );

    return mask;

}

////////////////////////////////////////////////////////////////////////////
// compute the total number of configurations considering all
// possible values of paremeters whose mask boolean is set true

long double Explorer::get_space_size(const Space_mask& mask) const
{
    double size = 1;

    if (mask.L_d_int ) size = size*(model_inverter.L_d_int.get_size());
    if (mask.L_s_int ) size = size*(model_inverter.L_s_int.get_size());
    if (mask.L_g_int ) size = size*(model_inverter.L_g_int.get_size());
    if (mask.L_d_pin ) size = size*(model_inverter.L_d_pin.get_size());
    if (mask.L_s_pin ) size = size*(model_inverter.L_s_pin.get_size());
    if (mask.L_g_pin ) size = size*(model_inverter.L_g_pin.get_size());
    if (mask.L_dH_ext) size = size*(model_inverter.L_dH_ext.get_size());
    if (mask.L_sH_ext) size = size*(model_inverter.L_sH_ext.get_size());
    if (mask.L_gH_ext) size = size*(model_inverter.L_gH_ext.get_size());
    if (mask.L_dL_ext) size = size*(model_inverter.L_dL_ext.get_size());
    if (mask.L_sL_ext) size = size*(model_inverter.L_sL_ext.get_size());
    if (mask.L_gL_ext) size = size*(model_inverter.L_gL_ext.get_size());
    if (mask.L_Hwire ) size = size*(model_inverter.L_Hwire.get_size());
    if (mask.L_Lwire ) size = size*(model_inverter.L_Lwire.get_size());

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
void Explorer::reset_sim_counter()
{
    sim_counter=0;
    unique_configs.clear();
}

////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParametersNumber()
{

    vector<pair<int,int> > v;

    v.push_back(pair<int,int>(1,model_inverter.L_d_int.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_s_int.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_g_int.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_d_pin.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_s_pin.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_g_pin.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_dH_ext.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_sH_ext.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_gH_ext.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_dL_ext.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_sL_ext.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_gL_ext.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_Hwire.get_size()));
    v.push_back(pair<int,int>(1,model_inverter.L_Lwire.get_size()));


    return v;
}

////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParameterRanges()
{
    vector<pair<int,int> > v;

    v.push_back(pair<int,int>(model_inverter.L_d_int.get_first(), model_inverter.L_d_int.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_s_int.get_first(), model_inverter.L_s_int.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_g_int.get_first(), model_inverter.L_g_int.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_d_pin.get_first(), model_inverter.L_d_pin.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_s_pin.get_first(), model_inverter.L_s_pin.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_g_pin.get_first(), model_inverter.L_g_pin.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_dH_ext.get_first(), model_inverter.L_dH_ext.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_sH_ext.get_first(), model_inverter.L_sH_ext.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_gH_ext.get_first(), model_inverter.L_gH_ext.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_dL_ext.get_first(), model_inverter.L_dL_ext.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_sL_ext.get_first(), model_inverter.L_sL_ext.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_gL_ext.get_first(), model_inverter.L_gL_ext.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_Hwire.get_first(), model_inverter.L_Hwire.get_last()));
    v.push_back(pair<int,int>(model_inverter.L_Lwire.get_first(), model_inverter.L_Lwire.get_last()));
    return v;
}



////////////////////////////////////////////////////////////////////////////


void Explorer::load_space_file(const string& filename)
{
    std::ifstream input_file (filename.c_str());
    string word;

    if (!input_file)
    {
        string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"ERROR: cannot load " + filename);
        exit(-1);
    }
    else
    {
        go_until("[BEGIN_SPACE]",input_file);

        double default_val;
        vector<double> values;

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_d_int.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_s_int.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_g_int.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_d_pin.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_s_pin.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_g_pin.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_dH_ext.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_sH_ext.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_gH_ext.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_dL_ext.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_sL_ext.set_values(values,default_val);


        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_gL_ext.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_Hwire.set_values(values,default_val);

        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>default_val) && (default_val!=0)) { values.push_back(default_val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> default_val;  // get default value
        model_inverter.L_Lwire.set_values(values,default_val);

    }

}

string Explorer::get_space_file_line(Parameter param)
{
    std::ostringstream output_stringstream;

    param.set_to_first();
    output_stringstream << "\n" << param.get_label() << " ";
    do { output_stringstream << param.get_val() << " "; } while (param.increase());
    output_stringstream << "0";
    output_stringstream << "\n DEFAULT " << param.get_default();

    return output_stringstream.str();
}

void Explorer::save_space_file(const string& filename)
{
    std::ofstream output_file(filename.c_str());
    if (!output_file)
    {
        cout << "\n Error while saving " << filename ;
        string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"ERROR: cannot save " + filename);
        sleep(2);
    }
    else
    {
        output_file << "\n\n [BEGIN_SPACE]";
        output_file << get_space_file_line(model_inverter.L_d_int);
        output_file << get_space_file_line(model_inverter.L_s_int);
        output_file << get_space_file_line(model_inverter.L_g_int);
        output_file << get_space_file_line(model_inverter.L_d_pin);
        output_file << get_space_file_line(model_inverter.L_s_pin);
        output_file << get_space_file_line(model_inverter.L_g_pin);
        output_file << get_space_file_line(model_inverter.L_dH_ext);
        output_file << get_space_file_line(model_inverter.L_sH_ext);
        output_file << get_space_file_line(model_inverter.L_gH_ext);
        output_file << get_space_file_line(model_inverter.L_dL_ext);
        output_file << get_space_file_line(model_inverter.L_sL_ext);
        output_file << get_space_file_line(model_inverter.L_gL_ext);
        output_file << get_space_file_line(model_inverter.L_Hwire);
        output_file << get_space_file_line(model_inverter.L_Lwire);
        output_file << "\n\n [END_SPACE]";
    }

}

string Explorer::get_base_dir() const
{
    return getenv(BASE_DIR);
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

    saveSimulations(pareto_set,filename+".pareto.exp");
    */

}

