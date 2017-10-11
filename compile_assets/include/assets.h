#include <fstream> 

namespace assets {

#if defined(ASSETS_ROOT)
typedef std::ifstream istream_type;
#else 
typedef std::ifstream istream_type;
#endif

istream_type open(const char* path);
}