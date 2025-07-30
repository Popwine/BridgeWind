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
#include "gmsh_reader.h"

#include <vtkGenericOpenGLRenderWindow.h>



#include <vtkCGNSReader.h>


#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkQuad.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkProperty.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkMultiBlockDataSet.h>
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
    

    explicit MainWindow(const QString& projectPath, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // 按钮点击事件的槽
    void onAnalyzeButtonClicked();
    void onGenerateMeshButtonClicked();
    void onStartCalculationButtonClicked();
    void onStopCalculationButtonClicked();
	void onDisplayMeshButtonClicked();
	void onDisplayContourButtonClicked();

    // 接收后台服务完成信号的槽
    void onAnalysisFinished(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer);
    void onMeshingFinished();
    void onSolverFinished();

    // 通用的日志和错误处理槽
    void appendLogMessage(const QString& message);
    void handleError(const QString& errorMessage);

    void startSolverTask();

    
private:
    BridgeWind::SimulationParameters m_currentParams;
    vtkGenericOpenGLRenderWindow* m_renderWindow;
    vtkNew<vtkRenderer> m_renderer;
    vtkNew<vtkActor> m_actor;
    vtkNew<vtkPolyDataMapper> m_mapper;
    vtkNew<vtkCGNSReader> m_cgnsReader;
    Ui::MainWindow* ui; // 由 .ui 文件生成
    QString m_projectPath;
    void setupUiConnections();
    void cleanupWorker(QThread*& thread, QObject*& worker); // 清理函数
	void setupVtkRenderWindow();
    void updateVtkRenderWindow();

    std::vector<double> getPressureDataForNodes(int numPoints) const;
    void renderContourPlot(vtkUnstructuredGrid* uGridWithData);
    void updateVtkRenderCounterWindow();
    

    // 存储工作流中的中间产物
    std::shared_ptr<BridgeWind::TopologyAnalyzer> m_analyzerResult;
    

    // 只需要一个后台线程和服务对象的指针，因为我们一次只运行一个任务
    QThread* m_workerThread = nullptr;
    QObject* m_currentWorker = nullptr; // 用一个通用的QObject*来指向当前的服务对象
	
};


#endif // MAINWINDOW_H