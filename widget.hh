#ifndef WIDGET_HH
#define WIDGET_HH

#include <QWidget>
#include "lockin2/xygraph/xyscene.hh"
#include "metropolis.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void getValues();
    void onPriorChanged();

private:
    qreal interpolate(const QList<QPointF>& xys, qreal x);

    Ui::Widget *ui;

    XYScene* _scene;
    XYPointList* _pointlist;

    Metropolis* _metropolis;
};

#endif // WIDGET_HH
