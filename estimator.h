
#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <math.h>
#include "stdio.h"
#include "assert.h"

#include "model_inverter.h"
#include "matlab_interface.h"
#include "common.h"

using namespace std;

class Estimator {
public:
	Estimator();
	~Estimator();

	Estimate get_estimate(const Dynamic_stats& dynamic_stats);

};

#endif
