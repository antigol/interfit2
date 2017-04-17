#ifndef METROPOLIS_H
#define METROPOLIS_H

#include <QObject>
#include "function.hh"
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

    // Data
    QList<QPointF>* data;
    double time_from;
    double time_to;

    // Priors
    ParametersUnion mus;
    ParametersUnion sigmas;

    bool run_flag;

    QMutex mutex;

    QVector<ParametersUnion> walkers; // parallel tempering

signals:
    void evolved();

private:
    virtual void run() override;

    double residues(const ParametersUnion &p);
};

#endif // METROPOLIS_H
