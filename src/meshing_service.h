#pragma once 
#ifndef MESHING_SERVICE_H
#define MESHING_SERVICE_H
#include <QObject>
#include <memory>
#include <QString>

#include "topology_analyzer.h"
#include "simulation_parameters.h"



namespace BridgeWind
{
    class MeshingService : public QObject
    {
        Q_OBJECT
    public:
		MeshingService(QObject* parent = nullptr);
    signals:
        void progressUpdated(const QString& message);
        void finished(); // 输出CGNS文件路径
        void errorOccurred(const QString& errorMessage);

    public slots:
        // 输入是 TopologyAnalyzer 和其他参数
        void run(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer,
            const BridgeWind::SimulationParameters& params);
    };
}
#endif // MESHING_SERVICE_H