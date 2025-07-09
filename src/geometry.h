#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <vector>
#include <string>
#include "vector2d.h"
#include "libdxfrw.h"
#include "drw_interface.h"
#include <stdexcept>
#ifndef PI
#define PI 3.14159265358979323846264338
#endif // PI

namespace BridgeWind {
    class Point {
    public:
        double x;
        double y;
        Point() : x(0.0), y(0.0) {};
        Point(double xCoord, double yCoord) : x(xCoord), y(yCoord) {};
		~Point() = default;
		void printCADCommand() const {
			std::cout << "Point " << x << "," << y << " " << std::endl;
		}
		bool isSame(const Point& other) const {
			return (std::abs(x - other.x) < 1e-8) && (std::abs(y - other.y) < 1e-8);
		}
    };
	class Line {
    public:
		Point begin;
		Point end;
        Line(const Point& b, const Point& e);
        Line(const Point& begin, double angle, double length);
	    void printCADCommand() const;
        double distanceToPoint(const Point& p) const ;
        Point getPerpendicularFoot(const Point& p) const;
        double angle() const;
        double length() const;
        bool isOnLine(const Point& p) const;
    };

    class Rectangle {
    public:
        Point bottomLeft; // smaller value
        Point topRight; // bigger value
        Rectangle(const Point& bl, const Point& tr);
        ~Rectangle() = default;
        void printCADCommand() const;

    };

	class Arc {
    public:
		Point center;
		double radius;
		double startAngle; // in radians
		double endAngle;   // in radians
        Arc(const Point& c, double r, double sa, double ea);
        bool isInArc(double angle) const;
		bool isOnArc(const Point& p) const;
        Rectangle getBoundingBox() const;
        Point getStartPoint() const;
        Point getEndPoint() const;
        
	};

	class Geometry {
    public:
		Geometry();
		~Geometry() = default;

		std::vector<Line> lines;
		std::vector<Arc> arcs;
		Rectangle boundingBox;
    private:
        bool isBoundingBoxReal;
		double epsilon; // for floating point comparison
    public:

        void addLine(const Point& begin, const Point& end);
        void addArc(const Point& center, double radius, double startAngle, double endAngle);
		void loadFromDXF(const std::string& dxfFilePath);
        void print() const;

		// Floating point comparison methods
        bool isEQ(double a, double b) const;
        bool isGT(double a, double b) const;
		bool isLT(double a, double b) const;
        bool isGE(double a, double b) const;
		bool isLE(double a, double b) const;

		double getEpsilon() const { return epsilon; }
        std::vector<Point> getAllIntersectionPoints() const;
        std::vector<Point> getAllIntersectionPointsNoEndPoints() const;

	};
	

	class DXFReader : public DRW_Interface {
	public:

		std::vector<DRW_Circle> circles;
		std::vector<DRW_Line> lines;
		std::vector<DRW_Arc> arcs;
		std::vector<DRW_Polyline> polylines;
		std::vector<DRW_LWPolyline> lwPolylines; 
		DXFReader() : DRW_Interface() {};


		void addCircle(const DRW_Circle& data) override {
			circles.push_back(data);
		};
		void addLine(const DRW_Line& data) override {
			lines.push_back(data);
		};
		void addArc(const DRW_Arc& data) override {
			arcs.push_back(data);
		};
		void addPolyline(const DRW_Polyline& data) override {
			polylines.push_back(data);
		};
		void addLWPolyline(const DRW_LWPolyline& data) override {
			lwPolylines.push_back(data);
		};
		void printResult() const ;


        void addHeader(const DRW_Header* data) override {}
        void addLType(const DRW_LType& data) override {}
        void addLayer(const DRW_Layer& data) override {}
        void addDimStyle(const DRW_Dimstyle& data) override {}
        void addVport(const DRW_Vport& data) override {}
        void addTextStyle(const DRW_Textstyle& data) override {}
        void addAppId(const DRW_AppId& data) override {}

        // -- Blocks --
        void addBlock(const DRW_Block& data) override {}
        void setBlock(const int handle) override {}
        void endBlock() override {}

        // -- Entities (that you don't care about) --
        void addPoint(const DRW_Point& data) override {}
        void addRay(const DRW_Ray& data) override {}
        void addXline(const DRW_Xline& data) override {}
        void addEllipse(const DRW_Ellipse& data) override {}
        void addSpline(const DRW_Spline* data) override {}
        void addKnot(const DRW_Entity& data) override {}
        void addInsert(const DRW_Insert& data) override {}
        void addTrace(const DRW_Trace& data) override {}
        void add3dFace(const DRW_3Dface& data) override {}
        void addSolid(const DRW_Solid& data) override {}
        void addMText(const DRW_MText& data) override {}
        void addText(const DRW_Text& data) override {}
        void addDimAlign(const DRW_DimAligned* data) override {}
        void addDimLinear(const DRW_DimLinear* data) override {}
        void addDimRadial(const DRW_DimRadial* data) override {}
        void addDimDiametric(const DRW_DimDiametric* data) override {}
        void addDimAngular(const DRW_DimAngular* data) override {}
        void addDimAngular3P(const DRW_DimAngular3p* data) override {}
        void addDimOrdinate(const DRW_DimOrdinate* data) override {}
        void addLeader(const DRW_Leader* data) override {}
        void addHatch(const DRW_Hatch* data) override {}
        void addViewport(const DRW_Viewport& data) override {}
        void addImage(const DRW_Image* data) override {}
        void linkImage(const DRW_ImageDef* data) override {}
        void addComment(const char* comment) override {}
        void addPlotSettings(const DRW_PlotSettings* data) override {}

        // -- Write functions --
        void writeHeader(DRW_Header& data) override {}
        void writeBlocks() override {}
        void writeBlockRecords() override {}
        void writeEntities() override {}
        void writeLTypes() override {}
        void writeLayers() override {}
        void writeTextstyles() override {}
        void writeVports() override {}
        void writeDimstyles() override {}
        void writeObjects() override {}
        void writeAppId() override {}





	};


	std::vector<Point> getIntersectionPoints(
		const Line& line1,
		const Line& line2
	);
	std::vector<Point> getIntersectionPoints(
		const Line& line,
		const Arc& arc
	);
    std::vector<Point> getIntersectionPoints(
        const Arc& arc,
        const Line& line
    );
    std::vector<Point> getIntersectionPoints(
        const Arc& arc1,
        const Arc& arc2
    );
    
	std::vector<Point> getIntersectionPointsNoEndPoints(
		const Line& line1,
		const Line& line2
	);
	std::vector<Point> getIntersectionPointsNoEndPoints(
		const Line& line,
		const Arc& arc
	);
	std::vector<Point> getIntersectionPointsNoEndPoints(
		const Arc& arc,
		const Line& line
	);
	std::vector<Point> getIntersectionPointsNoEndPoints(
		const Arc& arc1,
		const Arc& arc2
	);
    void formatAngle(double& angle);
    double distance(const Point& p1, const Point& p2);
}


#endif // GEOMETRY_H