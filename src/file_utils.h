#pragma once 

#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <filesystem>
#include <string>


namespace BridgeWind
{
	class FileUtils
	{
	public:
		// 禁止创建 FileUtils 的实例，因为它只包含静态成员
		FileUtils() = delete;
		static bool createFolder(const std::filesystem::path& path);

		// 复制文件
		// overwrite: 如果目标文件已存在，是否覆盖
		static bool copyFile(const std::filesystem::path& source,
			const std::filesystem::path& destination,
			bool overwrite = true);
	}
	;
}

#endif // FILE_UTILS_H