
#ifndef PARAMETER_H
#define PARAMETER_H

#define NOT_VALID -1

#include <iostream>
#include <vector>

using namespace std;

class Parameter {
public:
    Parameter();

    Parameter(const string &l, vector<double> possible_values, double default_val);
    Parameter(double *possible_values, double default_val); // deprecated


    ~Parameter();

    int get_val() const;
    int get_size() const;
    int get_default() const;
    void set_val(double new_value);
    void set_label(const string& label);
    string get_label() const;

    void set_to_default();
    void set_to_first();
    void set_to_last();
    void set_random();

    int get_pos(int value);
    bool increase();
    bool decrease();

    vector<int> get_values(); // mau


    void set_values(vector<double> values, double default_val);
    void set_values(Parameter parameter);

    int get_first();
    int get_last();


private:

    vector<int> values; // vector of possible values
    int current;  // index of current value in the array of values
    int default_value;
    string label;

};

#endif
