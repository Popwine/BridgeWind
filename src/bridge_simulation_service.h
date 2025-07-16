#pragma once
#ifndef BRIDGE_SIMULATION_SERVICE_H
#define BRIDGE_SIMULATION_SERVICE_H


#include <string>
#include <functional>
#include <filesystem> 
#include <iostream>
#include <stdexcept>
#include <windows.h>
#include <memory>

#include "geometry.h"
#include "gmsh2cgns.h"
#include "topology_analyzer.h"
#include "geo_generator.h"
#include "app_controller.h"
#include "hypara_generator.h"
#include "string_literals.h"
#include "simulation_parameters.h"

namespace BridgeWind
{
    class BridgeSimulationService {
    public:
        // --- 这是未来与UI通信的“信号” ---
        // 定义信号类型，参数是进度描述字符串
        using ProgressSignal = std::function<void(const std::string&)>;
        // 定义错误信号类型
        using ErrorSignal = std::function<void(const std::string&)>;
        // 定义成功信号类型
        using FinishedSignal = std::function<void()>;

        // --- 连接“槽函数”的方法 ---
        void connectProgressSignal(ProgressSignal slot) { onProgress = slot; }
        void connectErrorSignal(ErrorSignal slot) { onError = slot; }
        void connectFinishedSignal(FinishedSignal slot) { onFinished = slot; }

        // --- 核心公共接口 ---
        // 这个函数将执行完整的计算流程。
        // 为了简单起见，我们先让它同步执行。未来可以轻松地把它放到线程里。
        bool run(const SimulationParameters& params);

    private:
        // --- 私有成员变量，用来存储“槽函数” ---
        ProgressSignal onProgress = nullptr;
        ErrorSignal    onError = nullptr;
        FinishedSignal onFinished = nullptr;

        // --- 私有辅助函数，将大任务分解为小步骤 ---
        bool runMeshing(const SimulationParameters& params, std::string& outCgnsFilePath);
		bool prepareHyparaFiles(const SimulationParameters& params);
		bool runPhengLeiMeshAndPartition(const SimulationParameters& params);
        bool runSolver(const SimulationParameters& params, const std::string& cgnsFilePath);

        // 辅助函数，用于发出信号
        void emitProgress(const std::string& message);
        void emitError(const std::string& message);
        void emitFinished();

        void createFolder(std::filesystem::path path);
        void copyFile(const std::filesystem::path& source, const std::filesystem::path& destination);
        void copyPHengLEIExecutable(const std::filesystem::path& workdir);
    };
}



#endif// BRIDGE_SIMULATION_SERVICE_H