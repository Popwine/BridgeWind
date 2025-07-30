#include "mainwindow.h"

#include "bridge_simulation_service.h" // 包含头文件
#include "ui_mainwindow.h"
#include "flow_data_reader.h"

#include <filesystem>
#include <algorithm> 

#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>
#include <QDebug>




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



MainWindow::MainWindow(const QString& projectPath, QWidget* parent) 
:
	m_projectPath(projectPath), QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this); // 使用Qt Designer生成的UI设置函数
	ui->generateMeshButton->setEnabled(false);
	ui->startSimulationButton->setEnabled(false);
	ui->stopSimulationButton->setEnabled(false);
	this->setWindowTitle("BridgeWind - " + m_projectPath);

	qRegisterMetaType<std::shared_ptr<BridgeWind::TopologyAnalyzer>>("std::shared_ptr<BridgeWind::TopologyAnalyzer>");
	qRegisterMetaType<BridgeWind::SimulationParameters>("BridgeWind::SimulationParameters");

	setupUiConnections();
	setupVtkRenderWindow();
}
void MainWindow::setupUiConnections()
{
	
	connect(
		ui->projectBrowseButton,
		&QPushButton::clicked,
		this,
		[=]() {
			QString projectPath = QFileDialog::getExistingDirectory(
				this,
				tr("Choose a project working directory"),
				ui->workingDir->text()
			);
			if (!projectPath.isEmpty()) {
				ui->workingDir->setText(projectPath);
			}
		}
	);
	connect(
		ui->dxfBrowseButton,
		&QPushButton::clicked,
		this,
		[=]() {
			QString dxfFilePath = QFileDialog::getOpenFileName(
				this,
				tr("Choose a DXF file"),
				ui->dxfDir->text(),
				tr("DXF Files (*.dxf);;All Files (*)")
			);

			if (!dxfFilePath.isEmpty()) {
				ui->dxfDir->setText(dxfFilePath);
			}
		}
	);

	connect(ui->geometryAnalyzeButton, &QPushButton::clicked, this, &MainWindow::onAnalyzeButtonClicked);
	connect(ui->generateMeshButton, &QPushButton::clicked, this, &MainWindow::onGenerateMeshButtonClicked);
	connect(ui->startSimulationButton, &QPushButton::clicked, this, &MainWindow::onStartCalculationButtonClicked);
	connect(ui->stopSimulationButton, &QPushButton::clicked, this, &MainWindow::onStopCalculationButtonClicked);
	connect(ui->displayMeshButton, &QPushButton::clicked, this, &MainWindow::onDisplayMeshButtonClicked);
	connect(ui->displayContourButton, &QPushButton::clicked, this, &MainWindow::onDisplayContourButtonClicked);
}

MainWindow::~MainWindow()
{
	// 确保退出时, 后台线程能被干净地关闭
	if (m_workerThread && m_workerThread->isRunning()) {
		m_workerThread->quit();
		m_workerThread->wait();
	}
	delete ui;
}






void MainWindow::onAnalyzeButtonClicked()
{
	m_currentParams.dxfFilePath = ui->dxfDir->text().toStdString();
	m_currentParams.workingDirectory = ui->workingDir->text().toStdString();
	// --- 1. UI状态管理 ---
	ui->geometryAnalyzeButton->setEnabled(false);
	ui->generateMeshButton->setEnabled(false);
	ui->startSimulationButton->setEnabled(false);
	appendLogMessage(tr("Starting geometry analysis..."));

	// --- 2. 创建后台任务 ---
	m_workerThread = new QThread();
	auto worker = new BridgeWind::GeometryAnalysisService();
	m_currentWorker = worker; // 记录当前的工作者
	worker->moveToThread(m_workerThread);

	// --- 3. 连接信号与槽 ---
	// a. 任务完成后, 调用 onAnalysisFinished
	connect(worker, &BridgeWind::GeometryAnalysisService::finished, this, &MainWindow::onAnalysisFinished);
	// b. 任务出错时, 调用 handleError
	connect(worker, &BridgeWind::GeometryAnalysisService::errorOccurred, this, &MainWindow::handleError);
	// c. 线程结束后, 自动清理
	connect(m_workerThread, &QThread::finished, this, [this]() {
		cleanupWorker(m_workerThread, m_currentWorker);
		});

	// --- 4. 启动任务 ---
	// 使用元调用来异步触发run, 并传递参数
	QString qdxfDir = QString::fromStdString(m_currentParams.dxfFilePath);
	QMetaObject::invokeMethod(worker, "run", Qt::QueuedConnection,
		Q_ARG(QString, qdxfDir));

	m_workerThread->start();
}

