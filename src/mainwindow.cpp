#include "mainwindow.h"

#include "bridge_simulation_service.h" 

#include "flow_data_reader.h"

#include <filesystem>
#include <algorithm> 
#include <functional>
#include <optional> 

#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QWidget>
#include <QMap> 
#include <QMessageBox>
#include <QTimer>
#include <QTextDocument>
#include <QTextBlock>
#include <QStringList>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

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
#include <vtkCellData.h>
#include <vtkContourFilter.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>
#include <vtkLineSource.h>
#include <vtkArcSource.h>
#include <vtkAppendPolyData.h>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <WinUser.h>
#include <dwmapi.h>
#endif

namespace {
    inline void executeSafely(QWidget* parentWidget, const std::function<void()>& action) {
        try {
            action();
        }
        catch (const std::invalid_argument& e) {
            QMessageBox::critical(parentWidget, QObject::tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
        }
        catch (const std::exception& e) {
            QMessageBox::critical(parentWidget, QObject::tr("Error"), QString("Exception occurred: %1").arg(e.what()));
        }
        catch (...) {
            QMessageBox::critical(parentWidget, QObject::tr("Error"), QObject::tr("Unknown Error"));
        }
    }
    /**
     * @brief 判断字符串是否符合指定格式，并提取第一个数据。
     * @param input The input string.
     * @return 如果格式匹配，返回包含第一个数据的 std::optional；否则返回 std::nullopt。
     */
    std::optional<QString> extractFirstData(const QString& input) {
        // 创建正则表达式对象。使用 static 可以避免每次调用都重新编译，提高性能。
        static const QRegularExpression regex(R"(^\[.*?\]\s+(\S+).*)");
        // C++11原始字符串字面量 R"(...)" 可以避免写双反斜杠 \\

        QRegularExpressionMatch match = regex.match(input);

        if (match.hasMatch()) {
            // hasMatch() 返回 true，说明整个字符串匹配成功

            // captured(1) 获取第一个捕获组的内容
            // captured(0) 是整个匹配的字符串
            return match.captured(1);
        }

        // 如果不匹配，返回一个空的 optional
        return std::nullopt;
    }
}

MainWindow::MainWindow(const QString& projectName, const QString& projectPath, QWidget* parent)
:
    m_projectName(projectName), 
    m_projectPath(projectPath), 
    QMainWindow(parent), 
    ui(new Ui::MainWindow),
    m_flowDataReader(projectPath.toStdString())
{
    //setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->setupUi(this);
#if defined(Q_OS_WIN)
    // 获取窗口句柄
    HWND hwnd = (HWND)this->winId();
    // 扩展客户区以覆盖整个窗口
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
#endif 

    setupModel();

    setupUiFromModel();

    setupUiConnections();

    ui->sectionTypeComboBox->setCurrentIndex(0);
    onSectionTypeChanged(0);

    ui->projectNameLable->setText(projectName);
    m_currentParams.workingDirectory = projectPath.toStdString();


    setupToogleGroups();
    
    setupVtkRenderWindow();
    setupRightWidget();

    ui->circumferentialMeshNumberEdit->setValidator(new QIntValidator(0, 100000000, this));
    ui->radialMeshNumberEdit->setValidator(new QIntValidator(0, 100000000, this));

    ui->radialGrowthRateEdit->setValidator(new QDoubleValidator(0.1, 1, 16, this));
    ui->fieldSizeValueEdit->setValidator(new QDoubleValidator(0.0, 1e10, 16, this));
    ui->fieldToSizeRatioEdit->setValidator(new QDoubleValidator(0.0, 1e5, 16, this));


    //QList<int> currentSizes = ui->middleWidgetSplitter->sizes();
    //for (const auto i : currentSizes) {
    //    std::cout << i << std::endl;
    //}
    //currentSizes[0] = 1;
    //currentSizes[1] = 1;
    //ui->middleWidgetSplitter->setSizes(currentSizes);


    qRegisterMetaType<std::shared_ptr<BridgeWind::TopologyAnalyzer>>("std::shared_ptr<BridgeWind::TopologyAnalyzer>");
    qRegisterMetaType<BridgeWind::SimulationParameters>("Bridg eWind::SimulationParameters");
    
    ui->progressBar->hide();

    setupSimulationPart();

    connect(this, &MainWindow::iterUpdated, this, &MainWindow::onIterUpdated);

    ui->setGeometryButton->setChecked(true);
    ui->stepStack->setCurrentIndex(0);

    ui->rightScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->logViewer->setLineWrapMode(QTextEdit::NoWrap);
}


// 步骤 3.1: 填充截面数据
void MainWindow::setupModel()
{
    m_sectionDefs.append({
        SectionTypes::Undefined, //id
        "section_undefined", "Select a section",
        {
            
        }
        });
    m_sectionDefs.append({
        SectionTypes::Circle, //id
        "section_circle", "Circle",
        {
            {"param_diameters", "Diameter D(m)", 1.0}
        }
        });

    m_sectionDefs.append({
        SectionTypes::Rectangle,
        "section_rectangle", "Rectangle",
        {
            {"param_width", "Width W(m)", 8.0},
            {"param_height", "Height H(m)", 2.0}
        }
        });

    m_sectionDefs.append({
        SectionTypes::ChamferedRectangle,
        "section_chamfered_rectangle", "Chamfered Rectangle",
        {
            {"param_width", "Width W(m)", 4.0},
            {"param_height", "Height H(m)", 2.0},
            {"param_chamfered_radius", "Chamfered Radius R(m)", 0.2}
        }
        });

    m_sectionDefs.append({
        SectionTypes::StreamlinedBoxGirder,
        "section_streamlined_box_girder", "Streamlined Box Girder",
        {
            {"param_total_width", "Total Width B(m)", 12},
            {"param_total_height", "Total Height H(m)", 1.5},
            {"param_bottom_width", "Bottom Width B1(m)", 8},
            {"param_slope", "Top Slope i(%)", 2},
            {"param_angle_1", "Angle 1 a1(°)", 30},
            {"param_angle_2", "Angle 2 a2(°)", 20},
        }
        });
}

// 步骤 3.2: 根据数据模型动态创建UI
void MainWindow::setupUiFromModel()
{
    for (const auto& def : m_sectionDefs) {
        // A. 填充ComboBox，显示翻译后的文本，并绑定稳定的ID作为内部数据
        ui->sectionTypeComboBox->addItem(tr(def.nameComment), QVariant::fromValue(def.id));

        // B. 创建一个可滚动的区域，以防参数过多
        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true); // 这一步至关重要！

        // C. 创建一个容器Widget，放置在滚动区内
        QWidget* contentWidget = new QWidget();

        // 使用QFormLayout可以自动对齐“标签:输入框”
        QVBoxLayout* vBoxLayout = new QVBoxLayout(contentWidget);
        
        // D. 遍历该截面的所有参数，并为每个参数创建标签和输入框
        for (const auto& param : def.parameters) {
            // 使用tr()翻译参数的键
            QLabel* label = new QLabel(tr(param.comment));
            QLineEdit* lineEdit = new QLineEdit(QString::number(param.defaultValue));
            auto font = label->font();
            font.setPointSize(10);


            label->setFont(font);
            lineEdit->setFont(font);
            connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::onGeoParameterEditingFinished);

            // 设置验证器，只允许输入浮点数
            lineEdit->setValidator(new QDoubleValidator(0, 1e9, 4, this));
           
            vBoxLayout->addWidget(label);
            vBoxLayout->addWidget(lineEdit);
        }
        vBoxLayout->addStretch();
        // E. 将包含所有参数的contentWidget设置给滚动区

        
        scrollArea->setWidget(contentWidget);

