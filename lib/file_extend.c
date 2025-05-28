#include "file_extend.h"

__int64 int64_get_file_length(const char* fname)
{
    FILE* file = fopen(fname, "rb");
    if (!file)
    {
        return -1;
    }
    __int64 file_size;
#ifdef _WIN32
    _fseeki64(file, 0, SEEK_END);
    file_size = _ftelli64(file);
#else
    fseeko(file, 0, SEEK_END);
    file_size = ftello(file);
#endif
    fclose(file);
    return file_size;
}

int get_file_md5(const char* file_name, char* buf)
{

}