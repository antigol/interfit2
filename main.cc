#include <QApplication>
#include "widget.hh"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    Widget w;
    w.show();

    return app.exec();
}
