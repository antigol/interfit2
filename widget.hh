#ifndef WIDGET_HH
#define WIDGET_HH

#include <QWidget>
#include "lockin2/xygraph/xyscene.hh"

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

private:
    qreal interpolate(const QList<QPointF>& xys, qreal x);

    Ui::Widget *ui;

    XYScene* _scene;
    XYPointList* _pointlist;
};

#endif // WIDGET_HH
