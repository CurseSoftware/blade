#ifndef BLADE_RESOURCES_FS_H
#define BLADE_RESOURCES_FS_H

#include "core/types.h"
#include <fstream>
#include <ios>
#include <optional>
#include <vector>

namespace blade
{
    namespace resources
    {
        namespace fs
        {
            enum class file_mode
            {
                read,
                read_binaray,
                write,
                write_binaray,
                append,
                append_binary,
                read_write,
                read_write_binary
            };

            inline std::ios::openmode to_ios_openmode(const file_mode mode) noexcept
            {
                switch (mode)
                {
                    case file_mode::read:
                        return std::ios::in;
                    
                    case file_mode::write:
                        return std::ios::out;
                    
                    case file_mode::read_binaray:
                        return std::ios::binary | std::ios::in;
                    
                    case file_mode::write_binaray:
                        return std::ios::binary | std::ios::out;
                    
                    case file_mode::append:
                        return std::ios::out | std::ios::app;
                    
                    case file_mode::append_binary:
                        return std::ios::out | std::ios::app | std::ios::binary;
                    
                    case file_mode::read_write:
                        return std::ios::in | std::ios::out;
                    
                    case file_mode::read_write_binary:
                        return std::ios::in | std::ios::out | std::ios::binary;
                }
            }

            class file
            {
                public:
                    static std::optional<file> from_path(const char* path, const file_mode mode) noexcept;

                    bool open() noexcept;
                    void close() noexcept;
                    bool read_line() noexcept;
                    bool write_line() noexcept;

                    /**
                     * @brief Read the entire contents of the file. If file is already completely read, then returns those contents.
                     * @return `Some(std::reference_wrapper<std::vector<u8>>)` if read successfully. `std::nullopt` otherwise.
                     */
                    std::optional<std::reference_wrapper<std::vector<u8>>> read_all() noexcept;

                    bool is_open() const noexcept;

                    bool readable() const noexcept 
                    { 
                        return 
                            _mode == file_mode::read 
                            || _mode == file_mode::read_binaray
                            || _mode == file_mode::read_write 
                            || _mode == file_mode::read_write_binary
                        ;
                    }
                    
                    bool writeable() const noexcept
                    { 
                        return 
                            _mode == file_mode::write 
                            || _mode == file_mode::write_binaray
                            || _mode == file_mode::read_write 
                            || _mode == file_mode::read_write_binary
                        ;
                    }
                    
                    bool binary() const noexcept
                    { 
                        return 
                            _mode == file_mode::write_binaray
                            || _mode == file_mode::read_binaray
                            || _mode == file_mode::read_write_binary
                        ;
                    }
                    
                    bool appendable() const noexcept
                    { 
                        return 
                            _mode == file_mode::append
                            || _mode == file_mode::append_binary
                        ;
                    }
                
                private:
                    [[nodiscard]] explicit file() noexcept {}

                    static std::string normalize_path_(const std::string& path) noexcept;

                    std::fstream _handle;
                    std::string _path    {};
                    bool _is_valid       { false };
                    bool _is_complete    { false };
                    file_mode _mode;

                    std::vector<u8> _contents {};
            };
        } // fs namespace
    } // resources namespace
} // blade namespace

#endif // BLADE_RESOURCES_FS_H