void MainWindow::onGenerateMeshButtonClicked()
{
	m_currentParams.dxfFilePath = ui->dxfDir->text().toStdString();
	m_currentParams.workingDirectory = ui->workingDir->text().toStdString();
	
	// 测试参数
	m_currentParams.filedSizeDefineMethod = BridgeWind::filedSizeDefineMethod::DiameterToMaxSizeRatio;
	m_currentParams.filedToSizeRatio = 10; // 计算域宽度与几何宽度的比率
	m_currentParams.radialMeshNumber = 150; // 径向网格数
	// --- 1. UI状态管理 ---
	ui->geometryAnalyzeButton->setEnabled(false);
	ui->generateMeshButton->setEnabled(false);
	appendLogMessage(tr("Starting mesh generation..."));

	// --- 2. 创建后台任务 ---
	m_workerThread = new QThread();
	auto worker = new BridgeWind::MeshingService();
	m_currentWorker = worker;
	worker->moveToThread(m_workerThread);

	// --- 3. 连接信号与槽 ---
	connect(worker, &BridgeWind::MeshingService::progressUpdated, this, &MainWindow::appendLogMessage);
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
}

void MainWindow::onStartCalculationButtonClicked()
{
	// --- 1. 准备参数和UI ---
	// 从UI获取参数, 并存储到成员变量中, 以便 startSolverTask 能访问
	m_currentParams.dxfFilePath = ui->dxfDir->text().toStdString();
	m_currentParams.workingDirectory = ui->workingDir->text().toStdString();
	// ... 其他参数 ...

	// UI状态管理
	ui->geometryAnalyzeButton->setEnabled(false);
	ui->generateMeshButton->setEnabled(false);
	ui->startSimulationButton->setEnabled(false);
	ui->stopSimulationButton->setEnabled(true);

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
			disconnect(ui->stopSimulationButton, &QPushButton::clicked, m_currentWorker, nullptr);
		}
		cleanupWorker(m_workerThread, m_currentWorker);
		});

	// --- 4. 启动线程 ---
	// 启动线程后, Qt会自动在后台线程的事件循环中发射 started() 信号, 
	// 从而安全地调用我们的 startSolverTask()。
	m_workerThread->start();
}

void MainWindow::onStopCalculationButtonClicked()
{
	// 1. 立即提供UI反馈
	appendLogMessage(tr("Sending stop signal to the solver..."));
	ui->stopSimulationButton->setEnabled(false); // 防止重复点击

	// 2. 检查后台工作者是否存在且正在运行
	// m_currentWorker 应该指向正在运行的 SolverService
	if (m_currentWorker && m_workerThread && m_workerThread->isRunning()) {

		// 3. 安全地跨线程调用后台服务的 stop() 槽函数
		// QMetaObject::invokeMethod 是Qt提供的标准、安全的跨线程调用方法。
		// Qt::QueuedConnection 确保这个调用被放入后台线程的事件队列中执行, 
		// 而不是立即在UI线程中尝试执行。
		QMetaObject::invokeMethod(m_currentWorker, "stop", Qt::QueuedConnection);
	}
	else {
		appendLogMessage(tr("[Warning] Solver is not currently running."));
	}
}

void MainWindow::onDisplayMeshButtonClicked() {
	updateVtkRenderWindow();
}

void MainWindow::onDisplayContourButtonClicked() {
	updateVtkRenderCounterWindow();
}
void MainWindow::onAnalysisFinished(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer)
{
	appendLogMessage(tr("Geometry analysis finished successfully."));
	m_analyzerResult = analyzer; // 保存分析结果
	ui->geometryAnalyzeButton->setEnabled(true);
	ui->generateMeshButton->setEnabled(true); // **启用下一步的按钮**
	m_workerThread->quit(); // 结束线程
}

void MainWindow::onMeshingFinished()
{
	appendLogMessage(tr("Mesh generation finished successfully."));
	
	ui->geometryAnalyzeButton->setEnabled(true);
	ui->generateMeshButton->setEnabled(true);

	ui->startSimulationButton->setEnabled(true); // **启用下一步的按钮**
	m_workerThread->quit();
}

void MainWindow::onSolverFinished()
{
	appendLogMessage(tr("Solver finished."));
	ui->geometryAnalyzeButton->setEnabled(true);
	ui->generateMeshButton->setEnabled(true);
	ui->startSimulationButton->setEnabled(true);
	ui->stopSimulationButton->setEnabled(false); // 禁用停止按钮
	m_workerThread->quit();
}

