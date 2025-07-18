#pragma once

#ifndef GEOMETRY_ANALYSIS_SERVICE_H
#define GEOMETRY_ANALYSIS_SERVICE_H

#include <QObject>
#include <memory>

#include "topology_analyzer.h"

namespace BridgeWind {
	class GeometryAnalysisService : public QObject {
		Q_OBJECT
	public:
		explicit GeometryAnalysisService(QObject* parent = nullptr);
	signals:
		void progressUpdated(const QString& message);
		void errorOccurred(const QString& errorMessage);
		void finished(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer);

	public slots:
		void run(const QString& dxfFilePath);
		
	};
}









#endif // !GEOMETRY_ANALYSIS_SERVICE_H
