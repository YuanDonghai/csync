#include "file_extend.h"

__int64 file_length_int64(const char* fname)
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

int file_md5_hex(const char* file_name, char* buf)
{
    FILE* fp = fopen(file_name, "rb");
    if (!fp)
    {
        return -1;
    }
    char ch_out[32];
    char ch_out_hex[64];
    memset(ch_out, 0, 32);
    md5_stream(fp, ch_out);
    fclose(fp);
    trans_ascii_to_hex(ch_out, 32, ch_out_hex);
    ch_out_hex[32] = 0;
    memcpy(buf, ch_out_hex, 33);
    return 0;
}

int file_check_exist(const char* fname)
{
    FILE* file = fopen(fname, "rb");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int file_create(const char* fname)
{
    FILE* file = fopen(fname, "wb");
    if (file)
    {
        fclose(file);
        return 0;
    }
    return 1;
}

int file_delete(const char* fname)
{
    FILE* file = fopen(fname, "rb");
    if (file)
    {
        fclose(file);
        return remove(fname);
    }
    return 0;
}

int read_file_to_buff(char* fname, char* data, long len)
{
    FILE* file = fopen(fname, "rb");
    if (!file)
    {
        return -1;
    }
    fseek(file, 0, SEEK_SET);
    fread(data, 1, len, file);
    fclose(file);
    return 0;
}

void modify_os_file_name(int os_type, char* fname)
{
    char ch[2] = "\\/";
    for (int i = 0;i < strlen(fname);i++)
    {
        if (fname[i] == ch[(os_type + 1) % 2])
        {
            fname[i] = ch[os_type % 2];
        }
    }
}


void format_file_name(char* fname)
{
    int i = 0;
    int start_index = 0;
    for (i = 0;i < strlen(fname);i++)
    {
        if (fname[i] == '\\' || fname[i] == '/')
        {
            continue;
        }
        else
        {
            start_index = i;
            break;
        }
    }
    if (start_index != 0)
    {
        int j = 0;
        for (i = start_index;i < strlen(fname);i++)
        {
            fname[j++] = fname[i];
        }
        fname[j] = 0;
    }
}

#if defined(_WIN32) || defined(_WIN64)
long file_get_modify_time(const char* fname)
{
    HANDLE hFile;
    FILETIME ftCreate, ftAccess, ftModify;
    SYSTEMTIME stUTC, stLocal;
    time_t fileTime;
    wchar_t wfilename[FILE_PATH_MAX_LEN];
    char_to_wchar(fname, wfilename, FILE_PATH_MAX_LEN, CP_ACP);
    hFile = CreateFile(wfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftModify))
    {
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    ULARGE_INTEGER uli;
    uli.LowPart = ftModify.dwLowDateTime;
    uli.HighPart = ftModify.dwHighDateTime;
    fileTime = (time_t)((uli.QuadPart - 116444736000000000ULL) / 10000000ULL);
    return fileTime;
}

int file_update_modify_time(const char* fname, time_t f_time)
{
    HANDLE hFile;
    FILETIME ftModify;
    ULARGE_INTEGER uli;
    uli.QuadPart = f_time * 10000000ULL + 116444736000000000ULL;
    ftModify.dwLowDateTime = uli.LowPart;
    ftModify.dwHighDateTime = uli.HighPart;

    wchar_t wfilename[FILE_PATH_MAX_LEN];
    char_to_wchar(fname, wfilename, FILE_PATH_MAX_LEN, CP_ACP);

    hFile = CreateFileW(wfilename, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    if (!SetFileTime(hFile, NULL, NULL, &ftModify))
    {
        CloseHandle(hFile);
        return 1;
    }

    CloseHandle(hFile);
    return 0;
}

int directory_create(const char* dname)
{
    DWORD attributes = GetFileAttributesA(dname);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        if (CreateDirectoryA(dname, NULL))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return 0;
    }
    return -1;
}

int directory_delete(const char* dname)
{
    wchar_t w_dirname[FILE_PATH_MAX_LEN];
    char_to_wchar(dname, w_dirname, FILE_PATH_MAX_LEN, CP_ACP);
    if (0 == file_type_check(w_dirname))
    {
        return !DeleteFile(w_dirname);
    }
    else
    {
        return directory_clean_file(w_dirname);
    }
}

int directory_clean_file(LPCTSTR dir_path)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR searchPath[FILE_PATH_MAX_LEN];
    TCHAR fullPath[FILE_PATH_MAX_LEN];

    _stprintf_s(searchPath, FILE_PATH_MAX_LEN, _T("%s\\*"), dir_path);
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    do
    {
        const TCHAR* name = findFileData.cFileName;
        if (_tcscmp(name, _T(".")) == 0 || _tcscmp(name, _T("..")) == 0)
        {
            continue;
        }

        _stprintf_s(fullPath, FILE_PATH_MAX_LEN, _T("%s\\%s"), dir_path, name);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            directory_clean_file(fullPath);
        }
        else
        {
            if (!DeleteFile(fullPath))
            {
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
    }

    FindClose(hFind);
    if (!RemoveDirectory(dir_path))
    {
        return -1;
    }

    return 0;
}

int file_type_check(wchar_t* path)
{
    DWORD attributes = GetFileAttributes(path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return -1;
    }
    else
    {
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

long os_update_time(long timel)
{
    struct tm tm_time;
    gmtime_s(&tm_time, &timel);

    SYSTEMTIME st;
    st.wYear = tm_time.tm_year + 1900;
    st.wMonth = tm_time.tm_mon + 1;
    st.wDay = tm_time.tm_mday;
    st.wHour = tm_time.tm_hour;
    st.wMinute = tm_time.tm_min;
    st.wSecond = tm_time.tm_sec;
    st.wMilliseconds = 0;

    if (!SetSystemTime(&st))
    {
        return 1;
    }

    return 0;
}

int get_time_adj_status()
{
    DWORD timeAdjustment = 0;
    DWORD timeIncrement = 0;
    BOOL isTimeAdjustmentDisabled = FALSE;
    int is_time_adj_s = 0;

    if (GetSystemTimeAdjustment(&timeAdjustment, &timeIncrement, &isTimeAdjustmentDisabled))
    {
        if (!isTimeAdjustmentDisabled)
        {
            is_time_adj_s = 1;
        }
        else
        {
            is_time_adj_s = 0;
        }
    }
    else
    {
        is_time_adj_s = 0;
    }
    return is_time_adj_s;
}

#elif defined(__linux__)
long file_get_modify_time(const char* fname)
{
    struct stat file_stat;

    if (stat(fname, &file_stat) == -1)
    {

        return 0;
    }

    time_t last_modified = file_stat.st_mtime;

    return last_modified;
}

int file_update_modify_time(const char* fname, time_t f_time)
{
    struct timespec times[2];
    times[0].tv_sec = 0;
    times[0].tv_nsec = UTIME_OMIT;

    times[1].tv_sec = f_time;
    times[1].tv_nsec = 0;

    if (utimensat(AT_FDCWD, fname, times, 0) == -1)
    {
        return 1;
    }

    return 0;
}

int directory_create(const char* dirname)
{
    if (mkdir(dirname, 0755) == -1)
    {
        return -1;
    }
    return 0;
}

int directory_delete(const char* dirname)
{
    return remove(dirname);
}

int file_type_check(const char* path)
{
    struct stat path_stat;
    if (lstat(path, &path_stat) < 0)
    {
        return -1;
    }
    if (S_ISREG(path_stat.st_mode))
    {
        return 0;
    }
    if (S_ISDIR(path_stat.st_mode))
    {
        return 1;
    }
}

long os_update_time(long timel)
{
    struct timeval tv;
    tv.tv_sec = timel;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) == -1)
    {
        return 1;
    }
    return 0;
}

int get_time_adj_status()
{
    FILE* fp = popen("timedatectl status | grep 'synchronized: yes'", "r");
    if (fp == NULL)
    {
        return 0;
    }

    char buffer[256];
    int is_synchronized = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (strstr(buffer, "synchronized: yes"))
        {
            is_synchronized = 1;
            break;
        }
    }
    pclose(fp);
    return is_synchronized;
}
#else
//others
#endif