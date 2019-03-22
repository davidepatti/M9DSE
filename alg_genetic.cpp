
// ********************************************************************
//   GA-based exploration
// ********************************************************************

//******************************************************************************************//
#include "explorer.h"
#include "common.h"
#include "containers.h"
#include "variator.h"
#include "selector.h"
//#include <values.h>
#define CHROMOSOME_DIM N_PARAMS // TO_CHECK
#define DEF_TOURNAMENT 2
#define DEF_HASH_TABLE_SIZE   128

void Explorer::start_GA(const GA_parameters& parameters)
{
    current_algo="GA";
    string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
    int myrank = get_mpi_rank();


    if (Options.approx_settings.enabled == 1)
        current_algo+="_fuzzy";
    if (Options.approx_settings.enabled == 2)
        current_algo+="_ANN";

    eud.ht_ga = new HashGA(DEF_HASH_TABLE_SIZE);
    eud.history.clear();
    eud.pareto.clear();

    Exploration_stats stats;
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);
    reset_sim_counter();

    string file_name;
    file_name = current_algo;

    write_to_log(myrank,logfile,"Starting "+file_name);

    SimulateBestWorst();

    // GA init
    init_GA(); // call it before creating anything GA related

    int pop_size = parameters.population_size;
    common* comm = new common(pop_size, pop_size, pop_size, N_OBJ); // (alpha, mu, lambda, dim)
    variator* var = new variator(parameters.pcrossover, parameters.pmutation, CHROMOSOME_DIM, comm); // (xover_p, mutation_p, chromo_dim)
    selector* sel = new selector(DEF_TOURNAMENT, comm); // (tournament)
    int generation = 0;
    const int MAX_GENERATIONS = parameters.max_generations;

    // GA start
    GA_evaluate(var->get_offspring()); // evaluate initial population

    var->write_ini();		// write ini population
    write_to_log(myrank,logfile,"Initial population"+ to_string(var->get_ini()));

    sel->read_ini();		// read ini population
    sel->select_initial();		// do selection
    sel->write_arc();		// write arc population (all individuals that could ever be used again)
    sel->write_sel();		// write sel population

    while ( generation++ < MAX_GENERATIONS )
    {
        write_to_log(myrank,logfile,"Iteration " + to_string(generation));

        var->read_arc();
        write_to_log(myrank,logfile, "Archive population");
        write_to_log(myrank,logfile, to_string(var->get_arc()));

        var->read_sel();
        write_to_log(myrank,logfile,"Selected population");
        write_to_log(myrank,logfile,to_string(var->get_sel()));

        bool adjustedOperators = false;
        var->variate(adjustedOperators);	// create offspring

        GA_evaluate(var->get_offspring()); // evaluate offspring population

        var->write_var();		// write var population
        write_to_log(myrank,logfile,"Variated population");
        write_to_log(myrank,logfile,to_string(var->get_var()));

        sel->read_var();		// read var population
        sel->select_normal();		// do selection
        sel->write_arc();		// write arc population (all individuals that could ever be used again)
        sel->write_sel();		// write sel population

        // save pareto-set every report_pareto_step generations
        if ( generation % parameters.report_pareto_step == 0)
        {
            char temp[30];
            sprintf(temp, "_%d", generation);
            stats.end_time = time(NULL);
            stats.n_sim = get_sim_counter();
            save_stats(stats,file_name+string(temp)+".stat");
            save_simulations(eud.pareto, file_name+string(temp)+".pareto.exp");
            save_simulations(eud.history, file_name+".history");
        }
    }
    var->read_arc();
    write_to_log(myrank,logfile,"Final archive population");
    write_to_log(myrank,logfile,to_string(var->get_arc()));

    var->write_output();
    // save history
    save_simulations(eud.history, file_name+".history");

    // save statistics
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats, file_name+".stat");
}

//********************************************************************

void Explorer::init_GA()
{
    vector<alleleset> alleles;

    // model_inverter parameters

    // TODO M9fix
    alleles.push_back((model_inverter.L_d_int.get_values()));
    alleles.push_back((model_inverter.L_s_int.get_values()));
    alleles.push_back((model_inverter.L_g_int.get_values()));

    alleles.push_back((model_inverter.L_d_pin.get_values()));
    alleles.push_back((model_inverter.L_s_pin.get_values()));
    alleles.push_back((model_inverter.L_g_pin.get_values()));

    alleles.push_back((model_inverter.L_dH_ext.get_values()));
    alleles.push_back((model_inverter.L_sH_ext.get_values()));
    alleles.push_back((model_inverter.L_gH_ext.get_values()));

    alleles.push_back((model_inverter.L_dL_ext.get_values()));
    alleles.push_back((model_inverter.L_sL_ext.get_values()));
    alleles.push_back((model_inverter.L_gL_ext.get_values()));

    alleles.push_back((model_inverter.L_Hwire.get_values()));
    alleles.push_back((model_inverter.L_Lwire.get_values()));

    individual::setAllelesets(alleles); // set static allele sets genome for individuals
}

/*************************************************************************/

Configuration Explorer::ind2conf(const individual& ind){
    Configuration conf;

    conf.L_d_int= ind.phenotype(0);
    conf.L_s_int= ind.phenotype(1);
    conf.L_g_int= ind.phenotype(2);
    conf.L_d_pin= ind.phenotype(3);
    conf.L_s_pin= ind.phenotype(4);
    conf.L_g_pin= ind.phenotype(5);
    conf.L_dH_ext= ind.phenotype(6);
    conf.L_sH_ext= ind.phenotype(7);
    conf.L_gH_ext= ind.phenotype(8);
    conf.L_dL_ext= ind.phenotype(9);
    conf.L_sL_ext= ind.phenotype(10);
    conf.L_gL_ext= ind.phenotype(11);
    conf.L_Hwire= ind.phenotype(12);
    conf.L_Lwire= ind.phenotype(13);
    return conf;
}

