#include "geometry.h"
#include <iostream>

namespace {
    // 所有只在本文件使用的东西都放在这里
    bool isAllUsed(
        const std::vector<std::pair<BridgeWind::Line, bool>>& lines, 
        const std::vector<std::pair<BridgeWind::Line, bool>>& arcs
    ) {
		for (const auto& line : lines) {
			if (!line.second) return false; // 如果有未使用的线段
		}
		for (const auto& arc : arcs) {
			if (!arc.second) return false; // 如果有未使用的圆弧
		}
		return true; // 所有线段和圆弧都已使用
    }
}
namespace BridgeWind {
    Point::Point(Point p, double dis, double angle)
        :
        x(p.x + cos(angle) * dis),
        y(p.y + sin(angle) * dis)
    {

    };

    void Point::printCADCommand() const {
        std::cout << "Point " << x << "," << y << " " << std::endl;
    }
    bool Point::isSame(const Point& other) const {
        return (std::abs(x - other.x) < BW_GEOMETRY_EPSILON) && (std::abs(y - other.y) < BW_GEOMETRY_EPSILON);
    }
    /*
    * for std::map
    */
    //bool Point::operator<(const Point& other) const {
    //    if (std::abs(x - other.x) > BW_GEOMETRY_EPSILON) return x < other.x;
    //    return y - other.y > BW_GEOMETRY_EPSILON; // 比较 y
    //}
    bool Point::operator==(const Point& other) const {
        return isSame(other);
    }
    double Point::distanceTo(const Point& other) const {
        return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    };

    bool PointCmp::operator()(const Point& a, const Point& b) const {
        // 1. Compare x-coordinates
        // Is 'a.x' definitively to the left of 'b.x's tolerance interval?
        if (a.x < b.x - BW_GEOMETRY_EPSILON) {
            return true;
        }
        // Is 'a.x' definitively to the right of 'b.x's tolerance interval?
        if (a.x > b.x + BW_GEOMETRY_EPSILON) {
            return false;
        }

        // If we reach here, the x-coordinates are considered "equivalent".
        // 2. Now, compare y-coordinates with the same logic.
        // Is 'a.y' definitively below 'b.y's tolerance interval?
        if (a.y < b.y - BW_GEOMETRY_EPSILON) {
            return true;
        }

        // If 'a.y' is not definitively below 'b.y', then 'a' is not "less than" 'b'.
        // This also covers the case where 'a' and 'b' are equivalent.
        return false;
    }