        // F. 将配置好的整个滚动区作为一个页面，添加到StackedWidget中
        
        ui->parameterStack->addWidget(scrollArea);
    }
}
void MainWindow::setupVtkRenderWindow() {
    


    vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();

    // --- 3. 创建 Mapper 和 Actor ---

    vtkNew<vtkTextActor> actor;
    
    actor->SetInput("BridgeWind v0.0\nChoose a section to start.");
    vtkTextProperty* textProperty = actor->GetTextProperty();
    textProperty->SetFontSize(24);
    textProperty->SetFontFamilyToCourier();
    textProperty->SetColor(0.7, 0.7, 0.7); // 黄色

    textProperty->SetJustificationToCentered(); // 水平居中对齐
    textProperty->SetVerticalJustificationToTop(); // 垂直顶部对齐

    // --- 3. 设置文本在窗口中的位置 ---
    // (x, y) 像素坐标，原点在左下角
    actor->SetPosition(250, 350);
    // --- 4. 将 Actor 添加到 Renderer ---
    // 使用我们之前在 .h 文件中声明的成员变量 m_renderer
    m_renderer->AddActor(actor);
    m_renderer->SetBackground(0.949, 0.949, 0.969); // 设置背景色

    // --- 5. 将 Renderer 添加到 RenderWindow ---
    // 这是将 VTK 管线连接到 Qt 控件的关键一步
    renderWindow->AddRenderer(m_renderer);

    // --- 6. (可选但推荐) 设置 2D 交互方式 ---
    vtkNew<vtkInteractorStyleImage> style;
    renderWindow->GetInteractor()->SetInteractorStyle(style);

    // --- 7. 重置相机视角并渲染 ---
    m_renderer->ResetCamera();
    renderWindow->Render();
}

void MainWindow::setupSimulationPart() {
    ui->velocityViaReynoldsRadio->setChecked(true);
    ui->velocityValueEdit->hide();
    ui->reynoldsNumberResult->hide();
    ui->velocityValueResult->setEnabled(false);
    ui->reynoldsNumberResult->setEnabled(false);
    ui->viscousModelComboBox->setEnabled(true);
    int coreCount = QThread::idealThreadCount();
    ui->numberOfCpusEdit->setValidator(new QIntValidator(2, coreCount, this));
    ui->timeStepEdit->setValidator(new QDoubleValidator(1e-10, 10, 16, this));
    ui->maxTimeStepEdit->setValidator(new QIntValidator(1, 100000000000, this));
    ui->saveAndPlotTimeStepIntervalEdit->setValidator(new QIntValidator(1, 100000000000, this));
    ui->recordTimeStepIntervalEdit->setValidator(new QIntValidator(1, 100000000000, this));
    ui->velocityValueEdit->setValidator(new QDoubleValidator(1e-10, 10, 16, this));
    ui->reynoldsNumberEdit->setValidator(new QDoubleValidator(1e-10, 10, 16, this));
    ui->attackAngelEdit->setValidator(new QDoubleValidator(-180, 180, 16, this));

}

void MainWindow::setupRightWidget() {

    ui->rightMeshButton->setAspectRatio(16, 9);
    ui->rightPressureButton->setAspectRatio(16, 9);
    ui->rightVelocityMagnitudeButton->setAspectRatio(16, 9);
    ui->rightXVelocityButton->setAspectRatio(16, 9);
    ui->rightYVelocityButton->setAspectRatio(16, 9);
    ui->rightDensityButton->setAspectRatio(16, 9);
    connect(ui->rightMeshButton, &QPushButton::clicked, this, &MainWindow::onRenderOptionChanged);
    connect(ui->rightPressureButton, &QPushButton::clicked, this, &MainWindow::onRenderOptionChanged);
    connect(ui->rightVelocityMagnitudeButton, &QPushButton::clicked, this, &MainWindow::onRenderOptionChanged);
    connect(ui->rightXVelocityButton, &QPushButton::clicked, this, &MainWindow::onRenderOptionChanged);
    connect(ui->rightYVelocityButton, &QPushButton::clicked, this, &MainWindow::onRenderOptionChanged);
    connect(ui->rightDensityButton, &QPushButton::clicked, this, &MainWindow::onRenderOptionChanged);

    ui->rightMeshButton->setStyleSheet(
        "QPushButton { "
        "image: url(:/res/images/mesh_background.png);"
        "}"
    );
    ui->rightPressureButton->setStyleSheet(
        "QPushButton { "
        "image: url(:/res/images/pressure_background.png);"
        "}"
    );
    ui->rightVelocityMagnitudeButton->setStyleSheet(
        "QPushButton { "
        "image: url(:/res/images/velocity_magnitude_background.png);"
        "}"
    );
    ui->rightXVelocityButton->setStyleSheet(
        "QPushButton { "
        "image: url(:/res/images/x_velocity_background.png);"
        "}"
    );
    ui->rightYVelocityButton->setStyleSheet(
        "QPushButton { "
        "image: url(:/res/images/y_velocity_background.png);"
        "}"
    );
    ui->rightDensityButton->setStyleSheet(
        "QPushButton { "
        "image: url(:/res/images/density_background.png);"
        "}"
    );
    QButtonGroup* vtkToggleGroup = new QButtonGroup(this);
    vtkToggleGroup->addButton(ui->rightMeshButton);
    vtkToggleGroup->addButton(ui->rightPressureButton);
    vtkToggleGroup->addButton(ui->rightVelocityMagnitudeButton);
    vtkToggleGroup->addButton(ui->rightXVelocityButton);
    vtkToggleGroup->addButton(ui->rightYVelocityButton);
    vtkToggleGroup->addButton(ui->rightDensityButton);

    // 设置为互斥模式
    vtkToggleGroup->setExclusive(true);
    ui->rightMeshButton->setChecked(true);
}
void MainWindow::renderVtkWindowWithGeometry(const BridgeWind::Geometry& geometry) {

    vtkNew<vtkAppendPolyData> appendFilter;



    for (const auto& line : geometry.lines) {
        vtkNew<vtkLineSource> lineSource;
        lineSource->SetPoint1(line.begin.x, line.begin.y, 0);
        lineSource->SetPoint2(line.end.x, line.end.y, 0);
        appendFilter->AddInputConnection(lineSource->GetOutputPort());
    }
    auto vtkArcs = geometry.getVtkFormatArcs();
    for (const auto& vArc : vtkArcs) {
        vtkNew<vtkArcSource> arcSource;
        arcSource->SetCenter(vArc.center.x, vArc.center.y, 0);
        arcSource->SetPoint1(vArc.p1.x, vArc.p1.y, 0);
        arcSource->SetPoint2(vArc.p2.x, vArc.p2.y, 0);
        arcSource->SetResolution(vArc.resolution);
        appendFilter->AddInputConnection(arcSource->GetOutputPort());
    }

    vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();



    vtkNew<vtkPolyDataMapper> combinedMapper;
    combinedMapper->SetInputConnection(appendFilter->GetOutputPort());

    vtkNew<vtkActor> combinedActor;
    combinedActor->SetMapper(combinedMapper);
    combinedActor->GetProperty()->SetColor(0, 0, 0);
    combinedActor->GetProperty()->SetLineWidth(2);

    // --- 4. 渲染 ---


    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(combinedActor); // 只需添加这一个 Actor

    renderWindow->AddRenderer(m_renderer);
    m_renderer->ResetCamera();
    renderWindow->Render();

}
void MainWindow::cleanupWorker(QThread*& thread, QObject*& worker)
{
    if (worker) {
        worker->deleteLater();
        worker = nullptr;
    }
    if (thread) {
        thread->deleteLater();
        thread = nullptr;
    }
}

