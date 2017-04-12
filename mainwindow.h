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
    void onValuesRecieved();
    void onPriorChanged();

    void on_actionOpen_triggered();

private:
    qreal interpolate(const QList<QPointF>& xys, qreal x);

    void timerEvent(QTimerEvent* event) override;

    Ui::MainWindow *ui;

    XYScene* _scene;
    XYPointList* _pointlist;

    Metropolis* _metropolis;
};

#endif // MAINWINDOW_H
