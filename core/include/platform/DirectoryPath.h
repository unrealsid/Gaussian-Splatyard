#pragma once

#include <iostream>
#include <filesystem>
#include <string>

namespace platform
{
    namespace fs = std::filesystem;

    class DirectoryPath
    {
    public:
        static DirectoryPath& get()
        {
            static DirectoryPath instance;
            return instance;
        }

        [[nodiscard]] std::string get_path(const std::string& sub_path = "") const;
        [[nodiscard]] std::string get_shader_path(const std::string& shader_folder, const std::string& shader_name) const;

        bool exists(const std::string& subPath) const;

        DirectoryPath(const DirectoryPath&) = delete;
        void operator=(const DirectoryPath&) = delete;

    private:
        fs::path rootPath;

        DirectoryPath();
    };
};