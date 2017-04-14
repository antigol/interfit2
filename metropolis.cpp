#include "metropolis.h"
#include <random>
#include <QDebug>
#include <QTime>
#include <QMutexLocker>

std::mt19937& generator() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
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
                temperature *= factor_temperature;

                ParametersUnion candidate = walkers[i];
                for (int j = 0; j < NPARAM; ++j) {
                    double r = 2.0 * std::generate_canonical<double, std::numeric_limits<double>::digits>(generator()) - 1.0;
                    r = r * r * r;
                    double a = std::pow(0.8, walkers.size() - i);
                    candidate.array[j] += a * r * sigmas.array[j];
                    Q_ASSERT(!std::isnan(candidate.array[j]));
                }

                if (candidate.by_names.deposition_rate < 0.0) candidate.by_names.deposition_rate = 0.0;
                if (candidate.by_names.global_factor < 0.0) candidate.by_names.global_factor = 0.0;
                if (candidate.by_names.layer_index < 0.0) candidate.by_names.layer_index = 0.0;
                if (candidate.by_names.layer_abs < 0.0) candidate.by_names.layer_abs = 0.0;
                if (candidate.by_names.substrate_index < 0.0) candidate.by_names.substrate_index = 0.0;
                if (candidate.by_names.substrate_abs < 0.0) candidate.by_names.substrate_abs = 0.0;
                if (candidate.by_names.polarization < 0.0) candidate.by_names.polarization = 0.0;
                if (candidate.by_names.polarization > 1.0) candidate.by_names.polarization = 1.0;

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
                double Ec = residues(candidate);
                double lnprob = (walkers_res[i] - Ec) / temperature + x - xc;

                // rand < proba <=> log(rand) < log(proba)
                if (std::log(std::generate_canonical<double, std::numeric_limits<double>::digits>(generator())) < lnprob) {
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

                double E1 = walkers_res[i-1];
                double E0 = walkers_res[i];

                // 1 / (q^i T0) - 1 / (q^(i-1) T0) = 1 / Ti - q / Ti = (1 - q) / Ti
                double lnprob = (1.0 - factor_temperature) / temperature * (E0 - E1);

                // rand < proba <=> log(rand) < log(proba)
                if (std::log(std::generate_canonical<double, std::numeric_limits<double>::digits>(generator())) < lnprob) {
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
    for (int i = 0; i < data->size(); ++i) {
        double fy = relfectance_in_function_of_time(p, data->at(i).x());
        double err = data->at(i).y() - fy;
        residues += err * err;
    }
    return residues / double(data->size());
}