void MainWindow::updateVtkWdindowWithMesh() {

    executeSafely(this, [&]() {
        appendLogMessageAndUpDateProgressBar(tr("Showing mesh..."), 1);
        std::filesystem::path workDir(m_currentParams.workingDirectory);
        std::filesystem::path fullPath = workDir / "grid" / "Bridge_Wind.msh";
        vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();


        G2C::Mesh mesh;
        G2C::GmeshReader reader;

        reader.load(fullPath.string(), mesh);

        appendLogMessageAndUpDateProgressBar(tr("Mesh loaded successfully"), 20);

        int numPoints = mesh.getNumNodes();
        int numCells = mesh.getNumQuadElements();

        vtkNew<vtkPoints> points;
        for (int i = 0; i < numPoints; ++i) {
            points->InsertNextPoint(mesh.getNodeByIndex(i).x, mesh.getNodeByIndex(i).y, 0.0);
        }
        auto connectivity_quads = mesh.getQuadElementsIndexConnectivity();

        appendLogMessageAndUpDateProgressBar(tr("Parsed points to vtk successfully."), 40);

        vtkNew<vtkCellArray> quads;
        for (int i = 0; i < numCells; ++i) {
            vtkNew<vtkQuad> quad;

            for (int j = 0; j < 4; ++j) {
                quad->GetPointIds()->SetId(j, connectivity_quads[i][j]);
            }
            quads->InsertNextCell(quad);
        }
        appendLogMessageAndUpDateProgressBar(tr("Parsed cells to vtk successfully."), 80);

        vtkNew<vtkUnstructuredGrid> uGrid;
        uGrid->SetPoints(points);
        uGrid->SetCells(VTK_QUAD, quads);

        // --- 3. 创建 Mapper 和 Actor ---
        vtkNew<vtkDataSetMapper> mapper;
        mapper->SetInputData(uGrid);

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetRepresentationToWireframe(); // 线框模式
        actor->GetProperty()->SetColor(0, 0, 0);

        // --- 4. 将 Actor 添加到 Renderer ---
        // 使用我们之前在 .h 文件中声明的成员变量 m_renderer
        m_renderer->RemoveAllViewProps();// 删除所有actor
        m_renderer->AddActor(actor);


        // --- 5. 将 Renderer 添加到 RenderWindow ---
        // 这是将 VTK 管线连接到 Qt 控件的关键一步
        renderWindow->AddRenderer(m_renderer);

        // --- 6. (可选但推荐) 设置 2D 交互方式 ---
        vtkNew<vtkInteractorStyleImage> style;
        renderWindow->GetInteractor()->SetInteractorStyle(style);

        // --- 7. 重置相机视角并渲染 ---
        m_renderer->ResetCamera();
        renderWindow->Render();
        appendLogMessageAndUpDateProgressBar(tr("Displayed mesh in vtk successfully."), 80);
        });
}
void MainWindow::startSolverTask()
{
    // 这个函数是在后台线程 m_workerThread 中被执行的！

    // --- 1. 创建工作对象 ---
    // 因为现在是在后台线程中 new, 所以 worker 从诞生起就属于后台线程。
    auto worker = new BridgeWind::SolverService();
    m_currentWorker = worker;

    // --- 2. 连接所有与 worker 相关的信号与槽 ---
    connect(worker, &BridgeWind::SolverService::logMessageReady, this, &MainWindow::appendLogMessage);
    connect(worker, &BridgeWind::SolverService::solverFinished, this, &MainWindow::onSolverFinished);
    connect(worker, &BridgeWind::SolverService::errorOccurred, this, &MainWindow::handleError);

    // 关键：这里连接停止按钮时, 连接类型要用 QueuedConnection
    // 因为 worker 在后台线程, 而 clicked 信号来自主线程。
    connect(ui->startOrStopSimulationButton, &QPushButton::clicked, worker, &BridgeWind::SolverService::stop, Qt::QueuedConnection);

    // --- 3. 启动任务 ---
    // 直接调用 run, 因为我们已经在同一个线程里了。
    // 我们需要把参数传递进来。可以通过成员变量或者Lambda捕获来做。
    // 这里用成员变量 m_currentParams (假设你在.h里定义了它)
    worker->run(m_currentParams); // m_currentParams 是在 onStartButtonClicked 里设置的
}
// 在 reynoldsNumberEdit 编辑结束后被调用
// 目标：根据 reynolds 计算 velocity 并更新 velocityValueEdit
void MainWindow::setVelByRey() {
    executeSafely(this, [&]() {
        if (m_geometry.isEmpty()) {
            appendLogMessage(tr("Warning: Cannot calculate velocity without geometry."));
            return;
        }

        double height = m_geometry.getBoundingBoxHeight();
        double density = 1.225;
        double viscosity = 1.7894e-5;
        double reynoldsNumber = ui->reynoldsNumberEdit->text().toDouble();

        double vel = reynoldsNumber * viscosity / density / height;

        // 【修正】更新正确的控件，并使用正确计算出的值
        ui->velocityValueResult->blockSignals(true);
        ui->velocityValueResult->setText(QString::number(vel));
        ui->velocityValueResult->blockSignals(false);
        m_currentParams.refReNumber = reynoldsNumber;
        m_currentParams.refDimensionalVelocity = vel;
        m_currentParams.refMachNumber = m_currentParams.refDimensionalVelocity / 340.295;
        });
}
// 在 velocityValueEdit 编辑结束后被调用
// 目标：根据 velocity 计算 reynolds 并更新 reynoldsNumberEdit
void MainWindow::setReyByVel() {
    executeSafely(this, [&]() {
        if (m_geometry.isEmpty()) {
            // 可以在这里弹窗提示，或者在状态栏显示信息，而不是抛出异常
            appendLogMessage(tr("Warning: Cannot calculate Reynolds number without geometry."));
            return;
        }

        double height = m_geometry.getBoundingBoxHeight();
        double density = 1.225;
        double viscosity = 1.7894e-5;
        double vel = ui->velocityValueEdit->text().toDouble();

        double rey = density * vel * height / viscosity;

        // 【修正】更新正确的控件：雷诺数输入框
        ui->reynoldsNumberResult->blockSignals(true);
        ui->reynoldsNumberResult->setText(QString::number(rey));
        ui->reynoldsNumberResult->blockSignals(false);
        m_currentParams.refReNumber = rey;
        m_currentParams.refDimensionalVelocity = vel;
        m_currentParams.refMachNumber = m_currentParams.refDimensionalVelocity / 340.295;
        });
}

