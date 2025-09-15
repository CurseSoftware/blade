#include "resources/fs.h"

#include <cstring>
#include <filesystem>

#include "core/defines.h"
#include <fstream>
#include <optional>
#include <vector>

#include "core/logger.h"

namespace blade
{
    namespace resources
    {
        namespace fs
        {
            std::optional<file> file::from_path(const char* path, const file_mode mode) noexcept
            {
                file f{};
                const std::string normalized_path = normalize_path_(path);


                f._path = normalized_path;

                logger::info("Normalized: {}", f._path);

                f._handle = std::fstream(normalized_path, std::ios::in | std::ios::binary);

                if (!f._handle.good())
                {
                    logger::error("Invalid handle");
                    return std::nullopt;
                }

                return f;
            }

            bool file::open() noexcept
            {
                if (is_open())
                {
                    logger::warn("Attempting to open file that is already opened");
                    return false;
                }

                _is_valid = false;

                _handle.open(_path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

                if (_handle.is_open())
                {
                    logger::error("Handle did not open properly");
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

                if (!std::filesystem::exists(_path))
                {
                    logger::error("File \"{}\" does not exist", _path);
                    return std::nullopt;
                }

                constexpr std::streamoff offset = 0;

                _handle.seekg(offset, std::ios::end);
                const std::size_t file_size = _handle.tellg();
                _handle.seekg(offset, std::ios::beg);

                _contents.resize(file_size);

                std::streamsize total_bytes_read = 0;
                _handle.exceptions(std::fstream::badbit | std::fstream::failbit);

                while (total_bytes_read < file_size)
                {
                    try
                    {
                        usize to_read = std::min<usize>(4096, file_size - total_bytes_read);
                        logger::info("Reading {} bytes...", to_read);
                        auto position_before_read = _handle.tellg();
                        _handle.read((char*)_contents.data() + total_bytes_read, to_read);

                        auto bytes_just_read = _handle.gcount();
                        auto position_after_read = _handle.tellg();

                        logger::error("Position before: {}. Position after: {}", (usize)position_before_read,
                                      (usize)position_after_read);

                        if (_handle.rdstate() & std::ios_base::failbit)
                        {
                            logger::error("Read failed");
                        }

                        total_bytes_read += bytes_just_read;
                        logger::info("Read {} bytes, total: {}", bytes_just_read, total_bytes_read);
                        if (bytes_just_read == 0)
                        {
                            break;
                        }
                    }
                    catch (std::ios_base::failure& err)
                    {
                        logger::error("Error reading ({})", _path);
                        logger::error("Error category: {}", err.code().category().name());
                        logger::error("Error code: {} - {}", err.code().value(), err.code().message());
                        logger::error("Errno: {} - {}", errno, std::strerror(errno));
                        return std::nullopt;
                    }
                    catch (std::exception& e)
                    {
                        logger::error("Unknown error reading file \"{}\": ", _path, e.what());
                        return std::nullopt;
                    }

                    // if (_handle.fail())
                    // {
                    //     logger::error(
                    //         "Unexpected error while reading file \"{}\". Bytes read: {}. Bytes left: {}. Total file size: {}",
                    //         _path, bytes_read, try_bytes_to_read, file_size);
                    //     return std::nullopt;
                    // }
                }

                if (!_handle.good())
                {
                    logger::error("File not good after read: \"{}\"", _path);
                    return std::nullopt;
                }

                _is_complete = true;

                return _contents;
            }

            const char* file::get_error_() const noexcept
            {
                if (_handle.rdstate() & std::ios_base::failbit)
                {
                    return "Non-fatal I/O error";
                }

                if (_handle.rdstate() & std::ios_base::badbit)
                {
                    return "Fatal I/O error";
                }

                return "";
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
