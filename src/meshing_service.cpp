#include "meshing_service.h"
#include "geo_generator.h"
#include "app_controller.h"
#include "file_utils.h"
#include "string_literals.h"
#include "gmsh2cgns.h"

namespace BridgeWind
{
	MeshingService::MeshingService(QObject* parent) :QObject(parent) {

	}
						
	void MeshingService::run(std::shared_ptr<BridgeWind::TopologyAnalyzer> analyzer,
		const BridgeWind::SimulationParameters& params) {
		try {

			std::filesystem::path workdir(params.workingDirectory);
			// 3. 生成.geo文件

			FileUtils::createFolder(workdir / "grid");

			emit progressUpdated(QString::fromStdString(Strings::GeneratingGeoFile));
			GeoGenerator geoGen(*analyzer, (workdir / "grid" / "Bridge_Wind.geo").string());

			geoGen.circumferentialMeshNumber = params.circumferentialMeshNumber;
			geoGen.radialMeshNumber = params.radialMeshNumber;
			geoGen.meshGrowthRate = params.radialMeshGrowthRate;
			geoGen.generateGeoFile();
			geoGen.finalize();
			emit progressUpdated(QString::fromStdString(Strings::GeoFileGenerated));

			// 4. 调用Gmsh生成.msh
			emit progressUpdated(QString::fromStdString(Strings::GeneratingMesh));
			GmshController gmshController(
				(workdir / "grid" / "Bridge_Wind.geo").string(),
				(workdir / "grid" / "Bridge_Wind.msh").string()
			);
			gmshController.generateMesh();

			// 5. 转换为.cgns
			emit progressUpdated(QString::fromStdString(Strings::ConvertingToCgns));
			G2C::convertGmshToCgns(
				(workdir / "grid" / "Bridge_Wind.msh").string(),
				(workdir / "grid" / "Bridge_Wind.cgns").string()
			);
			emit progressUpdated(QString::fromStdString(Strings::CgnsFileGenerated + (workdir / "grid" / "Bridge_Wind.cgns").string()));


		}
		catch (const std::exception& e) {
			emit errorOccurred(
				QString::fromStdString(
					Strings::ErrorMeshGeneration + std::string(e.what())
				)
			);
		}
	}
}
