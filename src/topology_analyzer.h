#ifndef TOPOLOGY_ANALYZER_H
#define TOPOLOGY_ANALYZER_H
#include "geometry.h"
#include <unordered_set>
namespace BridgeWind {
	class GraphNode;
	class GraphEdge;
    class Loop;

    class GraphNode {
    public:
        Point point;
        std::vector<GraphEdge*> edges;
        explicit GraphNode(const Point& p) : point(p) {};
        
    };

    class GraphEdge {
    public:
        GraphNode* startNode;
        GraphNode* endNode;
        enum class GeomType { LINE, ARC };
        GeomType type;
        union Primitive {
            const Line* line_ptr;
            const Arc* arc_ptr;
        } geometry;
        bool visited_forward = false;
        bool visited_backward = false;
        GraphEdge(GraphNode* s, GraphNode* e, const Line* l);

        GraphEdge(GraphNode* s, GraphNode* e, const Arc* a);
        
    };

    class Loop {
    private:
        bool properties_calculated;
        std::vector<const GraphNode*> nodes;
        std::vector<const GraphEdge*> edges;
        std::vector<double> lengthRatios;
        double length;
    public:
        explicit Loop(const std::vector<GraphEdge*>& orderedd_edges);
        double getArea() const;
        void print() const;
        double getLength() const;
        size_t segmentCount() const { return edges.size(); };
        const std::vector<const GraphNode*>& getNodes() const { return nodes; }
		const std::vector<const GraphEdge*>& getEdges() const { return edges; }
        const std::vector<double>& getLengthRatios() const;
    };

    class TopologyAnalyzer {
    public:
        explicit TopologyAnalyzer(const Geometry& geometry);
        void analyze();
        bool areAllElementsClosed() const;
        void printLoops() const;
		const Geometry& getSourceGeometry() const { return sourceGeometry; }
        const std::vector<std::unique_ptr<Loop>>& getLoops() const;
    private:
        const Geometry& sourceGeometry;
        std::map<Point, std::unique_ptr<GraphNode>, PointCmp> nodeMap;
        std::vector<std::unique_ptr<GraphEdge>> allEdges;
        std::vector<std::unique_ptr<Loop>> loops;
        // ... 其他成员 ...
    private:
        void buildGraph(); 
        void validateGraph() const;
        void findLoops();
        void buildHierarchy(); 
        GraphNode* findOrCreateNode(const Point& p);
        
        
    };

}



#endif // TOPOLOGY_ANALYZER_H