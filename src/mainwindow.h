#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QThread>
#include <QMetaType>
#include <QCloseEvent>

#include <memory>

#include "topology_analyzer.h"
#include "geometry_analysis_service.h"
#include "meshing_service.h"
#include "solver_service.h"
#include "gmsh_reader.h"
#include "section_definitions.h"
#include "ui_mainwindow.h"
#include "flow_data_reader.h"

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

enum class RenderOption {
    Mesh,
    Pressure,
    Density,
    XVelocity,
    YVelocity,
    VelocityMagnitude
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    

    explicit MainWindow(const QString& projectName, const QString& projectPath, QWidget* parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
private slots:
    // 当ComboBox选项改变时调用的槽函数
    void onSectionTypeChanged(int index);
    void onFieldSizeMethodChanged(int index);

    void onSetGeometryButtonClicked();
    void onSetMeshingButtonClicked();
    void onSetParametersButtonClicked();
    void onSetBuiltInModeButtonClicked();
    void onSetImportDxfModeButtonClicked();
    void onGeoParameterEditingFinished();
    void onDxfFileDropped(const QStringList& filePaths);
    void onDxfFileBrowseButtonClicked();
    void onGenerateMeshButtonClicked();
    void onStartSimulationButtonClicked();

    void saveAndCheckMeshParameters();

    //void onApplyButtonClicked();
    void generateMesh();


    void appendLogMessage(const QString& message);
    void updateProgressBar(int percentage);
    void appendLogMessageAndUpDateProgressBar(const QString& message, int percentage);
    void onMeshingFinished();
    void handleError(const QString& errorMessage);
    void onSolverFinished();

    void onOneOfTheVelocitySettingsChanged();
    
    void onCheckIterTimerTimeOut();
    void onIterUpdated(int iter);
    void onRenderOptionChanged();

    void onTimeStepChanged();

	void onSettingsButtonClicked();

    
signals:
    void iterUpdated(int iter);
private:
    BridgeWind::SimulationParameters m_currentParams;
    vtkGenericOpenGLRenderWindow* m_renderWindow;
    vtkNew<vtkRenderer> m_renderer;
    vtkNew<vtkActor> m_actor;
    vtkNew<vtkPolyDataMapper> m_mapper;
    vtkNew<vtkCGNSReader> m_cgnsReader;
    Ui::MainWindow* ui; // 由 .ui 文件生成

    QString m_projectName;
    QString m_projectPath;

    QList<SectionDef> m_sectionDefs;
    bool m_isDragging;      // 标志位，表示是否正在拖动
    QPoint m_dragPosition;  // 记录鼠标按下时的位置

    QWidget* m_titleBar;    // 我们的自定义标题栏
    BridgeWind::Geometry m_geometry;
    void setupUiConnections();
    void setupToogleGroups();
    void onMaximizeRestore();
    void setupModel();
    void setupUiFromModel();
    void setupVtkRenderWindow();
    void setupSimulationPart();
    void setupRightWidget();

    void renderVtkWindowWithGeometry(const BridgeWind::Geometry& geometry);

    void cleanupWorker(QThread*& thread, QObject*& worker);
    void updateVtkWdindowWithMesh();
    void startSolverTask();


    void setVelByRey();
    void setReyByVel();

    void collectSimulationParameters();

    void renderVtkWindowByRenderOption();
    void updateVtkWdindowWithContourByCurrentOption();
    void renderContourPlot(vtkUnstructuredGrid* uGridWithData);
    // 存储工作流中的中间产物
    std::shared_ptr<BridgeWind::TopologyAnalyzer> m_analyzerResult;
    

    // 只需要一个后台线程和服务对象的指针，因为我们一次只运行一个任务
    QThread* m_workerThread = nullptr;
    QObject* m_currentWorker = nullptr; // 用一个通用的QObject*来指向当前的服务对象
    QTimer* m_checkIterTimer = nullptr;

    int m_currentIter = 0;
    RenderOption m_currentRenderOption = RenderOption::Mesh;

    BridgeWind::FlowDataReader m_flowDataReader;
    int lastFlowLoadedIter = 0;
    bool isMeshGenerated = false;
};


#endif // MAINWINDOW_H