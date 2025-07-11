#include "topology_analyzer.h"
namespace BridgeWind{
    TopologyAnalyzer::TopologyAnalyzer(const Geometry& geometry) :
		sourceGeometry(geometry)
    {

    }

    void TopologyAnalyzer::buildGraph() {
        nodeMap.clear();
        allEdges.clear();
        //loops.clear();
        //danglingEdges.clear();

        // 1. ���������߶�
        for (const auto& line : sourceGeometry.lines) {
            GraphNode* startNode = findOrCreateNode(line.begin);
            GraphNode* endNode = findOrCreateNode(line.end);

            if (startNode == endNode) continue;

            // ʹ�ô������Ĺ��캯���������ø���ȫ��ָ���ȡ��ʽ
            auto newEdge = std::make_unique<GraphEdge>(startNode, endNode, &line);
            GraphEdge* rawEdgePtr = newEdge.get();

            startNode->edges.push_back(rawEdgePtr);
            endNode->edges.push_back(rawEdgePtr);
            allEdges.push_back(std::move(newEdge));
        }

        // 2. ��������Բ��
        for (const auto& arc : sourceGeometry.arcs) {
            Point startPoint = arc.getStartPoint();
            Point endPoint = arc.getEndPoint();
            //

            GraphNode* startNode = findOrCreateNode(startPoint);
            GraphNode* endNode = findOrCreateNode(endPoint);

            // ������Բ�����ﱻ���˵�
            if (startNode == endNode) continue;

            auto newEdge = std::make_unique<GraphEdge>(startNode, endNode, &arc);
            GraphEdge* rawEdgePtr = newEdge.get();

            startNode->edges.push_back(rawEdgePtr);
            endNode->edges.push_back(rawEdgePtr);
            allEdges.push_back(std::move(newEdge));
        }
    }
    void TopologyAnalyzer::analyze() {
        if (sourceGeometry.isIntersectionExist()) {
			throw std::runtime_error("Geometry contains intersection points, \
                which is not allowed for topology analysis.");
        }
        // 1. ����ͼ�����ӹ�ϵ
        buildGraph();

        // 2. ��֤ͼ�Ƿ���ϡ����нڵ��Ϊ2����Լ��
        validateGraph(); // <--- ���������

        // �������裨��ʱ���գ�
        findLoops();
        // buildHierarchy();
    }
    bool TopologyAnalyzer::areAllElementsClosed() const {
        // ���ͼ��û���κνڵ㣬������Ϊ�յıպ�ͼ��
        if (nodeMap.empty()) {
            return true;
        }

        // �������нڵ㣬ֻҪ��һ���Ȳ�Ϊ2���Ͳ��ǱպϵĻ�����
        for (const auto& pair : nodeMap) {
            if (pair.second->edges.size() != 2) {
                return false;
            }
        }

        return true;
    }

