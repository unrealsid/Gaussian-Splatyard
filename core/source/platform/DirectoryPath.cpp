#include "platform/DirectoryPath.h"

namespace platform
{
    DirectoryPath::DirectoryPath()
    {
        rootPath = fs::current_path();
    }

    std::string DirectoryPath::get_path(const std::string& sub_path) const
    {
        fs::path full_path = rootPath / sub_path;

        if (!fs::exists(full_path))
        {
            throw std::runtime_error("File not found: " + full_path.string());
        }

        return full_path.string();
    }

    std::string DirectoryPath::get_shader_path(const std::string& shader_folder, const std::string& shader_name) const
    {
        return get_path("shaders/" + shader_folder + "/" + shader_name + ".spv");
    }

    bool DirectoryPath::exists(const std::string& subPath) const
    {
        return fs::exists(rootPath / subPath);
    }
}
