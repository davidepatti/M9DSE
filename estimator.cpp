#include "estimator.h"
#include "common.h"

Estimator::Estimator(){

}
Estimator::~Estimator(){
}


Estimate Estimator::get_estimate(const Dynamic_stats &dyn_stats)
{
    assert(N_OBJ==3);
    string tmp;
    Estimate estimate;

    estimate.avg_err_VGS = (dyn_stats.err_VGS_H + dyn_stats.err_VGS_L)/2;
    estimate.avg_err_VDS = (dyn_stats.err_VDS_H + dyn_stats.err_VDS_L)/2;
    estimate.avg_err_ID = (dyn_stats.err_ID_H + dyn_stats.err_ID_L)/2;

    return estimate;
}



