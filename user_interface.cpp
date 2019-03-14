#include "user_interface.h"

#ifdef M9_MPI
#include "mpi.h"
#endif

// uncomment this line if you want interface to be verbose
//#define VERBOSE_INTERFACE

UserInterface::UserInterface(const string& path){

	base_path = path;

	matlab_interface = new MatlabInterface();
	my_explorer = new Explorer(matlab_interface);

	user_settings.default_settings_file = base_path + "M9_default.conf";
	load_settings(user_settings.default_settings_file);
}

UserInterface::~UserInterface(){
}

void UserInterface::interact(){
#ifdef M9_MPI
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    MPI_Comm_size(MPI_COMM_WORLD,&mysize);
    MPI_Request req;
    int chiudi = 1;
    
    if (myrank == 0) {
    	while (show_menu()!=113) ;
//G	show_menu();
	chiudi = 0;
	
	cout<<"\nProcessor "<<myrank<<" is exiting"<<endl;
	for (int p=1; p<mysize; p++) {
    		MPI_Isend(&chiudi,1,MPI_INT, p, 10, MPI_COMM_WORLD, &req); 
		MPI_Send(&chiudi,1,MPI_INT, p, 98, MPI_COMM_WORLD); 
	}		
    }
    else {
    	MPI_Irecv(&chiudi, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &req);
    	while (chiudi) my_explorer->simulate_space();
	cout<<"\nProcessor "<<myrank<<" is exiting"<<endl;
    }

#else
	myrank = 0;
	mysize = 1;
	while (show_menu()!=113);
#endif

}

void UserInterface::start_exploration_message()
{
	cout << "\n Ok, you are ready to start exploration.";
	cout << "\n Results will be saved in the directory:\n";
	cout << base_path + "/matlab-workspace/M9-explorer";
}


int UserInterface::show_menu()
{

	char ch;
	char c;
	long double space_size;
	struct GA_parameters ga_parameters;

	if (myrank == 0) {
		system("clear");

		cout << "\n =====================================================";
		cout << "\n   M9DSE   [release "<< VERSION<<"]";
		cout << "\n _____________________________________________________";
		cout << "\n ______m a i n________________________________________" << endl;

		cout << "\n [b] - General Settings ";
		cout << "\n [c] - Design Space ";
		cout << "\n [h] - About ";
		cout << "\n [q] - Quit ";
		cout << "\n _____________________________________________________";
		cout << "\n ______s p a c e___e x p l o r a t i o n______________" << endl;

		cout << "\n [g] - Genetic based (GA)";
		cout << "\n [s] - Sensivity analysis (SAP)";
		cout << "\n [e] - Exhaustive (EXHA)";
		cout << "\n [r] - Random (RANDOM)";
		cout << "\n [t] - run Explorer::test() code";

		cout << "\n _____________________________________________________";
		cout << "\n _______m a n u a l___t e s t ________________________" << endl;
		cout << "\n [3] - Execute benchmark";
		cout << "\n [4] - View execution statistics";
		cout << "\n [5] - Estimate objectives";
		cout << "\n [6] - Show system config";
		cout << "\n Make your choice >";

		//G    ch = 'r';
		cin >> ch;
	}
	string start;

	if ( ch=='g' || ch=='d' || ch=='s' || ch=='w' || ch=='p' || ch=='e' || ch =='r' || ch=='v')
		my_explorer->init_approximation();

	switch (ch)
	{
		case 'h':
			if (myrank == 0) info();
			if (myrank == 0) wait_key();
			break;


		case 'g': // mau

			if (myrank == 0) {
				cout << "\n\n GA based approach ";
				cout << "\n-------------------------------";
				cout << "\n\n Enter random seed (0 = auto): ";
				cin >> seed;
				if (seed == 0)
					srand((unsigned int)time((time_t*)NULL));
				else
					srand(seed);
				do
				{
					cout << "\n Please enter a value (divisible by 2) for Population size: ";
					cin >> ga_parameters.population_size;
				}
				while (ga_parameters.population_size%2!=0);

				cout << " Crossover prob: ";
				cin >> ga_parameters.pcrossover;
				cout << " Mutation prob: ";
				cin >> ga_parameters.pmutation;
				cout << " Max Generations: ";
				cin >> ga_parameters.max_generations;
				cout << " Report pareto step: ";
				cin >> ga_parameters.report_pareto_step;

				start_exploration_message();
				cout << "\n\n Start exploration (y/n) ? ";
				cin >> ch;
				if (ch=='y')
					my_explorer->start_GA(ga_parameters);
			}
			break;


		case 's':
			if (myrank == 0) {
				start_exploration_message();
				cout << "\n\n Start exploration (y/n) ? ";
				cin >> ch;
				if (ch=='y') my_explorer->start_SAP();
			}
			break;


		case 'e':

			if (myrank == 0) {
				start_exploration_message();

				cout << "\n\n WARNING: You are going to start an exhaustive exploration of all parameters .";
				cout << "\n Depending on range variation of space parameters simulation might take";
				cout << "\n too much time to be completed. ";
				space_size = my_explorer->get_space_size();
				cout << "\n Number of simulations to be performed: " << space_size;

				cout << "\n\n Proceed ? (y/n) ";

				cin >> c;

				if (c=='y') my_explorer->start_EXHA();
				wait_key();
			}
			break;

		case 'r':
			if (myrank == 0) {
				start_exploration_message();
				int n;
				cout << "\n Number of  random simulations:";
				//G	    n = 10000;
				cin >> n;
				cout << "\n Enter random seed (0 = auto):";
				//G	    seed = 1962;
				cin >> seed;
				if (seed==0)
					srand((unsigned int)time((time_t*)NULL));
				else
					srand(seed);
				my_explorer->start_RAND(n);
				wait_key();
			}
			break;


			break;



		case '3':
			if (myrank == 0) execute_benchmark();
			if (myrank == 0) wait_key();
			break;


		case '4':
			if (myrank == 0) view_statistics();
			if (myrank == 0) wait_key();
			break;

		case '5':
			if (myrank == 0) compute_cost();
			if (myrank == 0) wait_key();
			break;

		case '6':

			if (myrank == 0) show_system_config();
			if (myrank == 0) wait_key();
			break;

		case 'q':
			// will exit...
			break;
		default:
			cout << endl << "Not valid choice";
			wait_key();

	}

	return ch;
}

