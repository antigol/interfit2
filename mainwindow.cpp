#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _scene = new XYScene(this);
    ui->graphicsView->setScene(_scene);
    _pointlist = new XYPointList(QPen(), QBrush(), 2.0, QPen(QBrush(Qt::white), 2.0));
    _scene->addScatterplot(_pointlist);

    connect(ui->lockin_sig, SIGNAL(newValues()), this, SLOT(onValuesRecieved()));

    _metropolis = nullptr;

    startTimer(100);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (_metropolis) {
        delete _metropolis;
    }
    delete _pointlist;
}

//    double substrate_index;
//    double layer_index;
//    double angle;
//    double polarization;
//    double deposition_rate;
//    double time_offset;
//    double global_factor;

void MainWindow::start_metropolis()
{
    if (_metropolis != nullptr) {
        stop_metropolis();
    }

    QList<QLineEdit*> mus;
    mus << ui->lineEdit_substrate_index << ui->lineEdit_layer_index
        << ui->lineEdit_angle << ui->lineEdit_polarization
        << ui->lineEdit_deposition_rate << ui->lineEdit_time_offset
        << ui->lineEdit_global_factor;
    QList<QLineEdit*> sigmas;
    sigmas << ui->lineEdit_std_substrate_index << ui->lineEdit_std_layer_index
           << ui->lineEdit_std_angle << ui->lineEdit_std_polarization
           << ui->lineEdit_std_deposition_rate << ui->lineEdit_std_time_offset
           << ui->lineEdit_std_global_factor;

    _metropolis = new Metropolis();
    for (int j = 0; j < NPARAM; ++j) {
        bool ok;
        _metropolis->mu_[j] = mus[j]->text().toDouble(&ok);
        if (!ok) {
            delete _metropolis;
            _metropolis = nullptr;
            return;
        }
        _metropolis->sigma_[j] = sigmas[j]->text().toDouble(&ok);
        if (!ok) {
            delete _metropolis;
            _metropolis = nullptr;
            return;
        }
    }
    _metropolis->data = _pointlist;
    _metropolis->start(QThread::IdlePriority);
    _scene->addFunction(_metropolis->ground_function());
}

void MainWindow::stop_metropolis()
{
    if (_metropolis != nullptr) {
        _scene->getFunctionsList().clear();
        _metropolis->run_flag = false;
        _metropolis->wait(1000);
        delete _metropolis;
        _metropolis = nullptr;
    }
}

void MainWindow::onValuesRecieved()
{
    const QList<QPointF>& sig = ui->lockin_sig->values();
    const QList<QPointF>& ref = ui->lockin_ref->values();

    qreal offset = ui->lockin_ref->start_time().msecsTo(ui->lockin_sig->start_time()) / 1000.0;

    if (_metropolis) _metropolis->mutex.lock();
    _pointlist->clear();

    for (int i = 0; i < sig.size(); ++i) {
        qreal interp = interpolate(ref, sig[i].x() + offset);
        _pointlist->append(QPointF(sig[i].x(), sig[i].y() / interp));
    }
    if (_metropolis) _metropolis->mutex.unlock();

    _scene->regraph();
}

void MainWindow::onPriorChanged()
{

}

qreal MainWindow::interpolate(const QList<QPointF> &xys, qreal x)
{
    // assume xys ordered in x's

    if (xys.empty()) return 0.0;
    if (x <= xys.first().x()) return xys.first().y();
    if (x >= xys.last().x()) return xys.last().y();

    for (int i = 1; i < xys.size(); ++i) {
        if (x >= xys[i-1].x() && x <= xys[i].x()) {
            return xys[i-1].y() + (x - xys[i-1].x()) / (xys[i].x() - xys[i-1].x()) * (xys[i].y() - xys[i-1].y());
        }
    }
    return 0.0;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if (_metropolis) {
        Function* f = _metropolis->ground_function();

        QList<QLabel*> labels;
        labels << ui->label_substrate_index << ui->label_layer_index
            << ui->label_angle << ui->label_polarization
            << ui->label_deposition_rate << ui->label_time_offset
            << ui->label_global_factor;

        for (int j = 0; j < NPARAM; ++j) {
            labels[j]->setText(QString::number(f->parameters_[j]));
        }

//        qDebug() << f->domain(100);
//        qDebug() << f->y(100);

        _scene->regraph();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.readAll();
    QList<QByteArray> lines = content.split('\n');

    _pointlist->clear();
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].contains('#')) continue;
        QList<QByteArray> rows = lines[i].simplified().split(' ');
        if (rows.size() != 2) continue;
        _pointlist->append(QPointF(rows[0].toDouble(), rows[1].toDouble()));
    }
}

void MainWindow::on_pushButton_start_fit_clicked() {
    start_metropolis();
}

void MainWindow::on_pushButton_stop_fit_clicked() {
    stop_metropolis();
}

