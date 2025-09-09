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
#include "SettingsManager.h"
#include <QCommandLineParser>
#include <QDateTime>
// 引入 Windows 头文件
#ifdef _WIN32
#include <windows.h>
#endif

// 自定义消息处理器函数
void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // 将 QString 转换为字节数组以便输出
    QByteArray localMsg = msg.toLocal8Bit();

    // 为了更清晰的输出，可以添加时间戳和消息类型
    QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QByteArray logTimeBytes = logTime.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        // 对于 qDebug() 的消息，我们将其打印到标准输出
        fprintf(stdout, "[%s] Debug: %s\n", logTimeBytes.constData(), localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stdout, "[%s] Info: %s\n", logTimeBytes.constData(), localMsg.constData());
        break;
    case QtWarningMsg:
        // 对于 qWarning() 的消息，打印到标准错误
        fprintf(stderr, "[%s] Warning: %s (%s:%u, %s)\n", logTimeBytes.constData(), localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[%s] Critical: %s (%s:%u, %s)\n", logTimeBytes.constData(), localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[%s] Fatal: %s (%s:%u, %s)\n", logTimeBytes.constData(), localMsg.constData(), context.file, context.line, context.function);
        // 对于致命错误，我们可能希望程序中止
        abort();
    }

    // 刷新缓冲区，确保信息能立即显示
    fflush(stdout);
    fflush(stderr);
}

void attachToConsole()
{
#ifdef _WIN32
    // 尝试附加到父进程的控制台，如果失败（例如直接双击启动），则创建一个新的控制台
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        // 重定向标准输入、输出和错误流到新创建的控制台
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        freopen_s(&pCout, "CONOUT$", "w", stderr);
        freopen_s(&pCout, "CONIN$", "r", stdin);

        // 清除 iostream 的状态，使其能够正常工作
        std::cout.clear();
        std::cerr.clear();
        std::cin.clear();
    }
#endif
}

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("dlut");
    QCoreApplication::setApplicationName("BridgeWind");

    ProjectHistoryManager m;
    //m.addProject("first project", "C:/Projects/bridge_wind_case_test");


    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    // --- 命令行参数解析 ---
    QCommandLineParser parser;
    parser.setApplicationDescription("BridgeWind");
    parser.addHelpOption();

    // 添加一个名为 "console" 的选项，用于显示命令行
    QCommandLineOption consoleOption("console", "Show console for debugging output.");
    parser.addOption(consoleOption);

    // 解析应用的命令行参数
    parser.process(a);

    // 如果 "console" 选项被设置，则调用函数显示控制台
    if (parser.isSet(consoleOption)) {
        attachToConsole();
        qInstallMessageHandler(myMessageOutput);
    }

    // --- 你的应用代码 ---
    // 你现在可以像往常一样使用 qDebug, std::cout 等进行输出了
    // 如果显示了控制台，输出会打印在里面
    qDebug() << "This is a qDebug message.";
    std::cout << "This is a std::cout message." << std::endl;
    std::cerr << "This is an error message from std::cerr." << std::endl;

    // --- 添加资源路径打印代码 ---
    qDebug() << "--- Listing all available resources ---";
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
    qDebug() << "------------------------------------";
    // --- 打印代码结束 ---
    SettingsManager& settingsManager = SettingsManager::instance();
    QTranslator translator;
    if (settingsManager.getLanguage() != "en_US") {
		qDebug() << "Current language setting:" << settingsManager.getLanguage();

        // --- 主要解决方案：从文件系统加载 ---
        // 获取可执行文件所在的目录 (例如 C:/.../bin)
        QString appDir = QApplication::applicationDirPath();
        // 构建翻译文件夹的完整路径 (例如 C:/.../bin/translations)
        QString translationsDir = appDir;

        qDebug() << "Searching for translations in:" << translationsDir;

        // 使用 load 的高级版本，它会自动组合路径和文件名
        // 它会在 translationsDir 目录里寻找 bridgewind_zh_CN.qm
        if (translator.load(QLocale(settingsManager.getLanguage()), QLatin1String("bridgewind"), QLatin1String("_"), translationsDir)) {
            a.installTranslator(&translator);
            qDebug() << "Successfully loaded and installed Chinese translation from file system.";
        }
        else {
            qWarning() << "Failed to load translation from file system path:" << translationsDir;
            qWarning() << "Please ensure 'translations/bridgewind_" + settingsManager.getLanguage() + ".qm' exists relative to the executable.";
        }
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