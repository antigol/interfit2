#include <QApplication>
#include <lockin2/src/lockin_gui.hh>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    LockinGui lockin_1;
    lockin_1.show();

    LockinGui lockin_2;
    lockin_2.show();

    return app.exec();
}
