#ifndef FUNCTION_H
#define FUNCTION_H

#include "lockin2/xygraph/xyscene.hh"

#define NPARAM 7

struct Parameters {
    double substrate_index;
    double layer_index;
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
        double parameters_[7];
    };
};

#endif // FUNCTION_H
