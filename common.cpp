// common routines and functions
//
#include "common.h"
#include <cstdlib>

#ifdef M9_MPI
#include "mpi.h"
#endif
bool Configuration::is_feasible()
{
    return true;
}



string Configuration::get_header() const
{
    string s = "L_d_int L_s_int L_g_int L_d_pin L_s_pin L_g_pin L_dH_ext L_sH_ext L_gH_ext L_dL_ext L_sL_ext L_gL_ext L_Hwire L_Lwire";
    return s;

}

string Configuration::get_executable_dir() const
{
    assert(false);
    char s[100];

    return string(s);
}



void Simulation::add_simulation(const Simulation& other)
{
    if(avg_err_VGS_v.size() == 0 &&  avg_err_ID_v.size() == 0 && avg_err_VDS_v.size() == 0){ // single to multi-valued transform
        avg_err_VGS_v.push_back(avg_err_VGS);
        avg_err_ID_v.push_back(avg_err_ID);
        avg_err_VDS_v.push_back(avg_err_VDS);
    }
    if(other.avg_err_VGS_v.size()>0 &&  other.avg_err_ID_v.size()>0 && avg_err_VDS_v.size()>0) { // multiple valued simulation
        avg_err_VGS_v.insert(avg_err_VGS_v.end(), other.avg_err_VGS_v.begin(), other.avg_err_VGS_v.end());
        avg_err_ID_v.insert(avg_err_ID_v.end(), other.avg_err_ID_v.begin(), other.avg_err_ID_v.end());
        avg_err_VDS_v.insert(avg_err_VDS_v.end(), other.avg_err_VDS_v.begin(), other.avg_err_VDS_v.end());
    } else { // single valued simulation
        avg_err_VGS_v.push_back(other.avg_err_VGS);
        avg_err_ID_v.push_back(other.avg_err_ID);
        avg_err_VDS_v.push_back(other.avg_err_VDS);
    }
    // update mean values
    vector<double>::iterator it;
    double sum = 0;
    for(it = avg_err_VGS_v.begin(); it != avg_err_VGS_v.end(); it++)
        sum += *it;
    //cout<<"\n VGS: "<< avg_err_VGS <<" sum/size: "<< sum << "/" << avg_err_VGS_v.size() << endl;
    avg_err_VGS = sum / avg_err_VGS_v.size();
    sum = 0;
    for(it = avg_err_ID_v.begin(); it != avg_err_ID_v.end(); it++)
        sum += *it;
    avg_err_ID = sum / avg_err_ID_v.size();
    sum = 0;
    for(it = avg_err_VDS_v.begin(); it != avg_err_VDS_v.end(); it++)
        sum += *it;
    avg_err_VDS = sum / avg_err_VDS_v.size();
}


// File access utilities - mainly to improve readability
void go_until(const string& dest,ifstream& ifs)
{
    string word;
    while ( (ifs>>word) && (word.find(dest)==string::npos));
}

string skip_to(ifstream& ifs,int n)
{
    string word;
    for (int i=0;i<n;i++) ifs>>word;
    return word;
}

string skip_to(ifstream& ifs,const string& target)
{
    string word;
    while ( (ifs>>word) && (word.find(target)==string::npos));
    return word;
}

void wait_key()
{
    cout << "\n\n Press RETURN to continue..." << endl;
    getchar();
    getchar();
}
int count_word(const string& w, ifstream& ifs)
{
    string word;
    int n = 0;
    while ( ifs>>word ) if (word==w) n++;
    return n;
}

// redefinition of some commonly used C-style functions
extern "C" int atoi(const char*);
int atoi(const string& s)
{
    return atoi(s.c_str());
}

extern "C" long long atoll(const char*);

long long atoll(const string& s)
{
    return atoll(s.c_str());
}

extern "C" double atof(const char*);
double atof(const string& s)
{
    return atof(s.c_str());
}

double max(const double& a,const double& b)
{
    if (a>b) return a;
    return b;
}


void write_to_log(int pid,const string& logfile,const string& message)
{
    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    uora[24]=' '; // to avoid newline after date
    string cmd = "echo \""+uora+" [P"+to_string(pid)+"]: "+message+"\" >> "+logfile;

    system(cmd.c_str());
}

int get_mpi_rank()
{
#ifdef M9_MPI
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    return myrank;
#else
    return 0;
#endif
}

int get_mpi_size()
{
#ifdef M9_MPI
    int mysize;
    MPI_Comm_size(MPI_COMM_WORLD,&mysize);
    return mysize;
#else
    return 1;
#endif
}



