#ifndef GEO_GENERATOR_H
#define GEO_GENERATOR_H
#include "geometry.h"
#include "topology_analyzer.h"
#include  <fstream>
#define BW_GEO_RADIUS_MESH_NUM_VAR_NAME "Nr"
#define BW_GEO_CIRCUM_GROWTH_RATE_VAR_NAME "Rc"
#define BW_GEO_DEFAULT_RADIUS_GROWTH_RATE_VAR_NAME "Rr"
namespace BridgeWind{
	enum class BuiltInShapes {
		Rectangle,
		Circle

	};

	class GeoGenerator {
	private:
		const TopologyAnalyzer& analyzer;
		
		
	public:
		GeoGenerator() = delete;
		~GeoGenerator();
		//只接受 Geometry 对象的引用
		GeoGenerator(const TopologyAnalyzer& analyzer, const std::string& filename) ;
		std::string filename;
		


		void generateGeoFile() ;
		void finalize();
	private:
		std::ofstream ofs;
		std::vector<Point> writtedWallPoints; // 壁面点
		std::vector<Point> writtedFarfieldPoints; // 远场点
		std::vector<std::pair<std::string, int>> meshNumbersVariabls;
		std::vector<std::vector<int>> surfaceNodes;
		
		size_t wallPointStartIndex = 0;
		size_t wallArcCenterStartIndex = 0;
		size_t farfieldPointStartIndex = 0;
		size_t centerPointIndex = 0;
		double fieldDiameter = 0.0;

	public:
		int radialMeshNumber = 90; // 径向网格数
		int circumferentialMeshNumber = 360; // 周向网格数

		double meshGrowthRate = 0.95; // 网格增长率，从外向到中心
	private:
		size_t getWallPointStartIndex() const { return wallPointStartIndex; };
		size_t getWallArcCenterStartIndex() const { return wallArcCenterStartIndex; };
		size_t getFarfieldPointStartIndex() const { return farfieldPointStartIndex; };
		size_t getCenterPointIndex() const { return centerPointIndex; };

	private:
		void generateGeoFileSingleLoop() ;
		void generateGeoFileMultiLoop() ;
	private:
		// geo文件基础命令
		void geoSetVariable(std::string name, double value);
		void geoSetVariable(std::string name, int value);
		void geoSetPoint(int index, double x, double y, double z = 0.0);
		void geoSetPoint(int index, const Point& point);
		void geoSetLine(int index, int startPointIndex, int endPointIndex);
		void geoSetLine(
			int index,
			int startPointIndex,
			int endPointIndex,
			std::string meshPointCountVariable,
			std::string growthRateVariable
		);
		void geoSetArc(
			int index,
			int startPointIndex,
			int centerPointIndex,
			int endPointIndex,
			std::string meshPointCountVariable,
			std::string growthRateVariable
		);
		void geoWriteLineComment(const std::string& comment);
		void geoWriteSurface(int surfaceIndex, std::vector<int> nodes);
		void geoTransfiniteSurface(int n) ;
		void geoRecombineSurface(int n) ;
		void geoSetPhysicalCurve(std::string name, int index, std::vector<int> indexes);
		void geoSetPhysicalSurface(std::string name, int index, std::vector<int> indexes);

	private:
		// geo文件写入方法
		void geoWriteWallPoints(const Loop& loop, int& pointIndex);
		void geoWriteFarfieldPoints(const Loop& loop, int& pointIndex);

		void geoWriteWallLine(const Loop& loop, int& lineIndex);
		void geoWriteConnectLine(const Loop& loop, int& lineIndex);
		void geoWriteFarfieldLine(const Loop& loop, int& lineIndex);

		void geoWriteSurfaces(const Loop& loop, int& surfaceIndex) ;
		void geoTransfiniteSurfaces(int startIndex, int endIndex) ;
		void geoRecombineSurfaces(int startIndex, int endIndex) ;

		void geoSetPhysicalCurves(std::string name, int surfaceIndex, int startIndex, int endIndex);
		void geoSetPhysicalSurfaces(std::string name, int surfaceIndex, int startIndex, int endIndex);

		//void geoWriteStartPointByNext(
		//	int& pointIndex,
		//	const Line* line,
		//	const Line* nextLine
		//);
		//void geoWriteStartPointByNext(
		//	int& pointIndex,
		//	const Line* line,
		//	const Arc* nextArc
		//);
		//void geoWriteStartPointByNext(
		//	int& pointIndex,
		//	const Arc* arc,
		//	const Arc* nextArc
		//);
		//void geoWriteStartPointByNext(
		//	int& pointIndex,
		//	const Arc* arc,
		//	const Line* nextLine
		//);
		//void geoWriteStartPointByPrevious(
		//	int& pointIndex,
		//	const Line* line,
		//	const Line* previousLine
		//);
		//void geoWriteStartPointByPrevious(
		//	int& pointIndex,
		//	const Line* line,
		//	const Arc* previousArc
		//);
		//void geoWriteStartPointByPrevious(
		//	int& pointIndex,
		//	const Arc* arc,
		//	const Arc* previousArc
		//);
		//void geoWriteStartPointByPrevious(
		//	int& pointIndex,
		//	const Arc* arc,
		//	const Line* previousArc
		//);
		//void geoWriteWallPoints(const std::vector<const GraphEdge*>& edges, int& pointIndex);
		
	};
}

#endif // GEO_GENERATOR_H