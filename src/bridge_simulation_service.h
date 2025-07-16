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
        // --- ����δ����UIͨ�ŵġ��źš� ---
        // �����ź����ͣ������ǽ��������ַ���
        using ProgressSignal = std::function<void(const std::string&)>;
        // ��������ź�����
        using ErrorSignal = std::function<void(const std::string&)>;
        // ����ɹ��ź�����
        using FinishedSignal = std::function<void()>;

        // --- ���ӡ��ۺ������ķ��� ---
        void connectProgressSignal(ProgressSignal slot) { onProgress = slot; }
        void connectErrorSignal(ErrorSignal slot) { onError = slot; }
        void connectFinishedSignal(FinishedSignal slot) { onFinished = slot; }

        // --- ���Ĺ����ӿ� ---
        // ���������ִ�������ļ������̡�
        // Ϊ�˼����������������ͬ��ִ�С�δ���������ɵذ����ŵ��߳��
        bool run(const SimulationParameters& params);

    private:
        // --- ˽�г�Ա�����������洢���ۺ����� ---
        ProgressSignal onProgress = nullptr;
        ErrorSignal    onError = nullptr;
        FinishedSignal onFinished = nullptr;

        // --- ˽�и�����������������ֽ�ΪС���� ---
        bool runMeshing(const SimulationParameters& params, std::string& outCgnsFilePath);
		bool prepareHyparaFiles(const SimulationParameters& params);
		bool runPhengLeiMeshAndPartition(const SimulationParameters& params);
        bool runSolver(const SimulationParameters& params, const std::string& cgnsFilePath);

        // �������������ڷ����ź�
        void emitProgress(const std::string& message);
        void emitError(const std::string& message);
        void emitFinished();

        void createFolder(std::filesystem::path path);
        void copyFile(const std::filesystem::path& source, const std::filesystem::path& destination);
        void copyPHengLEIExecutable(const std::filesystem::path& workdir);
    };
}



#endif// BRIDGE_SIMULATION_SERVICE_H