    /*
    * initialize a rectangle with any two points
    */
    Rectangle::Rectangle(const Point& bl, const Point& tr)
        : bottomLeft(bl), topRight(tr) {
        if (bl.x > tr.x || bl.y > tr.y) {
            bottomLeft.x = std::min(bl.x, tr.x);
            bottomLeft.y = std::min(bl.y, tr.y);
            topRight.x = std::max(bl.x, tr.x);
            topRight.y = std::max(bl.y, tr.y);
        }
    }
    Geometry::Geometry() :
        boundingBox(
            Point(0, 0),
            Point(0, 0)),
        isBoundingBoxReal(false),
        epsilon(BW_GEOMETRY_EPSILON)
    {

    }
    void Geometry::addLine(const Point& begin, const Point& end) {
        lines.emplace_back(begin, end);
        if (isBoundingBoxReal) {
            // 更新边界框
            boundingBox.bottomLeft.x = std::min(boundingBox.bottomLeft.x, std::min(begin.x, end.x));
            boundingBox.bottomLeft.y = std::min(boundingBox.bottomLeft.y, std::min(begin.y, end.y));
            boundingBox.topRight.x = std::max(boundingBox.topRight.x, std::max(begin.x, end.x));
            boundingBox.topRight.y = std::max(boundingBox.topRight.y, std::max(begin.y, end.y));
        }
        else {
            // 初始化边界框
            boundingBox = Rectangle(begin, end);
            isBoundingBoxReal = true;
        }
        epsilon = std::max(boundingBox.topRight.x - boundingBox.bottomLeft.x,
			boundingBox.topRight.y - boundingBox.bottomLeft.y) * BW_GEOMETRY_EPSILON * 1e-3; 
    }
    void Geometry::addArc(const Point& center, double radius, double startAngle, double endAngle) {
		Arc arc(center, radius, startAngle, endAngle);
        arcs.push_back(arc);
		if (isBoundingBoxReal) {
			// 更新边界框
			boundingBox.topRight.x = std::max(boundingBox.topRight.x, arc.getBoundingBox().topRight.x);
			boundingBox.topRight.y = std::max(boundingBox.topRight.y, arc.getBoundingBox().topRight.y);
			boundingBox.bottomLeft.x = std::min(boundingBox.bottomLeft.x, arc.getBoundingBox().bottomLeft.x);
			boundingBox.bottomLeft.y = std::min(boundingBox.bottomLeft.y, arc.getBoundingBox().bottomLeft.y);
		}
		else {
			// 初始化边界框
			boundingBox = arc.getBoundingBox();
			isBoundingBoxReal = true;
			
        }
        epsilon = std::max(boundingBox.topRight.x - boundingBox.bottomLeft.x,
            boundingBox.topRight.y - boundingBox.bottomLeft.y) * BW_GEOMETRY_EPSILON * 1e-3; 
    }
    void DXFReader::printResult() const {
        std::cout << "\n--- DXF Read Results ---" << std::endl;
        std::cout << "Total Circles found: " << circles.size() << std::endl;
        std::cout << "Total Lines found: " << lines.size() << std::endl;
        std::cout << "Total Arcs found: " << arcs.size() << std::endl;
        std::cout << "Total Polylines found: " << polylines.size() << std::endl;
        std::cout << "Total LWPolylines found: " << lwPolylines.size() << std::endl;
        std::cout << "------------------------\n" << std::endl;

        // 打印每个圆的详细信息
        for (size_t i = 0; i < circles.size(); ++i) {
            const auto& circle = circles[i];
            std::cout << "Circle " << i + 1 << ": "
                << "Center=(" << circle.basePoint.x << ", " << circle.basePoint.y << "), "
                << "Radius=" << circle.radious << ", "
                << "Layer='" << circle.layer << "'"
                << std::endl;
        }
        if (!circles.empty()) std::cout << std::endl;


        // 打印每条线的详细信息
        for (size_t i = 0; i < lines.size(); ++i) {
            const auto& line = lines[i];
            std::cout << "Line " << i + 1 << ": "
                << "From=(" << line.basePoint.x << ", " << line.basePoint.y << ") "
                << "To=(" << line.secPoint.x << ", " << line.secPoint.y << "), "
                << "Layer='" << line.layer << "'"
                << std::endl;
        }
        if (!lines.empty()) std::cout << std::endl;

        for (size_t i = 0; i < arcs.size(); ++i) {
            const auto& arc = arcs[i];
            std::cout << "Arc " << i + 1 << ": "
                << "Center=(" << arc.basePoint.x << ", " << arc.basePoint.y << ") "
                << "Angle=(" << arc.staangle / PI * 180 << "°, " << arc.endangle / PI * 180 << "°), "
                << "Layer='" << arc.layer << "'"
                << std::endl;
        }

        if (!arcs.empty()) std::cout << std::endl;

        // 打印每条多段线的详细信息
        for (size_t i = 0; i < polylines.size(); ++i) {
            const auto& pline = polylines[i];
            std::cout << "Polyline " << i + 1 << ": "
                << "Vertices=" << pline.vertlist.size() << ", "
                << "Layer='" << pline.layer << "'"
                << (pline.flags & 1 ? ", Closed" : ", Open") // 检查闭合标志位
                << std::endl;

            // 遍历并打印多段线的每个顶点
            for (size_t j = 0; j < pline.vertlist.size(); ++j) {
                const auto& vertex = pline.vertlist[j];
                std::cout << "  - Vertex " << j + 1 << ": (" << vertex->basePoint.x << ", " << vertex->basePoint.y << ")" << std::endl;
            }
            std::cout << std::endl;

        }
        for (size_t i = 0; i < lwPolylines.size(); ++i) {
            const auto& lwPline = lwPolylines[i];
            std::cout << "Polyline " << i + 1 << ": "
                << "Vertices=" << lwPline.vertlist.size() << ", "
                << "Layer='" << lwPline.layer << "'"
                << (lwPline.flags & 1 ? ", Closed" : ", Open") // 检查闭合标志位
                << std::endl;

            // 遍历并打印多段线的每个顶点
            for (size_t j = 0; j < lwPline.vertlist.size(); ++j) {
                const auto& vertex = lwPline.vertlist[j];
                std::cout << "  - Vertex " << j + 1 << ": (" << vertex->x << ", " << vertex->y << ")" << std::endl;
            }
            std::cout << std::endl;

        }




    }

