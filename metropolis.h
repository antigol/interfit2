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

    void init_walkers();
    Function* ground_function();
    Function* hot_function();

    // Data
    QList<QPointF>* data;

    // Priors
    union {
        Parameters mu;
        double mu_[NPARAM];
    };
    union {
        Parameters sigma;
        double sigma_[NPARAM];
    };

    bool run_flag;

    QMutex mutex;

signals:
    void evolved();

private:
    virtual void run() override;

    double residues(Function &f);

    QList<Function> walkers; // parallel tempering
};

#endif // METROPOLIS_H