void MainWindow::collectSimulationParameters() {
    onOneOfTheVelocitySettingsChanged();
    m_currentParams.maxproc = ui->numberOfCpusEdit->text().toInt();
    int coreCount = QThread::idealThreadCount();
    if (m_currentParams.maxproc > coreCount) {
        throw std::runtime_error(
            "The number of cpu cores you set is greater than the number of logical cores of your cpu. Unable to calculate."
            "The number of cpu cores you set: " + std::to_string(m_currentParams.maxproc) +
            "The number of logical cores of your cpu: " + std::to_string(coreCount)
        );
    }
    
    m_currentParams.physicalTimeStep = ui->timeStepEdit->text().toDouble();
    m_currentParams.maxSimuStep = ui->maxTimeStepEdit->text().toInt();
    m_currentParams.intervalStepFlow = ui->saveAndPlotTimeStepIntervalEdit->text().toInt();
    m_currentParams.intervalStepPlot = ui->saveAndPlotTimeStepIntervalEdit->text().toInt();
    m_currentParams.intervalStepForce = ui->recordTimeStepIntervalEdit->text().toInt();
    m_currentParams.intervalStepRes = ui->recordTimeStepIntervalEdit->text().toInt();
    if (ui->velocityAbsoluteRadio->isChecked()) {
        m_currentParams.refDimensionalVelocity = ui->velocityValueEdit->text().toDouble();
        m_currentParams.refReNumber = ui->reynoldsNumberResult->text().toDouble();
    }

    if (ui->velocityViaReynoldsRadio->isChecked()) {
        m_currentParams.refDimensionalVelocity = ui->velocityValueResult->text().toDouble();
        m_currentParams.refReNumber = ui->reynoldsNumberEdit->text().toDouble();
    }

    m_currentParams.refMachNumber = m_currentParams.refDimensionalVelocity / 340.295;

    m_currentParams.refDimensionalTemperature = 288.15;    // 来流温度 (K)
    m_currentParams.refDimensionalDensity = 1.225;      // 来流密度 (kg/m^3)

    m_currentParams.forceReferenceLength = m_geometry.getBoundingBoxWidth();
    m_currentParams.forceReferenceArea = m_geometry.getBoundingBoxWidth();
    m_currentParams.forceReferenceLengthSpanWise = 1.0;
    if (ui->viscousModelComboBox->currentIndex() == 0) {
        m_currentParams.viscousModel = BridgeWind::viscousModel::Laminar;

    }
    else if (ui->viscousModelComboBox->currentIndex() == 1) {
        m_currentParams.viscousModel = BridgeWind::viscousModel::SSTKOmega;
    }
}
void MainWindow::renderVtkWindowByRenderOption() {
    executeSafely(this, [&]() {


        if (m_currentRenderOption == RenderOption::Mesh) {
            updateVtkWdindowWithMesh();
        }
        else {
            updateVtkWdindowWithContourByCurrentOption();
        }
        });
}
void MainWindow::updateVtkWdindowWithContourByCurrentOption() {
    int numPoints = m_flowDataReader.getNumPoints();
    int numCells = m_flowDataReader.getNumCells();

    vtkNew<vtkPoints> points;
    for (int i = 0; i < numPoints; ++i) {
        double x = m_flowDataReader.getPointX(i);
        double y = m_flowDataReader.getPointY(i);
        points->InsertNextPoint(x, y, 0.0);
    }

    const auto& connectivity_quads = m_flowDataReader.getCellConnectivity();

    vtkNew<vtkCellArray> quads;
    for (int i = 0; i < numCells; ++i) {
        vtkNew<vtkQuad> quad;

        for (int j = 0; j < 4; ++j) {
            quad->GetPointIds()->SetId(j, connectivity_quads[i][j]);
        }
        quads->InsertNextCell(quad);
    }

    vtkNew<vtkUnstructuredGrid> uGrid;
    uGrid->SetPoints(points);
    uGrid->SetCells(VTK_QUAD, quads);





    vtkNew<vtkDoubleArray> pressureArray;

    if (m_currentRenderOption == RenderOption::Density) {
        const std::vector<double>& dataValues = m_flowDataReader.getDensity();
        pressureArray->SetName("Density"); // 给数据命名非常重要
        pressureArray->SetNumberOfComponents(1); // 标量数据只有一个分量
        pressureArray->SetNumberOfTuples(numCells); // *** 关键：元组数量应与单元数量一致 ***

        // 3. 将您的数据拷贝到 VTK 数组中
        for (vtkIdType i = 0; i < numCells; ++i) {
            pressureArray->SetTuple1(i, dataValues[i]);
        }
    }
    else if(m_currentRenderOption == RenderOption::Pressure) {
        const std::vector<double>& dataValues = m_flowDataReader.getPressure();
        pressureArray->SetName("Pressure"); // 给数据命名非常重要
        pressureArray->SetNumberOfComponents(1); // 标量数据只有一个分量
        pressureArray->SetNumberOfTuples(numCells); // *** 关键：元组数量应与单元数量一致 ***

        // 3. 将您的数据拷贝到 VTK 数组中
        for (vtkIdType i = 0; i < numCells; ++i) {
            pressureArray->SetTuple1(i, dataValues[i]);
        }
    }
    else if (m_currentRenderOption == RenderOption::XVelocity) {
        const std::vector<double>& dataValues = m_flowDataReader.getVelocityX();
        pressureArray->SetName("X Velocity"); // 给数据命名非常重要
        pressureArray->SetNumberOfComponents(1); // 标量数据只有一个分量
        pressureArray->SetNumberOfTuples(numCells); // *** 关键：元组数量应与单元数量一致 ***

        // 3. 将您的数据拷贝到 VTK 数组中
        for (vtkIdType i = 0; i < numCells; ++i) {
            pressureArray->SetTuple1(i, dataValues[i]);
        }
    }
    else if (m_currentRenderOption == RenderOption::YVelocity) {
        const std::vector<double>& dataValues = m_flowDataReader.getVelocityY();
        pressureArray->SetName("Y Velocity"); // 给数据命名非常重要
        pressureArray->SetNumberOfComponents(1); // 标量数据只有一个分量
        pressureArray->SetNumberOfTuples(numCells); // *** 关键：元组数量应与单元数量一致 ***

        // 3. 将您的数据拷贝到 VTK 数组中
        for (vtkIdType i = 0; i < numCells; ++i) {
            pressureArray->SetTuple1(i, dataValues[i]);
        }
    }
    else if (m_currentRenderOption == RenderOption::VelocityMagnitude) {
        const std::vector<double>& dataValues = m_flowDataReader.getVelocityMagnitude();
        pressureArray->SetName("Velocity Magnitude"); // 给数据命名非常重要
        pressureArray->SetNumberOfComponents(1); // 标量数据只有一个分量
        pressureArray->SetNumberOfTuples(numCells); // *** 关键：元组数量应与单元数量一致 ***

        // 3. 将您的数据拷贝到 VTK 数组中
        for (vtkIdType i = 0; i < numCells; ++i) {
            pressureArray->SetTuple1(i, dataValues[i]);
        }
    }
    else {
        throw std::runtime_error("Unknown Render Option.");
    }



    // 2. 创建 VTK 数组来存储数据
    
   

    // 4. 将该数组附加到网格的点数据上
    uGrid->GetCellData()->SetScalars(pressureArray);

    // --- 调用新的渲染函数 ---
    renderContourPlot(uGrid.Get());

}

