#ifndef METROPOLIS_H
#define METROPOLIS_H

#include "function.h"
#include <QList>
#include <QPointF>
#include <QThread>

class Metropolis : public QThread
{
    Q_OBJECT

public:
    Metropolis();

    // Data
    QList<QPointF>* data;

    // Priors
    union {
        Parameters mu;
        double mu_[7];
    };
    union {
        Parameters sigma;
        double sigma_[7];
    };

    bool run_flag;

private:
    void run() override;

    double residues(const Function& f) const;

    double temperature0;
    double factor_temperature;

    QList<Function> walkers; // parallel tempering
};

#endif // METROPOLIS_H
