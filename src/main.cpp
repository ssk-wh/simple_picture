#include "ui/MainWindow.h"
#include "version.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SimplePicture");
    app.setApplicationVersion(EASYPIC_VERSION);
    app.setWindowIcon(QIcon(QStringLiteral(":/resources/app-icon.svg")));

    simplepic::MainWindow window;
    window.resize(1024, 768);
    window.show();

    // 命令行指定文件则打开
    const QStringList args = app.arguments();
    if (args.size() > 1) {
        window.openFile(args.at(1));
    }

    return app.exec();
}
