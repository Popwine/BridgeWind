#include "process_manager.h"
namespace BridgeWind {
    ProcessManager::ProcessManager(QObject* parent)
        : QObject(parent)
    {
        // 在构造函数中创建并配置底层的QProcess对象
        m_process = new QProcess(this);

        // 【核心】将QProcess的内部信号连接到我们自己的私有槽函数上。
        // 这样，当QProcess发生事件时，我们的ProcessManager就能得到通知。

        // 1. 连接标准输出信号
        connect(m_process, &QProcess::readyReadStandardOutput, this, &ProcessManager::onReadyReadStandardOutput);

        // 2. 连接标准错误输出信号
        connect(m_process, &QProcess::readyReadStandardError, this, &ProcessManager::onReadyReadStandardError);

        // 3. 连接进程结束信号
        //    这里使用QOverload来处理有重载的信号
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessManager::onFinished);

        // 4. 连接进程错误信号
        connect(m_process, &QProcess::errorOccurred, this, &ProcessManager::onErrorOccurred);
    }

    ProcessManager::~ProcessManager()
    {
        // QProcess对象因为设置了parent为this，会在ProcessManager析构时被Qt自动删除，
        // 所以这里通常不需要写 delete m_process;
    }

    void ProcessManager::start(const QString& program, const QStringList& arguments, const QString& workingDir)
    {
        // 设置工作目录
        m_process->setWorkingDirectory(workingDir);

        // 调用QProcess的start方法启动进程。这是一个异步调用，会立即返回。
        m_process->start(program, arguments);
    }

    void ProcessManager::stop()
    {
        // 首先尝试优雅地终止进程
        m_process->terminate();

        // QProcess的terminate在Windows上可能无效，所以我们增加一个等待和强制杀死机制
        // 等待最多3秒，看进程是否能自己结束
        if (!m_process->waitForFinished(3000))
        {
            // 如果3秒后还没结束，就强制杀死它
            emit outputReady(tr("[ERROR] The process cannot be terminated normally and will be forced to end.\n"));
            m_process->kill();
        }
    }

    // --- 私有槽函数的实现 ---

    void ProcessManager::onReadyReadStandardOutput()
    {
        // 从QProcess读取所有可用的标准输出数据
        QByteArray data = m_process->readAllStandardOutput();

        // 将读取到的字节数据转换成QString，并作为信号的参数发射出去
        // 使用 fromLocal8Bit 是因为Windows控制台程序通常输出本地编码（如GBK）
        emit outputReady(QString::fromLocal8Bit(data));
    }

    void ProcessManager::onReadyReadStandardError()
    {
        QByteArray data = m_process->readAllStandardError();
        // 对于错误输出，我们可以在前面加上一个标记
        emit outputReady(tr("[Error] ") + QString::fromLocal8Bit(data));
    }

    void ProcessManager::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        // 直接将来自QProcess的信号转发出去
        emit finished(exitCode, exitStatus);
    }

    void ProcessManager::onErrorOccurred(QProcess::ProcessError error)
    {
        // QProcess的错误枚举类型可能不够直观，我们把它转换成更易读的字符串
        emit errorOccurred(m_process->errorString());
    }
}