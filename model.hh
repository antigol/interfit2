#ifndef FUNCTION_H
#define FUNCTION_H

#include "lockin2/xygraph/xygraph.hh"

#define NPARAM 13

struct Parameters {
    // substrate
    double substrate_index;
    double substrate_abs;
    // intermediate layer
    double intlayer_index;
    double intlayer_abs;
    double intlayer_thickness;
    // deposition layer
    double layer2_index;
    double layer2_abs;
    // geometry
    double angle;
    double polarization;    
    // scale and shift factors
    double deposition_rate; // deposition rate in wavelength / seconds
    double time_offset; // in seconds
    double global_factor;
    double global_offset;
};

union ParametersUnion {
    Parameters by_names;
    double array[NPARAM];
};

double relfectance_in_function_of_time(const ParametersUnion& p, double time);

#endif // FUNCTION_H
