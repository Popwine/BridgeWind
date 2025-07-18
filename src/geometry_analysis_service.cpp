#include "geometry_analysis_service.h"
#include "string_literals.h"


namespace BridgeWind
{
	GeometryAnalysisService::GeometryAnalysisService(QObject* parent)
		: QObject(parent)
	{

	}

	void GeometryAnalysisService::run(const QString& dxfFilePath)
	{
		try {

			emit progressUpdated(QString::fromStdString(Strings::LoadingDXF + dxfFilePath.toStdString()));
			auto geo = std::make_shared<BridgeWind::Geometry>();
			geo->loadFromDXF(dxfFilePath.toStdString());
			emit progressUpdated(QString::fromStdString(Strings::DXFFileLoaded));


			emit progressUpdated(QString::fromStdString(Strings::TopologyAnalysisStarted));
			auto analyzer = std::make_shared<BridgeWind::TopologyAnalyzer>(geo);
			analyzer->analyze();
			emit progressUpdated(QString::fromStdString(Strings::TopologyAnalysisComplete));
			emit finished(analyzer);
		}
		catch (const std::exception& e) {
			emit errorOccurred(QString::fromStdString(Strings::ErrorOccurred + std::string(e.what())));
		}
	}
	
}
