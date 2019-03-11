
#ifndef MATLAB_INTERFACE_H
#define MATLAB_INTERFACE_H
#include<string>
#include <cstdlib>
#include <unistd.h>
#include <fstream>

#include "model_inverter.h"
#include "common.h"


using namespace std;

class MatlabInterface {

public: 
    
    MatlabInterface();
    
    ~MatlabInterface();

	void set_save_matlog(bool do_save_log);

	void set_environment(const string& path);

	//db
	void execute_sim(const string &path);	//db

	void save_model_config(const ModelInverter &, const string &path) const;
	void load_model_config(ModelInverter *p, const string &filename) const;

	Dynamic_stats get_dynamic_stats();

private:
	string base_path;
	bool do_save_log;
};

#endif
