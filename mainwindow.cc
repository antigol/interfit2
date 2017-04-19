#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    double substrate_index;
//    double substrate_abs;
//    double layer_index;
//    double layer_abs;
//    double angle;
//    double polarization;
//    double deposition_rate;
//    double time_offset;
//    double global_factor;

    _mus << ui->lineEdit_substrate_index
         << ui->lineEdit_substrate_abs
         << ui->lineEdit_layer_index
         << ui->lineEdit_layer_abs
         << ui->lineEdit_angle
         << ui->lineEdit_polarization
         << ui->lineEdit_deposition_rate
         << ui->lineEdit_time_offset
         << ui->lineEdit_global_factor
         << ui->lineEdit_global_offset;
    _sigmas << ui->lineEdit_std_substrate_index
            << ui->lineEdit_std_substrate_abs
            << ui->lineEdit_std_layer_index
            << ui->lineEdit_std_layer_abs
            << ui->lineEdit_std_angle
            << ui->lineEdit_std_polarization
            << ui->lineEdit_std_deposition_rate
            << ui->lineEdit_std_time_offset
            << ui->lineEdit_std_global_factor
            << ui->lineEdit_std_global_offset;
    _labels << ui->label_substrate_index
            << ui->label_substrate_abs
            << ui->label_layer_index
            << ui->label_layer_abs
            << ui->label_angle
            << ui->label_polarization
            << ui->label_deposition_rate
            << ui->label_time_offset
            << ui->label_global_factor
            << ui->label_global_offset;

    Q_ASSERT(_mus.size() == NPARAM);
    Q_ASSERT(_sigmas.size() == NPARAM);
    Q_ASSERT(_labels.size() == NPARAM);

    QSettings settings;
    for (int j = 0; j < NPARAM; ++j) {
        _mus[j]->setText(settings.value(QString("mu %1").arg(j), _mus[j]->text()).toString());
        _sigmas[j]->setText(settings.value(QString("sigma %1").arg(j), _sigmas[j]->text()).toString());
    }

    _pointlist.dotRadius = 0.0;
    _pointlist.linePen = QPen(QBrush(Qt::white), 2.0);
    ui->graph->pointLists << &_pointlist;

    connect(ui->lockin_sig, SIGNAL(newValue()), this, SLOT(onValuesRecieved()));

    _metropolis = nullptr;
}

MainWindow::~MainWindow()
{
    QSettings settings;
    for (int j = 0; j < NPARAM; ++j) {
        settings.setValue(QString("mu %1").arg(j), _mus[j]->text());
        settings.setValue(QString("sigma %1").arg(j), _sigmas[j]->text());
    }

    delete ui;
    if (_metropolis) {
        delete _metropolis;
    }
}

void MainWindow::start_metropolis()
{
    if (_metropolis != nullptr) {
        stop_metropolis();
    }

    _metropolis = new Metropolis();
    for (int j = 0; j < NPARAM; ++j) {
        bool ok;
        _metropolis->mus.array[j] = _mus[j]->text().toDouble(&ok);
        if (!ok) {
            delete _metropolis;
            _metropolis = nullptr;
            return;
        }
        _metropolis->sigmas.array[j] = _sigmas[j]->text().toDouble(&ok);
        if (!ok) {
            delete _metropolis;
            _metropolis = nullptr;
            return;
        }
    }
    _metropolis->init_walkers();
    _metropolis->data = &_pointlist;
    _metropolis->time_from = ui->lineEdit_from->text().toDouble();
    _metropolis->time_to = ui->lineEdit_to->text().toDouble();
    _metropolis->start(QThread::IdlePriority);

    ui->graph->functions.clear();
    _fits.resize(_metropolis->walkers.size());

    for (int i = 0; i < _metropolis->walkers.size(); ++i) {
        _fits[i].p = _metropolis->walkers[i];
        double x = double(i) / double(_metropolis->walkers.size());
        _fits[i].pen = QPen(QColor::fromHsvF((240.0 + x * 120.0) / 360.0, 1.0, 1.0));
        ui->graph->functions << &_fits[i];
    }

    connect(_metropolis, SIGNAL(evolved()), this, SLOT(onMetropolisEvolved()));
}