void MainWindow::renderContourPlot(vtkUnstructuredGrid* uGridWithData)
{
    // --- 1. 获取数据范围 ---
    double scalarRange[2];
    uGridWithData->GetScalarRange(scalarRange);

    // --- 2. 创建颜色映射表 (Lookup Table) ---
    vtkNew<vtkLookupTable> lut;
    lut->SetTableRange(scalarRange[0], scalarRange[1]);
    lut->SetHueRange(0.667, 0.0); // 从蓝色到红色
    lut->Build();

    // --- 3. 创建用于渲染的 Mapper 和 Actor ---
    vtkNew<vtkDataSetMapper> mainMapper;
    mainMapper->SetInputData(uGridWithData);

    // 将 Mapper 和颜色表关联
    mainMapper->SetLookupTable(lut);
    mainMapper->SetScalarRange(scalarRange[0], scalarRange[1]);

    // **********************************************************
    // *** 这是需要修改的核心行 ***
    // *** 从寻找点数据，改为寻找单元数据 ***
    mainMapper->SetScalarModeToUseCellData(); // <--- 修改这里！
    // **********************************************************

    mainMapper->ScalarVisibilityOn();

    vtkNew<vtkActor> mainActor;
    mainActor->SetMapper(mainMapper);

    // --- 4. 创建颜色标尺 (Scalar Bar) ---
    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetLookupTable(lut);
    scalarBar->SetTitle("Pressure");
    scalarBar->SetNumberOfLabels(5);
    // (可选) 让标签在深色背景下更清晰
    scalarBar->GetTitleTextProperty()->SetColor(0.0, 0.0, 0.0);
    scalarBar->GetLabelTextProperty()->SetColor(0.0, 0.0, 0.0);

    // --- 5. 更新渲染器 ---
    m_renderer->RemoveAllViewProps();      // 清空之前的所有内容
    m_renderer->AddActor(mainActor);       // 添加彩色的云图Actor
    m_renderer->AddActor2D(scalarBar);     // 添加2D的颜色标尺


    m_renderer->ResetCamera();
    ui->vtkRenderWidget->renderWindow()->Render();
}


void MainWindow::setupUiConnections()
{
    // 连接 sectionTypeComboBox
    connect(ui->sectionTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onSectionTypeChanged);

    // --- 新增代码 ---
    // 1. 将关闭按钮的 clicked() 信号连接到窗口的 close() 槽
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::close);

    // 2. 将最小化按钮的 clicked() 信号连接到窗口的 showMinimized() 槽
    connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::showMinimized);

    // 3. 将最大化按钮的 clicked() 信号连接到我们自定义的 onMaximizeRestore() 槽
    connect(ui->maximizeButton, &QPushButton::clicked, this, &MainWindow::onMaximizeRestore);

    connect(ui->sectionTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onSectionTypeChanged);
    connect(ui->fieldSizeMethodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFieldSizeMethodChanged);

    connect(ui->setGeometryButton, &QPushButton::clicked, this, &MainWindow::onSetGeometryButtonClicked);
    connect(ui->setParametersButton, &QPushButton::clicked, this, &MainWindow::onSetParametersButtonClicked);
    connect(ui->setMeshingButton, &QPushButton::clicked, this, &MainWindow::onSetMeshingButtonClicked);
    connect(ui->bottomSetMeshingButton, &QPushButton::clicked, this, &MainWindow::onSetMeshingButtonClicked);
    connect(ui->bottomSetSimulaionButton, &QPushButton::clicked, this, &MainWindow::onSetParametersButtonClicked);

    connect(ui->setBuiltInModeButton, &QPushButton::clicked, this, &MainWindow::onSetBuiltInModeButtonClicked);
    connect(ui->setImportDxfModeButton, &QPushButton::clicked, this, &MainWindow::onSetImportDxfModeButtonClicked);

    connect(ui->fileDropWidget, &FileDropWidget::filesDropped, this, &MainWindow::onDxfFileDropped);
    connect(ui->dxfBrowseButton, &QPushButton::clicked, this, &MainWindow::onDxfFileBrowseButtonClicked);


    connect(ui->generateMeshButton, &QPushButton::clicked, this, &MainWindow::onGenerateMeshButtonClicked);
    connect(ui->startOrStopSimulationButton, &QPushButton::clicked, this, &MainWindow::onStartSimulationButtonClicked);

    // 链接到槽saveAndCheckMeshParameters

    connect(ui->circumferentialMeshNumberEdit, &QLineEdit::editingFinished, this, &MainWindow::saveAndCheckMeshParameters);
    connect(ui->radialGrowthRateEdit, &QLineEdit::editingFinished, this, &MainWindow::saveAndCheckMeshParameters);
    connect(ui->radialMeshNumberEdit, &QLineEdit::editingFinished, this, &MainWindow::saveAndCheckMeshParameters);
    connect(ui->fieldSizeValueEdit, &QLineEdit::editingFinished, this, &MainWindow::saveAndCheckMeshParameters);
    connect(ui->fieldToSizeRatioEdit, &QLineEdit::editingFinished, this, &MainWindow::saveAndCheckMeshParameters);
    connect(ui->fieldSizeMethodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::saveAndCheckMeshParameters);

    connect(ui->velocityAbsoluteRadio, &QRadioButton::clicked, this, &MainWindow::onOneOfTheVelocitySettingsChanged);
    connect(ui->velocityViaReynoldsRadio, &QRadioButton::clicked, this, &MainWindow::onOneOfTheVelocitySettingsChanged);
    connect(ui->velocityValueEdit, &QLineEdit::textEdited, this, &MainWindow::onOneOfTheVelocitySettingsChanged);
    connect(ui->reynoldsNumberEdit, &QLineEdit::textEdited, this, &MainWindow::onOneOfTheVelocitySettingsChanged);
    connect(ui->timeStepEdit, &QLineEdit::textEdited, this, &MainWindow::onTimeStepChanged);

}

