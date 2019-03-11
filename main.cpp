
#include "user_interface.h"
#include "explorer.h"
#include "common.h"
#include <signal.h>

#ifdef M9DSE_MPI
#include "mpi.h"
#endif

int main(int argc, char *argv[])
{
	int rank;
	string logfile;
#ifdef M9DSE_MPI
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	char* base_path_cstr;
	int length;

	if(rank == 0) {
		base_path_cstr = getenv(BASE_DIR);
		length = strlen(base_path_cstr) + 1;
		//cerr << "DEBUG strlen : " << length << endl;
	}
	MPI_Bcast(&length,1,MPI_INT,0,MPI_COMM_WORLD);

	if(rank != 0){
		base_path_cstr = new char[length];
	}

	MPI_Bcast(base_path_cstr, length, MPI_CHAR, 0, MPI_COMM_WORLD);
	string base_path = string(base_path_cstr);
	setenv(BASE_DIR, base_path_cstr, 1);

	logfile = base_path + string(M9DSE_LOG_FILE);
	write_to_log(rank,logfile,"Starting MPI M9DSE process with rank "+to_string(rank)+" on base path "+base_path);

#else
	rank = 0;
	string base_path = string(getenv(BASE_DIR));
	string m9dsepath = base_path+string(M9DSE_PATH);
	logfile = base_path +string(M9DSE_LOG_FILE);
	write_to_log(rank,logfile,"Starting M9DSE process on base path "+base_path);
#endif

	UserInterface *ui;
	ui = new UserInterface(m9dsepath);
	ui->interact();

#ifdef M9DSE_MPI
	MPI_Finalize();
#endif
	write_to_log(rank,logfile,"Terminating M9DSE process");
	return EXIT_SUCCESS;
}
