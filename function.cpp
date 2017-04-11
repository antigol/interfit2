#include "function.h"
#include "thinfilm/thinfilm3.hh"

Function::Function()
{

}

qreal Function::y(qreal x)
{
    double thickness = parameters.deposition_rate * (x + parameters.time_offset);
    double cosTheta = std::cos(parameters.angle * M_PI / 180.0);

    double reflectanceP = 0.0;
    double reflectanceS = 0.0;
    double transmittanceP = 0.0;
    double transmittanceS = 0.0;

    std::vector<thinfilm::Layer> layers(1);
    layers[0].refractiveIndex = parameters.layer_index;
    layers[0].thickness = thickness;

    thinfilm::compute(cosTheta, 1.0, parameters.substrate_index, layers, &reflectanceP, &reflectanceS, &transmittanceP, &transmittanceS);

    double c = std::cos(parameters.polarization * M_PI / 180.0);
    double s = std::sin(parameters.polarization * M_PI / 180.0);
    return parameters.global_factor * (c*c * reflectanceP + s*s * reflectanceS);
}

bool Function::domain(qreal x) const
{
    double thickness = parameters.deposition_rate * (x + parameters.time_offset);
    return thickness >= 0;
}
