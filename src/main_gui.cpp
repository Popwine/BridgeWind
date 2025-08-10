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