    void TopologyAnalyzer::printLoops() const {
        if (loops.empty()) {
            std::cout << "No loops found." << std::endl;
            return;
        }
        std::cout << "Found " << loops.size() << " loops:" << std::endl;
        for (const auto& loop : loops) {
            loop->print();
        }
    }
    void TopologyAnalyzer::validateGraph() const{
        for (const auto& pair : nodeMap) {
            const GraphNode* node = pair.second.get(); // ��ȡԭʼָ��

            // ���ڵ�Ķȣ����ӵıߵ�������
            if (node->edges.size() != 2) {
                // ����Ȳ�Ϊ2��˵��ͼ�β�����Ԥ�ڣ��׳��쳣
                // �쳣��ϢӦ��������ϸ�����ڵ���
                std::string error_message = "Topology validation failed: Node at (" +
                    std::to_string(node->point.x) + ", " +
                    std::to_string(node->point.y) +
                    ") has " + std::to_string(node->edges.size()) +
                    " connected edges, but exactly 2 were expected.";
                throw std::runtime_error(error_message);
            }
        }
    }
    void TopologyAnalyzer::findLoops() {
        // �����һ�η����Ľ��
        loops.clear();

        // ʹ��һ�� set �������Ѿ�����ĳ�����ıߣ���ֹ�ظ�����
        std::unordered_set<const GraphEdge*> visited_edges;

        // ����ͼ�е�ÿһ���ߣ���ΪǱ���»������
        for (const auto& edge_ptr : allEdges) {
            const GraphEdge* start_edge = edge_ptr.get();

            // ����������Ѿ���������֮ǰ�ҵ��Ļ����ˣ�������
            if (visited_edges.count(start_edge)) {
                continue;
            }

            // --- �� start_edge ��ʼ��׷��һ���µĻ� ---
            std::vector<GraphEdge*> current_loop_edges;

            const GraphEdge* current_edge = start_edge;
            // ����ѡ��һ������ʼ������� startNode ����
            GraphNode* current_node = start_edge->startNode;

            while (true) {
                // ����ǰ�߼��뵽���ı��б���ѷ��ʼ�����
                current_loop_edges.push_back(const_cast<GraphEdge*>(current_edge));
                visited_edges.insert(current_edge);

                // ȷ����һ���ڵ�
                GraphNode* next_node = (current_edge->startNode == current_node)
                    ? current_edge->endNode
                    : current_edge->startNode;

                // ����һ���ڵ㣬�ҵ��������Ǹ�����������֮���Ψһһ����·
                // ��Ϊ validateGraph �Ѿ�ȷ���˶�Ϊ2����������߼��ǰ�ȫ��
                const GraphEdge* next_edge = (next_node->edges[0] == current_edge)
                    ? next_node->edges[1]
                    : next_node->edges[0];

                // ����״̬��Ϊ��һ��ѭ����׼��
                current_node = next_node;
                current_edge = next_edge;

                // ����Ƿ��Ѿ��ص���׷�ٵ����
                if (current_edge == start_edge) {
                    // ��·�պϣ�
                    // ���ռ�����������б���һ���µ� Loop ����
                    loops.push_back(std::make_unique<Loop>(current_loop_edges));

                    // ���� while ѭ����������ǰ����׷��
                    break;
                }

                // ��ȫ��飬��ֹ�������ͼ�ṹ��������ѭ��
                if (visited_edges.size() > allEdges.size()) {
                    throw std::runtime_error("Loop finding error: Graph structure is inconsistent.");
                }
            }
        }
    }
	GraphEdge::GraphEdge(GraphNode* s, GraphNode* e, const Line* l)
		: startNode(s), endNode(e), type(GeomType::LINE) {
		geometry.line_ptr = l;
	}

