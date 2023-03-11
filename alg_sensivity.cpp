#include "explorer.h"
#include "common.h"
#include <ctime>
// ********************************************************************
//  mono-objective sensivity based approach
// ********************************************************************

void Explorer::start_SAP()
{
    current_algo = "SAP";
    Exploration_stats stats;
    reset_sim_counter();
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);

    string file_name = "SAP_";
    vector<double> sens;
    vector<int> sorted_index;

    Configuration base_conf = create_configuration();

    vector<Space_mask> parameter_masks;

    // for each parameter is created a mask with only one value set to
    // true. These masks will allow to explore all values of each
    // parameter without changing the other
    for (int m=0;m<N_PARAMS;m++)
    {
        vector<bool> boolean_mask(N_PARAMS,false);
        boolean_mask[m] = true;
        Space_mask temp_mask = create_space_mask(boolean_mask);
        parameter_masks.push_back(temp_mask);

        vector<Configuration> space = build_space(temp_mask,base_conf);

        vector<Simulation> sims = simulate_space(space);

        double temp = get_sensivity_VGSVDS(sims);

        sens.push_back(temp);
    }

    string name = get_base_dir() + string(M9DSE_PATH);
    name+= "SAP_sensitivity.stat";

    FILE * fp = fopen(name.c_str(),"w");

    fprintf(fp,"\n%.14f        %% L_d_int",sens[0]);
    fprintf(fp,"\n%.14f        %% L_s_int",sens[1]);
    fprintf(fp,"\n%.14f        %% L_g_int",sens[2]);
    fprintf(fp,"\n%.14f        %% L_d_pin",sens[3]);
    fprintf(fp,"\n%.14f        %% L_s_pin",sens[4]);
    fprintf(fp,"\n%.14f        %% L_g_pin",sens[5]);
    fprintf(fp,"\n%.14f        %% L_dH_ext",sens[6]);
    fprintf(fp,"\n%.14f        %% L_sH_ext",sens[7]);
    fprintf(fp,"\n%.14f        %% L_gH_ext",sens[8]);
    fprintf(fp,"\n%.14f        %% L_dL_ext",sens[9]);
    fprintf(fp,"\n%.14f        %% L_sL_ext",sens[10]);
    fprintf(fp,"\n%.14f        %% L_gL_ext",sens[11]);
    fprintf(fp,"\n%.14f        %% L_Hwire",sens[12]);
    fprintf(fp,"\n%.14f        %% L_Lwire",sens[13]);

    fclose(fp);

    // sort indexes by decreasing sensitivity

    for (unsigned int j=0;j<sens.size();j++)
    {
        int max_index;
        double max_sens = -1;

        for (unsigned int i=0;i<sens.size();i++)
        {
            if (sens[i]>max_sens)
            {
                max_sens = sens[i];
                max_index = i;
            }
        }
        sens[max_index] = -1;
        sorted_index.push_back(max_index);
    }

    for (unsigned int i=0;i<sorted_index.size();i++)
    {
        int index = sorted_index[i];

        // this time there isn't the NO_L2_CHECK option, optimal
        // parameters values must be searched only in the feasible
        // configurations
        vector<Configuration> space = build_space(parameter_masks[index],base_conf);

        vector<Simulation> sims = simulate_space(space);
        vector<Simulation> ordered_sims = sort_by_VGSVDS_product(sims);

        // base_conf must be updated, considering the optimal value
        // of the the last parameter explored
        // base_conf should already have i-1 parameters set to their
        // optimal val, so the following line should change  the
        // last explored parameter

        base_conf = ordered_sims[0].config;

        char temp[10];
        sprintf(temp,"%d_",i);

        saveSimulations(ordered_sims, file_name + string(temp) + ".exp");
    }
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    saveStats(stats, file_name + ".stat");
}

// modified version of SAP, sensivity order of parameters is
// recomputed after a new optimal parameter values is discovered

//**************************************************************

// ********************************************************************
//   Pareto-based sensivity analysis
// ********************************************************************

