#include "widget.hh"
#include "ui_widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    _scene = new XYScene(this);
    ui->graphicsView->setScene(_scene);
    _pointlist = new XYPointList(QPen(), QBrush(), 2.0, QPen(QBrush(Qt::white), 2.0));
    _scene->addScatterplot(_pointlist);

    connect(ui->lockin_sig, SIGNAL(newValues()), this, SLOT(getValues()));
}

Widget::~Widget()
{
    delete ui;
    delete _pointlist;
}

void Widget::getValues()
{
    const QList<QPointF>& sig = ui->lockin_sig->values();
    const QList<QPointF>& ref = ui->lockin_ref->values();

    qreal offset = ui->lockin_ref->start_time().msecsTo(ui->lockin_sig->start_time()) / 1000.0;
    qDebug() << "offset = " << offset;

    _pointlist->clear();

    for (int i = 0; i < sig.size(); ++i) {
        qreal interp = interpolate(ref, sig[i].x() + offset);
        _pointlist->append(QPointF(sig[i].x(), sig[i].y() / interp));
    }

    _scene->regraph();
}

qreal Widget::interpolate(const QList<QPointF> &xys, qreal x)
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
