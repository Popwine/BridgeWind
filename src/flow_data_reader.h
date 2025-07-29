#pragma once

#ifndef FLOW_DATA_READER_H
#define FLOW_DATA_READER_H

#include <string>
#include <vector>
#include <filesystem>
#include <array>
#include "hdf5.h" // 仍然需要 HDF5 的核心头文件


namespace BridgeWind {

    // 使用 std::vector 和 std::array 的现代化 GridData 结构体
    // 这消除了手动内存管理的需要
    struct GridData {
        int grid_index = -1;
        std::array<hsize_t, 3> i_dims{}; // 固定大小的维度信息
        std::vector<double> coord_x;
        std::vector<double> coord_y;
        std::vector<double> coord_z;
        std::vector<int> cell_2_node_data;
    };

    class FtsGridDataReader {
    public:

        FtsGridDataReader(const std::string& workdir);

        ~FtsGridDataReader();

        // 主加载函数，将抛出 std::runtime_error 以报告错误
        void load();

        // 提供对加载数据的只读访问
        const std::vector<GridData>& getGrids() const;

    private:
        // C++ 风格的辅助函数，用于读取1D数据集并返回一个 vector
        // 使用模板以处理不同数据类型 (double, int, etc.)
        template<typename T>
        std::vector<T> readHdf5Dataset(hid_t group_id, const char* dataset_name);

        std::filesystem::path m_workdir;
        std::vector<GridData> m_grids; // 成员变量，用于存储所有网格分区的数据
    };
    struct FlowGroupData {
        int group_index = -1;
        int nTotalCell = 0;
        std::vector<double> q_data;
        hsize_t q_rows = 0;
        hsize_t q_cols = 0;

        // 方便地访问 q(row, col) 的元素
        double getQValue(hsize_t row, hsize_t col) const {
            if (row >= q_rows || col >= q_cols) {
                throw std::out_of_range("Q data access out of range for group " + std::to_string(group_index));
            }
            return q_data[row * q_cols + col];
        }
    };


    class FlowSolutionReader {
    public:
        explicit FlowSolutionReader(const std::string& workdir);
        ~FlowSolutionReader();

        // 主加载函数，会循环读取所有 Group
        void load();

        // --- 更新后的公共访问器 ---

        // 获取读取到的 Group 总数
        size_t getNumGroups() const;

        // 获取所有 Group 数据的只读引用
        const std::vector<FlowGroupData>& getFlowGroups() const;

        // 获取指定索引的 Group 数据的只读引用 (带边界检查)
        const FlowGroupData& getFlowGroup(size_t group_index) const;


    private:
        // --- 私有辅助函数 (无变化) ---
        template<typename T>
        T readScalarDataset(hid_t group_id, const char* dataset_name);

        template<typename T>
        std::vector<T> read2DDataset(hid_t group_id, const char* dataset_name, hsize_t& out_rows, hsize_t& out_cols);

        // --- 成员变量 ---
        std::filesystem::path m_workdir;

        // 将存储单个组的数据改为存储一个 FlowGroupData 的向量
        std::vector<FlowGroupData> m_flow_groups;
    };

    class FlowDataReader {

	public:
        FlowDataReader(const std::string& workdir);
		~FlowDataReader() {}
		// 主加载函数
		void load();
        const std::vector<GridData>&        getGridData(std::vector<GridData>& grids) const { return m_grid_reader.getGrids(); }
		const std::vector<FlowGroupData>&   getFlowGroups() const { return m_flow_reader.getFlowGroups(); }

        int getNumPoints() const;
		int getNumCells() const;

        double getPointX(int point_index) const;
        double getPointY(int point_index) const;
		const std::vector<std::vector<int>>& getCellConnectivity() const { return m_cell_connectivity; }
		const std::vector<double>& getPressure() const { return m_pressure; }
        const std::vector<double>& getVelocityX() const { return m_velocity_x; }
        const std::vector<double>& getVelocityY() const { return m_velocity_y; }
        const std::vector<double>& getVelocityMagnitude() const { return m_velocity_magnitude; }
        const std::vector<double>& getDensity() const { return m_density; }

    private:
		std::filesystem::path m_workdir;
		FtsGridDataReader m_grid_reader;
		FlowSolutionReader m_flow_reader;
        std::vector<std::vector<int>> m_cell_connectivity;
		int m_num_points = 0;
		int m_num_cells = 0;
		std::vector<double> m_pressure;
		std::vector<double> m_velocity_x;
		std::vector<double> m_velocity_y;
		std::vector<double> m_velocity_magnitude;
		std::vector<double> m_density;
        
    };

} // namespace BridgeWind



#endif // FLOW_DATA_READER_H