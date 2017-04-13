#ifndef FUNCTION_H
#define FUNCTION_H

#include "lockin2/xygraph/xyscene.hh"

#define NPARAM 9

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
};

class Function : public XYFunction
{
public:
    Function();

    qreal y(qreal x) override;
    bool domain(qreal x) const override;

    union {
        Parameters parameters;
        double parameters_[NPARAM];
    };
};

#endif // FUNCTION_H
