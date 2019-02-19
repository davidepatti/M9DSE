
#include "model_inverter.h"

ModelInverter::ModelInverter(){
    L_d_int.set_label("L_d_int");
    L_s_int.set_label("L_s_int");
    L_g_int.set_label("L_g_int");
    L_d_pin.set_label("L_d_pin");
    L_s_pin.set_label("L_s_pin");

    L_g_pin.set_label("L_g_pin");
    L_dH_ext.set_label("L_dH_ext");
    L_sH_ext.set_label("L_sH_ext");
    L_gH_ext.set_label("L_gH_ext");
    L_dL_ext.set_label("L_dL_ext");

    L_sL_ext.set_label("L_sL_ext");
    L_gL_ext.set_label("L_gL_ext");

    L_Hwire.set_label("L_Hwire");
    L_Lwire.set_label("L_Lwire");
}

ModelInverter::~ModelInverter(){
}

void ModelInverter::set_to_default()
{
    L_d_int.set_to_default();
    L_s_int.set_to_default();
    L_g_int.set_to_default();
    L_d_pin.set_to_default();
    L_s_pin.set_to_default();

    L_g_pin.set_to_default();
    L_dH_ext.set_to_default();
    L_sH_ext.set_to_default();
    L_gH_ext.set_to_default();
    L_dL_ext.set_to_default();

    L_sL_ext.set_to_default();
    L_gL_ext.set_to_default();

    L_Hwire.set_to_default();
    L_Lwire.set_to_default();
}

void ModelInverter::set_config(const Configuration& conf)
{
    L_d_int.set_val(conf.L_d_int);
    L_s_int.set_val(conf.L_s_int);
    L_g_int.set_val(conf.L_g_int);
    L_d_pin.set_val(conf.L_d_pin);
    L_s_pin.set_val(conf.L_s_pin);

    L_g_pin.set_val(conf.L_g_pin);
    L_dH_ext.set_val(conf.L_dH_ext);
    L_sH_ext.set_val(conf.L_sH_ext);
    L_gH_ext.set_val(conf.L_gH_ext);
    L_dL_ext.set_val(conf.L_dL_ext);

    L_sL_ext.set_val(conf.L_sL_ext)
    L_gL_ext.set_val(conf.L_gL_ext);

    L_Hwire.set_val(conf.L_Hwire);
    L_Lwire.set_val(conf.L_Lwire);

}

