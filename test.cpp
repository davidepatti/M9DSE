
// Example of usage of Explorer class from a function not included in
// user_interface.c

#include "explorer.h"

void test()
{
    // assuming matlab source and M9DSE unpacked in $HOME dir
    string base_dir = string(getenv("HOME"));

    //get a matlab interface
    matlab_interface* interface = new matlab_interface(base_dir);

    // create a new explorer object connected to the interface
    Explorer* my_explorer = new Explorer(interface);

    //suppose we want explore a space where only two particular
    //parameters can vary , for example , L1D size and  n. of integer_units .
    //
    
    // first create an appropriated space mask :
    
    // UNSET_ALL means : no parameter can be modified
    Space_mask mask1 = my_explorer->get_space_mask(UNSET_ALL); 

    // modify mask values associated to the parameters we want
    // explore:

    // build a space of all possible explorations associated to this
    // mask :

    Configuration base_conf = get_config();
    base_conf.L1I_size = 128;

    vector<Configuration> space1 = my_explorer->build_space(mask1,base_conf);

    // simultate space 
    vector<Simulation> sims1 = my_explorer->simulate_space(space1);

    // get non dominated simulations
    vector<Simulation> pareto_set1 = my_explorer->get_pareto(sims1);

    // save them
    my_explorer->save_simulations(pareto_set1,"TEMP_TEST");
}
