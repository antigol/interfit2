#include "metropolis.hh"
#include <random>
#include <QDebug>
#include <QTime>
#include <QMutexLocker>

std::mt19937& generator() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

double randd() {
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(generator());
}

Metropolis::Metropolis()
{
    walkers.resize(10);
}

Metropolis::~Metropolis()
{
    if (isRunning()) {
        run_flag = false;
        wait(1000);
    }
}

void Metropolis::init_walkers()
{
    for (int i = 0; i < walkers.size(); ++i) {
        for (int j = 0; j < NPARAM; ++j) {
            std::normal_distribution<double> dist(mus.array[j], sigmas.array[j]);
            walkers[i].array[j] = dist(generator());
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
 * log P(T | D) = - residues / (2 sigma^2) - sum (T_j - mu_j)^2 / (2 simga_j ^2) + Constants(sigma, sigma_j, P(D))
 *
 * accept proba = P(T_new | D) / P(T | D)
 *
 *
 * Parallel tempering
 * T = Temperature = 2 * sigma^2 * SIZE_XY
 * E = Energy = residues
 *
 * accept proba = exp((1/T_a - 1/T_b) * (E_b - E_a)
 */

void Metropolis::run()
{
    run_flag = true;

    QVector<double> walkers_res;
    for (int i = 0; i < walkers.size(); ++i) {
        walkers_res << residues(walkers[i]);
    }

    while (run_flag) {
        msleep(50);

        mutex.lock();
        double moment1 = 0.0;
        double moment2 = 0.0;
        for (int l = 0; l < data->size(); ++l) {
            moment1 += data->at(l).y();
            moment2 += data->at(l).y() * data->at(l).y();
        }
        moment1 /= data->size();
        moment2 /= data->size();
        mutex.unlock();

        // temperature_i = factor_temperature^i * temperature0
        double std_ = std::sqrt(moment2 - moment1 * moment1);
        double temperature0 = std_ * 3.6e-07;
        double factor_temperature = 3.0;

        // Metropolis-Hastings
        for (int k = 0; k < 10; ++k) {
            double temperature = temperature0 / factor_temperature;
            for (int i = 0; i < walkers.size(); ++i) {
                temperature *= factor_temperature; // temperature of the current walker

                // Creation of a candidate from the current "position" (in the space of parameters)
                ParametersUnion candidate = walkers[i];
                for (int j = 0; j < NPARAM; ++j) {
                    double r = 2.0 * randd() - 1.0;
                    r = r * r * r;
                    double a = std::pow(0.8, walkers.size() - i);
                    candidate.array[j] += a * r * sigmas.array[j];
                }

                // These bounds checks probably violate the detailed balance but we dont care about the distribution, only the ground state matter
                if (candidate.by_names.deposition_rate < 0.0) candidate.by_names.deposition_rate = 0.0;
                if (candidate.by_names.global_factor < 0.0) candidate.by_names.global_factor = 0.0;
                if (candidate.by_names.layer_index < 0.0) candidate.by_names.layer_index = 0.0;
                if (candidate.by_names.layer_abs < 0.0) candidate.by_names.layer_abs = 0.0;
                if (candidate.by_names.substrate_index < 0.0) candidate.by_names.substrate_index = 0.0;
                if (candidate.by_names.substrate_abs < 0.0) candidate.by_names.substrate_abs = 0.0;
                if (candidate.by_names.polarization < 0.0) candidate.by_names.polarization = 0.0;
                if (candidate.by_names.polarization > 1.0) candidate.by_names.polarization = 1.0;

                // Compute the probabilities ratio : P(candidate parameters | measure) / P(current parameters | measure)
                // where P(parameters | measure) = P(measure | parameters)  P(parameters) / P(measure)
                //  where P(measure | parameters) = Boltzmann distribution where energy is replaced by the MSE
                //  where P(parameters) are the Priors (given by the user via the GUI)

                // prior contribution
                double x = 0.0;
                double xc = 0.0;
                for (int j = 0; j < NPARAM; ++j) {
                    if (sigmas.array[j] == 0.0) continue;

                    double tmp = (walkers[i].array[j] - mus.array[j]) / sigmas.array[j];
                    x += tmp * tmp / 2.0;

                    tmp = (candidate.array[j] - mus.array[j]) / sigmas.array[j];
                    xc += tmp * tmp / 2.0;
                }

                // energy (residues) contribution
                double E = walkers_res[i];
                double Ec = residues(candidate);

                // log( P(candidate parameters | measure) / P(current parameters | measure) )
                double lnprob = (E - Ec) / temperature + x - xc;

                // rand < proba <=> log(rand) < log(proba)
                if (std::log(randd()) < lnprob) {
                    walkers[i] = candidate;
                    walkers_res[i] = Ec;
                }
            }
        }

        // Parallel tempering : swaps
        for (int k = 0; k < 2; ++k) {
            double temperature = temperature0 / factor_temperature;
            for (int i = 1; i < walkers.size(); ++i) {
                temperature *= factor_temperature;

                // We have two systems with temperature T_a and T_b
                // Each system has its own current micro-state S_a and S_b
                // With respectively energies E_a and E_b
                // Probability of this configuration = Exp[- E_a / T_a] * Exp[- E_b / T_b]  (divided by the partition function)
                // Probability of the swaped configuration = Exp[- E_b / T_a] * Exp[- E_a / T_b]
                // Swap probability = ratio of configurations probabilities = Exp[(E_b - E_a) (T_a - T_b) / (T_a T_b)]

                // In this computation the Priors probabilities cancels out

                double E_cold = walkers_res[i-1];
                double E_hot = walkers_res[i];

                // 1 / (q^i T0) - 1 / (q^(i-1) T0) = 1 / Ti - q / Ti = (1 - q) / Ti
                double lnprob = (1.0 - factor_temperature) / temperature * (E_hot - E_cold);

                // rand < proba <=> log(rand) < log(proba)
                if (std::log(randd()) < lnprob) {
                    std::swap(walkers[i], walkers[i-1]);
                    std::swap(walkers_res[i], walkers_res[i-1]);
                }
            }
        }

        emit evolved();
    }
}

double Metropolis::residues(const ParametersUnion &p)
{
    QMutexLocker lock(&mutex);

    double residues = 0.0;
    int i = 0;
    for (; i < data->size(); ++i) {
        if (data->at(i).x() >= time_from) break;
    }
    for (; i < data->size(); ++i) {
        if (data->at(i).x() > time_to) break;
        double fy = relfectance_in_function_of_time(p, data->at(i).x());
        double err = data->at(i).y() - fy;
        residues += err * err;
    }
    return residues / double(data->size());
}
