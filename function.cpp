#include "function.h"
#include "thinfilm/thinfilm3.hh"

Function::Function()
{

}

qreal Function::y(qreal time)
{
    return relfectance_in_function_of_time(p, time);
}

bool Function::domain(qreal time) const
{
    double thickness = p.by_names.deposition_rate * (time + p.by_names.time_offset);
    return thickness >= 0;
}

double relfectance_in_function_of_time(const ParametersUnion &p, double time)
{
    double thickness = p.by_names.deposition_rate * (time + p.by_names.time_offset);
    if (thickness < 0.0)
        return -1.0;

    double cosTheta = std::cos(p.by_names.angle * M_PI / 180.0);

    double reflectanceP = 0.0;
    double reflectanceS = 0.0;
    double transmittanceP = 0.0;
    double transmittanceS = 0.0;

    std::vector<thinfilm::Layer> layers(1);
    layers[0].refractiveIndex = thinfilm::complex(p.by_names.layer_index, p.by_names.layer_abs);
    layers[0].thickness = thickness;

    thinfilm::compute(cosTheta, 1.0, thinfilm::complex(p.by_names.substrate_index, p.by_names.substrate_abs), layers, &reflectanceP, &reflectanceS, &transmittanceP, &transmittanceS);

    double reflectance = p.by_names.polarization * reflectanceP + (1.0 - p.by_names.polarization) * reflectanceS;

    reflectance = p.by_names.global_factor * reflectance + p.by_names.global_offset;

    if (std::isnan(reflectance) || std::isinf(reflectance))
        return -1.0;

    return reflectance;
}
