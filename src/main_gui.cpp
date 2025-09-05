#include "mainwindow.h"
#include "welcomedialog.h"
#include <QApplication>
#include <QSettings>
#include <QDialog>
#include <QTranslator>
#include <QDebug>
#include "flow_data_reader.h"
#include <QDirIterator>
#include "ProjectHistoryManager.h"
int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("dlut");
    QCoreApplication::setApplicationName("BridgeWind");

    ProjectHistoryManager m;
    //m.addProject("first project", "C:/Projects/bridge_wind_case_test");


    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    // --- 添加资源路径打印代码 ---
    qDebug() << "--- Listing all available resources ---";
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
    qDebug() << "------------------------------------";
    // --- 打印代码结束 ---

    QTranslator translator;

    // --- 主要解决方案：从文件系统加载 ---
    // 获取可执行文件所在的目录 (例如 C:/.../bin)
    QString appDir = QApplication::applicationDirPath();
    // 构建翻译文件夹的完整路径 (例如 C:/.../bin/translations)
    QString translationsDir = appDir;

    qDebug() << "Searching for translations in:" << translationsDir;

    // 使用 load 的高级版本，它会自动组合路径和文件名
    // 它会在 translationsDir 目录里寻找 bridgewind_zh_CN.qm
    if (translator.load(QLocale("zh_CN"), QLatin1String("bridgewind"), QLatin1String("_"), translationsDir)) {
        a.installTranslator(&translator);
        qDebug() << "Successfully loaded and installed Chinese translation from file system.";
    }
    else {
        qWarning() << "Failed to load translation from file system path:" << translationsDir;
        qWarning() << "Please ensure 'translations/bridgewind_zh_CN.qm' exists relative to the executable.";
    }



    a.setWindowIcon(QIcon(":/icons/res/icons/app_icon.ico"));
    QFont defaultFont = QApplication::font();
    //defaultFont.setPointSize(12);
    defaultFont.setFamily("Microsoft YaHei UI");
    a.setFont(defaultFont);
    QFile qssFile(":/styles/default_theme.qss");
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = QLatin1String(qssFile.readAll());
        a.setStyleSheet(style);
        qssFile.close();
    }
    else {
        qWarning() << "Could not open QSS file";
    }


    WelcomeDialog welcomeDialog;
    
    if (welcomeDialog.exec() == QDialog::Accepted) {
        
        QString projectPath = welcomeDialog.finalProjectPath();
        QString projectName = welcomeDialog.finalProjectName();
        // 2. 将路径传递给 MainWindow
        MainWindow w(projectName, projectPath); // 需要修改 MainWindow 的构造函数
        w.show();

        // 5. 让应用程序进入事件循环
        return a.exec();
    }

    else {
       
        return 0;
    }

    //QString projectName = "示例工程";
    //QString projectPath = "C:/Projects/bridge_wind_case_test/test5-streamline";
    //MainWindow w(projectName, projectPath); // 需要修改 MainWindow 的构造函数
    //w.show();
    //return a.exec();
}