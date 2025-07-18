#include "file_utils.h"
#include <stdexcept> // 用于抛出异常

namespace BridgeWind
{

    bool FileUtils::createFolder(const std::filesystem::path& path)
    {
        try {
            // create_directories 如果文件夹已存在，不会报错，这正是我们想要的
            return std::filesystem::create_directories(path);
        }
        catch (const std::filesystem::filesystem_error& e) {
            // 可以选择记录日志或直接抛出更具体的异常
            throw std::runtime_error("Failed to create directory: " + path.string() + " - " + e.what());
        }
    }

    bool FileUtils::copyFile(const std::filesystem::path& source,
        const std::filesystem::path& destination,
        bool overwrite)
    {
        if (!std::filesystem::exists(source)) {
            throw std::runtime_error("Source file does not exist: " + source.string());
        }

        // 确保目标文件夹存在
        if (destination.has_parent_path()) {
            createFolder(destination.parent_path());
        }

        auto options = overwrite ? std::filesystem::copy_options::overwrite_existing
            : std::filesystem::copy_options::none;

        try {
            return std::filesystem::copy_file(source, destination, options);
        }
        catch (const std::filesystem::filesystem_error& e) {
            throw std::runtime_error("Failed to copy file from " + source.string() +
                " to " + destination.string() + " - " + e.what());
        }
    }

} // namespace BridgeWind