void MainWindow::setupToogleGroups() {
    QButtonGroup* toggleGroupGeoOrSim = new QButtonGroup(this);
    toggleGroupGeoOrSim->addButton(ui->setGeometryButton);
    toggleGroupGeoOrSim->addButton(ui->setMeshingButton);
    toggleGroupGeoOrSim->addButton(ui->setParametersButton);
    // 设置为互斥模式
    toggleGroupGeoOrSim->setExclusive(true);
    ui->setGeometryButton->setChecked(true);

    QButtonGroup* toggleGroupBuiltinOrDxf = new QButtonGroup(this);
    toggleGroupBuiltinOrDxf->addButton(ui->setBuiltInModeButton);
    toggleGroupBuiltinOrDxf->addButton(ui->setImportDxfModeButton);
    // 设置为互斥模式
    toggleGroupBuiltinOrDxf->setExclusive(true);
    ui->setBuiltInModeButton->setChecked(true);

    QButtonGroup* toggleGroupBuiltinOrDxfLine = new QButtonGroup(this);
    toggleGroupBuiltinOrDxfLine->addButton(ui->setImportDxfModeButtonLine);
    toggleGroupBuiltinOrDxfLine->addButton(ui->setBuiltInModeButtonLine);
    // 设置为互斥模式
    toggleGroupBuiltinOrDxfLine->setExclusive(true);
    ui->setBuiltInModeButtonLine->setChecked(true);

}
void MainWindow::onMaximizeRestore()
{
    if (isMaximized()) {
        showNormal(); // 如果当前是最大化状态，则恢复正常大小
    }
    else {
        showMaximized(); // 否则，最大化窗口
    }
}
MainWindow::~MainWindow()
{

}
void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_workerThread && m_workerThread->isRunning()) {
        appendLogMessage(tr("Window is closing, attempting to stop the running simulation..."));

        // 如果当前的后台任务是求解器服务，就调用它的 stop() 方法
        // 我们的 stop() 信号已经连接到了 ProcessManager::stop()
        if (dynamic_cast<BridgeWind::SolverService*>(m_currentWorker)) {
            // 使用元对象系统安全地调用另一个线程对象的槽函数
            QMetaObject::invokeMethod(m_currentWorker, "stop", Qt::QueuedConnection);
        }

        // 等待线程优雅地退出
        m_workerThread->quit(); // 请求线程的事件循环退出
        if (!m_workerThread->wait(5000)) { // 最多等待5秒钟
            appendLogMessage(tr("Worker thread did not stop gracefully, forcing termination."));
            m_workerThread->terminate(); // 如果超时仍未退出，则强制终止
            m_workerThread->wait();      // 等待强制终止完成
        }
        appendLogMessage(tr("Simulation stopped."));
    }

    // 接受关闭事件，允许窗口继续关闭
    QMainWindow::closeEvent(event);
}
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
#if defined(Q_OS_WIN)
    if (eventType == "windows_generic_MSG")
    {
        MSG* msg = static_cast<MSG*>(message);

        // 1. 拦截窗口尺寸计算消息，欺骗系统客户区占满了整个窗口
        if (msg->message == WM_NCCALCSIZE)
        {
            // 当 wParam 为 TRUE 时，lParam 指向一个 NCCALCSIZE_PARAMS 结构体。
            // 通过返回 0，我们告诉 Windows 不要在窗口的非客户区（标题栏和边框）上进行任何绘制。
            // 这会有效地移除标准的窗口框架，但保留 DWM 提供的阴影和动画效果。
            if (msg->wParam == TRUE)
            {
                *result = 0;
                return true; // 我们已经处理了这个消息，不要再传递了
            }
        }

        // 2. 拦截鼠标命中测试消息，告诉系统鼠标在哪个区域
        if (msg->message == WM_NCHITTEST)
        {
            *result = 0;
            const LONG borderWidth = 8; // 窗口边框宽度，用于缩放
            RECT winrect;
            GetWindowRect(msg->hwnd, &winrect);

            long x = LOWORD(msg->lParam);
            long y = HIWORD(msg->lParam);

            // --- 关键改动：让按钮区域响应为普通客户区 ---
            QPoint pos = mapFromGlobal(QPoint(x, y));
            if (ui->minimizeButton->geometry().contains(pos) ||
                ui->maximizeButton->geometry().contains(pos) ||
                ui->closeButton->geometry().contains(pos))
            {
                // 如果在按钮上，让基类处理，这样按钮才能收到点击事件
                return QMainWindow::nativeEvent(eventType, message, result);
            }

            // --- 拖动区域 ---
            if (ui->titleBar->geometry().contains(pos)) {
                *result = HTCAPTION;
                return true;
            }

            // --- 边框缩放区域 (可选，但强烈推荐) ---
            if (x >= winrect.left && x < winrect.left + borderWidth) {
                *result = HTLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth) {
                *result = HTRIGHT;
            }
            if (y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOP;
            }
            if (y < winrect.bottom && y >= winrect.bottom - borderWidth) {
                *result = HTBOTTOM;
            }
            if (x >= winrect.left && x < winrect.left + borderWidth && y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOPLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth && y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOPRIGHT;
            }
            if (x >= winrect.left && x < winrect.left + borderWidth && y < winrect.bottom && y >= winrect.bottom - borderWidth) {
                *result = HTBOTTOMLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth && y < winrect.bottom && y >= winrect.bottom - borderWidth) {
                *result = HTBOTTOMRIGHT;
            }

            if (*result != 0) {
                return true;
            }
        }
    }
#endif
    // 对于我们不处理的所有其他消息，调用基类实现
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::onSectionTypeChanged(int index)
{

    ui->parameterStack->setCurrentIndex(index);
    qDebug() << "setCurrentIndex: " << index;
    onGeoParameterEditingFinished();
}
void MainWindow::onFieldSizeMethodChanged(int index) {
    ui->fieldSizeMethonStack->setCurrentIndex(index);
}
void MainWindow::onSetGeometryButtonClicked() {
    ui->stepStack->setCurrentIndex(0);
}
void MainWindow::onSetMeshingButtonClicked() {
    ui->setMeshingButton->setChecked(true);
    ui->stepStack->setCurrentIndex(1);
}
void MainWindow::onSetParametersButtonClicked() {
    ui->setParametersButton->setChecked(true);
    this->onOneOfTheVelocitySettingsChanged();
    this->onTimeStepChanged();
    ui->stepStack->setCurrentIndex(2);
}
void MainWindow::onSetBuiltInModeButtonClicked() {
    ui->geometryModeStack->setCurrentIndex(0);
    ui->setBuiltInModeButtonLine->setChecked(true);
    m_currentParams.geometryDefineMethod = BridgeWind::geometryDefineMethod::BuiltIn;
}
void MainWindow::onSetImportDxfModeButtonClicked() {
    ui->geometryModeStack->setCurrentIndex(1);
    ui->setImportDxfModeButtonLine->setChecked(true);
    m_currentParams.geometryDefineMethod = BridgeWind::geometryDefineMethod::FromDXF;
}


