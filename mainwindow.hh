#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include "lockin2/xygraph/xygraph.hh"
#include "metropolis.hh"

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
    void onMetropolisEvolved();

    void on_pushButton_start_fit_clicked();
    void on_pushButton_stop_fit_clicked();
    void on_actionOpen_triggered();
    void on_actionSave_ratio_triggered();

private:
    Ui::MainWindow *ui;
    QList<QLineEdit*> _mus;
    QList<QLineEdit*> _sigmas;
    QList<QLabel*> _labels;

    class Function : public XY::Function
    {
    public:
        qreal y(qreal time) override;
        bool domain(qreal time) const override;

        ParametersUnion p;
    };

    XY::PointList _pointlist;
    QVector<Function> _fits;

    Metropolis* _metropolis;
};

#endif // MAINWINDOW_H
