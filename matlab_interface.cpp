
#include "matlab_interface.h"
#include "environment.h"
#include "common.h"
#include <cstdio>
#include <sys/stat.h>
#include <math.h>

MatlabInterface::MatlabInterface()
{
    set_save_matlog(false);
}

MatlabInterface::~MatlabInterface()
{
	cout << "\n Destroying MatlabInterface object...";
}


void MatlabInterface::set_save_matlog(bool save_log)
{
	this->do_save_log = save_log;
}

void MatlabInterface::execute_sim() //db
{

	string script = "cd "+string(getenv(BASE_DIR))+string(MATLAB_WORKSPACE)+", Simula, quit";
	string command = string(MATLAB_BIN)+" -nosplash -nodesktop -r \""+script+"\"";

	// /matlab -nosplash -nodesktop -r "Simula, quit"

	//string command = "nc 127.0.0.1 30000";

	cout << M9DSE_TAG << "executing: " << command;
	system(command.c_str());
}

// read statistics from PD_STATS and m5stats.txt files
Dynamic_stats MatlabInterface::get_dynamic_stats()
{
	Dynamic_stats stats;

	FILE * fp;
	string filename = getenv(BASE_DIR)+string(MATLAB_WORKSPACE)+string(MATLAB_OUT_FILE);
	fp = fopen(filename.c_str(),"r");


	if (fp!=NULL) {
		fscanf(fp,"%lf %lf %lf %lf %lf %lf",&stats.err_VGS_H,&stats.err_VDS_H,&stats.err_ID_H,
			   &stats.err_VGS_L,&stats.err_VDS_L,&stats.err_ID_L);
		fclose(fp);

		if (isnan(stats.err_VGS_H)
			|| isnan(stats.err_VDS_H)
			|| isnan(stats.err_ID_H)
			|| isnan(stats.err_VGS_L)
			|| isnan(stats.err_VDS_L)
			|| isnan(stats.err_ID_L))
		{
			write_to_log(get_mpi_rank(),string(getenv(BASE_DIR))+string(M9DSE_LOG_FILE),"WARNING: discarding conf because NaN occurrence");
			stats.err_VGS_H = BIG_VGS;
			stats.err_VDS_H = BIG_VDS;
			stats.err_ID_H = BIG_ID;
			stats.err_VGS_L = BIG_VGS;
			stats.err_VDS_L = BIG_VDS;
			stats.err_ID_L  = BIG_ID;
		}

	}
	else {
		cout << "Cannot open " << filename;
		exit(-1);
	}

	return stats;
}


void MatlabInterface::save_model_config(const ModelInverter &p, const string &path) const
{
	string filename = path;
	std::ofstream output_file(filename.c_str());

	if (!output_file)
	{
		string logfile = base_path + string(M9DSE_LOG_FILE);
		int myid = get_mpi_rank();
		cerr << "FATAL ERROR: cannot save " + filename;
		exit(EXIT_FAILURE);
	}
	else
	{
		output_file << "\n% DO NOT EDIT: this file is generated by M9DSE explorer " << endl;

		output_file << "%% Induttanze " << endl;
		output_file << "% Induttanze parassite di bonding interne al package " << endl;

		output_file << "\nL_d_int=" << p.L_d_int.get_val();
		output_file << "\nL_s_int=" << p.L_s_int.get_val();
		output_file << "\nL_g_int=" <<   p.L_g_int.get_val();

		output_file << "% Induttanze parassite dei pin esterne al package (supponiamo che G e S siano uguali)" << endl;
		output_file << "\nL_d_pin=" <<  p.L_d_pin.get_val();
		output_file << "\nL_s_pin=" <<  p.L_s_pin.get_val();
		output_file << "\nL_g_pin=" << p.L_g_pin.get_val();

		output_file << "% Induttanze parassite del PCB " << endl;
		output_file << "\nL_dH_ext=" <<   p.L_dH_ext.get_val();
		output_file << "\nL_sH_ext=" <<    p.L_sH_ext.get_val();
		output_file << "\nL_gH_ext=" <<    p.L_gH_ext.get_val();
		output_file << "\nL_dL_ext=" <<   p.L_dL_ext.get_val();
		output_file << "\nL_sL_ext=" <<    p.L_sL_ext.get_val();
		output_file << "\nL_gL_ext=" <<    p.L_gL_ext.get_val();

		output_file << "% Induttanze parassite dei cavi per i collegamenti di Kelvin-Source " << endl;
		output_file << "\nL_Hwire=" <<   p.L_Hwire.get_val();
		output_file << "\nL_Lwire=" <<   p.L_Lwire.get_val();
	}
}

void MatlabInterface::load_model_config(ModelInverter *p, const string &filename) const
{
	std::ifstream input_file(filename.c_str());

	if (!input_file) {
		cerr << "FATAL ERROR: cannot open file " + filename;
		exit(EXIT_FAILURE);
	}
	else
	{
		int val;
		go_until("L_d_int",input_file);
		input_file>>val;
		p->L_d_int.set_val(val);

		go_until("L_s_int",input_file);
		input_file>>val;
		p->L_s_int.set_val(val);

		go_until("L_g_int",input_file);
		input_file>>val;
		p->L_g_int.set_val(val);

		////////////////////////////////////////////
		go_until("L_d_pin",input_file);
		input_file>>val;
		p->L_d_pin.set_val(val);

		go_until("L_s_pin",input_file);
		input_file>>val;
		p->L_s_pin.set_val(val);

		go_until("L_g_pin",input_file);
		input_file>>val;
		p->L_g_pin.set_val(val);

		/////////////////////////////////////////////

		go_until("L_dH_ext",input_file);
		input_file>>val;
		p->L_dH_ext.set_val(val);

		go_until("L_sH_ext",input_file);
		input_file>>val;
		p->L_sH_ext.set_val(val);

		go_until("L_gH_ext",input_file);
		input_file>>val;
		p->L_gH_ext.set_val(val);

		go_until("L_dL_ext",input_file);
		input_file>>val;
		p->L_dL_ext.set_val(val);

		go_until("L_sL_ext",input_file);
		input_file>>val;
		p->L_sL_ext.set_val(val);

		go_until("L_gL_ext",input_file);
		input_file>>val;
		p->L_gL_ext.set_val(val);
		////////////////////////////////////////////////

		go_until("L_Hwire",input_file);
		input_file>>val;
		p->L_Hwire.set_val(val);

		go_until("L_Lwire",input_file);
		input_file>>val;
		p->L_Lwire.set_val(val);
	}
}