void MainWindow::onGeoParameterEditingFinished() {
    try {


        qDebug() << "Parameter editing finished. Updating values...";

        // 1. 获取当前选中的截面索引
        int currentIndex = ui->sectionTypeComboBox->currentIndex();
        if (currentIndex < 0) {
            qDebug() << "No section selected.";
            return; // 如果没有有效的选项，则直接返回
        }

        // 2. 从模型中获取当前截面的定义
        const SectionDef& currentDef = m_sectionDefs[currentIndex];
        qDebug() << "Current Section:" << currentDef.nameComment;

        // 3. 从 QStackedWidget 中获取当前的页面 (它是一个 QScrollArea)
        QScrollArea* scrollArea = qobject_cast<QScrollArea*>(ui->parameterStack->widget(currentIndex));
        if (!scrollArea) {
            qDebug() << "Error: Could not find QScrollArea at index" << currentIndex;
            return;
        }

        // 4. 从 QScrollArea 中获取其内容 QWidget
        QWidget* contentWidget = scrollArea->widget();
        if (!contentWidget) {
            qDebug() << "Error: ScrollArea has no content widget";
            return;
        }

        // 5. 在内容 QWidget 中查找所有的 QLineEdit 子控件
        // findChildren 会返回一个列表，其顺序与添加时的顺序一致
        QList<QLineEdit*> lineEdits = contentWidget->findChildren<QLineEdit*>();

        // 安全检查：确保找到的 LineEdit 数量与模型中的参数数量一致
        if (lineEdits.count() != currentDef.parameters.count()) {
            qDebug() << "Error: Mismatch between found LineEdits and parameter definitions!";
            return;
        }

        // 6. 遍历 QLineEdit 列表，提取数据并与参数键关联
        // 使用 QMap 来存储 "参数键 -> 值" 的映射
        QMap<QString, double> currentParameters;
        for (int i = 0; i < lineEdits.count(); ++i) {
            const ParameterDef& paramDef = currentDef.parameters[i]; // 获取对应的参数定义
            QLineEdit* lineEdit = lineEdits[i];                      // 获取对应的输入框

            QString key = QString::fromUtf8(paramDef.key); // 从模型获取key
            bool ok;
            double value = lineEdit->text().toDouble(&ok); // 从UI获取text并转为double

            if (ok) {
                currentParameters.insert(key, value);
            }
        }

        // 7. (演示) 打印所有获取到的参数和值
        qDebug() << "--- Collected Parameters ---";
        for (auto it = currentParameters.constBegin(); it != currentParameters.constEnd(); ++it) {
            qDebug() << it.key() << ":" << it.value();
        }
        qDebug() << "--------------------------";





        m_geometry.clear();
        if (currentDef.id == SectionTypes::Undefined) {
            return;
        }
        else if (currentDef.id == SectionTypes::Circle) {

            m_geometry.resetAsCircle(currentParameters["param_diameters"]);

        }
        else if (currentDef.id == SectionTypes::Rectangle) {
            m_geometry.resetAsRectangle(
                currentParameters["param_width"],
                currentParameters["param_height"]
            );
        }
        else if (currentDef.id == SectionTypes::ChamferedRectangle) {
            m_geometry.resetAsChamferedRectangle(
                currentParameters["param_width"],
                currentParameters["param_height"],
                currentParameters["param_chamfered_radius"]
            );
        }
        else if (currentDef.id == SectionTypes::StreamlinedBoxGirder) {
            m_geometry.resetAsStreamlinedBoxGirder(
                currentParameters["param_total_width"],
                currentParameters["param_total_height"],
                currentParameters["param_bottom_width"],
                currentParameters["param_slope"],
                currentParameters["param_angle_1"],
                currentParameters["param_angle_2"]
            );
        }
        renderVtkWindowWithGeometry(m_geometry);
    }
    catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        // 其他标准异常
        QMessageBox::critical(this, tr("Error"), QString("Exception occurred: %1").arg(e.what()));
    }
    catch (...) {
        // 未知异常
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error"));
    }

}


void MainWindow::onDxfFileDropped(const QStringList& filePaths) {
    try {

        if (filePaths.size() == 1) {
            QString filePath = filePaths[0];
            m_geometry.clear();
            m_geometry.loadFromDXF(filePath.toStdString());
            renderVtkWindowWithGeometry(m_geometry);
        }
        else if (filePaths.size() > 1){
            throw std::runtime_error("Please don't drop more than one file.");
        }
        else if (filePaths.size() == 0) {
            throw std::runtime_error("No file was dropped.");
        }

    }
    catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        // 其他标准异常
        QMessageBox::critical(this, tr("Error"), QString("Exception occurred: %1").arg(e.what()));
    }
    catch (...) {
        // 未知异常
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error"));
    }
}
void MainWindow::onDxfFileBrowseButtonClicked() {



        
    try {

        QString dxfFilePath = QFileDialog::getOpenFileName(
            this,
            tr("Choose a DXF file"),
            m_projectPath,
            tr("DXF Files (*.dxf);;All Files (*)")
        );
        m_geometry.clear();
        m_geometry.loadFromDXF(dxfFilePath.toStdString());
        renderVtkWindowWithGeometry(m_geometry);

    }
    catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        // 其他标准异常
        QMessageBox::critical(this, tr("Error"), QString("Exception occurred: %1").arg(e.what()));
    }
    catch (...) {
        // 未知异常
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error"));
    }

}

void MainWindow::onGenerateMeshButtonClicked() {
    saveAndCheckMeshParameters();

    executeSafely(this, [&]() {
        if (m_geometry.isEmpty()) {
            throw std::runtime_error("Geoetry is Empty.");
        }
        auto ptrGeo = std::make_shared<BridgeWind::Geometry>(m_geometry);
        auto analyzer = std::make_shared<BridgeWind::TopologyAnalyzer>(ptrGeo);

        try {
            analyzer->analyze();
        }
        catch (const std::exception& e) {

            throw std::runtime_error(std::string( "Exception occurred when analyzing the geometry: ") + e.what());
        }
        catch (...) {

            throw std::runtime_error(std::string("Unknown error occurred when analyzing the geometry: "));
        }
        m_analyzerResult = analyzer;

        appendLogMessage(tr("Starting mesh generation..."));


        m_workerThread = new QThread();
        auto worker = new BridgeWind::MeshingService();
        m_currentWorker = worker;
        worker->moveToThread(m_workerThread);

        // --- 3. 连接信号与槽 ---
        connect(worker, &BridgeWind::MeshingService::progressUpdated, this, &MainWindow::appendLogMessageAndUpDateProgressBar);
        connect(worker, &BridgeWind::MeshingService::finished, this, &MainWindow::onMeshingFinished);
        connect(worker, &BridgeWind::MeshingService::errorOccurred, this, &MainWindow::handleError);
        connect(m_workerThread, &QThread::finished, this, [this]() {
            cleanupWorker(m_workerThread, m_currentWorker);
            });

        // --- 4. 启动任务 ---


        QMetaObject::invokeMethod(worker, "run", Qt::QueuedConnection,
            Q_ARG(std::shared_ptr<BridgeWind::TopologyAnalyzer>, m_analyzerResult),
            Q_ARG(BridgeWind::SimulationParameters, m_currentParams));

        m_workerThread->start();





        });

    
}
void MainWindow::onStartSimulationButtonClicked() {
    executeSafely(this, [&]() {
        collectSimulationParameters();
        // 启动Timer
        m_checkIterTimer = new QTimer(this);
        connect(m_checkIterTimer, &QTimer::timeout, this, &MainWindow::onCheckIterTimerTimeOut);
        m_checkIterTimer->setInterval(1000);
        m_checkIterTimer->start();
    
        m_currentParams.print();
        appendLogMessage(tr("Preparing solver thread..."));

        // --- 2. 创建并配置线程 ---
        m_workerThread = new QThread();

        // --- 3. 连接信号 ---
        // a. 将线程的 started 信号连接到我们新的任务启动槽
        connect(m_workerThread, &QThread::started, this, &MainWindow::startSolverTask);

        // b. 连接线程的 finished 信号用于清理
        connect(m_workerThread, &QThread::finished, this, [this]() {
            // 在清理前, 断开停止按钮的连接
            if (m_currentWorker) {
                disconnect(ui->startOrStopSimulationButton, &QPushButton::clicked, m_currentWorker, nullptr);
            }
            cleanupWorker(m_workerThread, m_currentWorker);
            });

        // --- 4. 启动线程 ---
        // 启动线程后, Qt会自动在后台线程的事件循环中发射 started() 信号, 
        // 从而安全地调用我们的 startSolverTask()。
        m_workerThread->start();
        });
}

