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
        assert(false);

        return estimate;
    }


    assert(false);
    return estimate;
}



