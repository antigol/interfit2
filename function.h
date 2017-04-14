#ifndef FUNCTION_H
#define FUNCTION_H

#include "lockin2/xygraph/xyscene.hh"

#define NPARAM 10

struct Parameters {
    double substrate_index;
    double substrate_abs;
    double layer_index;
    double layer_abs;
    double angle;
    double polarization;
    double deposition_rate;
    double time_offset;
    double global_factor;
    double global_offset;
};

union ParametersUnion {
    Parameters by_names;
    double array[NPARAM];
};

class Function : public XYFunction
{
public:
    Function();

    qreal y(qreal time) override;
    bool domain(qreal time) const override;

    ParametersUnion p;
};

double relfectance_in_function_of_time(const ParametersUnion& p, double time);

#endif // FUNCTION_H
