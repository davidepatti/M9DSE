
#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "environment.h"
#include "parameter.h"
#include "common.h"

#include <fstream>
#include <cstdlib>
#include <string>

class ModelInverter {

public:

	ModelInverter();

	~ModelInverter();

	void set_to_default();
	void set_config(const Configuration&);

	// Induttanze parassite di bonding interne al package Parameter L_d_int;
	Parameter L_d_int;
	Parameter L_s_int;
	Parameter L_g_int;

	//% Induttanze parassite dei pin esterne al package (supponiamo che G e S siano uguali)
	Parameter L_d_pin;
	Parameter L_s_pin;
	Parameter L_g_pin;

	//% Induttanze parassite del PCB
	Parameter L_dH_ext;
	Parameter L_sH_ext;
	Parameter L_gH_ext;
	Parameter L_dL_ext;
	Parameter L_sL_ext;
	Parameter L_gL_ext;

	//% Induttanze parassite dei cavi per i collegamenti di Kelvin-Source

	Parameter L_Hwire;
	Parameter L_Lwire;

	/*
	L_d_int
	L_s_int
	L_g_int

	L_d_pin
	L_s_pin
	L_g_pin

	L_dH_ext
	L_sH_ext
	L_gH_ext
	L_dL_ext
	L_sL_ext
	L_gL_ext


	L_Hwire
	L_Lwire
	 */
};

#endif
