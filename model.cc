#include "model.hh"
#include "thinfilm/thinfilm.hh"

double relfectance_in_function_of_time(const ParametersUnion &p, double time)
{
    double thickness = p.by_names.deposition_rate * (time + p.by_names.time_offset);
    if (thickness < 0.0)
        return -1.0;

    double cosTheta = std::cos(p.by_names.angle * M_PI / 180.0);

    double reflectanceP = 0.0;
    double reflectanceS = 0.0;

    std::vector<thinfilm::Layer> layers(2);
    layers[0].refractiveIndex = thinfilm::complex(p.by_names.layer2_index, p.by_names.layer2_abs);
    layers[0].thickness = thickness;

    layers[1].refractiveIndex = thinfilm::complex(p.by_names.intlayer_index, p.by_names.intlayer_abs);
    layers[1].thickness = p.by_names.intlayer_thickness;

    thinfilm::reflectance(cosTheta, 1.0, thinfilm::complex(p.by_names.substrate_index, p.by_names.substrate_abs), layers, &reflectanceP, &reflectanceS);

    double reflectance = p.by_names.polarization * reflectanceP + (1.0 - p.by_names.polarization) * reflectanceS;

    reflectance = p.by_names.global_factor * reflectance + p.by_names.global_offset;

    if (std::isnan(reflectance) || std::isinf(reflectance))
        return -1.0;

    return reflectance;
}
