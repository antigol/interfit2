#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

    void on_actionOpen_triggered();
    void on_pushButton_start_fit_clicked();
    void on_pushButton_stop_fit_clicked();

private:
    qreal interpolate(const QList<QPointF>& xys, qreal x);

    void timerEvent(QTimerEvent* event) override;

    Ui::MainWindow *ui;

    XYScene* _scene;
    XYPointList* _pointlist;

    Metropolis* _metropolis;
};

#endif // MAINWINDOW_H