void UserInterface::edit_user_settings()
{
	string ch;
	do {
		system("clear");

#ifdef M9_MPI
		cout << "\n WARNING!";
	cout << "\n ----------------------------------------------------------";
        cout << "\n You are running an MPI enabled binary of M9DSE explorer.";
        cout << "\n Current version does not support settings update to slaves precesses.";
        cout << "\n This means that every change you make here will NOT be used by the other";
        cout << "\n slave instancies of M9DSE explorer.";
        cout << "\n In order to set properly your preferences for each process you should ";
        cout << "\n modify directly the file M9_default.conf and then relaunch the mpirun command";
        wait_key();

    if (!user_settings.save_restore)
    {
	cout << "\n WARNING !!!!";
	cout << "\n****************";
	cout << "\n M9_MPI compilation detected, save/restore simulations NOT enabled!";
	cout << "\n Currently, MPI works ONLY if save/restore simulations (multidir) otion is enabled.";
	cout << "\n SEE the VERY IMPORTANT NOTE on file MPI.README (section III).";
	wait_key();
    }
#endif


		cout << "\n  S e t t i n g s  ";
		cout << "\n ----------------------------------------------------------";
		cout << "\n  (1) - Objective ID               --> " << status_string(user_settings.objective_avg_errID);
		cout << "\n  (2) - Objective VDS     --> " << status_string(user_settings.objective_avg_errVDS);
		cout << "\n  (3) - Objective VGS             --> " << status_string(user_settings.objective_avg_errVGS);
		cout << "\n  (5) - save simulated spaces        --> " << status_string(user_settings.save_spaces);
		cout << "\n  (8) - Save matlab tcc logs       --> " << status_string(user_settings.save_matlog);
		cout << "\n  (9) - save estimation detail files --> " << status_string(user_settings.save_estimation);
		cout << "\n (12) - Approximation                --> " << user_settings.approx_settings.enabled;
		if (user_settings.approx_settings.enabled>0)
		{
			cout << " ( " << user_settings.approx_settings.threshold <<
				 " , " << user_settings.approx_settings.min <<
				 " - " << user_settings.approx_settings.max << " )";
		}

		cout << "\n ----------------------------------------------------------";
		cout << "\n (s) - Save current settings";
		cout << "\n (q) - Return to main menu";
		cout << "\n ----------------------------------------------------------";
		cout << "\n Make your choice:";

		cin >> ch;

		if (ch=="1") user_settings.objective_avg_errID =   !user_settings.objective_avg_errID;
		if (ch=="2") user_settings.objective_avg_errVDS = !user_settings.objective_avg_errVDS;
		if (ch=="3") user_settings.objective_avg_errVGS = !user_settings.objective_avg_errVGS;
		if (ch=="5") user_settings.save_spaces = !user_settings.save_spaces;
		if (ch=="8")
		{
			user_settings.save_matlog = !user_settings.save_matlog;
            matlab_interface->set_save_matlog(user_settings.save_matlog);
		}
		if (ch=="9") user_settings.save_estimation = !user_settings.save_estimation;
		if (ch=="12")
		{
			cout << "\n Select Approximation method:";
			cout << "\n (0) None";
			cout << "\n (1) Fuzzy System";
			cout << "\n (2) Artificial Neural Network";
			cout << "\n select: ";
			cin >> user_settings.approx_settings.enabled;

			if (user_settings.approx_settings.enabled>0)
			{
				cout << "\n Enter % error threshold : ";
				cin >> user_settings.approx_settings.threshold;
				cout << "\n Min number of simulations: ";
				cin >> user_settings.approx_settings.min;
				cout << "\n Max number of simulations: ";
				cin >> user_settings.approx_settings.max;
			}

		}

		if (ch=="s") save_settings_wrapper();

	} while(ch!="q");

	my_explorer->set_options(user_settings);
}

