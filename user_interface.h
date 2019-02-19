
#ifndef INTERFACE_USER_DEMO_H

#define INTERFACE_USER_DEMO_H

#include "explorer.h"
#include "estimator.h"
#include "matlab_interface.h"
#include "model_inverter.h"
#include "environment.h"
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

class UserInterface {
public:
	UserInterface(const string& dir);
	~UserInterface();
	void interact();
private:
	struct UserSettings user_settings;

	MatlabInterface * trimaran_interface;
	Explorer * my_explorer;

	int show_menu();

	void edit_user_settings();

	void save_settings(string settings_file);
	void save_settings_wrapper();
	void load_settings(string settings_file);

	void set_subspace_wrapper();

	void start_exploration_message();
	void edit_exploration_space();
	void reload_system_config();
	void compile_hmdes_file();
	void compile_benchmark();
	void execute_benchmark();
	void view_statistics();
	void compute_cost();
	void choose_benchmark();
	void info();
	void show_system_config();
	void schedule_explorations();

	inline string status_string(bool b);
	string word;
	string base_path;
	unsigned int seed;

	int myrank;
	int mysize;
};

#endif
