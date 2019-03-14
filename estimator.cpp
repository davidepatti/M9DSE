#include "estimator.h"
#include "common.h"

Estimator::Estimator(){

}
Estimator::~Estimator(){
}


Estimate Estimator::get_estimate(const Dynamic_stats &dyn_stats)
{
    string tmp;
    Estimate estimate;

    // if stats are marked as not valid, dummy values are immediately
    // returned to allow continue_on_failure feature
    if (!dyn_stats.valid)
    {
        estimate.avg_err_VGS = (dyn_stats.err_VGS_H + dyn_stats.err_VGS_L)/2;
        estimate.avg_err_VDS = (dyn_stats.err_VDS_H + dyn_stats.err_VDS_L)/2;
        estimate.avg_err_ID = (dyn_stats.err_ID_H + dyn_stats.err_ID_L)/2;

        return estimate;
    }

    assert(false);
    return estimate;
}