inline string UserInterface::status_string(bool b)
{
	if (b) return "ENABLED";
	return " --- ";
}


void UserInterface::info()
{
	system("clear");

	cout << "\n     M9DSE  - release " << VERSION ;
	cout << "\n ******************************************************** ";
	cout << "\n";
	cout << "\n";
	cout << "\n For usage instructions see README file.";
	cout << "\n";
	cout << "\n For more informations on avg_err_ID, power, avg_err_VGS estimation models and";
	cout << "\n DSE algorithms implemented in M9DSE Explorer please refer to documentation";
	cout << "\n section in: ";
	cout << "\n" ;
	cout << "\n Enjoy your exploration ;) !";
}

void UserInterface::show_system_config() {

	system("clear");
	reload_system_config();

	/*
	cout <<"\n -----------------------------------------";
	cout<<"\n L_d_int:\t"<< my_explorer->model_inverter.num_clusters.get_val();
	cout<<"\n L_s_int:\t"<< my_explorer->model_inverter.integer_units.get_val();
	cout<<"\n L_g_int:\t"<< my_explorer->model_inverter.float_units.get_val();
	cout<<"\n L_s_pin:\t"<< my_explorer->model_inverter.memory_units.get_val();
	cout<<"\n L_d_pin:\t"<< my_explorer->model_inverter.branch_units.get_val();

	//cout<<"\n local_memory_units "<< my_explorer->model_inverter.L_s_pin.get_val();

	cout <<"\n -----------------------------------------";
	cout<<"\n L_g_pin:\t"<< my_explorer->model_inverter.gpr_static_size.get_val();
	cout<<"\n L_dH_ext:\t"<< my_explorer->model_inverter.fpr_static_size.get_val();
	cout<<"\n L_sH_ext:\t"<< my_explorer->model_inverter.pr_static_size.get_val();
	cout<<"\n L_gH_ext:\t"<< my_explorer->model_inverter.cr_static_size.get_val();
	cout<<"\n L_dL_ext:\t"<< my_explorer->model_inverter.btr_static_size.get_val();

	 */

}

void UserInterface::set_subspace_wrapper()
{
	string subspaces_dir = base_path + "/matlab-workspace/M9-explorer/SUBSPACES/";
	string filename;

	cout << "\n\n List of available subspace files: \n";
	string command = "ls "+subspaces_dir;
	system(command.c_str());
	cout << "\n Enter file name without path (e.g. 'my_space.sub' or  use 'x' to cancel): ";
	cin >> filename;
	if (filename!="x")
	{
		cout << "\n Setting space " << filename << " as current subspace..." << endl;
		my_explorer->load_space_file(subspaces_dir+filename);
		// A small workaround is required since
		// link current.sub must be created without full path
		char old_path[200];
		getcwd(old_path,200);
		chdir(subspaces_dir.c_str());
		command = "ln -sf "+filename+" current.sub";
		system(command.c_str());
		chdir(old_path);
	}
}


