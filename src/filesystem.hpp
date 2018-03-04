#ifndef FAFNIR_FILESYSTEM_HPP
#define FAFNIR_FILESYSTEM_HPP

#ifdef __GNUC__
#   include <experimental/filesystem>
#else
#   include <filesytsem>
#endif
namespace fs = std::experimental::filesystem;
#endif //FAFNIR_FILESYSTEM_HPP
