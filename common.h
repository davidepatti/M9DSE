
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
// ---------------------------------------------------------------------------

#define M9DSE_TAG "\n    >> M9DSE: "
#ifndef NULL
#define NULL 0
#endif
#define TRIANGLE_SET 1
#define GAUSSIAN_SET 2

#define N_PARAMS 14

#define BIG_VDS      100
#define BIG_VGS      100
#define BIG_ID        100

#define N_OBJ 3


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
	bool objective_avg_errID;
	bool objective_avg_errVDS;
	bool objective_avg_errVGS;

	bool objective_errID_H;
	bool objective_errVDS_H;
	bool objective_errVGS_H;

	bool objective_errID_L;
	bool objective_errVDS_L;
	bool objective_errVGS_L;

	bool save_spaces;
	bool save_estimation;
	bool save_objectives_details;
	string default_settings_file;

	bool save_restore;
	//G
	bool save_matlog;
};

struct Space_mask
{

	bool L_d_int;
	bool L_s_int;
	bool L_g_int;
	bool L_d_pin;
	bool L_s_pin;
	bool L_g_pin;
	bool L_dH_ext;
	bool L_sH_ext;
	bool L_gH_ext;
	bool L_dL_ext;
	bool L_sL_ext;
	bool L_gL_ext;
	bool L_Hwire;
	bool L_Lwire;
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
	string get_string() const;
	string get_executable_dir() const;

};

struct Simulation
{
	double avg_err_VGS, avg_err_ID, avg_err_VDS;
	double err_VGS_H, err_ID_H, err_VDS_H;
	double err_VGS_L, err_ID_L, err_VDS_L;
	Configuration config;
	bool simulated;
	void add_simulation(const Simulation&);
};

struct Dynamic_stats
{
	double err_VGS_L, err_ID_L, err_VDS_L;
	double err_VGS_H, err_ID_H,err_VDS_H;

};
// ---------------------------------------------------------------------------
typedef struct
{
	double avg_err_VGS;
	double avg_err_VDS;
	double avg_err_ID;

} Estimate;
// ---------------------------------------------------------------------------
struct Exploration_stats
{
	double space_size;
	double feasible_size;
	int n_sim;
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

		return (t1.config.L_d_int== t2.config.L_d_int&&
				t1.config.L_s_int== t2.config.L_s_int&&
				t1.config.L_g_int== t2.config.L_g_int&&
				t1.config.L_d_pin== t2.config.L_d_pin&&
				t1.config.L_s_pin== t2.config.L_s_pin&&
				t1.config.L_g_pin== t2.config.L_g_pin&&
				t1.config.L_dH_ext== t2.config.L_dH_ext&&
				t1.config.L_sH_ext== t2.config.L_sH_ext&&
				t1.config.L_gH_ext== t2.config.L_gH_ext&&
				t1.config.L_dL_ext== t2.config.L_dL_ext&&
				t1.config.L_sL_ext== t2.config.L_sL_ext&&
				t1.config.L_gL_ext== t2.config.L_gL_ext&&
				t1.config.L_Hwire== t2.config.L_Hwire&&
				t1.config.L_Lwire== t2.config.L_Lwire);
	}


	virtual unsigned int T2Index(Simulation& t1) {
		double something = (t1.config.L_d_int + t1.config.L_s_int+ t1.config.L_g_int+ t1.config.L_d_pin+
		t1.config.L_s_pin+ t1.config.L_g_pin+ t1.config.L_dH_ext + t1.config.L_sH_ext + t1.config.L_gH_ext +
		t1.config.L_dL_ext + t1.config.L_sL_ext + t1.config.L_gL_ext + t1.config.L_Hwire + t1.config.L_Lwire);

		unsigned int some_int = something * 10e9;

		unsigned int hsize =  vhash.size();
		unsigned int  index = some_int % hsize;
		return index;
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
