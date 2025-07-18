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

#include <Qstring>
#include <Qobject>

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
    class BridgeSimulationService : public QObject {
        Q_OBJECT
    public:
        BridgeSimulationService(QObject* parent = nullptr);

    signals:
        //    将原来的 std::function 定义改为真正的 Qt 信号
        //    信号只有声明，没有实现，由 moc 自动生成
        void progressUpdated(const QString& message);
        void errorOccurred(const QString& errorMessage);
        void finished();




    public slots:
        bool run(const SimulationParameters& params);

    private:


        // --- 私有辅助函数，将大任务分解为小步骤 ---
        bool runMeshing(const SimulationParameters& params, std::string& outCgnsFilePath);
		bool prepareHyparaFiles(const SimulationParameters& params);
		bool runPhengLeiMeshAndPartition(const SimulationParameters& params);
        bool runSolver(const SimulationParameters& params, const std::string& cgnsFilePath);



        void createFolder(std::filesystem::path path);
        void copyFile(const std::filesystem::path& source, const std::filesystem::path& destination);
        void copyPHengLEIExecutable(const std::filesystem::path& workdir);
    };
}



#endif// BRIDGE_SIMULATION_SERVICE_H