void MainWindow::saveAndCheckMeshParameters() {
    

    executeSafely(this, [&]() {

        m_currentParams.circumferentialMeshNumber = ui->circumferentialMeshNumberEdit->text().toInt();
        m_currentParams.radialMeshNumber = ui->radialMeshNumberEdit->text().toInt();
        m_currentParams.radialMeshGrowthRate = ui->radialGrowthRateEdit->text().toDouble();
        m_currentParams.filedToSizeRatio = ui->fieldToSizeRatioEdit->text().toDouble();
        m_currentParams.userDefinedFieldDiameter = ui->fieldSizeValueEdit->text().toDouble();

        if (m_currentParams.circumferentialMeshNumber <= 100) {
            throw std::invalid_argument("Circumferential mesh number must be greater than 100");

        }
        if (m_currentParams.radialMeshNumber < 150) {
            throw std::invalid_argument("Radial mesh number couldn't be less tan 150");
        }
        if (m_currentParams.radialMeshGrowthRate < 0.1 || m_currentParams.radialMeshGrowthRate >1) {
            throw std::invalid_argument("The growth rate should be greater than 0.1 and less than 1.");
        }
        if (m_currentParams.filedToSizeRatio > 10000) {
            throw std::invalid_argument("N is too big.");
        }
        if (m_currentParams.filedToSizeRatio < 3) {
            throw std::invalid_argument("N is too small.");
        }


        m_currentParams.fieldSizeDefineMethod = [this]() {
            int index = this->ui->fieldSizeMethonStack->currentIndex();
            if (index == 0) {
                return BridgeWind::fieldSizeDefineMethod::DiameterToMaxSizeRatio;
            }
            else if (index == 1) {
                return BridgeWind::fieldSizeDefineMethod::UserDefined;
            }
            return BridgeWind::fieldSizeDefineMethod::Default;
            }();


        });

    
}


void MainWindow::generateMesh() {

}

void MainWindow::appendLogMessage(const QString& message)
{
    if (message.back() == "\n"){
        QString message_copy = message;
        message_copy.chop(1);
        if (message_copy.back() == "\r") {
            message_copy.chop(1);
        }

        // 1. 获取当前时间
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        // 2. 构造带时间戳的完整日志条目
        QString logEntry = QString("[%1] %2").arg(currentTime, message_copy);

        // 3. 将日志条目追加到 QTextEdit 控件上
        //    ui->logViewer 是你在 .ui 文件中为 QTextEdit 设置的 objectName
        ui->logViewer->append(logEntry);

        // 4. (可选但推荐) 自动滚动到底部
        //    这确保用户总能看到最新的日志, 而不需要手动滚动。
        ui->logViewer->ensureCursorVisible();
    }
    else {
        // 1. 获取当前时间
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        // 2. 构造带时间戳的完整日志条目
        QString logEntry = QString("[%1] %2").arg(currentTime, message);

        // 3. 将日志条目追加到 QTextEdit 控件上
        //    ui->logViewer 是你在 .ui 文件中为 QTextEdit 设置的 objectName
        ui->logViewer->append(logEntry);

        // 4. (可选但推荐) 自动滚动到底部
        //    这确保用户总能看到最新的日志, 而不需要手动滚动。
        ui->logViewer->ensureCursorVisible();
    }
}
void MainWindow::updateProgressBar(int percentage) {
    ui->progressBar->show();
    ui->progressBar->setTextVisible(false);
    ui->progressBar->setValue(percentage);
    
    if (percentage == 100) {
        ui->progressBar->hide();
    }

}
void MainWindow::appendLogMessageAndUpDateProgressBar(const QString& message, int percentage) {
    appendLogMessage(message);
    updateProgressBar(percentage);
    
}
void MainWindow::onMeshingFinished()
{
    appendLogMessage(tr("Mesh generation finished successfully."));


    m_workerThread->quit();
    ui->rightMeshButton->setChecked(true);
    updateVtkWdindowWithMesh();
    ui->progressBar->hide();
}

void MainWindow::handleError(const QString& errorMessage)
{
    appendLogMessage(tr("[CRITICAL ERROR] ") + errorMessage);


    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
    }
    ui->progressBar->hide();

}
void MainWindow::onSolverFinished()
{
    appendLogMessage(tr("Solver finished."));

    m_workerThread->quit();
}



void MainWindow::onOneOfTheVelocitySettingsChanged() {
    if (ui->velocityAbsoluteRadio->isChecked()) {
        
        setReyByVel();
        ui->velocityValueEdit->show();
        ui->velocityValueResult->hide();
        ui->reynoldsNumberEdit->hide();
        ui->reynoldsNumberResult->show();

    }

    if (ui->velocityViaReynoldsRadio->isChecked()) {
        
        setVelByRey();
        ui->velocityValueEdit->hide();
        ui->velocityValueResult->show();
        ui->reynoldsNumberEdit->show();
        ui->reynoldsNumberResult->hide();
    }
    onTimeStepChanged();
}
void MainWindow::onCheckIterTimerTimeOut() {

    //std::cout << "onCheckIterTimerTimeOut called" << std::endl;
    int n = 3;
    if (!ui->logViewer || n <= 0) {
        return ;
    }

    QTextDocument* doc = ui->logViewer->document();
    QStringList lastLines;

    // 1. 直接获取最后一个文本块
    QTextBlock block = doc->lastBlock();

    // 2. 反向循环 N 次，或者直到 block 失效（到达文档开头）
    for (int i = 0; i < n && block.isValid(); ++i) {
        // block.text() 获取当前行的纯文本
        lastLines.prepend(block.text()); // 使用 prepend 将新行插入到列表开头，以保持正确顺序

        // 移动到上一个文本块
        block = block.previous();
    }
    int maxIter = -1;
    for (const auto& string : lastLines) {
        auto result = extractFirstData(string);
        if (result.has_value()) {

            // 如果需要转成整数
            bool ok;
            int value = result.value().toInt(&ok);
            if (ok) {
                if (value > maxIter) {
                    maxIter = value;
                }
                
            }
        }

    }
    if (maxIter > m_currentIter) {
        m_currentIter = maxIter;

        emit iterUpdated(m_currentIter);
    }
    
}
void MainWindow::onIterUpdated(int iter) {

    std::cout << "iter updated: " << iter << std::endl;
    
    if (iter > 1 && 
        (iter > lastFlowLoadedIter + m_currentParams.intervalStepFlow)
        ) 
    {
        std::cout << "reload flow data" << std::endl;
        executeSafely(this, [&]() {
            m_flowDataReader.load();

            });
        std::cout << "flow data loaded" << std::endl;
        lastFlowLoadedIter = iter - 1;

        renderVtkWindowByRenderOption();
    }
}
void MainWindow::onRenderOptionChanged() {
    if      (ui->rightMeshButton->isChecked()) {
        m_currentRenderOption = RenderOption::Mesh;
    }
    else if (ui->rightPressureButton->isChecked()) {
        m_currentRenderOption = RenderOption::Pressure;
    }
    else if (ui->rightVelocityMagnitudeButton->isChecked()) {
        m_currentRenderOption = RenderOption::VelocityMagnitude;
    }
    else if (ui->rightXVelocityButton->isChecked()) {
        m_currentRenderOption = RenderOption::XVelocity;
    }
    else if (ui->rightYVelocityButton->isChecked()) {
        m_currentRenderOption = RenderOption::YVelocity;
    }
    else if (ui->rightDensityButton->isChecked()) {
        m_currentRenderOption = RenderOption::Density;
    }

    if (m_currentIter > m_currentParams.intervalStepFlow) {

        executeSafely(this, [&]() {
            renderVtkWindowByRenderOption();
            });
    }
}


void MainWindow::onTimeStepChanged() {
    double realTimeStep = ui->timeStepEdit->text().toDouble() * m_currentParams.forceReferenceLength / m_currentParams.refDimensionalVelocity;
    ui->realTimeStepEdit->setText(QString::number(realTimeStep));
}