#include "mainwindow.h"

#include "bridge_simulation_service.h" // 包含头文件
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this); // 使用Qt Designer生成的UI设置函数
	ui->generateMeshButton->setEnabled(false);
	ui->startSimulationButton->setEnabled(false);
	ui->stopSimulationButton->setEnabled(false);
		

	qRegisterMetaType<std::shared_ptr<BridgeWind::TopologyAnalyzer>>("std::shared_ptr<BridgeWind::TopologyAnalyzer>");
	qRegisterMetaType<BridgeWind::SimulationParameters>("BridgeWind::SimulationParameters");
	setupUiConnections();
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
}

MainWindow::~MainWindow()
{
	// 确保退出时，后台线程能被干净地关闭
	if (m_workerThread && m_workerThread->isRunning()) {
		m_workerThread->quit();
		m_workerThread->wait();
	}
	delete ui;
}






void MainWindow::onAnalyzeButtonClicked()
{
	params.dxfFilePath = ui->dxfDir->text().toStdString();
	params.workingDirectory = ui->workingDir->text().toStdString();
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
	// a. 任务完成后，调用 onAnalysisFinished
	connect(worker, &BridgeWind::GeometryAnalysisService::finished, this, &MainWindow::onAnalysisFinished);
	// b. 任务出错时，调用 handleError
	connect(worker, &BridgeWind::GeometryAnalysisService::errorOccurred, this, &MainWindow::handleError);
	// c. 线程结束后，自动清理
	connect(m_workerThread, &QThread::finished, this, [this]() {
		cleanupWorker(m_workerThread, m_currentWorker);
		});

	// --- 4. 启动任务 ---
	// 使用元调用来异步触发run，并传递参数
	QString qdxfDir = QString::fromStdString(params.dxfFilePath);
	QMetaObject::invokeMethod(worker, "run", Qt::QueuedConnection,
		Q_ARG(QString, qdxfDir));

	m_workerThread->start();
}

void MainWindow::onGenerateMeshButtonClicked()
{
	params.dxfFilePath = ui->dxfDir->text().toStdString();
	params.workingDirectory = ui->workingDir->text().toStdString();
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
		Q_ARG(BridgeWind::SimulationParameters, params));

	m_workerThread->start();
}

void MainWindow::onStartCalculationButtonClicked()

{
	params.dxfFilePath = ui->dxfDir->text().toStdString();
	params.workingDirectory = ui->workingDir->text().toStdString();
	// --- 1. UI状态管理 ---
	ui->geometryAnalyzeButton->setEnabled(false);
	ui->generateMeshButton->setEnabled(false);
	ui->startSimulationButton->setEnabled(false);
	ui->stopSimulationButton->setEnabled(true); // 启用停止按钮
	appendLogMessage(tr("Starting CFD solver..."));

	// --- 2. 创建后台任务 ---
	m_workerThread = new QThread();
	auto worker = new BridgeWind::SolverService();
	m_currentWorker = worker;
	worker->moveToThread(m_workerThread);

	// --- 3. 连接信号与槽 ---
	connect(worker, &BridgeWind::SolverService::logMessageReady, this, &MainWindow::appendLogMessage);
	connect(worker, &BridgeWind::SolverService::solverFinished, this, &MainWindow::onSolverFinished);
	connect(worker, &BridgeWind::SolverService::errorOccurred, this, &MainWindow::handleError);
	connect(ui->stopSimulationButton, &QPushButton::clicked, worker, &BridgeWind::SolverService::stop, Qt::DirectConnection); // 直接连接停止按钮
	connect(m_workerThread, &QThread::finished, this, [this]() {
		// 停止按钮与worker的连接需要断开，防止野指针
		disconnect(ui->stopSimulationButton, &QPushButton::clicked, m_currentWorker, nullptr);
		cleanupWorker(m_workerThread, m_currentWorker);
		});

	// --- 4. 启动任务 ---
	BridgeWind::SimulationParameters params; // TODO: 从UI获取参数
	// ...
	QMetaObject::invokeMethod(worker, "run", Qt::QueuedConnection,
		Q_ARG(BridgeWind::SimulationParameters, params));

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
		// Qt::QueuedConnection 确保这个调用被放入后台线程的事件队列中执行，
		// 而不是立即在UI线程中尝试执行。
		QMetaObject::invokeMethod(m_currentWorker, "stop", Qt::QueuedConnection);
	}
	else {
		appendLogMessage(tr("[Warning] Solver is not currently running."));
	}
}
void MainWindow::onAnalysisFinished(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer)
{
	appendLogMessage(tr("Geometry analysis finished successfully."));
	m_analyzerResult = analyzer; // 保存分析结果
	ui->geometryAnalyzeButton->setEnabled(true);
	ui->generateMeshButton->setEnabled(true); // **启用下一步的按钮**
	m_workerThread->quit(); // 结束线程
}

void MainWindow::onMeshingFinished(const QString& cgnsFilePath)
{
	appendLogMessage(tr("Mesh generation finished successfully. File at: %1").arg(cgnsFilePath));
	m_cgnsFilePath = cgnsFilePath; // 保存网格路径
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
	//    这确保用户总能看到最新的日志，而不需要手动滚动。
	ui->logViewer->ensureCursorVisible();
}

void MainWindow::handleError(const QString& errorMessage)
{
	appendLogMessage(tr("[CRITICAL ERROR] ") + errorMessage);
	// 恢复所有按钮的状态
	ui->geometryAnalyzeButton->setEnabled(true);
	ui->generateMeshButton->setEnabled(m_analyzerResult != nullptr);
	ui->startSimulationButton->setEnabled(!m_cgnsFilePath.isEmpty());
	ui->stopSimulationButton->setEnabled(false);
	if (m_workerThread && m_workerThread->isRunning()) {
		m_workerThread->quit();
	}
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