void MainWindow::appendLogMessage(const QString& message)
{
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

void MainWindow::handleError(const QString& errorMessage)
{
	appendLogMessage(tr("[CRITICAL ERROR] ") + errorMessage);
	// 恢复所有按钮的状态
	ui->geometryAnalyzeButton->setEnabled(true);
	ui->generateMeshButton->setEnabled(m_analyzerResult != nullptr);
	ui->startSimulationButton->setEnabled(false);
	ui->stopSimulationButton->setEnabled(false);
	if (m_workerThread && m_workerThread->isRunning()) {
		m_workerThread->quit();
	}
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
	connect(ui->stopSimulationButton, &QPushButton::clicked, worker, &BridgeWind::SolverService::stop, Qt::QueuedConnection);

	// --- 3. 启动任务 ---
	// 直接调用 run, 因为我们已经在同一个线程里了。
	// 我们需要把参数传递进来。可以通过成员变量或者Lambda捕获来做。
	// 这里用成员变量 m_currentParams (假设你在.h里定义了它)
	worker->run(m_currentParams); // m_currentParams 是在 onStartButtonClicked 里设置的
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


void MainWindow::setupVtkRenderWindow() {
	// --- 准备工作 ---
	m_currentParams.workingDirectory = ui->workingDir->text().toStdString();


	vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();

	// --- 3. 创建 Mapper 和 Actor ---

	vtkNew<vtkActor> actor;

	actor->GetProperty()->SetRepresentationToWireframe(); // 线框模式
	actor->GetProperty()->SetColor(0.2, 0.8, 0.2);

	// --- 4. 将 Actor 添加到 Renderer ---
	// 使用我们之前在 .h 文件中声明的成员变量 m_renderer
	m_renderer->AddActor(actor);
	m_renderer->SetBackground(0.1, 0.2, 0.3); // 设置背景色

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


void MainWindow::updateVtkRenderWindow() {
	// --- 准备工作 ---
	m_currentParams.workingDirectory = ui->workingDir->text().toStdString();



	std::filesystem::path workDir(m_currentParams.workingDirectory);
	std::filesystem::path fullPath = workDir / "grid" / "Bridge_Wind.msh";
	vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();


	G2C::Mesh mesh;
	G2C::GmeshReader reader;

	reader.load(fullPath.string(), mesh);

	
	
	int numPoints = mesh.getNumNodes();
	int numCells = mesh.getNumQuadElements();

	vtkNew<vtkPoints> points;
	for (int i = 0; i < numPoints; ++i) {
		points->InsertNextPoint(mesh.getNodeByIndex(i).x, mesh.getNodeByIndex(i).y, 0.0);
	}
	auto connectivity_quads = mesh.getQuadElementsIndexConnectivity();

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

	// --- 3. 创建 Mapper 和 Actor ---
	vtkNew<vtkDataSetMapper> mapper;
	mapper->SetInputData(uGrid);

	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToWireframe(); // 线框模式
	actor->GetProperty()->SetColor(0.2, 0.8, 0.2);

	// --- 4. 将 Actor 添加到 Renderer ---
	// 使用我们之前在 .h 文件中声明的成员变量 m_renderer
	m_renderer->RemoveAllViewProps();// 删除所有actor
	m_renderer->AddActor(actor);
	m_renderer->SetBackground(0.1, 0.2, 0.3); // 设置背景色

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


std::vector<double> MainWindow::getPressureDataForNodes(int numPoints) const {
	BridgeWind::FtsGridDataReader reader(m_currentParams.workingDirectory);
	reader.load();


	std::vector<double> pressureData;
	pressureData.reserve(numPoints);
	for (int i = 0; i < numPoints; ++i) {
		// 创建一个模拟的压力分布，例如一个简单的梯度
		pressureData.push_back(static_cast<double>(i));
	}
	return pressureData;
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

	// 为了看得更清楚，可以将背景设为稍亮的颜色
	m_renderer->SetBackground(0.8, 0.8, 0.8);

	m_renderer->ResetCamera();
	ui->vtkRenderWidget->renderWindow()->Render();
}

std::vector<double> getPressureDataForCells(int numCells) {
	// --- 模拟数据 ---
	// 这里我们为每个单元生成一个模拟的压力值
	// 在您的实际应用中，您需要从文件或计算结果中读取真实数据
	std::vector<double> cell_data(numCells);
	for (int i = 0; i < numCells; ++i) {
		// 例如，一个简单的基于索引的模拟值
		cell_data[i] = static_cast<double>(i) ;
	}
	return cell_data;
}
void MainWindow::updateVtkRenderCounterWindow() {
	BridgeWind::FlowDataReader flowReader(m_currentParams.workingDirectory);
	flowReader.load(); // 加载流场数据

	int numPoints = flowReader.getNumPoints();
	int numCells = flowReader.getNumCells();

	vtkNew<vtkPoints> points;
	for (int i = 0; i < numPoints; ++i) {
		double x = flowReader.getPointX(i);
		double y = flowReader.getPointY(i);
		points->InsertNextPoint(x, y, 0.0);
	}

	const auto& connectivity_quads = flowReader.getCellConnectivity();

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




	const std::vector<double>& pressureValues = flowReader.getDensity();






	// 2. 创建 VTK 数组来存储数据
	vtkNew<vtkDoubleArray> pressureArray;
	pressureArray->SetName("Pressure"); // 给数据命名非常重要
	pressureArray->SetNumberOfComponents(1); // 标量数据只有一个分量
	pressureArray->SetNumberOfTuples(numCells); // *** 关键：元组数量应与单元数量一致 ***

	// 3. 将您的数据拷贝到 VTK 数组中
	for (vtkIdType i = 0; i < numCells; ++i) {
		pressureArray->SetTuple1(i, pressureValues[i]);
	}

	// 4. 将该数组附加到网格的点数据上
	uGrid->GetCellData()->SetScalars(pressureArray);

	// --- 调用新的渲染函数 ---
	renderContourPlot(uGrid.Get());

}