    void Geometry::loadFromDXF(const std::string& dxfFilePath) {
        DXFReader reader;
        dxfRW fileReader(dxfFilePath.c_str());


        bool success = fileReader.read(&reader, false);

        // 4. 检查读取是否成功
        if (success) {
            std::cout << "DXF file read successfully!" << std::endl;

            // 读取完成后，所有数据都存储在你的 entity_handler 对象中了
            std::cout << "Found " << reader.circles.size() << " circles." << std::endl;
            std::cout << "Found " << reader.lines.size() << " lines." << std::endl;
            std::cout << "Found " << reader.arcs.size() << " arcs." << std::endl;
            std::cout << "Found " << reader.polylines.size() << " polylines." << std::endl;
            std::cout << "Found " << reader.lwPolylines.size() << " lwPolylines." << std::endl;
            reader.printResult();
        }
        else {
            throw std::runtime_error("Failed to read DXF file: " + dxfFilePath);
        }
        for (const auto& line : reader.lines) {
            Point basePoint = { line.basePoint.x, line.basePoint.y };
            Point secPoint = { line.secPoint.x, line.secPoint.y };
            addLine(basePoint, secPoint);
        }
        for (const auto& arc : reader.arcs) {
            Point center = { arc.basePoint.x, arc.basePoint.y };
            addArc(center, arc.radious, arc.staangle, arc.endangle);
        }
        for (const auto& circle : reader.circles) {
            Point center = { circle.basePoint.x, circle.basePoint.y };
            addArc(center, circle.radious, 0.0 * PI, 0.5 * PI);
            addArc(center, circle.radious, 0.5 * PI, 1.0 * PI);
			addArc(center, circle.radious, 1.0 * PI, 1.5 * PI);
			addArc(center, circle.radious, 1.5 * PI, 2.0 * PI);
			// 注意：这里假设圆是完整的，所以我们添加了四个半圆来表示完整的圆

        }
        for (const auto& polyline : reader.polylines) {
            for (size_t i = 0; i < polyline.vertlist.size() - 1; ++i) {
                const auto& vertex1 = polyline.vertlist[i]->basePoint;
                const auto& vertex2 = polyline.vertlist[i + 1]->basePoint;
                Point start = { vertex1.x, vertex1.y };
                Point end = { vertex2.x, vertex2.y };
                addLine(start, end);
            }
        }
        for (const auto& lwPolyline : reader.lwPolylines) {
            for (size_t i = 0; i < lwPolyline.vertlist.size() - 1; ++i) {
                const auto& vertex1 = lwPolyline.vertlist[i];
                const auto& vertex2 = lwPolyline.vertlist[i + 1];
                Point start = { vertex1->x, vertex1->y };
                Point end = { vertex2->x, vertex2->y };
                addLine(start, end);

            }
        }

    };
    void Geometry::print() const {
        std::cout << "Geometry contains " << lines.size() << " lines and " << arcs.size() << " arcs." << std::endl;
        for (const auto& line : lines) {
            std::cout << "Line from (" << line.begin.x << ", " << line.begin.y << ") to ("
                << line.end.x << ", " << line.end.y << ")" << std::endl;
        }
        for (const auto& arc : arcs) {
            std::cout << "Arc centered at (" << arc.center.x << ", " << arc.center.y
                << ") with radius " << arc.radius
                << ", from angle " << arc.startAngle * 180 / PI
                << "° to angle " << arc.endAngle * 180 / PI
                << "°" << std::endl;
        }
        // bounding box
        std::cout << "Bounding Box: "
            << "Bottom Left=(" << boundingBox.bottomLeft.x << ", " << boundingBox.bottomLeft.y << "), "
            << "Top Right=(" << boundingBox.topRight.x << ", " << boundingBox.topRight.y << ")"
            << std::endl;
    }

    bool Geometry::isEQ(double a, double b) const {
        return std::fabs(a - b) < BW_GEOMETRY_EPSILON;
    }
    bool Geometry::isGT(double a, double b) const {
        return (a - b) > BW_GEOMETRY_EPSILON;
    }
    bool Geometry::isLT(double a, double b) const {
        return (b - a) > BW_GEOMETRY_EPSILON;
    }
    bool Geometry::isGE(double a, double b) const {
        return isEQ(a, b) || isGT(a, b);
    }
    bool Geometry::isLE(double a, double b) const {
        return isEQ(a, b) || isLT(a, b);
    }