void MainWindow::stop_metropolis()
{
    if (_metropolis != nullptr) {
        _metropolis->run_flag = false;
        _metropolis->wait(1000);
        delete _metropolis;
        _metropolis = nullptr;

        ui->graph->functions.clear();
        ui->graph->functions << &_fits[0];
        ui->graph->update();
    }
}

void MainWindow::onValuesRecieved()
{
    const QList<QPointF>& sig = ui->lockin_sig->values();
    const QList<QPointF>& ref = ui->lockin_ref->values();

    qreal offset = qreal(ui->lockin_ref->start_time().msecsTo(ui->lockin_sig->start_time())) / 1000.0;

    if (_metropolis) _metropolis->mutex.lock();
    if (sig.size() < _pointlist.size()) {
        _pointlist.clear();
    }

    if (!ref.isEmpty()) {
        int j = 0;

        for (int i = _pointlist.size(); i < sig.size(); ++i) {
            // interpolate ref
            qreal x_ref = sig[i].x() + offset;
            while (j + 1 < ref.size() && ref[(j + ref.size()) / 2].x() < x_ref) {
                j = (j + ref.size()) / 2;
            }
            while (j + 1 < ref.size() && ref[j + 1].x() < x_ref) {
                j = j + 1;
            } // j is the last such that ref[j].x() < x_ref

            qreal y_ref = ref[j].y();
            if (j + 1 < ref.size() && ref[j].x() < x_ref) {
                y_ref = ref[j].y() + (x_ref - ref[j].x()) / (ref[j+1].x() - ref[j].x()) * (ref[j+1].y() - ref[j].y());
                _pointlist.append(QPointF(sig[i].x(), sig[i].y() / y_ref));
            } else {
                break;
            }
        }
    }

    if (!_pointlist.empty() && ui->graph->xmax() < _pointlist.last().x() && ui->graph->xmax() + 0.1 * ui->graph->xwidth() > _pointlist.last().x()) {
        ui->graph->setxmax(ui->graph->xmax() + 0.1 * ui->graph->xwidth());
    }

    if (_metropolis) _metropolis->mutex.unlock();

    ui->graph->update();
}

void MainWindow::onMetropolisEvolved()
{
    if (_metropolis) {
        for (int j = 0; j < NPARAM; ++j) {
            _labels[j]->setText(QString::number(_metropolis->walkers.first().array[j]));
        }

        for (int i = 0; i < _metropolis->walkers.size(); ++i) {
            _fits[i].p = _metropolis->walkers[i];
        }

        ui->graph->update();
    }
}

void MainWindow::on_pushButton_start_fit_clicked() {
    start_metropolis();
}

void MainWindow::on_pushButton_stop_fit_clicked() {
    stop_metropolis();
}

void MainWindow::on_actionOpen_triggered()
{
    QSettings settings;
    QString fileName = QFileDialog::getOpenFileName(this, "", settings.value("pwd").toString());

    if (fileName.isEmpty()) return;
    settings.setValue("pwd", fileName);

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.readAll();
    file.close();

    QList<QByteArray> lines = content.split('\n');

    stop_metropolis();

    _pointlist.clear();
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].contains('#')) continue;
        QList<QByteArray> rows = lines[i].simplified().split(' ');
        if (rows.size() != 2) continue;
        _pointlist.append(QPointF(rows[0].toDouble(), rows[1].toDouble()));
    }

    ui->graph->autoZoom();
    ui->graph->update();
}

void MainWindow::on_actionSave_ratio_triggered()
{
    QSettings settings;
    QString fileName = QFileDialog::getSaveFileName(this, "", settings.value("pwd").toString());

    if (fileName.isEmpty()) return;
    settings.setValue("pwd", fileName);

    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QByteArray content = "# time[sec] ratio[arbitraty]\n";
    for (int i = 0; i < _pointlist.size(); ++i) {
        double t = _pointlist[i].x();
        double r = _pointlist[i].y();
        content.append(QByteArray::number(t, 'g', 15) + " " + QByteArray::number(r, 'g', 15) + "\n");
    }

    file.write(content);
    file.close();
}
