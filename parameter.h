
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

    double get_val() const;
    int get_size() const;
    double get_default() const;
    void set_val(double new_value);
    void set_label(const string& label);
    string get_label() const;

    void set_to_default();
    void set_to_first();
    void set_to_last();
    void set_random();

    double get_pos(double value);
    bool increase();
    bool decrease();

    vector<double> get_values(); // mau


    void set_values(vector<double> values, double default_val);
    void set_values(Parameter parameter);

    double get_first();
    double get_last();


private:

    vector<double> _values; // vector of possible values
    int current;  // index of current value in the array of values
    double default_value;
    string label;

};

#endif
