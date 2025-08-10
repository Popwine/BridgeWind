
#include "solver_service.h"
#include "file_utils.h"
#include "hypara_generator.h"
#include "process_manager.h"
#include "string_literals.h"
#include "app_controller.h"

namespace BridgeWind
{
    SolverService::SolverService(QObject* parent) : QObject(parent) {
        m_processManager = new ProcessManager(); // 在构造时创建
        

        // 连接ProcessManager的信号到我们自己的信号
        connect(m_processManager, &ProcessManager::outputReady, this, &SolverService::logMessageReady);
        connect(m_processManager, &ProcessManager::finished, this, &SolverService::solverFinished);
        connect(m_processManager, &ProcessManager::errorOccurred, this, &SolverService::errorOccurred);
    }

    void SolverService::run(const BridgeWind::SimulationParameters& params) {
		emit logMessageReady(tr("Preparing to run the solver..."));
		// 1. 准备 Hypara 文件
        
        // --- 步骤 2: 准备 Hypara 文件 ---
        emit progressUpdated(QString::fromStdString(Strings::HyparaFileGenerateStarted));
        if (!prepareHyparaFiles(params)) {
            
			emit errorOccurred(QString::fromStdString(Strings::HyparaFileGenerationFailed));
        }

        // --- 步骤 3: 运行 PHengLEI 网格划分和分区 ---
        emit progressUpdated(QString::fromStdString(Strings::RunningPHengLEIMeshAndPartition));
        if (!runPhengLeiMeshAndPartition(params)) {
            // runPhengLeiMeshAndPartition 内部会调用 errorOccurred(QString::fromStdString
			emit errorOccurred(QString::fromStdString(Strings::MeshGenerationFailed));
        }
        emit progressUpdated(QString::fromStdString(Strings::PHengLEIMeshAndPartitionCompleted));

        // --- 步骤 4: 运行求解器 ---
        emit progressUpdated(QString::fromStdString(Strings::SolverStarting));
        if (!runSolver(params)) {
            // runSolver 内部会调用 errorOccurred(QString::fromStdString
			emit errorOccurred(QString::fromStdString(Strings::ErrorSolverStarted));
        }
        emit progressUpdated(QString::fromStdString(Strings::SolverStarted));

        //emit solverFinished(); // 全部成功



        
    }

    void SolverService::stop() {
        emit logMessageReady(tr("Attempting to stop the solver..."));
        m_processManager->stop();
    }

    SolverService::~SolverService() {
        // 确保ProcessManager被删除
        // 如果ProcessManager的parent是this，则不需要手动delete
    }

    bool SolverService::prepareHyparaFiles(const SimulationParameters& params) {
        try {
            auto workdir = std::filesystem::path(params.workingDirectory);

            FileUtils::createFolder(workdir / "bin");
            // 从模板加载 Hypara 文件
            BridgeWind::HyparaGenerator hyparaGen("PHengLEI_template/bin");
            //设置速度模式为4-直接指定速度
            hyparaGen.cfdParaSubsonic.set<int>("inflowParaType", 0);

            // ----------从 params 中设置 Hypara 文件的参数----------
            hyparaGen.partition.set<int>("maxproc", params.maxproc);
            hyparaGen.cfdParaSubsonic.set<double>("physicalTimeStep", params.physicalTimeStep);
            hyparaGen.cfdParaSubsonic.set<int>("maxSimuStep", params.maxSimuStep);
            hyparaGen.cfdParaSubsonic.set<int>("intervalStepFlow", params.intervalStepFlow);
            hyparaGen.cfdParaSubsonic.set<int>("intervalStepPlot", params.intervalStepPlot);
            hyparaGen.cfdParaSubsonic.set<int>("intervalStepForce", params.intervalStepForce);
            hyparaGen.cfdParaSubsonic.set<int>("intervalStepRes", params.intervalStepRes);
            //hyparaGen.cfdParaSubsonic.set<double>("refDimensionalVelocity", params.refDimensionalVelocity);
            //hyparaGen.cfdParaSubsonic.set<double>("refDimensionalTemperature", params.refDimensionalTemperature);
            //hyparaGen.cfdParaSubsonic.set<double>("refDimensionalDensity", params.refDimensionalDensity);
            hyparaGen.cfdParaSubsonic.set<double>("refReNumber", params.refReNumber);
            hyparaGen.cfdParaSubsonic.set<double>("refMachNumber", params.refMachNumber);
            hyparaGen.cfdParaSubsonic.set<double>("forceReferenceLength", params.forceReferenceLength);
            hyparaGen.cfdParaSubsonic.set<double>("forceReferenceArea", params.forceReferenceArea);
            hyparaGen.cfdParaSubsonic.set<double>("forceReferenceLengthSpanWise", params.forceReferenceLengthSpanWise);

            if (params.viscousModel == viscousModel::Laminar) {
                hyparaGen.cfdParaSubsonic.set<int>("viscousType", 1);
                hyparaGen.cfdParaSubsonic.set<std::string>("viscousName", "laminar");
            }
            if (params.viscousModel == viscousModel::SSTKOmega) {
                hyparaGen.cfdParaSubsonic.set<int>("viscousType", 4);
                hyparaGen.cfdParaSubsonic.set<std::string>("viscousName", "2eq-kw-menter-sst");
            }

            std::filesystem::path original_grid_file(
                hyparaGen.partition.get<std::string>(
                    "original_grid_file"
                )
            );
            std::string partition_grid_file =
                std::string("./grid/") +
                original_grid_file.stem().string() +
                "__" +
                std::to_string(params.maxproc) +
                ".fts";

            hyparaGen.partition.set<std::string>(
                "partition_grid_file",
                partition_grid_file);
            hyparaGen.cfdParaSubsonic.set<std::string>(
                "gridfile",
                partition_grid_file
            );

            // ----------参数设置结束----------

            hyparaGen.saveAll(workdir / "bin");

            FileUtils::copyFile("PHengLEI_template/bin/cfd_para.hypara", workdir / "bin/cfd_para.hypara");



        }
        catch (const std::exception& e) {
            errorOccurred(QString::fromStdString(Strings::ErrorSimulation + std::string(e.what())));
            return false;
        }
        emit progressUpdated(QString::fromStdString(Strings::HyparaFileGenerated));
        return true;

    }


