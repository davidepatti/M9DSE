
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

	MatlabInterface * matlab_interface;
	Explorer * my_explorer;

	int show_menu();

	void edit_user_settings();

    void set_subspace_wrapper();

	void start_exploration_message();

	void reload_system_config();

	void execute_benchmark();
	void view_statistics();
	void compute_cost();

	void info();
	void show_system_config();

	inline string status_string(bool b);
	string word;
	string base_path;
	unsigned int seed;

	int myrank;
	int mysize;
};

#endif
