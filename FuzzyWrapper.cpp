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
	double appoggio[20];
	assert(false);
	// TODO M9FIX
	/*
	appoggio[0] = double(p.L_s_int.get_pos(conf.integer_units));
	appoggio[17] = double(mem.L2U.associativity.get_pos(conf.L2U_assoc));
	appoggio[18] = double(sim.avg_err_VGS);
	appoggio[19] = double(sim.avg_err_vds);
	 */
	return (Learn(appoggio,&(appoggio[18])));
}

Simulation CFuzzyFunctionApproximation::Estimate1(Configuration conf, ModelInverter &p) {
	Simulation sim;
	double appoggio[20];
	// TODO M9FIX
	assert(false);
	/*
	appoggio[0] = double(p.L_s_int.get_pos(conf.integer_units));
	appoggio[1] = double(p.L_g_int.get_pos(conf.float_units));
	appoggio[2] = double(p.L_d_pin.get_pos(conf.branch_units));
	appoggio[3] = double(p.L_s_pin.get_pos(conf.memory_units));
	appoggio[4] = double(p.L_g_pin.get_pos(conf.gpr_static_size));
	appoggio[5] = double(p.L_dH_ext.get_pos(conf.fpr_static_size));
	appoggio[6] = double(p.L_sH_ext.get_pos(conf.pr_static_size));
	appoggio[7] = double(p.L_gH_ext.get_pos(conf.cr_static_size));
	appoggio[8] = double(p.L_dL_ext.get_pos(conf.btr_static_size));
	appoggio[17] = double(mem.L2U.associativity.get_pos(conf.L2U_assoc));
	appoggio[18] = 0.0f;
	appoggio[19] = 0.0f;
	 */
	EstimateG(appoggio,&(appoggio[18]));
	sim.config = conf;
	sim.avg_err_id = -1.0f;
	sim.avg_err_VGS = double(appoggio[18]);
	sim.avg_err_vds = double(appoggio[19]);
	sim.simulated = false;
	cout << "\n-----------Estimate 1 : " << sim.avg_err_VGS << " __ " << sim.avg_err_vds;
	return (sim);
}

Dynamic_stats CFuzzyFunctionApproximation::Estimate2(Configuration conf) {
	Dynamic_stats dyn;
	return dyn;
}
