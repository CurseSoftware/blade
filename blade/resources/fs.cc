#include "resources/fs.h"
#include "core/defines.h"
#include <fstream>
#include <optional>
#include <vector>

namespace blade
{
    namespace resources
    {
        namespace fs
        {
            std::optional<file> file::from_path(const char* path, const file_mode mode) noexcept
            {
                file f {};
                const std::string normalized_path = normalize_path_(path);


                f._path = normalized_path;

                f._handle = std::fstream(normalized_path);

                if (!f._handle.good())
                {
                    return std::nullopt;
                }

                return f;
            }

            bool file::open() noexcept
            {
                if (is_open())
                {
                    return false;
                }

                _is_valid = false;

                _handle.open(_path, to_ios_openmode(_mode));
                
                if (_handle.is_open())
                {
                    return false;
                }

                _is_valid = true;
                return true;
                
            }

            bool file::is_open() const noexcept
            {
                return _handle.is_open();
            }

            void file::close() noexcept
            {
                if (is_open())
                {
                    _handle.close();
                    _is_valid = false;
                }
            }

            std::optional<std::vector<u8>> file::read_all() noexcept
            {
                if (!is_open())
                {
                    return std::nullopt;
                }

                if (_is_complete)
                {
                    return std::ref(_contents);
                }

                std::streamoff offset = 0;

                _handle.seekg(offset, _handle.end);
                std::size_t file_size = _handle.tellg();
                _handle.seekg(offset, std::ios::beg);

                _contents.resize(file_size);

                _handle.read(reinterpret_cast<char*>(_contents.data()), file_size);
                _is_complete = true;

                return std::ref(_contents);
            }

            std::string file::normalize_path_(const std::string& path) noexcept
            {

                std::string normalized_path = path;

#ifdef BLADE_PLATFORM_WINDOWS
                for (char& c : normalized_path)
                {
                    if (c == '/')
                        c = '\\';
                }
#elif defined(BLADE_PLATFORM_LINUX)
                for (char& c : normalized_path)
                {
                    if (c == '\\')
                        c = '/';
                }
#endif
                return normalized_path;
            }
        } // fs namespace
    } // rerouces namespace
} // blade namespace
