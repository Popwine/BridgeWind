#include "process_manager.h"
namespace BridgeWind {
    ProcessManager::ProcessManager(QObject* parent)
        : QObject(parent)
    {
        // 在构造函数中创建并配置底层的QProcess对象
        m_process = new QProcess();
        m_process->moveToThread(this->thread());
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
        // 3. 因为m_process没有parent了，我们必须手动删除它，否则会内存泄漏！
        //    使用deleteLater()是线程安全的删除方式。
        if (m_process) {
            // 在删除前，确保所有连接都断开
            m_process->disconnect();
            // 尝试停止进程
            if (m_process->state() != QProcess::NotRunning) {
                m_process->terminate();
                m_process->waitForFinished(1000);
                m_process->kill();
            }
            m_process->deleteLater();
        }
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
        // 确保我们有一个有效且正在运行的进程对象
        // (请将 m_process 替换为您 QProcess 成员变量的实际名称)
        if (!m_process || m_process->state() != QProcess::Running) {
            return;
        }

        // 获取我们启动的进程(mpiexec.exe)的ID (PID)
        qint64 pid = m_process->processId();
        if (pid == 0) {
            return; // 没有PID，无法继续
        }

        emit outputReady(QString("Stopping process tree with root PID: %1").arg(pid));

#if defined(Q_OS_WIN)
        // 在Windows上，我们使用系统命令 taskkill 来终止进程及其整个进程树
        // /F: 强制终止进程。
        // /T: 终止指定的进程以及由它启动的任何子进程。这是最关键的参数！
        // /PID: 指定要终止的进程的PID。
        QString command = QString("taskkill /F /T /PID %1").arg(pid);

        // 同步执行这个命令，确保它执行完毕
        QProcess::execute(command);

#else
        // 在 Linux/macOS 上，可以通过向负的进程组ID发送信号来杀死整个进程组
        // 这假设 QProcess 在一个新的会话中启动了进程，通常是这样的。
        QString command = QString("kill -9 -%1").arg(pid);
        QProcess::execute(command);
#endif

        // 作为备用方案，我们仍然可以调用 QProcess 的 kill，尽管 taskkill 应该已经处理了所有事情。
        m_process->kill();
        // 短暂等待，以确保进程状态更新
        m_process->waitForFinished(1000);
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