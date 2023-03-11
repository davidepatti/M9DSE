#include "FuzzyApprox.h"
//#include "common.h"

using namespace std;
bool CFuzzyFunctionApproximation::Init(double _threshold, int _min, int _max, int nouts)
{

	cout << "\n--------------------------------------------------------";
	cout << "\n Fuzzy Function Approximation Initializated";
	cout << "\n--------------------------------------------------------";

	// verifica se la classe � gi� stata inizializzata e cancella tutto

	//if (GetRules() > 0) Clean();

	OutDim = nouts;
	InDim = 18;

	//  if (!StartUp(10000, _threshold)) return (false);
	if (!StartUp(10000, _threshold,_min,_max)) return (false);

	return (true);
}

bool CFuzzyFunctionApproximation::FuzzySetsInit(const vector<pair<int,int> >& min_max) {


	int nins = min_max.size();

	double *InMin = new double[nins];
	double *InMax = new double[nins];
	int *InSets = new int[nins];

	for (int i=0; i<nins; i++)
	{
		InMin[i] = min_max[i].first;
		InMax[i] = min_max[i].second;
		InSets[i] = int(InMax[i]);
	}

	//int InSets[18] = {5,5,2,3,	3,3,3,3,3,	9,4,3,	9,4,3,	9,4,3};

	if (!GenerateInputFuzzySets(nins, InSets, InMin, InMax)) return (false);
	delete[] InMin;
	delete[] InMax;
	delete[] InSets;
	return (true);
}

bool CFuzzyFunctionApproximation::Learn(const Configuration& conf,const Dynamic_stats& dyn)
{
	return false;
}

bool CFuzzyFunctionApproximation::Learn(Configuration conf, Simulation sim, ModelInverter &p) {
	double appoggio[17];

    appoggio[0]  = double(p.L_d_int.get_pos(conf.L_d_int));
    appoggio[1]  = double(p.L_s_int.get_pos(conf.L_s_int));
    appoggio[2]  = double(p.L_g_int.get_pos(conf.L_g_int));
    appoggio[3]  = double(p.L_d_pin.get_pos(conf.L_d_pin));
    appoggio[4]  = double(p.L_s_pin.get_pos(conf.L_s_pin));
    appoggio[5]  = double(p.L_g_pin.get_pos(conf.L_g_pin));
    appoggio[6]  = double(p.L_dH_ext.get_pos(conf.L_dH_ext));
    appoggio[7]  = double(p.L_sH_ext.get_pos(conf.L_sH_ext));
    appoggio[8]  = double(p.L_gH_ext.get_pos(conf.L_gH_ext));
    appoggio[9]  = double(p.L_dL_ext.get_pos(conf.L_dL_ext));
    appoggio[10] = double(p.L_sL_ext.get_pos(conf.L_sL_ext));
    appoggio[11] = double(p.L_gL_ext.get_pos(conf.L_gL_ext));
    appoggio[12] = double(p.L_Hwire.get_pos(conf.L_Hwire));
    appoggio[13] = double(p.L_Lwire.get_pos(conf.L_Lwire));
    appoggio[14] = double(sim.avg_err_VGS);
    appoggio[15] = double(sim.avg_err_VDS);
    appoggio[16] = double(sim.avg_err_ID);

	return (Learn(appoggio,&(appoggio[14])));
}

Simulation CFuzzyFunctionApproximation::Estimate1(Configuration conf, ModelInverter &p) {
	Simulation sim;
	double appoggio[17];

    appoggio[0]  = double(p.L_d_int.get_pos(conf.L_d_int));
    appoggio[1]  = double(p.L_s_int.get_pos(conf.L_s_int));
    appoggio[2]  = double(p.L_g_int.get_pos(conf.L_g_int));
    appoggio[3]  = double(p.L_d_pin.get_pos(conf.L_d_pin));
    appoggio[4]  = double(p.L_s_pin.get_pos(conf.L_s_pin));
    appoggio[5]  = double(p.L_g_pin.get_pos(conf.L_g_pin));
    appoggio[6]  = double(p.L_dH_ext.get_pos(conf.L_dH_ext));
    appoggio[7]  = double(p.L_sH_ext.get_pos(conf.L_sH_ext));
    appoggio[8]  = double(p.L_gH_ext.get_pos(conf.L_gH_ext));
    appoggio[9]  = double(p.L_dL_ext.get_pos(conf.L_dL_ext));
    appoggio[10] = double(p.L_sL_ext.get_pos(conf.L_sL_ext));
    appoggio[11] = double(p.L_gL_ext.get_pos(conf.L_gL_ext));
    appoggio[12] = double(p.L_Hwire.get_pos(conf.L_Hwire));
    appoggio[13] = double(p.L_Lwire.get_pos(conf.L_Lwire));
    appoggio[14] = 0.0f;
    appoggio[15] = 0.0f;
    appoggio[16] = 0.0f;

	EstimateG(appoggio,&(appoggio[14]));
	sim.config = conf;
	sim.avg_err_VGS = double(appoggio[14]);
	sim.avg_err_VDS = double(appoggio[15]);
    sim.avg_err_ID = -1.0f; // TODO: M9 Check this
	sim.simulated = false;
	cout << "\n-----------Estimate 1 : " << sim.avg_err_VGS << " __ " << sim.avg_err_VDS;
	return (sim);
}

Dynamic_stats CFuzzyFunctionApproximation::Estimate2(Configuration conf) {
	Dynamic_stats dyn;
	return dyn;
}
