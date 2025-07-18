#pragma once
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>


namespace BridgeWind {


    /**
     * @class ProcessManager
     * @brief 一个通用的、基于QProcess的外部进程管理器。
     *
     * 这个类封装了QProcess，提供了一个更简洁的API来启动、停止外部进程，
     * 并通过信号实时报告进程的输出和状态。
     * 它是异步和非阻塞的。
     */
    class ProcessManager : public QObject
    {
        Q_OBJECT // 必须包含，以支持信号和槽

    public:
        /**
         * @brief 构造函数
         * @param parent 父QObject，用于自动内存管理
         */
        explicit ProcessManager(QObject* parent = nullptr);

        /**
         * @brief 析构函数
         */
        ~ProcessManager();

    public slots:
        /**
         * @brief 启动一个外部进程（异步）。
         * @param program 要执行的程序（如 "mpiexec" 或 "C:/path/to/gmsh.exe"）。
         * @param arguments 传递给程序的命令行参数列表。
         * @param workingDir 程序的工作目录。
         */
        void start(const QString& program, const QStringList& arguments, const QString& workingDir);

        /**
         * @brief 尝试终止正在运行的进程。
         */
        void stop();

    signals:
        /**
         * @brief 当进程有新的标准输出或标准错误输出时发射。
         * @param output 捕获到的输出文本。
         */
        void outputReady(const QString& output);

        /**
         * @brief 当进程结束时发射。
         * @param exitCode 进程的退出码（0通常表示成功）。
         * @param exitStatus 进程的退出状态（正常退出还是崩溃）。
         */
        void finished(int exitCode, QProcess::ExitStatus exitStatus);

        /**
         * @brief 当启动进程或进程运行期间发生错误时发射。
         * @param errorString 描述错误的字符串。
         */
        void errorOccurred(const QString& errorString);

    private slots:
        // 这些是私有槽，用来连接和处理来自底层QProcess对象的原始信号。

        /**
         * @brief 当底层QProcess有可读的标准输出时被调用。
         */
        void onReadyReadStandardOutput();

        /**
         * @brief 当底层QProcess有可读的标准错误输出时被调用。
         */
        void onReadyReadStandardError();

        /**
         * @brief 当底层QProcess结束后被调用。
         */
        void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

        /**
         * @brief 当底层QProcess发生错误时被调用。
         */
        void onErrorOccurred(QProcess::ProcessError error);

    private:
        QProcess* m_process; // 指向底层QProcess对象的指针
    };
}





#endif // PROCESS_MANAGER_H