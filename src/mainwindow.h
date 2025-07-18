#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QThread>
#include <QMetaType>
#include <memory>
#include "topology_analyzer.h"
#include "geometry_analysis_service.h"
#include "meshing_service.h"
#include "solver_service.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // 1. 这是Qt的模板化声明
QT_END_NAMESPACE



namespace BridgeWind {
	class BridgeSimulationService;
    struct SimulationParameters;
}

Q_DECLARE_METATYPE(std::shared_ptr<BridgeWind::TopologyAnalyzer>)
Q_DECLARE_METATYPE(BridgeWind::SimulationParameters)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // 按钮点击事件的槽
    void onAnalyzeButtonClicked();
    void onGenerateMeshButtonClicked();
    void onStartCalculationButtonClicked();
    void onStopCalculationButtonClicked();

    // 接收后台服务完成信号的槽
    void onAnalysisFinished(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer);
    void onMeshingFinished(const QString& cgnsFilePath);
    void onSolverFinished();

    // 通用的日志和错误处理槽
    void appendLogMessage(const QString& message);
    void handleError(const QString& errorMessage);

private:
    BridgeWind::SimulationParameters params;
    void setupUiConnections();
    void cleanupWorker(QThread*& thread, QObject*& worker); // 清理函数

    Ui::MainWindow* ui; // 由 .ui 文件生成

    // 存储工作流中的中间产物
    std::shared_ptr<BridgeWind::TopologyAnalyzer> m_analyzerResult;
    QString m_cgnsFilePath;

    // 只需要一个后台线程和服务对象的指针，因为我们一次只运行一个任务
    QThread* m_workerThread = nullptr;
    QObject* m_currentWorker = nullptr; // 用一个通用的QObject*来指向当前的服务对象
};


#endif // MAINWINDOW_H