void UserInterface::load_settings(string settings_file)
{

	FILE * fp;
	fp = fopen(settings_file.c_str(),"r");

	// if there is no config file a default one is created
	if (fp==NULL)
	{
		cout << "\n WARNING:configuration file not found... creating it ";
		user_settings.objective_avg_errID = false;
		user_settings.objective_avg_errVDS = true;
		user_settings.objective_avg_errVGS = false;
		user_settings.save_spaces = false;
		user_settings.save_estimation = false;
		user_settings.approx_settings.enabled = 0;
		user_settings.approx_settings.threshold = 0;
		user_settings.approx_settings.min = 0;
		user_settings.approx_settings.max = 0;
		user_settings.save_restore = false;
		user_settings.save_matlog = false;

		fp= fopen(settings_file.c_str(),"w");
		fclose(fp);
		save_settings(settings_file);
	}

	else fclose(fp);

	std::ifstream input_file (settings_file.c_str());

	if (!input_file)
	{
		cout << "\n Error while loading " << settings_file;
		wait_key();
	}
	else
	{
		cout << "LOADING SETTINGS (false)"<< endl;
	    /*
		user_settings.objective_avg_errID = false;
		user_settings.objective_avg_errVDS = false;
		user_settings.objective_avg_errVGS = false;
		user_settings.save_spaces = false;
		user_settings.save_estimation = false;
		user_settings.save_matlog = false;
        user_settings.save_restore = false;

		go_until("avg_err_ID",input_file);
		input_file >> word;
		if (word=="ENABLED") user_settings.objective_avg_errID = true;

		go_until("avg_err_VDS",input_file);
		input_file >> word;
		if (word=="ENABLED") user_settings.objective_avg_errVDS = true;

		go_until("avg_err_VGS",input_file);
		input_file >> word;
		if (word=="ENABLED") user_settings.objective_avg_errVGS = true;

		go_until("save_spaces",input_file);
		input_file >> word;
		if (word=="ENABLED") user_settings.save_spaces = true;


		go_until("save_estimation",input_file);
		input_file >> word;
		if (word=="ENABLED") user_settings.save_estimation = true;

		go_until("save_matlog",input_file);
		input_file >> word;
		if (word=="ENABLED") user_settings.save_matlog= true;



        matlab_interface->set_save_matlog(user_settings.save_matlog);

		my_explorer->set_options(user_settings);
	     */
	}
}

void UserInterface::save_settings_wrapper()
{
	string dir = getenv(BASE_DIR)+string(M9DSE_PATH);

	cout << "\n User settings file will be saved in: ";
	cout << "\n " << dir ;

	save_settings(dir+"M9_default.conf");
}


void UserInterface::save_settings(string settings_file)
{
	std::ofstream output_file(settings_file.c_str());

	if (!output_file)
	{
		cout << "\n Error while saving " << settings_file ;
		wait_key();
	}
	else
	{
		output_file << "\n\n# M9DSE Explorer settings";

		output_file << "\navg_err_ID " << status_string(user_settings.objective_avg_errID);
		output_file << "\navg_err_VDS " << status_string(user_settings.objective_avg_errVDS);
		output_file << "\navg_err_VGS " << status_string(user_settings.objective_avg_errVGS);
		output_file << "\nsave_spaces " << status_string(user_settings.save_spaces);
		output_file << "\nsave_estimation " << status_string(user_settings.save_estimation);
		output_file << "\nsave_matlog " << status_string(user_settings.save_matlog);

		cout << "\n Ok, saved current settings in " << settings_file;
	}
}



void UserInterface::reload_system_config()
{
    cout << "RELOADING SYSTEM CONFIG (false)" << endl;
    // TODO M9DSE
    /*
	string filename = base_path+"/matlab-workspace/M9-explorer/step_by_step/machines/";
	cout << "\n\n Loading model_inverter configuration: " << filename;
    matlab_interface->load_model_config(&(my_explorer->model_inverter), filename);
     */

}


void UserInterface::execute_benchmark() {

	reload_system_config();
	matlab_interface->execute_sim();
}


void UserInterface::view_statistics() {

	system("clear");
	Dynamic_stats stats = matlab_interface->get_dynamic_stats();

	cout << "\n D y n a m i c  S t a t s ";
	cout << "\n------------------------------------------------" << endl;
	cout << " err_VGS_H\terr_VDS_H\terr_ID_H\terr_VGS_L\terr_VDS_L\terr_ID_L" << endl;
	cout << stats.err_VGS_H << "\t"<< stats.err_VDS_H<<"\t"<< stats.err_ID_H << "\t" <<
		    stats.err_VGS_L << "\t" << stats.err_VDS_L << "\t" << stats.err_ID_L << endl;
}

void UserInterface::compute_cost() {

	system("clear");

	Dynamic_stats dynamic_stats = matlab_interface->get_dynamic_stats();
	Estimate estimate = my_explorer->estimator.get_estimate(dynamic_stats);

	cout << "\n  O b j e c t i v e s ";
	cout << "\n ----------------------------------------------------";
	cout << "\n avg_err_VGS: "<< estimate.avg_err_VGS << endl;
	cout << "\n avg_err_VDS: "<< estimate.avg_err_VDS << endl;
	cout << "\n avg_err_ID: "<< estimate.avg_err_ID << endl;
}

