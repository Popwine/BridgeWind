#include "flow_data_reader.h"
#include <iostream>
#include <regex>
#include <stdexcept> // for std::runtime_error
#include <functional> // for std::function
#include <utility> // for std::move
#include <fstream>

namespace { // 使用匿名命名空间来隐藏实现细节

    // 辅助函数，用于查找 FTS 文件 (与您原始代码相同)
    std::string findFtsFile(const std::filesystem::path& workdir) {
        std::filesystem::path gridPath = workdir / "grid";
        if (!std::filesystem::exists(gridPath)) {
            throw std::runtime_error("Grid directory does not exist: " + gridPath.string());
        }

        std::regex pattern(R"(Bridge_Wind__\d+_0\.fts)");
        for (const auto& entry : std::filesystem::directory_iterator(gridPath)) {
            if (entry.is_regular_file() && std::regex_match(entry.path().filename().string(), pattern)) {
                return entry.path().filename().string();
            }
        }
        throw std::runtime_error("No matching .fts file found in the grid directory.");
    }

    // RAII 包装器，用于自动管理 HDF5 句柄
    class H5Resource {
    public:
        // 构造函数获取句柄和一个关闭它的函数
        H5Resource(hid_t id, std::function<void(hid_t)> closer) : id_(id), closer_(closer) {}

        // 析构函数自动调用关闭函数
        ~H5Resource() {
            if (id_ >= 0) {
                closer_(id_);
            }
        }

        // 删除拷贝构造和赋值，确保资源所有权的唯一性
        H5Resource(const H5Resource&) = delete;
        H5Resource& operator=(const H5Resource&) = delete;

        // 实现移动构造和赋值
        H5Resource(H5Resource&& other) noexcept : id_(other.id_), closer_(std::move(other.closer_)) {
            other.id_ = -1; // 防止旧对象关闭句柄
        }
        H5Resource& operator=(H5Resource&& other) noexcept {
            if (this != &other) {
                if (id_ >= 0) closer_(id_);
                id_ = other.id_;
                closer_ = std::move(other.closer_);
                other.id_ = -1;
            }
            return *this;
        }

        // 允许将此对象像 hid_t 一样传递给 HDF5 C API 函数
        operator hid_t() const { return id_; }

    private:
        hid_t id_;
        std::function<void(hid_t)> closer_;
    };

    // 类型特性(trait)，用于将 C++ 类型映射到 HDF5 原生类型
    template<typename T> struct H5Type;
    template<> struct H5Type<double> { static hid_t id() { return H5T_NATIVE_DOUBLE; } };
    template<> struct H5Type<int> { static hid_t id() { return H5T_NATIVE_INT; } };
    template<> struct H5Type<hsize_t> { static hid_t id() { return H5T_NATIVE_HSIZE; } };

} // namespace


namespace BridgeWind {

    FtsGridDataReader::FtsGridDataReader(const std::string& workdir) : m_workdir(workdir) {}

    FtsGridDataReader::~FtsGridDataReader() {} // 析构函数现在为空，因为 vector 会自动清理内存

    const std::vector<GridData>& FtsGridDataReader::getGrids() const {
        return m_grids;
    }

