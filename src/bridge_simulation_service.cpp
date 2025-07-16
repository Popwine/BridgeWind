#include "bridge_simulation_service.h"

namespace BridgeWind
{
    bool BridgeSimulationService::run(const SimulationParameters& params) {
        std::string cgnsFilePath;
        
        // --- 步骤 1: 网格生成 ---
        emitProgress(Strings::MeshGenerationStarted);
        if (!runMeshing(params, cgnsFilePath)) {
            // runMeshing 内部会调用 emitError
            return false;
        }
        emitProgress(Strings::MeshGenerationComplete + cgnsFilePath);

		// --- 步骤 2: 准备 Hypara 文件 ---
		emitProgress(Strings::HyparaFileGenerateStarted);
		if (!prepareHyparaFiles(params)) {
			// prepareHyparaFiles 内部会调用 emitError
			return false;
		}

		// --- 步骤 3: 运行 PHengLEI 网格划分和分区 ---
		emitProgress(Strings::RunningPHengLEIMeshAndPartition);
		if (!runPhengLeiMeshAndPartition(params)) {
			// runPhengLeiMeshAndPartition 内部会调用 emitError
			return false;
		}
		emitProgress(Strings::PHengLEIMeshAndPartitionCompleted);

		// --- 步骤 4: 运行求解器 ---
		emitProgress(Strings::SolverStarted);
		if (!runSolver(params, cgnsFilePath)) {
			// runSolver 内部会调用 emitError
			return false;
		}
		emitProgress(Strings::SolverStarted + cgnsFilePath);

        emitFinished(); // 全部成功
        return true;
    }

    bool BridgeSimulationService::runMeshing(const SimulationParameters& params, std::string& outCgnsFilePath) {
        try {

			std::filesystem::path workdir(params.workingDirectory);

            // 1. 加载DXF
            emitProgress(Strings::LoadingDXF + params.dxfFilePath);
            BridgeWind::Geometry geo;
            geo.loadFromDXF(params.dxfFilePath);
			

            // 2. 拓扑分析
            emitProgress(Strings::TopologyAnalysisStarted);
            BridgeWind::TopologyAnalyzer analyzer(geo);
            analyzer.analyze();
            emitProgress(Strings::TopologyAnalysisComplete);

            // 3. 生成.geo文件
			createFolder(workdir / "grid");

            emitProgress(Strings::GeneratingGeoFile);
			GeoGenerator geoGen(analyzer, (workdir / "grid" / "bridge_wind.geo").string());

            geoGen.circumferentialMeshNumber = 60;
			geoGen.radialMeshNumber = 60;
			geoGen.generateGeoFile();
			geoGen.finalize();
            emitProgress(Strings::GeoFileGenerated);

            // 4. 调用Gmsh生成.msh
            emitProgress(Strings::GeneratingMesh);
			GmshController gmshController(
				(workdir / "grid" / "bridge_wind.geo").string(),
				(workdir / "grid" / "bridge_wind.msh").string()
			);
            gmshController.generateMesh();

            // 5. 转换为.cgns
            emitProgress(Strings::ConvertingToCgns);
			G2C::convertGmshToCgns(
				(workdir / "grid" / "bridge_wind.msh").string(),
				(workdir / "grid" / "bridge_wind.cgns").string()
			);
			emitProgress(Strings::CgnsFileGenerated + (workdir / "grid" / "bridge_wind.cgns").string());


        }
        catch (const std::exception& e) {
            emitError(Strings::MeshGenerationFailed + std::string(e.what()));
            return false;
        }

        return true;
    }
    bool BridgeSimulationService::prepareHyparaFiles(const SimulationParameters& params) {
        try {
            auto workdir = std::filesystem::path(params.workingDirectory);

            createFolder(workdir / "bin");
            // 从模板加载 Hypara 文件
            BridgeWind::HyparaGenerator hyparaGen("PHengLEI_template/bin");
			// ----------从 params 中设置 Hypara 文件的参数----------
			hyparaGen.cfdParaSubsonic.set<double>("refReNumber", params.refReNumber);
			hyparaGen.partition.set<int>("maxproc", params.maxproc);

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

            copyFile("PHengLEI_template/bin/cfd_para.hypara", workdir / "bin/cfd_para.hypara");

            

		}
        catch (const std::exception& e) {
            emitError(Strings::ErrorSimulation + std::string(e.what()));
            return false;
        }
		emitProgress(Strings::HyparaFileGenerated);
		return true;

        

    }
    bool BridgeSimulationService::runPhengLeiMeshAndPartition(const SimulationParameters& params) {
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
			emitProgress(Strings::PHengLEIGridGenerationCompleted);
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
            emitError(Strings::ErrorSimulation + std::string(e.what()));
            return false;
        }
		emitProgress(Strings::HyparaFileGenerated);
		return true;
    }

    bool BridgeSimulationService::runSolver(const SimulationParameters& params, const std::string& cgnsFilePath) {
        try {
            std::filesystem::path workdir(params.workingDirectory);
            HyparaFile keyFile(workdir / "bin" / "key.hypara");
            keyFile.load();
            keyFile.set<int>("nsimutask", 0);
            keyFile.set<std::string>("parafilename", "./bin/cfd_para_subsonic.hypara");
            keyFile.save();


			
		}
        catch (const std::exception& e) {
            emitError(Strings::ErrorSolver + std::string(e.what()));
            return false;
        }
		return true;
    }

    void BridgeSimulationService::emitProgress(const std::string& message) {
        if (onProgress) {
            onProgress(message);
        }
    }
    void BridgeSimulationService::emitError(const std::string& message){
		if (onError) {
			onError(message);
		}
	}
    void BridgeSimulationService::emitFinished() {
        if (onFinished) {
            onFinished();
        }
    }

    void BridgeSimulationService::createFolder(std::filesystem::path path) {
        std::filesystem::create_directories(path);
    }

    void BridgeSimulationService::copyFile(const std::filesystem::path& source, const std::filesystem::path& destination) {
        if (!std::filesystem::exists(source)) {
            throw std::runtime_error("Source file does not exist: " + source.string());
        }
        std::filesystem::create_directories(destination.parent_path());

        
        if (!std::filesystem::copy_file(
            source, destination,
            std::filesystem::copy_options::overwrite_existing))
        {
			throw std::runtime_error("Failed to copy file from " + source.string() + " to " + destination.string());
        }
        

        
    }


    void BridgeSimulationService::copyPHengLEIExecutable(const std::filesystem::path& workdir) {
        // 复制 PHengLEI 可执行文件到工作目录
        copyFile(
            "PHengLEI_template/PHengLEIv3d0.exe", 
            workdir / "PHengLEIv3d0.exe");
        copyFile(
            "PHengLEI_template/tecio.dll",
            workdir / "tecio.dll"
        );
    }
}