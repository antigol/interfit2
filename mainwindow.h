#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include "lockin2/xygraph/xyscene.hh"
#include "metropolis.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void start_metropolis();
    void stop_metropolis();

    void onValuesRecieved();
    void onPriorChanged();
    void onMetropolisEvolved();

    void on_pushButton_start_fit_clicked();
    void on_pushButton_stop_fit_clicked();
    void on_actionOpen_triggered();
    void on_actionSave_ratio_triggered();

private:
    qreal interpolate(const QList<QPointF>& xys, qreal x);

    Ui::MainWindow *ui;
    QList<QLineEdit*> _mus;
    QList<QLineEdit*> _sigmas;
    QList<QLabel*> _labels;

    XYScene* _scene;
    XYPointList _pointlist;
    QVector<Function> _fits;

    Metropolis* _metropolis;
};

#endif // MAINWINDOW_H
