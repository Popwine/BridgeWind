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
#include "section_definitions.h"
#include "ui_mainwindow.h"


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
    

    explicit MainWindow(const QString& projectName, const QString& projectPath, QWidget* parent = nullptr);
    ~MainWindow();
protected:

    bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
private slots:
    // 当ComboBox选项改变时调用的槽函数
    void onSectionTypeChanged(int index);
    void onSetGeometryButtonClicked();
    void onSetParametersButtonClicked();
    void onSetBuiltInModeButtonClicked();
    void onSetImportDxfModeButtonClicked();


    //void onApplyButtonClicked();


    
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
    void setupUiConnections();
    void setupTooglePairs();
    void onMaximizeRestore();
    void setupModel();
    void setupUiFromModel();
    

    // 存储工作流中的中间产物
    std::shared_ptr<BridgeWind::TopologyAnalyzer> m_analyzerResult;
    

    // 只需要一个后台线程和服务对象的指针，因为我们一次只运行一个任务
    QThread* m_workerThread = nullptr;
    QObject* m_currentWorker = nullptr; // 用一个通用的QObject*来指向当前的服务对象
	
};


#endif // MAINWINDOW_H