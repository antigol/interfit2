#include "metropolis.h"
#include <random>

std::mt19937& generator() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

Metropolis::Metropolis()
{
    temperature0 = 1e-3;
    factor_temperature = 2.0;

    // temperature_i = factor_temperature^i * temperature0

    for (int i = 0; i < 10; ++i) {
        Function p;
        walkers.append(p);
    }

    for (int j = 0; j < 7; ++j) {
        std::normal_distribution<double> dist(mu_[j], sigma_[j]);

        for (int i = 0; i < walkers.size(); ++i) {
            walkers[i].parameters_[j] = dist(generator());
        }
    }
}

/* D = measured data
 * T = parameters
 *
 * P(T | D) = P(D | T)  P(T) / P(D)
 *
 * D = (X, Y)
 * N = normal distribution
 * hypothesis : Y = model(X, T) + sigma N
 * P(D | T) = exp(- residues / (2 sigma^2)) / Normalization(sigma)
 *
 * P(T) = prior = product of normal distributions
 *
 * log P(T | D) = - residues / (2 sigma^2) - sum (T_j - mu_j) / (2 simga_j ^2) + Constants(sigma, sigma_j, P(D))
 *
 * accept proba = P(T_new | D) / P(T | D)
 *
 *
 * Parallel tempering
 * T = Temperature = 2 * sigma^2
 * E = Energy = residues
 *
 * accept proba = exp((1/T_a - 1/T_b) * (E_b - E_a)
 */

void Metropolis::run()
{
    run_flag = true;
    while (run_flag) {
        //TODO do some metropolis-hastings

        for (int i = 1; i < walkers.size(); ++i) {
            // 1 / (q^i T0) - 1 / (q^(i-1) T0) = (1 - q) / (q^i T0)
            double lnprob = (1.0 - factor_temperature) / std::pow(factor_temperature, i) / temperature0 * (residues(walkers[i-1]) - residues(walkers[i]));

            // rand < proba   <=> log(rand) < log(proba)
            if (std::log(std::generate_canonical<double>(generator())) < lnprob) {
                std::swap(walkers[i], walkers[i-1]);
            }
        }
    }
}

double Metropolis::residues(const Function &f) const
{
    double residues = 0.0;

    for (int i = 0; i < data->size(); ++i) {
        double err = data->at(i).y() - f.y(data->at(i).x());
        residues += err * err;
    }
    return residues;
}