/*************************************************************************/

void Explorer::GA_evaluate(population* pop)
{
    vector<Configuration> vconf;
    vector<int> indexes;
    vector<Simulation> vsim(pop->size());
    vconf.reserve(pop->size());
    indexes.reserve(pop->size());

    string logfile = get_base_dir()+string(M9DSE_LOG_FILE);
    int myrank = get_mpi_rank();

    for(int index=0; index < pop->size(); index++)
    {
        Configuration conf = ind2conf(pop->at(index));
        Simulation sim;
        sim.config = conf;

        if(!conf.is_feasible()){
            assert(false);
            write_to_log(myrank,logfile,"WARNING: GA configuration " + to_string(index) + " not feasible");
            sim.avg_err_VDS = BIG_VDS;
            sim.avg_err_VGS = BIG_VGS;
            sim.avg_err_ID = BIG_ID;
            vsim[index] = sim;
        }
        else {
            Simulation *psim = eud.ht_ga->searchT(sim);
            if(psim != NULL) { // present in cache
                vsim[index] = *psim;
            } else { // not present in cache
                indexes.push_back(index); // save index for later use
                vconf.push_back(conf); // schedule configuration for simulation
            }
        }
    } // for pop


    vector<Simulation> results;

    results = simulate_space(vconf);

    // cache results
    for(int i=0; i<results.size(); i++){
        Simulation sim = results[i];
        eud.history.push_back(sim);
        bool cacheable = sim.simulated;
        if(cacheable){
            eud.ht_ga->addT(sim);
            eud.pareto.push_back(sim);
            eud.pareto = get_pareto(eud.pareto); //FIXME TODO perche lo fa ogni volta invece di farlo solo alla fine?
        } else if (!isDominated(sim, eud.pareto)){ //if it could be a pareto solution, the configuration is simulated
            //FIXME
            //		explorer->set_force_simulation(true);
            //		sim = explorer->simulate_space(vconf)[0];
            //		explorer->set_force_simulation(false);
            eud.history[eud.history.size() - 1] = sim; // updates history with new simulated values
            eud.ht_ga->addT(sim);
            eud.pareto.push_back(sim);
            eud.pareto = get_pareto(eud.pareto); // funziona solo con multibench con la media
        }

        //G
        vsim[indexes[i]] = sim; // update simulation vector with new results
    } // for results


    // reinsert simulation values into GA

    assert(vsim.size() == pop->size());
    //    int disp = bench * n_obj;
    for(int i=0; i<pop->size(); i++)
    {
        (*pop)[i].objectives[0] = vsim[i].avg_err_VDS;

        if( (*pop)[i].objectives_dim() > 1)
            (*pop)[i].objectives[1] = vsim[i].avg_err_VGS;
        if( (*pop)[i].objectives_dim() > 2)
            (*pop)[i].objectives[2] = vsim[i].avg_err_ID;
    }

}

/*************************************************************************/

void Explorer::SimulateBestWorst()
{
    vector<pair<int,int> > minmax = getParameterRanges();

    Configuration cnf_best;
    cnf_best.L_d_int = minmax[0].second;
    cnf_best.L_s_int = minmax[1].second;
    cnf_best.L_g_int = minmax[2].second;
    cnf_best.L_d_pin = minmax[3].second;
    cnf_best.L_s_pin = minmax[4].second;
    cnf_best.L_g_pin = minmax[5].second;
    cnf_best.L_dH_ext = minmax[6].second;
    cnf_best.L_sH_ext = minmax[7].second;
    cnf_best.L_gH_ext = minmax[8].second;
    cnf_best.L_dL_ext = minmax[9].second;
    cnf_best.L_sL_ext = minmax[10].second;
    cnf_best.L_gL_ext = minmax[11].second;
    cnf_best.L_Hwire = minmax[12].second;
    cnf_best.L_Lwire = minmax[13].second;


    Configuration cnf_worst;
    cnf_worst.L_d_int = minmax[0].first;
    cnf_worst.L_s_int = minmax[1].first;
    cnf_worst.L_g_int = minmax[2].first;
    cnf_worst.L_d_pin = minmax[3].first;
    cnf_worst.L_s_pin = minmax[4].first;
    cnf_worst.L_g_pin = minmax[5].first;
    cnf_worst.L_dH_ext = minmax[6].first;
    cnf_worst.L_sH_ext = minmax[7].first;
    cnf_worst.L_gH_ext = minmax[8].first;
    cnf_worst.L_dL_ext = minmax[9].first;
    cnf_worst.L_sL_ext = minmax[10].first;
    cnf_worst.L_gL_ext = minmax[11].first;
    cnf_worst.L_Hwire = minmax[12].first;
    cnf_worst.L_Lwire = minmax[13].first;

    vector<Configuration> cnf_best_worst;

    cnf_best_worst.push_back(cnf_best);
    cnf_best_worst.push_back(cnf_worst);
//G FIXME re-enable
/*
  vector<Simulation> sim_best_worst = simulate_space(cnf_best_worst);
  eud.history.push_back(sim_best_worst[0]);
  eud.history.push_back(sim_best_worst[1]);
  eud.ht_ga->addT(sim_best_worst[0]);
  eud.ht_ga->addT(sim_best_worst[1]);
  eud.pareto.push_back(sim_best_worst[0]);
  eud.pareto.push_back(sim_best_worst[1]);
*/
}

