
/***********************************************************************
 * common.h 
 ***********************************************************************
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "hash.h"

// IMPORTANT:
// you should set this to "DIR" if you install matlab and
// matlab-workspace directories to the content of environment
// variable $DIR
// DAV #define BASE_DIR "PWD"
#define BASE_DIR "HOME"

#define DEFAULT_BENCH "fir_int"
#define MAIN_HMDES2 "hpl_pd_elcor_std.hmdes2"
#define EXPLORER_HMDES2 "explorer.hmdes2"
#define COMPILER_PARAM "compiler_param" //db
#define EE_TAG "\n    >> EE: "
#define EE_LOG_PATH "/matlab-workspace/M9-explorer/M9.log"
#define N_PARAMS 27

// ---------------------------------------------------------------------------

#ifndef NULL
#define NULL 0
#endif
#define TRIANGLE_SET 1
#define GAUSSIAN_SET 2

#define BIG_CYCLES      1e30
#define BIG_VGS      1e30
#define BIG_ID        1e30

// added by andrea.araldo@gmail.com
// TODO: non so se questo sia il posto giusto per mettere questo
enum EParameterType
{
	ke_gpr_static_size, ke_fpr_static_size, ke_pr_static_size,
	ke_cr_static_size, ke_btr_static_size,

	ke_num_clusters, ke_integer_units, ke_float_units, ke_branch_units,
	ke_memory_units,

	ke_L1D_size, ke_L1D_block_size, ke_L1D_associativity,
	ke_L1I_size, ke_L1I_block_size, ke_L1I_associativity,
	ke_L2U_size, ke_L2U_block_size, ke_L2U_associativity,

	ke_tcc_region, ke_max_unroll_allowed, ke_regroup_only,
	ke_do_classic_opti, ke_do_prepass_scalar_scheduling,
	ke_do_postpass_scalar_scheduling, ke_do_modulo_scheduling,
	ke_memvr_profiled
};



using namespace std;

void wait_key();
void go_until(const string& dest,ifstream& ifs);
string skip_to(ifstream& ifs,int n);
string skip_to(ifstream& ifs,const string& target);
int count_word(const string& w,ifstream& ifs);
long long atoll(const string& s);
double atof(const string& s);
int atoi(const string& s);
string get_base_dir();
double max(const double& a,const double& b);
string noyes(int x);
bool file_exists(const string& filename);
int get_mpi_rank();
int get_mpi_size();
void write_to_log(int id, const string& logfile,const string& message);

typedef unsigned long long uint64;

template<typename T> std::string to_string(const T& t){
	std::stringstream s;
	s << t;
	return s.str();
}

struct UserSettings
{
	string benchmark;
	bool objective_id;
	bool objective_vds;
	bool objective_VGS;
	bool save_spaces;
	bool save_estimation;
	bool save_objectives_details;
	string default_settings_file;
	struct
	{
		int enabled;
		float threshold;
		int min,max;
	} approx_settings;

	bool save_restore;
	//G
	bool save_matlog;
};

struct Space_mask
{
	// TODO fix M9DSE
	bool L1D_block; bool L1D_size; bool L1D_assoc;
	bool L1I_block; bool L1I_size; bool L1I_assoc;
	bool L2U_block; bool L2U_size; bool L2U_assoc;

};

// ---------------------------------------------------------------------------
//  GA parameters used in exploration
struct GA_parameters
{
	int population_size;
	float pcrossover;
	float pmutation;
	int max_generations;
	int report_pareto_step;
};

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// predefined mask types 
enum Mask_type { SET_ALL, UNSET_ALL}; //db

struct Rule {
	int* antecedents;
	double* consequents;
	double* degrees;
};

struct Configuration
{
	// Induttanze parassite di bonding interne al package

	double L_d_int;  // Riferimento Q3D 0.38e-9;   % Valore modificabile
	double L_s_int;  // Riferimento Q3D 6.41e-9;   % Valore modificabile
	double L_g_int;  // Riferimento Q3D 8.76e-9;   % Valore modificabile

	//% Induttanze parassite dei pin esterne al package (supponiamo che G e S siano uguali)
	double L_d_pin;  // Riferimento Q3D 13.46e-9;   % Valore modificabile
	double L_s_pin;  // Riferimento Q3D 10.95e-9;   % Valore modificabile
	double L_g_pin;  // Riferimento Q3D 10.99e-9;

	//% Induttanze parassite del PCB
	double L_dH_ext;//  % Riferimento Q3D 50e-9;   % Valore modificabile
	double L_sH_ext;//  % Riferimento Q3D 12.5e-9;   % Valore modificabile
	double L_gH_ext;//  % Riferimento Q3D 12e-9;   % Valore modificabile
	double L_dL_ext;//  % Riferimento Q3D 12.5e-9;   % Valore modificabile
	double L_sL_ext;//  % Riferimento Q3D 48e-9;   % Valore modificabile
	double L_gL_ext;//  % Riferimento Q3D 12e-9;   % Valore modificabile

	//% Induttanze parassite dei cavi per i collegamenti di Kelvin-Source

	double L_Hwire; // % Riferimento Q3D 10.95e-9;   % Valore modificabile
	double L_Lwire; // % Riferimento Q3D 10.95e-9;   % Valore modificabile

	bool is_feasible(); // mau
	void invalidate();
	bool check_difference(const Configuration&,Space_mask);
	string get_header() const;
	string get_executable_dir() const;
	string get_mem_dir() const;

	string configuration_to_string() const;
	//</added by andrea.araldo@gmail.com>

};

struct Simulation
{
	double avg_err_VGS, avg_err_id, avg_err_vds;
	Configuration config;
	bool simulated;
	void add_simulation(const Simulation&);
private:
	vector<double> avg_err_VGS_v, avg_err_id_v, avg_err_vds_v;
};

struct Dynamic_stats
{
	bool valid; // if false, something weird happened while compiling/simulating

	double err_VGS_L, err_id_L, err_vds_L;
	double err_VGS_H, err_id_H,err_vds_H;

};
// ---------------------------------------------------------------------------
typedef struct
{
	double avg_err_VGS;
	double avg_err_vds;
	double avg_err_id;

} Estimate;
// ---------------------------------------------------------------------------
struct Exploration_stats
{
	double space_size;
	double feasible_size;
	int n_sim;
	int recompilations;
	time_t start_time;
	time_t end_time;
};

// ---------------------------------------------------------------------------
// simple cache to store already simulated configurations
class HashGA : public Hash<Simulation> // mau
{
public:
	HashGA(int _size) : Hash<Simulation>(_size) {}

	virtual bool equal(Simulation &t1, Simulation &t2) {
	    // TODO M9FIX
	    assert(false);
		return (false);
	}

	virtual unsigned int T2Index(Simulation& t) {
		// TODO M9FIX
		assert(false);
		return false;
	//	( (t.config.L1D_block + t.config.do_modulo_scheduling +		//db t.config.memvr_profiled ) % vhash.size() ); //db
	}
};

// ---------------------------------------------------------------------------
// wrapper for the communication between Explorer and ga evaluation function
struct ExportUserData //G
{
//G  class Explorer     *explorer;
	HashGA             *ht_ga;
//G  HashGA	     *ht_hy;
	vector<Simulation> history;
	vector<Simulation> pareto;
};
/***********************************************************************
 * debug routines
 ***********************************************************************
 */
#define message(msg) cout << "MSG: " << msg << endl;
#define fatal(msg) { cerr << "FATAL: " << msg << endl; exit(-1); }
#define warning(msg) cerr << "WARNING: " << msg << endl;

#ifdef DEBUG
#define debug(msg) cout << "DEBUG: " << msg << endl;
#define tracemark cout << "---> " << __FILE__ << ":" << __LINE__ << endl;
#else
#define debug(msg)
#define tracemark
#endif


#endif