    std::vector<Point> Geometry::getAllIntersectionPoints() const {
        std::vector<Point> intersectionPoints;
        
        for (const auto& line : lines) {
            for (const auto& arc : arcs) {
                auto points = getIntersectionPoints(line, arc);
                intersectionPoints.insert(intersectionPoints.end(), points.begin(), points.end());
            }
        }
		for (size_t i = 0; i < lines.size(); ++i) {
			for (size_t j = i + 1; j < lines.size(); ++j) {
				auto points = getIntersectionPoints(lines[i], lines[j]);
				intersectionPoints.insert(intersectionPoints.end(), points.begin(), points.end());
			}
		}
		for (size_t i = 0; i < arcs.size(); ++i) {
			for (size_t j = i + 1; j < arcs.size(); ++j) {
				auto points = getIntersectionPoints(arcs[i], arcs[j]);
				intersectionPoints.insert(intersectionPoints.end(), points.begin(), points.end());
			}
		}
        return intersectionPoints;
    }
    std::vector<Point> Geometry::getAllIntersectionPointsNoEndPoints() const {
        std::vector<Point> intersectionPoints;

        for (const auto& line : lines) {
            for (const auto& arc : arcs) {
                auto points = getIntersectionPointsNoEndPoints(line, arc);
                intersectionPoints.insert(intersectionPoints.end(), points.begin(), points.end());
            }
        }
        for (size_t i = 0; i < lines.size(); ++i) {
            for (size_t j = i + 1; j < lines.size(); ++j) {
                auto points = getIntersectionPointsNoEndPoints(lines[i], lines[j]);
                intersectionPoints.insert(intersectionPoints.end(), points.begin(), points.end());
            }
        }
        for (size_t i = 0; i < arcs.size(); ++i) {
            for (size_t j = i + 1; j < arcs.size(); ++j) {
                auto points = getIntersectionPointsNoEndPoints(arcs[i], arcs[j]);
                intersectionPoints.insert(intersectionPoints.end(), points.begin(), points.end());
            }
        }
        return intersectionPoints;
    }
    bool Geometry::isIntersectionExist() const {
        if (getAllIntersectionPointsNoEndPoints().size() > 0) {
            return true; // 存在交点
        }
        else return false;
    }
    double Geometry::getBoundingBoxWidth() const {
        return boundingBox.topRight.x - boundingBox.bottomLeft.x;
    };
    double Geometry::getBoundingBoxHeight() const {
        return boundingBox.topRight.y - boundingBox.bottomLeft.y;
    };
    Arc::Arc(const Point& c, double r, double sa, double ea) : center(c), radius(r), startAngle(sa), endAngle(ea) {
        if (radius <= 0) {
            throw std::invalid_argument("Radius must be positive.");
        }
		if (startAngle < 0 || startAngle > 2 * PI || endAngle < 0 || endAngle > 2 * PI) {
			throw std::invalid_argument("Angles must be in the range [0, 2π].");
		}

    };
    bool Arc::isInArc(double angle) const {
        if (angle < 0 || angle > 2 * PI) {
            throw std::invalid_argument("Angle must be in the range [0, 2π].");
        }
        if (startAngle < endAngle) {
			// 正常情况，弧线不穿过0度
			return angle >= startAngle && angle <= endAngle;
		}
        else {
            // 弧线穿过0度
            return angle >= startAngle || angle <= endAngle;
        }
    }
    bool Arc::isOnArc(const Point& p) const {
		// 计算点到圆心的距离
		double distanceToCenter = distance(center, p);
		// 检查是否在圆上
        if (fabs(distanceToCenter - radius) > BW_GEOMETRY_EPSILON) {
			return false; // 点不在圆上
		}
		// 计算点的角度
		double angle = atan2(p.y - center.y, p.x - center.x);
		formatAngle(angle); // 确保角度在 [0, 2π] 范围内
		return isInArc(angle); // 检查角度是否在弧线范围内
    }
    Rectangle Arc::getBoundingBox() const {
        std::vector<double> xCoords;
		std::vector<double> yCoords;

        // begin
		xCoords.push_back(center.x + radius * cos(startAngle));
		yCoords.push_back(center.y + radius * sin(startAngle));
		// end
		xCoords.push_back(center.x + radius * cos(endAngle));
		yCoords.push_back(center.y + radius * sin(endAngle));
        // 0 degree point (if exit)
		if (
            startAngle > endAngle // 弧线穿过0度处
            || std::fabs(endAngle - startAngle - 2 * PI) < BW_GEOMETRY_EPSILON //纯圆
            ) {
			xCoords.push_back(center.x + radius);
			yCoords.push_back(center.y);
		}
		// 90 degree point (if exit)
        if (
			isInArc(PI * 0.5) 
            || std::fabs(endAngle - startAngle - 2 * PI) < BW_GEOMETRY_EPSILON //纯圆
            ) {
			xCoords.push_back(center.x);
			yCoords.push_back(center.y + radius);

        }
		// 180 degree point (if exit)
        if (
            isInArc(PI)
            || std::fabs(endAngle - startAngle - 2 * PI) < BW_GEOMETRY_EPSILON //纯圆
            ) {
			xCoords.push_back(center.x - radius);
			yCoords.push_back(center.y);

        }
        // 270 degree point (if exit)
        if (
            isInArc(PI * 1.5)
            || std::fabs(endAngle - startAngle - 2 * PI) < BW_GEOMETRY_EPSILON //纯圆
            ) {
			xCoords.push_back(center.x);
			yCoords.push_back(center.y - radius);

        }
		// 计算最小和最大坐标
		double minX = *std::min_element(xCoords.begin(), xCoords.end());
		double minY = *std::min_element(yCoords.begin(), yCoords.end());
		double maxX = *std::max_element(xCoords.begin(), xCoords.end());
		double maxY = *std::max_element(yCoords.begin(), yCoords.end());

		return Rectangle(Point(minX, minY), Point(maxX, maxY));
		
    }
    Point Arc::getStartPoint() const {
        return Point(
            center.x + radius * cos(startAngle),
            center.y + radius * sin(startAngle)
        );
    };
    Point Arc::getEndPoint() const {
        return Point(
            center.x + radius * cos(endAngle),
            center.y + radius * sin(endAngle)
        );
    };
	Point Arc::getCenterPoint() const {
		return center;
	}

    
	double Arc::length() const {
		// 计算弧长
		double angleDifference = endAngle - startAngle;
		if (angleDifference < 0) {
			angleDifference += 2 * PI; // 确保角度差为正
		}
		return radius * angleDifference; // 弧长公式
	}
    Line::Line(const Point& b, const Point& e) : begin(b), end(e) {
        if (std::fabs(b.x - e.x) < BW_GEOMETRY_EPSILON && std::fabs(b.y - e.y) < BW_GEOMETRY_EPSILON) {
            throw std::invalid_argument("Line cannot have zero length.");
        }
    }
    Line::Line(const Point& begin, double angle, double length) :
		begin(begin) ,
        end(
            begin.x + length * cos(angle),
            begin.y + length * sin(angle)
        )
    {
		if (length < BW_GEOMETRY_EPSILON) {
			throw std::invalid_argument("Length must be positive.");
		}
		
    }
    void Line::printCADCommand() const {
		std::cout << "line "
			<< begin.x << "," << begin.y <<" "
			<< end.x << "," << end.y << " "
			<< std::endl << std::endl;
    }
    /*
	* Calculate the distance from a point to the line segment defined by this Line.
    * It calculates the distance from a point to an infinite straight line, 
    * not the distance from a point to a line segment.
    */
    double Line::distanceToPoint(const Point& p) const {
        double A = end.y - begin.y;
        double B = begin.x - end.x;
        double C = end.x * begin.y - begin.x * end.y;
        return std::abs(A * p.x + B * p.y + C) / std::sqrt(A * A + B * B);
    }
    Point Line::getPerpendicularFoot(const Point& p) const {
        double A = end.y - begin.y;
        double B = begin.x - end.x;
        double C = end.x * begin.y - begin.x * end.y;
        double denominator = A * A + B * B;
        if (denominator == 0) {
            throw std::runtime_error("Line has zero length, cannot calculate perpendicular foot.");
        }
        double x = (B * (B * p.x - A * p.y) - A * C) / denominator;
        double y = (A * (-B * p.x + A * p.y) - B * C) / denominator;
        return Point(x, y);
    }
    /*
    * 
    */
    double Line::angle() const {
		double angle = atan2(end.y - begin.y, end.x - begin.x);
        if (angle < 0) {
			angle += 2 * PI; 
        }
		return angle; // 返回弧度值
    }
    double Line::length() const {
        return std::sqrt((end.x - begin.x) * (end.x - begin.x) +
            (end.y - begin.y) * (end.y - begin.y));
    }
    bool Line::isOnLine(const Point& p) const {
        double crossProduct = (end.y - begin.y) * (p.x - begin.x) - (end.x - begin.x) * (p.y - begin.y);
        if (std::fabs(crossProduct) > BW_GEOMETRY_EPSILON) {
            return false; // 点不在直线上
        }
        double dotProduct = (p.x - begin.x) * (end.x - begin.x) + (p.y - begin.y) * (end.y - begin.y);
        return dotProduct >= 0 && dotProduct <= length() * length(); // 点在线段上
    }
    void Rectangle::printCADCommand() const {
		Line line1(bottomLeft, Point(topRight.x, bottomLeft.y));
		Line line2(Point(topRight.x, bottomLeft.y), topRight);
		Line line3(topRight, Point(bottomLeft.x, topRight.y));
		Line line4(Point(bottomLeft.x, topRight.y), bottomLeft);
		line1.printCADCommand();
		line2.printCADCommand();
		line3.printCADCommand();
		line4.printCADCommand();
    }