void Explorer::start_PBSA()
{
    current_algo = "PBSA";
    Exploration_stats stats;
    reset_sim_counter();

    stats.space_size = get_space_size();

    stats.start_time = time(NULL);

    vector<double> sens;

    Configuration base_conf = create_configuration();

    vector<Space_mask> parameter_masks;

    vector<vector<Simulation> > parameters_sim_collection;

    vector<Simulation> all_parameter_sims;

    // for each parameter is created a mask with only one value set to
    // true. These masks will allow to explore all values of each
    // parameter without changing the other
    for (int m=0;m<N_PARAMS;m++)
    {
        vector<bool> boolean_mask(N_PARAMS,false);
        boolean_mask[m] = true;
        Space_mask temp_mask = create_space_mask(boolean_mask);
        parameter_masks.push_back(temp_mask);

        vector<Configuration> space = build_space(temp_mask);
        vector<Simulation> sims = simulate_space(space);

        // cumulate simulations for each parameter
        // this is needed by get_sensivity_PBSA(...)
        append_simulations(all_parameter_sims,sims);
        parameters_sim_collection.push_back(sims);
    }

    int pippo;
    //each element of parameters_sim_collection is a vector of sims
    //where only a particulare parameter is made to vary in its values
    //range
    for(unsigned int p=0;p<parameters_sim_collection.size();p++)
    {
        double temp = get_sensivity_PBSA(parameters_sim_collection[p],all_parameter_sims);
        sens.push_back(temp);
        cout << M9DSE_TAG << "sensivity " << p << ": " << sens[p];
    }

    string name = get_base_dir() + string(M9DSE_PATH);
    name+= "PBSA_sensitivity.stat";

    FILE * fp = fopen(name.c_str(),"w");

    fprintf(fp,"\n%.14f        %% L_d_int",sens[0]);
    fprintf(fp,"\n%.14f        %% L_s_int",sens[1]);
    fprintf(fp,"\n%.14f        %% L_g_int",sens[2]);
    fprintf(fp,"\n%.14f        %% L_d_pin",sens[3]);
    fprintf(fp,"\n%.14f        %% L_s_pin",sens[4]);
    fprintf(fp,"\n%.14f        %% L_g_pin",sens[5]);
    fprintf(fp,"\n%.14f        %% L_dH_ext",sens[6]);
    fprintf(fp,"\n%.14f        %% L_sH_ext",sens[7]);
    fprintf(fp,"\n%.14f        %% L_gH_ext",sens[8]);
    fprintf(fp,"\n%.14f        %% L_dL_ext",sens[9]);
    fprintf(fp,"\n%.14f        %% L_sL_ext",sens[10]);
    fprintf(fp,"\n%.14f        %% L_gL_ext",sens[11]);
    fprintf(fp,"\n%.14f        %% L_Hwire",sens[12]);
    fprintf(fp,"\n%.14f        %% L_Lwire",sens[13]);

    fclose(fp);

    // sort indexes by decreasing sensitivity

    vector<int> sorted_index;

    for (unsigned int j=0;j<sens.size();j++)
    {
        int max_index;
        double max_sens = -1;

        for (unsigned int i=0;i<sens.size();i++)
        {
            if (sens[i]>max_sens)
            {
                max_sens = sens[i];
                max_index = i;
            }
        }
        sens[max_index] = -1;
        sorted_index.push_back(max_index);
    }

    // Initialization of pareto set simulations.
    // Create a space with only the default configuration, simulate it
    // and put the result in the pareto set simulation.

    vector<Configuration> initial_space ;
    initial_space.push_back(base_conf);

    // initial space has only one element ! (SIM_SPACE_18)
    vector<Simulation> pareto_set = simulate_space(initial_space);

    for (unsigned int i=0;i<sorted_index.size();i++)
    {
        int index = sorted_index[i];

        // we must combine two configuration spaces:

        // a space where only one parameter varies on its range
        vector<Configuration> param_space = build_space(parameter_masks[index]);

        // ...and the space of pareto set
        vector<Configuration> pareto_space = extract_space(pareto_set);

        // we must combine each single value of parameter that is
        // modified in the first space with all configuration of the
        // second space

        Space_mask other_params = negate_mask(parameter_masks[index]);

        vector<Configuration> new_space = build_space_cross_merge(param_space,
                                                                  pareto_space,
                                                                  parameter_masks[index],
                                                                  other_params );
        vector<Simulation> sims = simulate_space(new_space);

        append_simulations(pareto_set,sims);
        remove_dominated_simulations(pareto_set);

        char temp[10];
        sprintf(temp,"%d_",i);
        string file_name = "PBSA_stage"+string(temp);
        saveSimulations(pareto_set, file_name + ".exp");
    }

    string file_name = "PBSA";
    saveSimulations(pareto_set, file_name + ".pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    saveStats(stats, file_name + ".stat");
}
// ********************************************************************
// multiobjective sensivity 
// ********************************************************************
double Explorer::get_sensivity_PBSA(const vector<Simulation>& simulations,const vector<Simulation>& all_sims)
{
    assert(n_obj==3);
    vector<Simulation> VGS_sorted = sort_by_VGS(all_sims);
    vector<Simulation> VDS_sorted = sort_by_VDS(all_sims);
    vector<Simulation> ID_sorted = sort_by_ID(all_sims);


    double max_VGS = VGS_sorted[VGS_sorted.size()-1].avg_err_VGS;
    double max_VDS = VDS_sorted[VDS_sorted.size()-1].avg_err_VDS;
    double max_ID = ID_sorted[ID_sorted.size()-1].avg_err_ID;

    // copy simulations as normalized_sims, then each element of
    // normalized_sims will be overwritten with its normalized value
    vector<Simulation> normalized_sims;
    append_simulations(normalized_sims,simulations);

    for (unsigned i=0;i<normalized_sims.size();i++)
    {
        normalized_sims[i].avg_err_VGS = normalized_sims[i].avg_err_VGS/max_VGS;
        normalized_sims[i].avg_err_VDS = normalized_sims[i].avg_err_VDS/max_VDS;
        normalized_sims[i].avg_err_ID = normalized_sims[i].avg_err_ID/max_ID;
    }

    double current_distance;
    double max_distance = 0;

    // depending on which objectives has been selected, 'distance'
    // function will use an appropriate distance metric
    for (unsigned int i=0;i<normalized_sims.size();i++)
    {
        for (unsigned int j=0;j<normalized_sims.size();j++)
        {
            current_distance = distance(normalized_sims[i],normalized_sims[j]);
            if (current_distance>max_distance) max_distance = current_distance;
        }
    }

    return max_distance;
}
//**************************************************************
double Explorer::get_sensivity_VGSVDS(const vector<Simulation>& sims)
{
    assert(n_obj==3);
    vector<Simulation> temp = sort_by_VGSVDS_product(sims);

    double min_product = (temp[0].avg_err_VGS)*(temp[0].avg_err_VDS);
    double max_product = (temp[temp.size()-1].avg_err_VGS)*(temp[temp.size()-1].avg_err_VDS);

    return max_product-min_product;
}
