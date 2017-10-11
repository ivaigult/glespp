
#include <assets.h>
#include <cassert>

namespace assets {

istream_type open(const char* path) {
    std::ifstream result(std::string(ASSETS_ROOT) + path);
    assert(result.good());
    return std::move(result);
}

}