    std::vector<Point> getIntersectionPoints(
        const Line& line1,
        const Line& line2
    ) {
		const double EPS = BW_GEOMETRY_EPSILON; // 定义一个小的 BW_GEOMETRY_EPSILON 值用于浮点数比较
        double x1 = line1.begin.x, y1 = line1.begin.y;
        double x2 = line1.end.x, y2 = line1.end.y;
        double x3 = line2.begin.x, y3 = line2.begin.y;
        double x4 = line2.end.x, y4 = line2.end.y;

        double denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);

        // --- Case 1: 线段平行或共线 ---
        if (std::abs(denom) < EPS) {
            // 判断是否共线 (collinear)
            double cross_product_314 = (x1 - x3) * (y4 - y3) - (y1 - y3) * (x4 - x3);

            if (std::abs(cross_product_314) < EPS) {
                // 三点共线，说明两线段在同一条直线上
                // 现在检查它们的一维投影是否重叠

                // 计算线段1和线段2在X轴和Y轴上的投影范围
                double min_x1 = std::min(x1, x2), max_x1 = std::max(x1, x2);
                double min_y1 = std::min(y1, y2), max_y1 = std::max(y1, y2);
                double min_x2 = std::min(x3, x4), max_x2 = std::max(x3, x4);
                double min_y2 = std::min(y3, y4), max_y2 = std::max(y3, y4);

                // 检查投影是否有交集（这是你原来的代码，是正确的）
                bool x_overlap_possible = (min_x1 <= max_x2 + EPS) && (min_x2 <= max_x1 + EPS);
                bool y_overlap_possible = (min_y1 <= max_y2 + EPS) && (min_y2 <= max_y1 + EPS);

                if (x_overlap_possible && y_overlap_possible) {
                    // 投影有交集，现在区分是“重叠”还是“接触”
                    // 计算重叠区间的端点
                    Point overlap_start(std::max(min_x1, min_x2), std::max(min_y1, min_y2));
                    Point overlap_end(std::min(max_x1, max_x2), std::min(max_y1, max_y2));

                    // 计算重叠区间的“距离”或“长度”
                    // 如果重叠区间的起点和终点是同一个点（在EPS容差内），则为“接触”
                    if (overlap_start.distanceTo(overlap_end) < EPS) {
                        // 这是一个单点接触，行为与正常相交一致
                        return { overlap_start }; // 或者 overlap_end，它们是同一个点
                    }
                    else {
                        // 重叠区间的长度大于0，这是一个真正的重叠
                        // 此时抛出异常是合理的，因为交点有无限个
                        std::string message = "The two lines are collinear and overlap. The two lines are: Line1: ("
                            + std::to_string(x1) + ", " + std::to_string(y1) + ") to (" +
                            std::to_string(x2) + ", " + std::to_string(y2) + "), "
                            + "Line2: (" + std::to_string(x3) + ", " + std::to_string(y3) + ") to (" +
                            std::to_string(x4) + ", " + std::to_string(y4) + ")";
                        throw std::runtime_error(message);
                    }
                }
            }

            // 如果平行但不共线，或者共线但分离，都返回空
            return {};
        }