    bool SolverService::copyPHengLEIExecutable(const std::filesystem::path& workdir) {
        try {
            FileUtils::copyFile(
                "PHengLEI_template/PHengLEIv3d0.exe",
                workdir / "PHengLEIv3d0.exe"
            );
            FileUtils::copyFile(
                "PHengLEI_template/tecio.dll",
                workdir / "tecio.dll"
            );
		}
        catch (const std::exception& e) {
            emit errorOccurred(QString::fromStdString(Strings::ErrorOccurred + std::string(e.what())));
            return false;
        }
		return true;
    }

    bool SolverService::runPhengLeiMeshAndPartition(const SimulationParameters& params) {
        try {

            std::filesystem::path workdir(params.workingDirectory);
            HyparaFile keyFile(workdir / "bin" / "key.hypara");
            keyFile.load();
            copyPHengLEIExecutable(workdir);
            AppController controller;
            // 1 - 生成网格
            keyFile.set<int>("nsimutask", 1);
            keyFile.set<std::string>("parafilename", "./bin/grid_para.hypara");
            keyFile.save();
            controller.simpleExecuteWithCreateProcess(
                (workdir / "PHengLEIv3d0.exe").string(),
                workdir.string()
            );
            emit progressUpdated(QString::fromStdString(Strings::PHengLEIGridGenerationCompleted));
            // 3 - 划分网格
            keyFile.set<int>("nsimutask", 3);
            keyFile.set<std::string>("parafilename", "./bin/partition.hypara");
            keyFile.save();
            controller.simpleExecuteWithCreateProcess(
                (workdir / "PHengLEIv3d0.exe").string(),
                workdir.string()
            );

        }
        catch (const std::exception& e) {
            errorOccurred(QString::fromStdString(Strings::ErrorSimulation + std::string(e.what())));
            return false;
        }
        emit progressUpdated(QString::fromStdString(Strings::HyparaFileGenerated));
        return true;
    }
    bool SolverService::runSolver(const SimulationParameters& params) {
        try {
            std::filesystem::path workdir(params.workingDirectory);
            HyparaFile keyFile(workdir / "bin" / "key.hypara");
            keyFile.load();
            keyFile.set<int>("nsimutask", 0);
            keyFile.set<std::string>("parafilename", "./bin/cfd_para_subsonic.hypara");
            keyFile.save();

			// 准备运行求解器
            QString program = "mpiexec";
            QStringList arguments;
            arguments << "-n" << QString::number(params.maxproc) << ".\\PHengLEIv3d0.exe";
            QString workingDir = QString::fromStdString(params.workingDirectory);

            emit logMessageReady(tr("Starting solver: ") + program + " " + arguments.join(" "));
            m_processManager->start(program, arguments, workingDir);

        }
        catch (const std::exception& e) {
            errorOccurred(QString::fromStdString(Strings::ErrorSolver + std::string(e.what())));
            return false;
        }
        return true;
    }
}