    // 模板化的读取函数的实现
    template<typename T>
    std::vector<T> FtsGridDataReader::readHdf5Dataset(hid_t group_id, const char* dataset_name) {
        H5Resource dset(H5Dopen2(group_id, dataset_name, H5P_DEFAULT), H5Dclose);
        if (dset < 0) {
            throw std::runtime_error(std::string("Failed to open dataset: ") + dataset_name);
        }

        H5Resource space(H5Dget_space(dset), H5Sclose);
        int rank = H5Sget_simple_extent_ndims(space);
        if (rank < 0) {
            throw std::runtime_error(std::string("Failed to get rank for dataset: ") + dataset_name);
        }
        if (rank != 1) {
            throw std::runtime_error(std::string("Dataset is not 1-dimensional: ") + dataset_name);
        }

        hsize_t dim;
        H5Sget_simple_extent_dims(space, &dim, NULL);

        std::vector<T> data(dim); // 使用 vector 自动分配内存

        herr_t status = H5Dread(dset, H5Type<T>::id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
        if (status < 0) {
            throw std::runtime_error(std::string("Failed to read data from dataset: ") + dataset_name);
        }

        return data; // 返回 vector
    }


    void FtsGridDataReader::load() {
        m_grids.clear(); // 每次加载时清空旧数据

        try {
            std::string ftsFileName = findFtsFile(m_workdir);
            std::string ftsFilePath = (m_workdir / "grid" / ftsFileName).string();

            H5Resource file(H5Fopen(ftsFilePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
            if (file < 0) {
                throw std::runtime_error("Failed to open HDF5 file: " + ftsFilePath);
            }

            std::cout << "Successfully opened file: " << ftsFilePath << std::endl;

            int grid_count = 0;
            while (true) {
                std::string grid_group_name = "Grid-" + std::to_string(grid_count);

                if (H5Lexists(file, grid_group_name.c_str(), H5P_DEFAULT) <= 0) {
                    std::cout << "No more grids found. Total grids read: " << grid_count << std::endl;
                    break;
                }

                std::cout << "--- Reading group: " << grid_group_name << " ---" << std::endl;

                GridData current_grid; // 创建一个 grid 对象
                current_grid.grid_index = grid_count;

                H5Resource grid_group(H5Gopen2(file, grid_group_name.c_str(), H5P_DEFAULT), H5Gclose);
                if (grid_group < 0) {
                    throw std::runtime_error("Failed to open group: " + grid_group_name);
                }

                // --- 1. Read iDimensions ---
                {
                    H5Resource dset(H5Dopen2(grid_group, "iDimensions", H5P_DEFAULT), H5Dclose);
                    if (dset < 0) throw std::runtime_error("Could not open 'iDimensions' in " + grid_group_name);

                    herr_t status = H5Dread(dset, H5Type<hsize_t>::id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, current_grid.i_dims.data());
                    if (status < 0) throw std::runtime_error("Failed to read 'iDimensions' in " + grid_group_name);

                    printf("iDimensions: Nodes=%llu, Cells=%llu, Faces=%llu\n",
                        (unsigned long long)current_grid.i_dims[0],
                        (unsigned long long)current_grid.i_dims[1],
                        (unsigned long long)current_grid.i_dims[2]);
                }

                // --- 2. Read GridCoordinates ---
                {
                    H5Resource coord_group(H5Gopen2(grid_group, "GridCoordinates", H5P_DEFAULT), H5Gclose);
                    if (coord_group < 0) throw std::runtime_error("Could not open 'GridCoordinates' in " + grid_group_name);

                    current_grid.coord_x = readHdf5Dataset<double>(coord_group, "CoordinateX");
                    current_grid.coord_y = readHdf5Dataset<double>(coord_group, "CoordinateY");
                    current_grid.coord_z = readHdf5Dataset<double>(coord_group, "CoordinateZ");
                    printf("Read %zu nodes from GridCoordinates.\n", current_grid.coord_x.size());
                }

                // --- 3. Read CellTopology ---
                {
                    H5Resource topo_group(H5Gopen2(grid_group, "CellTopology", H5P_DEFAULT), H5Gclose);
                    if (topo_group < 0) throw std::runtime_error("Could not open 'CellTopology' in " + grid_group_name);

                    current_grid.cell_2_node_data = readHdf5Dataset<int>(topo_group, "cell2Node");
                    printf("Read %zu indices from cell2Node.\n", current_grid.cell_2_node_data.size());
                }

                m_grids.push_back(std::move(current_grid)); // 将数据移入成员变量
                grid_count++;
                printf("---------------------------\n\n");
            }

        }
        catch (const std::exception& e) {
            // 捕获所有标准异常并重新抛出，以便上层调用者可以处理
            std::cerr << "An error occurred during HDF5 file processing: " << e.what() << std::endl;
            throw;
        }
    }


    FlowSolutionReader::FlowSolutionReader(const std::string& workdir)
        : m_workdir(workdir) {
    }

    FlowSolutionReader::~FlowSolutionReader() {}

    // --- 公共访问器实现 ---
    size_t FlowSolutionReader::getNumGroups() const {
        return m_flow_groups.size();
    }

    const std::vector<FlowGroupData>& FlowSolutionReader::getFlowGroups() const {
        return m_flow_groups;
    }

    const FlowGroupData& FlowSolutionReader::getFlowGroup(size_t group_index) const {
        // 使用 .at() 会自动进行边界检查，如果越界会抛出 std::out_of_range
        return m_flow_groups.at(group_index);
    }

    // --- 私有辅助函数实现 (无变化) ---

    template<typename T>
    T FlowSolutionReader::readScalarDataset(hid_t group_id, const char* dataset_name) {
        H5Resource dset(H5Dopen2(group_id, dataset_name, H5P_DEFAULT), H5Dclose);
        if (dset < 0) throw std::runtime_error(std::string("Failed to open scalar dataset: ") + dataset_name);
        T value;
        if (H5Dread(dset, H5Type<T>::id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, &value) < 0) {
            throw std::runtime_error(std::string("Failed to read scalar dataset: ") + dataset_name);
        }
        return value;
    }

    template<typename T>
    std::vector<T> FlowSolutionReader::read2DDataset(hid_t group_id, const char* dataset_name, hsize_t& out_rows, hsize_t& out_cols) {
        H5Resource dset(H5Dopen2(group_id, dataset_name, H5P_DEFAULT), H5Dclose);
        if (dset < 0) throw std::runtime_error(std::string("Failed to open 2D dataset: ") + dataset_name);
        H5Resource space(H5Dget_space(dset), H5Sclose);
        if (H5Sget_simple_extent_ndims(space) != 2) throw std::runtime_error(std::string("Dataset '") + dataset_name + "' is not 2D.");
        hsize_t dims[2];
        H5Sget_simple_extent_dims(space, dims, NULL);
        out_rows = dims[0];
        out_cols = dims[1];
        std::vector<T> data(out_rows * out_cols);
        if (H5Dread(dset, H5Type<T>::id(), H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data()) < 0) {
            throw std::runtime_error(std::string("Failed to read 2D dataset: ") + dataset_name);
        }
        return data;
    }

    // --- 主加载函数实现 (已修改为循环读取) ---

    void FlowSolutionReader::load() {
        m_flow_groups.clear(); // 每次加载时清空旧数据

        try {
            

            std::string flowFilePath = (m_workdir / "results" / "flow_0.dat").string();
            H5Resource file(H5Fopen(flowFilePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
            if (file < 0) {
                
                throw std::runtime_error("Failed to open HDF5 flow file: " + flowFilePath);
                
            }
            
            std::cout << "Successfully opened flow file: " << flowFilePath << std::endl;

            int group_count = 0;
            while (true) {
                std::string group_name = "Group" + std::to_string(group_count);

                // 检查 Group 是否存在
                if (H5Lexists(file, group_name.c_str(), H5P_DEFAULT) <= 0) {
                    std::cout << "No more groups found. Total groups read: " << group_count << std::endl;
                    break; // 如果不存在，退出循环
                }

                std::cout << "--- Reading group: " << group_name << " ---" << std::endl;

                FlowGroupData current_group_data;
                current_group_data.group_index = group_count;

                // 打开当前 Group
                H5Resource current_group(H5Gopen2(file, group_name.c_str(), H5P_DEFAULT), H5Gclose);
                if (current_group < 0) {
                    throw std::runtime_error("Failed to open group: " + group_name);
                }

                // 读取 nTotalCell
                current_group_data.nTotalCell = readScalarDataset<int>(current_group, "nTotalCell");
                std::cout << "Read nTotalCell: " << current_group_data.nTotalCell << std::endl;

                // 读取 q 数据集
                current_group_data.q_data = read2DDataset<double>(current_group, "q",
                    current_group_data.q_rows,
                    current_group_data.q_cols);
                std::cout << "Read 'q' dataset with dimensions: " << current_group_data.q_rows
                    << " x " << current_group_data.q_cols << std::endl;

                // 将读取到的数据存入 vector
                m_flow_groups.push_back(std::move(current_group_data));

                group_count++;
                printf("---------------------------\n\n");
            }

        }
        catch (const std::exception& e) {
            std::cerr << "An error occurred while reading the flow data file: " << e.what() << std::endl;
            throw;
        }
    }


    FlowDataReader::FlowDataReader(const std::string& workdir)
        : m_workdir(workdir),
        m_grid_reader(workdir),
        m_flow_reader(workdir)

    {

    }

    void FlowDataReader::load() {
        m_flow_reader.load(); // 先加载流场数据
        m_grid_reader.load(); // 然后加载网格数据

        int n_grids = m_grid_reader.getGrids().size();
        int n_groups = m_flow_reader.getNumGroups();
        if (n_grids != n_groups) {
            throw std::runtime_error("Number of grids (" + std::to_string(n_grids) +
                ") does not match number of flow groups (" + std::to_string(n_groups) + ").");
        }
        if (n_grids == 0 || n_groups == 0) {
            throw std::runtime_error("No grids or flow groups found in the specified directory.");
        }

        std::cout << "Successfully loaded " << n_grids << " grids and " << n_groups << " flow groups." << std::endl;

        if (m_grid_reader.getGrids().empty()) {
            throw std::runtime_error("No grids available to determine number of points.");
        }


        m_num_points = 0;
        for (const auto& grid : m_grid_reader.getGrids()) {
            if (grid.coord_x.size() != grid.coord_y.size() || grid.coord_x.size() != grid.coord_z.size()) {
                throw std::runtime_error("Inconsistent coordinate sizes in grid data.");
            }
            m_num_points += static_cast<int>(grid.coord_x.size());

        }
        std::cout << "Total number of points across all grids: " << m_num_points << std::endl;

        
        if (m_grid_reader.getGrids().empty()) {
            throw std::runtime_error("No grids available to determine number of cells in fts file.");
        }
        if (m_flow_reader.getFlowGroups().empty()) {
            throw std::runtime_error("No grids available to determine number of cells in dat file.");
        }
        int num_cells_fts = 0;
        for (const auto& grid : m_grid_reader.getGrids()) {
            if (grid.cell_2_node_data.empty()) {
                throw std::runtime_error("No cell data found in grid.");
            }
            num_cells_fts += static_cast<int>(grid.cell_2_node_data.size() / 4); // 假设每个单元由4个节点组成
        }

        int num_cells_dat = 0;

        for (const auto& group : m_flow_reader.getFlowGroups()) {
            if (group.nTotalCell <= 0) {
                throw std::runtime_error("nTotalCell <= 0");
            }
            num_cells_dat += group.nTotalCell;

        }
        if (num_cells_dat != num_cells_fts) {
            throw std::runtime_error("Cell number form fts doesn't match that from dat. num_cells_fts: " + std::to_string(num_cells_fts)
                + ", num_cells_dat: " + std::to_string(num_cells_dat) + ". ");
        }




        std::cout << "Total number of cells across all grids: " << num_cells_fts << std::endl;
		m_num_cells = num_cells_fts; // 设置成员变量

		// parse connectivity data
		m_cell_connectivity.clear(); // 清空旧数据
        int group_start_point_index = 0;
		for (const auto& grid : m_grid_reader.getGrids()) {
			if (grid.cell_2_node_data.empty()) {
				throw std::runtime_error("No cell data found in grid.");
			}
			if (grid.coord_x.size() != grid.coord_y.size() || grid.coord_x.size() != grid.coord_z.size()) {
				throw std::runtime_error("Inconsistent coordinate sizes in grid data.");
			}
			if (grid.cell_2_node_data.size() % 4 != 0) {
				throw std::runtime_error("Cell to node data size is not a multiple of 4.");
			}
			int n_cells = static_cast<int>(grid.cell_2_node_data.size() / 4); // 假设每个单元由4个节点组成
			for (int i = 0; i < n_cells; ++i) {
				std::vector<int> cell_nodes;
				for (int j = 0; j < 4; ++j) { // 假设每个单元由4个节点组成
					cell_nodes.push_back(group_start_point_index + grid.cell_2_node_data[i * 4 + j]);
				}
				m_cell_connectivity.push_back(std::move(cell_nodes));
			}

            group_start_point_index += grid.coord_x.size();

		}
		std::cout << "Total number of cells in connectivity data: " << m_cell_connectivity.size() << std::endl;


        // parse rho u v w p
		m_pressure.clear();
		m_velocity_magnitude.clear();
		m_velocity_x.clear();
		m_velocity_y.clear();
		m_density.clear();
		for (const auto& group : m_flow_reader.getFlowGroups()) {
			if (group.q_rows <= 0 || group.q_cols <= 0) {
				throw std::runtime_error("Invalid dimensions for q data in group " + std::to_string(group.group_index));
			}
			if (group.q_cols < 5) {
				throw std::runtime_error("q data does not contain enough columns for velocity and pressure.");
			}
			// 假设 q 数据的列顺序为 [rho, u, v, w, p]
			for (size_t i = 0; i < group.nTotalCell; ++i) {
				//std::cout << "Processing cell " << i << " in group " << group.group_index << std::endl;
				m_density.push_back(group.q_data[i ]);
				m_velocity_x.push_back(group.q_data[i + group.q_cols]);
				m_velocity_y.push_back(group.q_data[i + group.q_cols * 2]);
				m_velocity_magnitude.push_back(std::sqrt(
					group.q_data[i + group.q_cols] * group.q_data[i + group.q_cols] +
					group.q_data[i + group.q_cols * 2] * group.q_data[i + group.q_cols * 2]));
				m_pressure.push_back(group.q_data[i + group.q_cols * 4]);

			}
		}
		std::cout << "Loaded pressure and velocity data for " << m_pressure.size() << " points." << std::endl;

    }

    int FlowDataReader::getNumPoints() const {
        
        return m_num_points;
    }

    int FlowDataReader::getNumCells() const {
        
        return m_num_cells;


    }
    double FlowDataReader::getPointX(int point_index) const {
        if (point_index < 0 || point_index >= m_num_points) {
            throw std::out_of_range("Point index out of range: " + std::to_string(point_index));
        }
		for (int i = 0; i < m_grid_reader.getGrids().size(); ++i) {
			const auto& grid = m_grid_reader.getGrids()[i];
			if (point_index < static_cast<int>(grid.coord_x.size())) {
				return grid.coord_x[point_index];
			}
			point_index -= static_cast<int>(grid.coord_x.size());
		}
		
    }
    double FlowDataReader::getPointY(int point_index) const {
        if (point_index < 0 || point_index >= m_num_points) {
            throw std::out_of_range("Point index out of range: " + std::to_string(point_index));
        }
        for (int i = 0; i < m_grid_reader.getGrids().size(); ++i) {
            const auto& grid = m_grid_reader.getGrids()[i];
            if (point_index < static_cast<int>(grid.coord_y.size())) {
                return grid.coord_y[point_index];
            }
            point_index -= static_cast<int>(grid.coord_y.size());
        }
    }
}// namespace BridgeWind