        // --- Case 2: 线段不平行，计算交点 ---
        double ua_num = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3));
        double ub_num = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3));

        double ua = ua_num / denom;
        double ub = ub_num / denom;

        // 使用带 BW_GEOMETRY_EPSILON 的比较来判断是否在线段上 [0, 1]
        if (ua >= -EPS && ua <= 1.0 + EPS && ub >= -EPS && ub <= 1.0 + EPS) {
            Point intersectionPoint(
                x1 + ua * (x2 - x1),
                y1 + ua * (y2 - y1)
            );
            return { intersectionPoint };
        }

        // 交点在延长线上，不属于线段交点
        return {};
    }
    std::vector<Point> getIntersectionPoints(
        const Line& line,
        const Arc& arc
    ) {
		double distance = line.distanceToPoint(arc.center);
        
		if (distance > arc.radius) {
			return {}; // 线段与圆弧没有交点
		}
        Point foot = line.getPerpendicularFoot(arc.center);
        
        
        
		if (distance < BW_GEOMETRY_EPSILON) {
			//线段长度为零，说明直线通过圆心
			std::vector<Point> intersectionPoints;
			double angle1 = line.angle();
			double angle2 = angle1 + PI; // 180度
			formatAngle(angle2);
			if (arc.isInArc(angle1)) {
                Point p(
                    arc.center.x + arc.radius * cos(angle1), 
                    arc.center.y + arc.radius * sin(angle1)
                );
                if (line.isOnLine(p)) {
					intersectionPoints.push_back(p);  
                }
				
			}
			if (arc.isInArc(angle2)) {
				Point p(
					arc.center.x + arc.radius * cos(angle2),
					arc.center.y + arc.radius * sin(angle2)
				);
				if (line.isOnLine(p)) {
					intersectionPoints.push_back(p);  
				}

			}
			return intersectionPoints; // 返回两个交点
		}

        Line perpendicularLine​(arc.center, foot);

        double perpendicularLine​Angle = perpendicularLine​.angle();
        if (std::fabs(distance - arc.radius) < BW_GEOMETRY_EPSILON) {
			// 线段与圆弧相切
			
			if (arc.isInArc(perpendicularLine​Angle)) {
				return { foot }; // 返回切点
			}
			else {
				return {}; // 切点不在圆弧上
			}
		}
        
		double halfChordAngle = acos(distance / arc.radius);// 半弦角
		double intersectionAngle1 = perpendicularLine​.angle() + halfChordAngle;
		double intersectionAngle2 = perpendicularLine​.angle() - halfChordAngle;
		formatAngle(intersectionAngle1);
		formatAngle(intersectionAngle2);
		std::vector<Point> intersectionPoints;
		if (arc.isInArc(intersectionAngle1)) {
            Point p(
                arc.center.x + arc.radius * cos(intersectionAngle1),
                arc.center.y + arc.radius * sin(intersectionAngle1)
            );
			if (line.isOnLine(p)) {
				intersectionPoints.push_back(p);
			}
			
		}
		if (arc.isInArc(intersectionAngle2)) {
            Point p(
                arc.center.x + arc.radius * cos(intersectionAngle2),
                arc.center.y + arc.radius * sin(intersectionAngle2)
            );
            if (line.isOnLine(p)) {
                intersectionPoints.push_back(p);
            }
		}
		return intersectionPoints;
		
        

    }
    std::vector<Point> getIntersectionPoints(
        const Arc& arc,
        const Line& line
    ) {
		// 直接调用上面的函数，参数顺序不同
		return getIntersectionPoints(line, arc);
    }
    std::vector<Point> getIntersectionPoints(
        const Arc& arc1,
        const Arc& arc2
    ) {
        std::vector<Point> finalIntersections;

        // --- Step 1: Find intersection points of the two full circles ---

        Point p1 = arc1.center;
        double r1 = arc1.radius;
        Point p2 = arc2.center;
        double r2 = arc2.radius;

        double d = distance(p1, p2);

        // 使用一个小的 BW_GEOMETRY_EPSILON 来处理浮点数比较
        const double EPS = BW_GEOMETRY_EPSILON;

        // Case 1: 圆心距离太远或太近，圆不相交
        // d > r1 + r2  (相离)
        // d < |r1 - r2| (内含)
        if (d > r1 + r2 + EPS || d < std::abs(r1 - r2) - EPS) {
            return {}; // 没有交点
        }

        // Case 2: 圆心重合
        if (d < EPS) {
            // Case 2.1: 半径不同，不可能相交
            if (std::abs(r1 - r2) > EPS) {
                return {};
            }

            // 从这里开始，我们确定两个圆弧同心且同半径

            // 为了检测重叠，我们需要规范化角度，使得 start < end
            // 这会让 isInArc 的判断更简单，但你的 isInArc 已经处理了 start > end 的情况，所以这一步可以省略。

            // 检查端点是否重合（单点接触）
            bool start1_eq_start2 = std::abs(arc1.startAngle - arc2.startAngle) < EPS;
            bool end1_eq_end2 = std::abs(arc1.endAngle - arc2.endAngle) < EPS;
            bool start1_eq_end2 = std::abs(arc1.startAngle - arc2.endAngle) < EPS;
            bool end1_eq_start2 = std::abs(arc1.endAngle - arc2.startAngle) < EPS;

            // Case 2.2: 两个圆弧完全相同
            if (start1_eq_start2 && end1_eq_end2) {
                throw std::runtime_error("The two arcs are identical and overlap completely.");
            }

            // Case 2.3: 两个圆弧在一个端点处相接
            if (start1_eq_end2) {
                // arc1的起点和arc2的终点重合
                // 还需要检查它们没有其他重叠部分，即方向相反
                double mid_arc1 = arc1.startAngle + 0.1; formatAngle(mid_arc1);
                double mid_arc2 = arc2.endAngle - 0.1;   formatAngle(mid_arc2);
                if (!arc2.isInArc(mid_arc1) && !arc1.isInArc(mid_arc2)) {
                    return { arc1.getStartPoint() };
                }
            }

            if (end1_eq_start2) {
                // arc1的终点和arc2的起点重合
                double mid_arc1 = arc1.endAngle - 0.1; formatAngle(mid_arc1);
                double mid_arc2 = arc2.startAngle + 0.1; formatAngle(mid_arc2);
                if (!arc2.isInArc(mid_arc1) && !arc1.isInArc(mid_arc2)) {
                    return { arc1.getEndPoint() };
                }
            }

            // Case 2.4: 检查是否存在线段重叠
            // 重叠的条件是：一个圆弧的某个端点，严格位于另一个圆弧的内部。
            // “严格位于内部”意味着它在弧上，但不是弧的端点。

            // 为了进行严格内部判断，我们需要一个 isInArc 的变体
            auto isStrictlyInsideArc = [EPS](const Arc& arc, const Point& p) {
                if (!arc.isOnArc(p)) return false;
                // 检查点p是否与arc的端点重合
                bool atStart = (distance(p, arc.getStartPoint()) < EPS);
                bool atEnd = (distance(p, arc.getEndPoint()) < EPS);
                return !atStart && !atEnd;
                };

            // 如果arc1的起点/终点严格位于arc2内部，或者反之，则存在重叠。
            if (isStrictlyInsideArc(arc2, arc1.getStartPoint()) ||
                isStrictlyInsideArc(arc2, arc1.getEndPoint()) ||
                isStrictlyInsideArc(arc1, arc2.getStartPoint()) ||
                isStrictlyInsideArc(arc1, arc2.getEndPoint()))
            {
                throw std::runtime_error("The two arcs overlap over a segment.");
            }

            // 如果通过了以上所有检查，说明两个圆弧虽然同心同半径，但角度范围是分离的。
            return {};
        }

        // --- 使用几何方法计算交点 ---

        // 'a' 是从圆心 p1 到两个交点连线（根轴）与 p1-p2 连线交点的距离
        // a = (r1^2 - r2^2 + d^2) / (2d)
        double a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);

        // 'h' 是根轴上交点到 p1-p2 连线的距离
        double h_squared = r1 * r1 - a * a;
        double h = (h_squared < 0) ? 0 : std::sqrt(h_squared);

        // 计算根轴与 p1-p2 连线的交点 P3
        Point p3;
        p3.x = p1.x + a * (p2.x - p1.x) / d;
        p3.y = p1.y + a * (p2.y - p1.y) / d;

        // 计算从 p3 到实际交点的偏移向量
        double offsetX = -(p2.y - p1.y) * (h / d);
        double offsetY = (p2.x - p1.x) * (h / d);

        // 得到两个潜在的交点
        Point i1 = { p3.x + offsetX, p3.y + offsetY };
        Point i2 = { p3.x - offsetX, p3.y - offsetY };

        // --- Step 2: Check if potential intersection points lie on BOTH arcs ---

        if (arc1.isOnArc(i1) && arc2.isOnArc(i1)) {
            finalIntersections.push_back(i1);
        }

        // 如果 h > 0，说明有两个不同的交点。如果 h=0（相切），i1 和 i2 是同一个点。
        if (h > EPS) {
            if (arc1.isOnArc(i2) && arc2.isOnArc(i2)) {
                finalIntersections.push_back(i2);
            }
        }

        return finalIntersections;
		
    }

    std::vector<Point> getIntersectionPointsNoEndPoints(
        const Line& line1,
        const Line& line2
    ) {
        // 1. 获取所有交点
        std::vector<Point> intersectionPoints = getIntersectionPoints(line1, line2);

        // 如果没有交点，直接返回
        if (intersectionPoints.empty()) {
            return {};
        }

        // 2. 使用 erase-remove idiom 一次性移除所有端点
        // remove_if 会将所有满足条件的元素（即端点）移动到向量末尾
        auto new_end = std::remove_if(intersectionPoints.begin(), intersectionPoints.end(),
            [&](const Point& p) {
                // 如果点 p 是任意一个端点，则返回 true (表示应该被移除)
                return p.isSame(line1.begin) || p.isSame(line1.end) ||
                    p.isSame(line2.begin) || p.isSame(line2.end);
            });

        // 3. 真正地从向量中删除这些元素
        intersectionPoints.erase(new_end, intersectionPoints.end());

        return intersectionPoints;
    }
    std::vector<Point> getIntersectionPointsNoEndPoints(
        const Arc& arc1,
        const Arc& arc2
    ) {
        // 1. 获取所有可能的交点（包括端点）
        std::vector<Point> intersectionPoints;
        try {
            intersectionPoints = getIntersectionPoints(arc1, arc2);
        }
        catch (const std::runtime_error& e) {
            // 如果原始函数因重叠而抛出异常，我们也应该将异常传递出去
            throw;
        }

        // 如果没有交点，直接返回
        if (intersectionPoints.empty()) {
            return {};
        }

        // 2. 准备所有端点以进行比较
        Point arc1_start = arc1.getStartPoint();
        Point arc1_end = arc1.getEndPoint();
        Point arc2_start = arc2.getStartPoint();
        Point arc2_end = arc2.getEndPoint();

        // 3. 使用 erase-remove idiom 移除所有是端点的交点
        auto new_end = std::remove_if(intersectionPoints.begin(), intersectionPoints.end(),
            [&](const Point& p) {
                // 如果点 p 与任意一个端点相同，则返回 true (表示应该被移除)
                return p.isSame(arc1_start) || p.isSame(arc1_end) ||
                    p.isSame(arc2_start) || p.isSame(arc2_end);
            });

        // 4. 真正地从向量中删除这些元素
        intersectionPoints.erase(new_end, intersectionPoints.end());

        return intersectionPoints;
    }
    /**
     * @brief Calculates the intersection points of a line and an arc, excluding
     * any intersections that occur at the endpoints of either geometry.
     *
     * @param line The line segment.
     * @param arc The arc.
     * @return A vector of points representing the interior intersections.
     */
    std::vector<Point> getIntersectionPointsNoEndPoints(
        const Line& line,
        const Arc& arc
    ) {
        // 1. 获取所有可能的交点（包括端点）
        std::vector<Point> intersectionPoints = getIntersectionPoints(line, arc);

        // 如果没有交点，直接返回
        if (intersectionPoints.empty()) {
            return {};
        }

        // 2. 准备所有端点以进行比较
        Point line_start = line.begin;
        Point line_end = line.end;
        Point arc_start = arc.getStartPoint();
        Point arc_end = arc.getEndPoint();

        // 3. 使用 erase-remove idiom 移除所有是端点的交点
        auto new_end = std::remove_if(intersectionPoints.begin(), intersectionPoints.end(),
            [&](const Point& p) {
                // 如果点 p 与任意一个端点相同，则返回 true (表示应该被移除)
                return p.isSame(line_start) || p.isSame(line_end) ||
                    p.isSame(arc_start) || p.isSame(arc_end);
            });

        // 4. 真正地从向量中删除这些元素
        intersectionPoints.erase(new_end, intersectionPoints.end());

        return intersectionPoints;
    }
    std::vector<Point> getIntersectionPointsNoEndPoints(
        const Arc& arc,
        const Line& line
    ) {
		return getIntersectionPointsNoEndPoints(line, arc);
    }

    void formatAngle(double& angle) {
        // 将角度转换为 [0, 2π) 范围
        while (angle < 0) {
            angle += 2 * PI;
        }
        while (angle >= 2 * PI) {
            angle -= 2 * PI;
        }
    }
    double distance(const Point& p1, const Point& p2) {
        return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }

}