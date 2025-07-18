#include "topology_analyzer.h"
namespace BridgeWind{
    TopologyAnalyzer::TopologyAnalyzer(std::shared_ptr<BridgeWind::Geometry> geometry) :
		sourceGeometry(geometry)
    {

    }

    void TopologyAnalyzer::buildGraph() {
        nodeMap.clear();
        allEdges.clear();
        //loops.clear();
        //danglingEdges.clear();

        // 1. 遍历所有线段
        for (const auto& line : sourceGeometry->lines) {
            GraphNode* startNode = findOrCreateNode(line.begin);
            GraphNode* endNode = findOrCreateNode(line.end);

            if (startNode == endNode) continue;

            // 使用带参数的构造函数，并采用更安全的指针获取方式
            auto newEdge = std::make_unique<GraphEdge>(startNode, endNode, &line);
            GraphEdge* rawEdgePtr = newEdge.get();

            startNode->edges.push_back(rawEdgePtr);
            endNode->edges.push_back(rawEdgePtr);
            allEdges.push_back(std::move(newEdge));
        }

        // 2. 遍历所有圆弧
        for (const auto& arc : sourceGeometry->arcs) {
            Point startPoint = arc.getStartPoint();
            Point endPoint = arc.getEndPoint();
            //

            GraphNode* startNode = findOrCreateNode(startPoint);
            GraphNode* endNode = findOrCreateNode(endPoint);

            // 完整的圆在这里被过滤掉
            if (startNode == endNode) continue;

            auto newEdge = std::make_unique<GraphEdge>(startNode, endNode, &arc);
            GraphEdge* rawEdgePtr = newEdge.get();

            startNode->edges.push_back(rawEdgePtr);
            endNode->edges.push_back(rawEdgePtr);
            allEdges.push_back(std::move(newEdge));
        }
    }
    void TopologyAnalyzer::analyze() {
        if (sourceGeometry->isIntersectionExist()) {
			throw std::runtime_error("Geometry contains intersection points, \
                which is not allowed for topology analysis.");
        }
        // 1. 构建图的连接关系
        buildGraph();

        // 2. 验证图是否符合“所有节点度为2”的约束
        validateGraph(); // <--- 在这里调用

        // 后续步骤（暂时留空）
        findLoops();
        // buildHierarchy();
    }
    bool TopologyAnalyzer::areAllElementsClosed() const {
        // 如果图中没有任何节点，可以视为空的闭合图形
        if (nodeMap.empty()) {
            return true;
        }

        // 遍历所有节点，只要有一个度不为2，就不是闭合的环集合
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

    const std::vector<std::unique_ptr<Loop>>& TopologyAnalyzer::getLoops() const {
        return loops;
    }
    void TopologyAnalyzer::validateGraph() const{
        for (const auto& pair : nodeMap) {
            const GraphNode* node = pair.second.get(); // 获取原始指针

            // 检查节点的度（连接的边的数量）
            if (node->edges.size() != 2) {
                // 如果度不为2，说明图形不符合预期，抛出异常
                // 异常信息应尽可能详细，便于调试
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
        // 清空上一次分析的结果
        loops.clear();

        // 使用一个 set 来跟踪已经属于某个环的边，防止重复处理
        std::unordered_set<const GraphEdge*> visited_edges;

        // 遍历图中的每一条边，作为潜在新环的起点
        for (const auto& edge_ptr : allEdges) {
            const GraphEdge* start_edge = edge_ptr.get();

            // 如果这条边已经被包含在之前找到的环里了，就跳过
            if (visited_edges.count(start_edge)) {
                continue;
            }

            // --- 从 start_edge 开始，追踪一个新的环 ---
            std::vector<GraphEdge*> current_loop_edges;

            const GraphEdge* current_edge = start_edge;
            // 任意选择一个方向开始，比如从 startNode 出发
            GraphNode* current_node = start_edge->startNode;

            while (true) {
                // 将当前边加入到环的边列表和已访问集合中
                current_loop_edges.push_back(const_cast<GraphEdge*>(current_edge));
                visited_edges.insert(current_edge);

                // 确定下一个节点
                GraphNode* next_node = (current_edge->startNode == current_node)
                    ? current_edge->endNode
                    : current_edge->startNode;

                // 在下一个节点，找到除了我们刚来的那条边之外的唯一一条出路
                // 因为 validateGraph 已经确保了度为2，所以这个逻辑是安全的
                const GraphEdge* next_edge = (next_node->edges[0] == current_edge)
                    ? next_node->edges[1]
                    : next_node->edges[0];

                // 更新状态，为下一次循环做准备
                current_node = next_node;
                current_edge = next_edge;

                // 检查是否已经回到了追踪的起点
                if (current_edge == start_edge) {
                    // 环路闭合！
                    // 用收集到的有序边列表创建一个新的 Loop 对象
                    loops.push_back(std::make_unique<Loop>(current_loop_edges));

                    // 跳出 while 循环，结束当前环的追踪
                    break;
                }

                // 安全检查，防止因意外的图结构导致无限循环
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
		if (it != nodeMap.end()) {// 如果节点已存在，返回其指针
			return it->second.get(); 
		}
		else {// 如果节点不存在，创建一个新的节点并存储在 nodeMap 中
			auto newNode = std::make_unique<GraphNode>(p);
			GraphNode* nodePtr = newNode.get();
			nodeMap[p] = std::move(newNode);// 将新节点存储到 nodeMap 中 newNode变为了空指针，转移了所有权
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
        : properties_calculated(false) // 初始化缓存标志
    {
        // Loop 构造函数的主要职责有以下三点：
        // 存储边 : 接收并存储构成环的所有 GraphEdge 指针。
        // 排序节点 : 根据边的连接关系，推导出构成环的 GraphNode 的正确连续顺序，并存储它们。
        // 初始化属性 : 可以在构造时就计算并缓存一些基本属性，如包围盒，
        // 或者仅仅是将 properties_calculated 标志设为 false，以便在首次需要时再进行“懒加载”计算。
        if (loop_edges_from_find.empty()) {
            return; // 处理空输入
        }

        // --- 步骤 1: 准备工作 ---

        // 虽然输入是有序的，但我们只把它当成一个集合来用，以保证逻辑的独立性和健壮性。
        // 如果边的数量很少，线性搜索也可以。用 set 是为了理论上的高效。
        std::unordered_set<const GraphEdge*> edge_pool(
            loop_edges_from_find.begin(),
            loop_edges_from_find.end()
        );

        // --- 步骤 2: 确定一个明确的起点和起始方向 ---

        // 从输入列表的第一条边开始我们的追踪
        const GraphEdge* first_edge_in_path = loop_edges_from_find[0];

        // 任意选择一个方向作为路径的开始
        GraphNode* start_node = first_edge_in_path->startNode;
        GraphNode* next_node_after_start = first_edge_in_path->endNode;

        // --- 步骤 3: 循环追踪，构建有序的节点和边列表 ---

        // 初始化我们的有序列表
        this->nodes.push_back(start_node);

        GraphNode* current_node = start_node;
        const GraphEdge* current_edge = first_edge_in_path;

        // 我们需要循环 loop_edges_from_find.size() 次来走完整个环
        for (size_t i = 0; i < loop_edges_from_find.size(); ++i) {
            // 1. 将当前正在“走”的边，添加到我们自己的有序边列表中
            this->edges.push_back(current_edge);

            // 2. 确定路径的下一个节点
            GraphNode* next_node = (current_edge->startNode == current_node)
                ? current_edge->endNode
                : current_edge->startNode;

            // 3. 如果下一个节点不是路径的起点，就将它加入到有序节点列表中
            //    (最后一个节点会是起点，我们不在循环中添加它)
            if (next_node != start_node) {
                this->nodes.push_back(next_node);
            }

            // 4. 如果已经回到了起点，可以提前结束循环（作为一种校验）
            if (next_node == start_node) {
                // 确保我们已经用完了所有的边
                if (i == loop_edges_from_find.size() - 1) {
                    break; // 正常结束
                }
                else {
                    throw std::logic_error("Loop formed prematurely. Inconsistent edge set.");
                }
            }

            // 5. 在 next_node 处，找到下一条要走的边
            //    (必须是除了 current_edge 之外的那条)
            if (next_node->edges.size() != 2) {
                throw std::logic_error("Node degree is not 2 during loop construction.");
            }

            const GraphEdge* next_edge = (next_node->edges[0] == current_edge)
                ? next_node->edges[1]
                : next_node->edges[0];

            // 6. 检查这条边是否在我们被告知的边池中
            if (edge_pool.count(next_edge) == 0) {
                throw std::logic_error("Path tracing led to an edge not in the provided loop set.");
            }

            // 7. 更新状态，为下一次迭代做准备
            current_node = next_node;
            current_edge = next_edge;
        }

        // --- 步骤 4: 最终校验 ---
        if (this->nodes.size() != loop_edges_from_find.size() || this->edges.size() != loop_edges_from_find.size()) {
            throw std::logic_error("Failed to construct a valid loop. Node/edge count mismatch.");
        }
		// --- 步骤 5: 计算环的长度 ---
        double total_length = 0.0;
        for (const auto& edge : edges) {
            if (edge->type == GraphEdge::GeomType::LINE) {
                total_length += edge->geometry.line_ptr->length();
            }
            else if (edge->type == GraphEdge::GeomType::ARC) {
                total_length += edge->geometry.arc_ptr->length();
            }
            else {
                throw std::runtime_error("Unknown edge type in Loop::length()");
            }
        }
		length = total_length; // 计算并缓存环的总长度
		// --- 步骤 6: 计算长度比例 ---
        for (const auto& edge : edges) {
            double edgeLength = 0.0;
            if (edge->type == GraphEdge::GeomType::LINE) {
                edgeLength = edge->geometry.line_ptr->length();
            }
            else if (edge->type == GraphEdge::GeomType::ARC) {
                edgeLength = edge->geometry.arc_ptr->length();
            }
            else {
                throw std::runtime_error("Unknown edge type in GeoGenerator::generateGeoFileSingleLoop");
            }
            this->lengthRatios.push_back(edgeLength / this->getLength());
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
    double Loop::getLength() const {
		return this->length; // 这里假设 length 已经被正确计算并缓存
    }
    const std::vector<double>& Loop::getLengthRatios() const {
        return lengthRatios;
    }
}