	GraphEdge::GraphEdge(GraphNode* s, GraphNode* e, const Arc* a)
		: startNode(s), endNode(e), type(GeomType::ARC) {
		geometry.arc_ptr = a;
	}
	GraphNode* TopologyAnalyzer::findOrCreateNode(const Point& p) {
		auto it = nodeMap.find(p);
		if (it != nodeMap.end()) {// ����ڵ��Ѵ��ڣ�������ָ��
			return it->second.get(); 
		}
		else {// ����ڵ㲻���ڣ�����һ���µĽڵ㲢�洢�� nodeMap ��
			auto newNode = std::make_unique<GraphNode>(p);
			GraphNode* nodePtr = newNode.get();
			nodeMap[p] = std::move(newNode);// ���½ڵ�洢�� nodeMap �� newNode��Ϊ�˿�ָ�룬ת��������Ȩ
			return nodePtr;
		}
	}
    /**
     * @brief Constructs a Loop from an ordered list of edges.
     *
     * This constructor takes a sequence of connected edges that form a closed loop
     * and correctly deduces the ordered sequence of nodes that define the loop's path.
     *
     * @param ordered_edges A vector of GraphEdge pointers, which must be connected
     *                      sequentially to form a single closed loop.
     */
    Loop::Loop(const std::vector<GraphEdge*>& loop_edges_from_find)
        : properties_calculated(false) // ��ʼ�������־
    {
        // Loop ���캯������Ҫְ�����������㣺
        // �洢�� : ���ղ��洢���ɻ������� GraphEdge ָ�롣
        // ����ڵ� : ���ݱߵ����ӹ�ϵ���Ƶ������ɻ��� GraphNode ����ȷ����˳�򣬲��洢���ǡ�
        // ��ʼ������ : �����ڹ���ʱ�ͼ��㲢����һЩ�������ԣ����Χ�У�
        // ���߽����ǽ� properties_calculated ��־��Ϊ false���Ա����״���Ҫʱ�ٽ��С������ء����㡣
        if (loop_edges_from_find.empty()) {
            return; // ���������
        }

        // --- ���� 1: ׼������ ---

        // ��Ȼ����������ģ�������ֻ��������һ���������ã��Ա�֤�߼��Ķ����Ժͽ�׳�ԡ�
        // ����ߵ��������٣���������Ҳ���ԡ��� set ��Ϊ�������ϵĸ�Ч��
        std::unordered_set<const GraphEdge*> edge_pool(
            loop_edges_from_find.begin(),
            loop_edges_from_find.end()
        );

        // --- ���� 2: ȷ��һ����ȷ��������ʼ���� ---

        // �������б�ĵ�һ���߿�ʼ���ǵ�׷��
        const GraphEdge* first_edge_in_path = loop_edges_from_find[0];

        // ����ѡ��һ��������Ϊ·���Ŀ�ʼ
        GraphNode* start_node = first_edge_in_path->startNode;
        GraphNode* next_node_after_start = first_edge_in_path->endNode;

        // --- ���� 3: ѭ��׷�٣���������Ľڵ�ͱ��б� ---

        // ��ʼ�����ǵ������б�
        this->nodes.push_back(start_node);

        GraphNode* current_node = start_node;
        const GraphEdge* current_edge = first_edge_in_path;

        // ������Ҫѭ�� loop_edges_from_find.size() ��������������
        for (size_t i = 0; i < loop_edges_from_find.size(); ++i) {
            // 1. ����ǰ���ڡ��ߡ��ıߣ���ӵ������Լ���������б���
            this->edges.push_back(current_edge);

            // 2. ȷ��·������һ���ڵ�
            GraphNode* next_node = (current_edge->startNode == current_node)
                ? current_edge->endNode
                : current_edge->startNode;

            // 3. �����һ���ڵ㲻��·������㣬�ͽ������뵽����ڵ��б���
            //    (���һ���ڵ������㣬���ǲ���ѭ���������)
            if (next_node != start_node) {
                this->nodes.push_back(next_node);
            }

            // 4. ����Ѿ��ص�����㣬������ǰ����ѭ������Ϊһ��У�飩
            if (next_node == start_node) {
                // ȷ�������Ѿ����������еı�
                if (i == loop_edges_from_find.size() - 1) {
                    break; // ��������
                }
                else {
                    throw std::logic_error("Loop formed prematurely. Inconsistent edge set.");
                }
            }

            // 5. �� next_node �����ҵ���һ��Ҫ�ߵı�
            //    (�����ǳ��� current_edge ֮�������)
            if (next_node->edges.size() != 2) {
                throw std::logic_error("Node degree is not 2 during loop construction.");
            }

            const GraphEdge* next_edge = (next_node->edges[0] == current_edge)
                ? next_node->edges[1]
                : next_node->edges[0];

            // 6. ����������Ƿ������Ǳ���֪�ı߳���
            if (edge_pool.count(next_edge) == 0) {
                throw std::logic_error("Path tracing led to an edge not in the provided loop set.");
            }

            // 7. ����״̬��Ϊ��һ�ε�����׼��
            current_node = next_node;
            current_edge = next_edge;
        }

        // --- ���� 4: ����У�� ---
        if (this->nodes.size() != loop_edges_from_find.size() || this->edges.size() != loop_edges_from_find.size()) {
            throw std::logic_error("Failed to construct a valid loop. Node/edge count mismatch.");
        }
    }
    void Loop::print() const{
		std::cout << "Loop with " << edges.size() << " edges:" << std::endl;
        int i = 1;
        for (const auto& edge : edges) {
			std::cout << "edge " << i << ": ";
			std::cout << "(" << edge->startNode->point.x << ", " << edge->startNode->point.y << ") to ";
			std::cout << "(" << edge->endNode->point.x << ", " << edge->endNode->point.y << ")";
            std::cout << std::endl;

			i++;
        }
    }
}