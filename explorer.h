
#ifndef EXPLORER_H
#define EXPLORER_H

#include "version.h"
#include "stdio.h"
#include "stdlib.h"
#include <vector>
#include <string>
#include <map>
#include "model_inverter.h"
#include "matlab_interface.h"
#include "estimator.h"
#include "parameter.h"
#include "hash.h" // mau
#include "FuzzyApprox.h" // ale
//#include "FannApprox.h" // ale
#include "containers.h" //G

// ---------------------------------------------------------------------------
#define EXPLORER_NOTHING_DONE 0
#define EXPLORER_BINARY_DONE 1
#define EXPLORER_ALL_DONE 2
#define EXPLORER_RETRY_TIME 1
// ---------------------------------------------------------------------------



class Explorer {
public:
    ModelInverter model_inverter;
    Estimator estimator;

    Explorer(MatlabInterface * pInterface);
    ~Explorer();

    // Functions to modify explorer options
    void set_options(const struct UserSettings& user_settings);

    string get_base_dir() const;

    // Exploration algorithms

    void start_SAP();
    void start_PBSA();
    void start_RAND(int random_size);
    void start_GA(const GA_parameters& parameters);
    void start_EXHA();
    void test(); // for testing only

    // Main function for simulating a parameter space
    vector<Simulation> simulate_space(const vector<Configuration>& space);
    int simulate_space();

    // Functions for building configuration spaces needed by
    // simultate_space(vector<Configuration>& space)


    vector<Configuration> extract_space(const vector<Simulation>& sims) const;

    vector<Configuration> build_space(const Space_mask& mask);
    vector<Configuration> build_space(const Space_mask& mask,Configuration base_config);
    vector<Configuration> build_space_cross_merge(const vector<Configuration>& s1,
                                                  const vector<Configuration>& s2,
                                                  const Space_mask& mask1,
                                                  const Space_mask& mask2) const;
    bool equivalent_spaces(const vector<Configuration>& s1,const vector<Configuration>& s2) const;

    long double get_space_size() const;
    long double get_space_size(const Space_mask& mask) const;


    Space_mask mask_union(Space_mask& m1,Space_mask& m2) const;
    Space_mask get_space_mask(Mask_type mask_type) const;
    Space_mask create_space_mask(const vector<bool>& boolean_mask);
    Space_mask negate_mask(Space_mask mask);

    vector<bool> get_boolean_mask(const Space_mask& mask);

    Configuration create_configuration() const; // default values
    Configuration create_configuration(const ModelInverter &p) const;
    Configuration create_configuration(const Space_mask& mask,const Configuration& base) const;

    bool configuration_present(const Configuration& conf,const vector<Configuration>& space) const;
    int simulation_present(const Simulation& sim,const vector<Simulation>& simulations) const;

    //*********************************************************//
    // function to manipulate Simulations

    vector<Simulation> get_pareto(const vector<Simulation>& simulations);
    vector<Simulation> get_pareto3d(const vector<Simulation>& simulations);
    vector<Simulation> get_pareto_VDSVGS(const vector<Simulation> &simulations);
    vector<Simulation> get_pareto_IDVDS(const vector<Simulation> &simulations);
    vector<Simulation> get_pareto_IDVGS(const vector<Simulation> &simulations);

    void remove_dominated_simulations(vector<Simulation>& sims);
    vector<Simulation> normalize(const vector<Simulation>& sims);

    vector<Simulation> sort_by_VDS(vector<Simulation> sims);
    vector<Simulation> sort_by_VGS(vector<Simulation> sims);
    vector<Simulation> sort_by_ID(vector<Simulation> sims);
    vector<Simulation> sort_by_VGSVDS_product(vector<Simulation> sims);

    double get_sensivity_VGSVDS(const vector<Simulation>& sim);
    double get_sensivity_PBSA(const vector<Simulation>& sim,const vector<Simulation>& all_sims);
    double distance(const Simulation& s1,const Simulation& s2);

    void append_simulations(vector<Simulation>& dest,const vector<Simulation>& new_sims);
    void save_simulations(const vector<Simulation>& simulations,const string& filename);
    void save_configurations(const vector<Configuration>& space,const string& filename);
    void save_stats(const Exploration_stats& stats,const string& filename);
    void save_estimation_file(const Dynamic_stats& ,const Estimate& ,string& filename) const;
    void save_objectives_details(const Dynamic_stats& dyn,const Configuration& conf, const string filename ) const;

    int get_sim_counter() const;

    void reset_sim_counter();

    CFunctionApproximation *function_approx;

    void set_fuzzy(bool);
    void init_approximation();

    void load_space_file(const string& space_name);
    void save_space_file(const string& space_name);

    vector<pair<int,int> > getParameterRanges();
    vector<pair<int,int> > getParametersNumber();

    int n_objectives() {return n_obj;}

private:

    void prepare_explorer(const Configuration &config);
    vector<Simulation> simulate_loop(const vector<Configuration>& space);


    void init_GA(); //G
    void SimulateBestWorst();
    //--------------------
    void GA_evaluate(population* pop); //G
    Configuration ind2conf(const individual& ind); //G
    MatlabInterface  * matlabInterface;

    Estimate estimate;
    Dynamic_stats dyn_stats;
    Configuration current_config;

    bool force_simulation;

    int n_obj;
    struct ExportUserData eud; //G
    struct UserSettings Options;

    int sim_counter;
    map<string,int> unique_configs;
    vector<Simulation> previous_simulations;
    string current_algo;
};

bool isDominated(Simulation sim, const vector<Simulation>& simulations);
#endif
