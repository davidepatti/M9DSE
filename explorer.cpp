
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

    // by default objectives include avg_err_VDS and average power
    Options.objective_avg_errID = true;
    Options.objective_avg_errVDS = true;
    Options.objective_avg_errVGS = true;

    Options.save_spaces = false;
    Options.save_estimation = false;

    Options.save_objectives_details = false;
    force_simulation = false;
    function_approx = NULL;

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


//********************************************************************
void Explorer::set_options(const struct UserSettings& user_settings)
{
    Options = user_settings;

    // Number of objectives
    n_obj = 0;

    if (Options.objective_avg_errVGS) n_obj++;
    if (Options.objective_avg_errVDS) n_obj++;
    if (Options.objective_avg_errID) n_obj++;

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
    save_simulations(simulations,filename+".exp");
    save_simulations(pareto_set,filename+".pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");
}


//**************************************************************

vector<Simulation> Explorer::normalize(const vector<Simulation>& sims)
{
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
        return get_pareto3d(simulations);

    if ( (Options.objective_avg_errVGS) && (Options.objective_avg_errVDS) )
        return get_pareto_VDSVGS(simulations);

    if ( (Options.objective_avg_errID) && (Options.objective_avg_errVDS) )
        return get_pareto_IDVDS(simulations);
    assert(false);
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
vector<Simulation> Explorer::get_pareto_IDVGS(const vector<Simulation> &simulations)
{
    double min_ID = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_VGS(simulations);

    while (sorted.size()>0)
    {
        if (sorted[0].avg_err_ID<=min_ID)
        {
            if ( (pareto_set.size()>0) && (pareto_set.back().avg_err_VGS==sorted[0].avg_err_VGS))
                pareto_set.pop_back();

            min_ID = sorted[0].avg_err_ID;
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
void Explorer::save_configurations(const vector<Configuration>& space, const string& filename)
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
    save_simulations(pseudo_sims,filename);
}
////////////////////////////////////////////////////////////////////////////
void Explorer::save_simulations(const vector<Simulation>& simulations, const string& filename)
{
    double id,VGS;
    double VGSVDS;
    double vds;

    FILE * fp;

    string path = get_base_dir();

    string file_path = path+filename;

    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    string pretitle = "\n%% M9DSE Explorer simulation file - created on " + uora;

    string pretitle2 ="\n%% Objectives: ";
    //G

    if (Options.objective_avg_errID) pretitle2+="ID, ";
    if (Options.objective_avg_errVGS) pretitle2+="VGS, ";
    if (Options.objective_avg_errVDS) pretitle2+="VDS";

    string title = "\n\n%% ";

    // currently, avg_err_VGS and power are mutually exclusive objectives
    if (Options.objective_avg_errVGS) title+="ID\tVGS\tVDS";
    else
        assert(false);

    fp=fopen(file_path.c_str(),"w");

    fprintf(fp,"\n%% ----------------------------------------------");
    fprintf(fp,"\n%% ----------------------------------------------");

    for (unsigned int i =0;i<simulations.size();i++)
    {
        id = simulations[i].avg_err_ID;
        VGS = simulations[i].avg_err_VGS;
        vds = simulations[i].avg_err_VDS;

        string conf_string = simulations[i].config.get_header();


        char ch = ' ';
        if (!simulations[i].simulated) ch = '*';

        fprintf(fp,"\n%.9f  %.9f  %.9f %%/  %s %c",id,VGS,vds,conf_string.c_str(),ch);
    }
    fclose(fp);
}

void Explorer::save_objectives_details(const Dynamic_stats& dyn,const Configuration& config, const string filename) const
{
    FILE * fp;
    fp=fopen(filename.c_str(),"a");

    string c = config.get_header();


    //
    fprintf(fp,"\n %g %g %g %g %g %g %s", dyn.err_VGS_L, dyn.err_ID_L,
            dyn.err_VDS_L, dyn.err_VGS_H, dyn.err_ID_H, dyn.err_VDS_H, c.c_str());

    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_stats(const Exploration_stats& stats,const string& file)
{
    FILE * fp;
    string file_path = get_base_dir()+"/matlab-workspace/M9-explorer/"+file;

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

/////////////////////////////////////////////////////////////////
void Explorer::prepare_explorer( const Configuration& config)
{
    // Note that this fuction does NOT create any directory, but
    // simply sets explorer class members according to the app and configuration
}


////////////////////////////////////////////////////////////////////////////
void Explorer::save_estimation_file(const Dynamic_stats &dynamic_stats, const Estimate &estimate, string &filename) const
{
    string file_path;
    file_path = get_base_dir()+"/matlab-workspace/M9-explorer/";
    file_path += filename;

    std::ofstream output_file(file_path.c_str());

    if (!output_file)
    {
        string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
        int myid = get_mpi_rank();
        write_to_log(myid,logfile,"WARNING: Error while saving " + file_path);
    }
    else {
        // TODO: M9DSE FIX
        /*
        output_file << "\n >>>>>>>> M9DSE Explorer estimation file: " << filename;
        output_file << "\n";
        output_file << "\n **************************************************";
        output_file << "\n " << processor.L_d_int.get_val();
        output_file << "\n " << processor.L_g_pin.get_val();
        output_file << "," << processor.L_dH_ext.get_val();
        output_file << "," << processor.L_sH_ext.get_val();
        output_file << "," << processor.L_gH_ext.get_val();
        output_file << "," << processor.L_dL_ext.get_val();
         */
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

    assert(false);
    /* TODO M9FIX
    b.push_back(mask.L1D_size);
    b.push_back(mask.L1D_block);
     */
    return b;
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::create_space_mask(const vector<bool>& boolean_mask)
{
    Space_mask mask;

    /* TODO: M9DSE
    mask.L1D_size = boolean_mask[0];
    mask.L1D_block = boolean_mask[1];
    mask.L1D_assoc = boolean_mask[2];
     */

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
/*
    model_inverter.num_clusters.set_to_first();
    do {
        if (mask.num_clusters) base_conf.num_clusters=model_inverter.num_clusters.get_val();

        model_inverter.integer_units.set_to_first();
        do {
            if (mask.integer_units) base_conf.integer_units=model_inverter.integer_units.get_val();

            model_inverter.float_units.set_to_first();
            do {
                if (mask.float_units) base_conf.float_units=model_inverter.float_units.get_val();

                model_inverter.memory_units.set_to_first();
                do {
                    if (mask.memory_units) base_conf.memory_units=model_inverter.memory_units.get_val();

                    model_inverter.branch_units.set_to_first();
                    do {
                        if (mask.branch_units) base_conf.branch_units=model_inverter.branch_units.get_val();

                        model_inverter.gpr_static_size.set_to_first();
                        do {
                            if (mask.gpr_static_size) base_conf.gpr_static_size=model_inverter.gpr_static_size.get_val();

                            model_inverter.fpr_static_size.set_to_first();
                            do {
                                if (mask.fpr_static_size) base_conf.fpr_static_size=model_inverter.fpr_static_size.get_val();

                                model_inverter.pr_static_size.set_to_first();
                                do {
                                    if (mask.pr_static_size) base_conf.pr_static_size=model_inverter.pr_static_size.get_val();

                                    model_inverter.cr_static_size.set_to_first();
                                    do {
                                        if (mask.cr_static_size) base_conf.cr_static_size=model_inverter.cr_static_size.get_val();

                                        model_inverter.btr_static_size.set_to_first();
                                        do {
                                            if (mask.btr_static_size) base_conf.btr_static_size=model_inverter.btr_static_size.get_val();

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
                                        }while ( (mask.btr_static_size) && (model_inverter.btr_static_size.increase() ) );
                                    }while ( (mask.cr_static_size) && (model_inverter.cr_static_size.increase() ) );
                                }while ( (mask.pr_static_size) && (model_inverter.pr_static_size.increase() ) );
                            } while ( (mask.fpr_static_size) && (model_inverter.fpr_static_size.increase() ) );
                        } while ( (mask.gpr_static_size) && (model_inverter.gpr_static_size.increase() ) );
                    } while ( (mask.branch_units) && (model_inverter.branch_units.increase()) );
                } while ( (mask.memory_units) && (model_inverter.memory_units.increase()) ) ;
            } while( (mask.float_units) && (model_inverter.float_units.increase()) );
        } while ( (mask.integer_units) && (model_inverter.integer_units.increase() ));
    } while ( (mask.num_clusters) && (model_inverter.num_clusters.increase() ));

 */
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
        // TODO M9DSE
        //if (mask1.gpr_static_size) base_conf.gpr_static_size=s1[n1].gpr_static_size;

        for(unsigned int n2=0;n2<s2.size();n2++)
        {
            // TODO M9DSE
            //if (mask2.gpr_static_size) base_conf.gpr_static_size=s2[n2].gpr_static_size;
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

    assert(false);
    if (space.size()==0) return false;

    for(unsigned int i = 0;i<space.size();i++)
    {
        /* TODO: M9DSE
        if(   (conf.gpr_static_size==space[i].gpr_static_size)
              &&(conf.fpr_static_size==space[i].fpr_static_size)
              */
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
                (simulations[i].avg_err_ID == sim.avg_err_ID) &&
                (simulations[i].avg_err_VGS == sim.avg_err_VGS) &&
                (simulations[i].avg_err_VDS == sim.avg_err_VDS)
                )
            return (i);
    }
    return (-1);
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::mask_union(Space_mask& m1,Space_mask& m2) const
{
    assert(false);
    Space_mask mask;
    /* TODO: M9DSE
    mask.gpr_static_size = (m1.gpr_static_size) || (m2.gpr_static_size);
     **/
    return mask;

}

////////////////////////////////////////////////////////////////////////////
// compute the total number of configurations considering all
// possible values of paremeters whose mask boolean is set true

long double Explorer::get_space_size(const Space_mask& mask) const
{
    double size = 1;

    assert(false);

    /* TODO: M9DSE
    if (mask.gpr_static_size) size = size*(model_inverter.gpr_static_size.get_size());
     */
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

    assert(false);
    vector<pair<int,int> > v;

    /* TODO M9DSE
    v.push_back(pair<int,int>(1, model_inverter.integer_units.get_size()) );
     */
}

////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParameterRanges()
{
    assert(false);
    vector<pair<int,int> > v;

    /* TODO M9DSE
    v.push_back(pair<int,int>(model_inverter.integer_units.get_first(), model_inverter.integer_units.get_last()));
     */
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

        cout << "LOADING SPACE (false)" << endl;
        //go_until("[BEGIN_SPACE]",input_file);

        int val;
        vector<int> values;

        /* TODO M9DSE
        values.clear();
        input_file >> word; // skip parameter label
        while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
        input_file >> word; // skip default label
        input_file >> val;  // get default value
        mem_hierarchy.L1D.block_size.set_values(values,val);
         */


//end add db
    }

}

void Explorer::save_space_file(const string& filename)
{
    assert(false);
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

        /* TODO M9DSE
        model_inverter.integer_units.set_to_first();

        output_file << "\ninteger_units ";
        do { output_file << model_inverter.integer_units.get_val() << " "; } while (model_inverter.integer_units.increase());
        output_file << "0";
        output_file << "\n DEFAULT " << model_inverter.integer_units.get_default();
         ****/

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
