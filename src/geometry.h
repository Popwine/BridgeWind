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
	class Line {
    public:
		G2C::vector2d<double> begin;
		G2C::vector2d<double> end;
		Line(const G2C::vector2d<double>& b, const G2C::vector2d<double>& e) : begin(b), end(e) {};
	    void printCADCommand() const;
        
    };

    class Rectangle {
    public:
        G2C::vector2d<double> bottomLeft; // smaller value
        G2C::vector2d<double> topRight; // bigger value
        Rectangle(const G2C::vector2d<double>& bl, const G2C::vector2d<double>& tr);
        ~Rectangle() = default;
        void printCADCommand() const;

    };

	class Arc {
    public:
		G2C::vector2d<double> center;
		double radius;
		double startAngle; // in radians
		double endAngle;   // in radians
        Arc(const G2C::vector2d<double>& c, double r, double sa, double ea);
        bool isInArc(double angle) const;
        Rectangle getBoundingBox() const;
        
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

        void addLine(const G2C::vector2d<double>& begin, const G2C::vector2d<double>& end);
        void addArc(const G2C::vector2d<double>& center, double radius, double startAngle, double endAngle);
		void loadFromDXF(const std::string& dxfFilePath);
        void print() const;
        bool isEQ(double a, double b) const;
        bool isGT(double a, double b) const;
		bool isLT(double a, double b) const;
        bool isGE(double a, double b) const;
		bool isLE(double a, double b) const;
		double getEpsilon() const { return epsilon; }


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


}


#endif // GEOMETRY_H