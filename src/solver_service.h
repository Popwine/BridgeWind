#pragma once 
#ifndef SOLVER_SERVICE_H
#define SOLVER_SERVICE_H
#include <QObject>
#include <filesystem>
#include "process_manager.h"
#include "simulation_parameters.h"



namespace BridgeWind {
    class SolverService : public QObject
    {
        Q_OBJECT
    public:
		SolverService(QObject* parent = nullptr); // 构造函数
        ~SolverService(); // 需要在析构函数中处理 ProcessManager

    signals:
        void logMessageReady(const QString& message);
		void progressUpdated(const QString& message);
        void solverFinished();
        void errorOccurred(const QString& errorMessage);

    public slots:
        void run(const BridgeWind::SimulationParameters& params);
        void stop(); // 新增的停止槽

    private:
        ProcessManager* m_processManager; // 将使用我们之前设计的进程管理器
        bool prepareHyparaFiles(const SimulationParameters& params);
        bool copyPHengLEIExecutable(const std::filesystem::path& workdir);
        bool runPhengLeiMeshAndPartition(const SimulationParameters& params);
		bool runSolver(const SimulationParameters& params);
        
        
    };
}







#endif //  SOLVER_SERVICE_H