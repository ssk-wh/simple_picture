#include "ui/MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("EasyPicture");
    app.setApplicationVersion("1.0.0");

    easypic::MainWindow window;
    window.resize(1024, 768);
    window.show();

    // 命令行指定文件则打开
    const QStringList args = app.arguments();
    if (args.size() > 1) {
        window.openFile(args.at(1));
    }

    return app.exec();
}
