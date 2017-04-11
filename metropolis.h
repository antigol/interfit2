#ifndef METROPOLIS_H
#define METROPOLIS_H

#include <QObject>
#include "function.h"
#include <QList>
#include <QPointF>
#include <QThread>
#include <QMutex>

class Metropolis : public QThread
{
    Q_OBJECT

public:
    Metropolis();
    virtual ~Metropolis();

    Function* ground_function();

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

    QMutex mutex;

private:
    virtual void run() override;

    double residues(Function &f);

    double temperature0;
    double factor_temperature;

    QList<Function> walkers; // parallel tempering
};

#endif // METROPOLIS_H
