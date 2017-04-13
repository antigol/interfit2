#include "metropolis.h"
#include <random>
#include <QDebug>
#include <QTime>

std::mt19937& generator() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

Metropolis::Metropolis()
{
    temperature0 = 1e-3;
    factor_temperature = 1.3;

    // temperature_i = factor_temperature^i * temperature0

    for (int i = 0; i < 15; ++i) {
        Function p;
        p.setPen(QPen(QBrush(Qt::red), 3.0));
        walkers.append(p);
    }

    for (int j = 0; j < NPARAM; ++j) {
        std::normal_distribution<double> dist(mu_[j], sigma_[j]);

        for (int i = 0; i < walkers.size(); ++i) {
            walkers[i].parameters_[j] = dist(generator());
            Q_ASSERT(!std::isnan(walkers[i].parameters_[j]));
        }
    }
}

Metropolis::~Metropolis()
{
    if (isRunning()) {
        run_flag = false;
        wait(1000);
    }
}

Function *Metropolis::ground_function()
{
    return &walkers[0];
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

    QList<double> walkers_res;
    for (int i = 0; i < walkers.size(); ++i) {
        walkers_res << residues(walkers[i]);
    }

    while (run_flag) {
        msleep(50);
        QTime time;
        time.start();

        // metropolis-hastings
        for (int k = 0; k < 10; ++k) {
            for (int i = 0; i < walkers.size(); ++i) {
                Function candidate = walkers[i];
                for (int j = 0; j < NPARAM; ++j) {
                    double r = 2.0 * std::generate_canonical<double, std::numeric_limits<double>::digits>(generator()) - 1.0;
                    r = r * r * r;
                    candidate.parameters_[j] += 0.001 * r * sigma_[j];
                    Q_ASSERT(!std::isnan(candidate.parameters_[j]));
                }

                if (candidate.parameters.deposition_rate < 0.0) candidate.parameters.deposition_rate = 0.0;
                if (candidate.parameters.global_factor < 0.0) candidate.parameters.global_factor = 0.0;
                if (candidate.parameters.layer_index < 0.0) candidate.parameters.layer_index = 0.0;
                if (candidate.parameters.substrate_index < 0.0) candidate.parameters.substrate_index = 0.0;
                while (candidate.parameters.polarization < mu.polarization) candidate.parameters.polarization += 360.0;
                candidate.parameters.polarization = std::fmod(candidate.parameters.polarization - mu.polarization + 180.0, 360.0) - 180.0 + mu.polarization;

                // prior contribution
                double x = 0.0;
                double xc = 0.0;
                for (int j = 0; j < NPARAM; ++j) {
                    double tmp = (walkers[i].parameters_[j] - mu_[j]) / sigma_[j];
                    x += tmp * tmp / 2.0;

                    tmp = (candidate.parameters_[j] - mu_[j]) / sigma_[j];
                    xc += tmp * tmp / 2.0;
                }
                double Ec = residues(candidate);
                double temp = std::pow(factor_temperature, i) * temperature0;
                double lnprob = (walkers_res[i] - Ec) / temp + x - xc;

                if (k == 0 && i == 0) {
                    qDebug() << "Ec = " << Ec;
                    qDebug() << "temp = " << temp;
                    qDebug() << "xc = " << xc;
                    qDebug() << "pol = " << candidate.parameters.polarization;
                }

                // rand < proba   <=> log(rand) < log(proba)
                if (std::log(std::generate_canonical<double, std::numeric_limits<double>::digits>(generator())) < lnprob) {
                    walkers[i] = candidate;
                    walkers_res[i] = Ec;
                }
            }
        }

        for (int k = 0; k < 2; ++k) {
            for (int i = 1; i < walkers.size(); ++i) {
                double E1 = walkers_res[i-1];
                double E0 = walkers_res[i];

                // 1 / (q^i T0) - 1 / (q^(i-1) T0) = 1 / Ti - q / Ti = (1 - q) / Ti
                double temp = std::pow(factor_temperature, i) * temperature0;
                double lnprob = (1.0 - factor_temperature) / temp * (E1 - E0);

                // rand < proba   <=> log(rand) < log(proba)
                if (std::log(std::generate_canonical<double, std::numeric_limits<double>::digits>(generator())) < lnprob) {
                    std::swap(walkers[i], walkers[i-1]);
                    std::swap(walkers_res[i], walkers_res[i-1]);

                    //                qDebug() << "SWAP " << i;
                }
            }
        }

        emit evolved();

        qDebug() << time.elapsed() << "ms";
        qDebug() << walkers[0].y(100);
        qDebug() << walkers[0].domain(100);
    }
}

double Metropolis::residues(Function &f)
{
    double residues = 0.0;

    mutex.lock();
    for (int i = 0; i < data->size(); ++i) {
        double fy = -2.0;
        if (f.domain(data->at(i).x()))
            fy = f.y(data->at(i).x());
        double err = data->at(i).y() - fy;
        residues += err * err;
    }
    mutex.unlock();
    return residues / double(data->size());
}
