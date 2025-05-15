#include "code.h"

#if defined(_WIN32) || defined(_WIN64)
const char* get_hostname()
{
    TCHAR compute_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(compute_name) / sizeof(TCHAR);
    char ch_computer_name[MAX_COMPUTERNAME_LENGTH * 4 + 1];
    memset(ch_computer_name, 0, MAX_COMPUTERNAME_LENGTH * 4 + 1);
    if (!GetComputerNameW(compute_name, &size))
    {
        memcpy(ch_computer_name, "unkown", strlen("unkown"));
    }
    else
    {
        wchar_to_char(compute_name, ch_computer_name, MAX_COMPUTERNAME_LENGTH * 4 + 1, CP_ACP);
    }
    return ch_computer_name;
}

const char* gen_uuid_str()
{
    char e_ch[39];
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);
    if (SUCCEEDED(hr))
    {
        memset(e_ch, 0, 39);
        WCHAR guidString[39];
        hr = StringFromGUID2(&guid, guidString, sizeof(guidString) / sizeof(guidString[0]));
        if (FAILED(hr) || hr == 0)
        {
            return "";
        }
        wchar_to_char(guidString, e_ch, 39, CP_ACP);
    }
    // clear {}
    memset(uuid_ch, 0, 39);
    memcpy(uuid_ch, &e_ch[1], strlen(e_ch) - 2);
    return uuid_ch;
}

void format_path(char* path)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int len = strlen(path);
    char* swap = (char*)malloc(len * sizeof(char));
    int exist_counts = 0;
    for (i = 0;i < len - 1;i++)
    {
        if ((path[i] == '\\') && (path[i + 1] == '\\'))
        {
            path[i] = 0x00;
            exist_counts++;
        }
        swap[j++] = path[i];
    }
    if (exist_counts > 0)
    {
        for (i = 0;i < j;i++)
        {
            path[i] = swap[i];
        }
        for (i = j;i < len;i++)
        {
            path[i] = 0x00;
        }
    }
}

void _sleep_or_Sleep(int ms)
{
    Sleep(ms);
}


int char_to_wchar(const char* char_str, wchar_t* wchar_str, int w_len, int code)
{
    if (char_str == NULL)
    {
        return 0;
    }
    int size_needed = MultiByteToWideChar(code, 0, char_str, -1, NULL, 0);
    if (size_needed == 0 || w_len < size_needed)
    {
        return 0;
    }
    else
    {
        if (MultiByteToWideChar(code, 0, char_str, -1, wchar_str, size_needed) == 0)
        {
            return 0;
        }
        else
        {
            return size_needed;
        }
    }
}

int wchar_to_char(wchar_t* wchar_str, const char* char_str, int w_len, int code)
{
    if (wchar_str == NULL)
    {
        return 0;
    }
    int sizeNeeded = WideCharToMultiByte(code, 0, wchar_str, -1, NULL, 0, NULL, NULL);
    if (sizeNeeded == 0 || w_len < sizeNeeded)
    {
        return 0;
    }
    if (WideCharToMultiByte(code, 0, wchar_str, -1, char_str, sizeNeeded, NULL, NULL) == 0)
    {
        return 0;
    }
    return sizeNeeded;
}

int char_to_char(char* char_str1, char* char_str2, int c_len, int code1, int code2)
{
    if (char_str1 == NULL)
    {
        return 0;
    }
    int sizeNeeded = MultiByteToWideChar(code1, 0, char_str1, -1, NULL, 0);
    if (sizeNeeded == 0)
    {
        return 0;
    }
    wchar_t* wchar_swap = (wchar_t*)malloc(sizeNeeded * sizeof(wchar_t));
    if (MultiByteToWideChar(code1, 0, char_str1, -1, wchar_swap, sizeNeeded) == 0)
    {
        free(wchar_swap);
        return 0;
    }
    int char_sizeNeeded = WideCharToMultiByte(code2, 0, wchar_swap, -1, NULL, 0, NULL, NULL);
    if (char_sizeNeeded == 0)
    {
        return 0;
    }
    char* ch_swap = (char*)malloc(char_sizeNeeded * sizeof(char));
    if (WideCharToMultiByte(code2, 0, wchar_swap, -1, ch_swap, char_sizeNeeded, NULL, NULL) == 0)
    {
        free(wchar_swap);
        free(ch_swap);
        return 0;
    }
    if (c_len < char_sizeNeeded)
    {
        free(wchar_swap);
        free(ch_swap);
        return 0;
    }
    memcpy(char_str2, ch_swap, char_sizeNeeded);
    free(wchar_swap);
    free(ch_swap);
    return char_sizeNeeded;
}

#elif defined(__linux__)
// linux
void sprintf_s(char* buffer, size_t size_of_buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, size_of_buffer, format, args);
    va_end(args);
}

void strcat_s(char* dst_buf, size_t size_of_buffer, char* src_buf)
{
    strcat(dst_buf, src_buf);
}

void format_path(char* path)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int len = strlen(path);
    char* swap = (char*)malloc(len * sizeof(char));
    int exist_counts = 0;
    for (i = 0;i < len - 1;i++)
    {
        if ((path[i] == '/') && (path[i + 1] == '/'))
        {
            path[i] = 0x00;
            exist_counts++;
        }
        swap[j++] = path[i];
    }
    if (exist_counts > 0)
    {
        for (i = 0;i < j;i++)
        {
            path[i] = swap[i];
        }
        for (i = j;i < len;i++)
        {
            path[i] = 0x00;
        }
    }
}

const char* get_hostname()
{
    char hostname[256];
    memset(hostname, 0, 256);
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        printf("hostname=%s\n", hostname);
        //return hostname;
        return "localhost";
    }
    else
    {
        return "localhost";
    }
}

const char* gen_uuid_str()
{
    uuid_t uuid;
    //char uuid_str[37];
    uuid_generate(uuid);
    //printf("gene uuiid\n");
    uuid_unparse(uuid, uuid_ch);
    //printf("uuid=%s\n", uuid_str);
    uuid_ch[36] = 0x00;
    return uuid_ch;
    //return "12212";
}

void _sleep_or_Sleep(int ms)
{
    usleep(1000 * ms);
}
#else
//others
#endif


void trans_hex_to_ascii(char* ch_in, int len, char* ch_out)
{
    int i = 0;
    int t = 0;
    for (i = 0;i < len;i = i + 2)
    {
        if (ch_in[i] > 47 && ch_in[i] < 58)
        {
            t = (ch_in[i] - 48) * 16;
        }
        if (ch_in[i] > 96 && ch_in[i] < 103)
        {
            t = (ch_in[i] - 87) * 16;
        }
        if (ch_in[i] > 64 && ch_in[i] < 71)
        {
            t = (ch_in[i] - 55) * 16;
        }

        if (ch_in[i + 1] > 47 && ch_in[i + 1] < 58)
        {
            t = t + (ch_in[i + 1] - 48);
        }
        if (ch_in[i + 1] > 96 && ch_in[i + 1] < 103)
        {
            t = t + (ch_in[i + 1] - 87);
        }
        if (ch_in[i + 1] > 64 && ch_in[i + 1] < 71)
        {
            t = t + (ch_in[i + 1] - 55);
        }
        ch_out[i / 2] = t;
    }
}

void trans_ascii_to_hex(char* ch_in, int len, char* ch_out)
{
    int i = 0;
    int t = 0;
    int v_h = 0, v_l = 0;
    int trans_data = 0;
    for (i = 0;i < len;i++, t = t + 2)
    {
        trans_data = ch_in[i] >= 0 ? ch_in[i] : ch_in[i] + 256;
        v_h = trans_data / 16;
        v_l = trans_data % 16;
        if (v_h >= 0 && v_h <= 9)
        {
            ch_out[t] = 48 + v_h;
        }
        else
        {
            ch_out[t] = 87 + v_h;
        }
        if (v_l >= 0 && v_l <= 9)
        {
            ch_out[t + 1] = 48 + v_l;
        }
        else
        {
            ch_out[t + 1] = 87 + v_l;
        }
    }
}

int load_file_to_json(struct json_object** json_data, const char* file_path)
{
    FILE* file = fopen(file_path, "r");
    if (!file)
    {
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    memset(data, 0, length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';
    *json_data = json_tokener_parse(data);
    free(data);
    return 0;
}

int dump_json_to_file(struct json_object* json_data, const char* file_path)
{
    FILE* file = fopen(file_path, "w");
    if (!file)
    {
        return 1;
    }
    fwrite(json_object_to_json_string_ext(json_data, JSON_C_TO_STRING_PRETTY), strlen(json_object_to_json_string_ext(json_data, JSON_C_TO_STRING_PRETTY)), 1, file);
    fclose(file);
    return 0;
}

const char* trans_char_to_local(char* char_str)
{
#if defined(_WIN32) || defined(_WIN64)
    char ch_r[FILE_PATH_MAX_LEN];
    char_to_char(char_str, ch_r, FILE_PATH_MAX_LEN, CP_UTF8, CP_ACP);
    return ch_r;
#elif defined(__linux__)
    return char_str;
#else

#endif
}

const char* trans_char_to_utf8(char* char_str)
{
#if defined(_WIN32) || defined(_WIN64)
    char ch_r[FILE_PATH_MAX_LEN];
    char_to_char(char_str, ch_r, FILE_PATH_MAX_LEN, CP_ACP, CP_UTF8);
    return ch_r;
#elif defined(__linux__)
    return char_str;
#else

#endif
}

void os_char_to_utf8(char* char_str, char* ch_out)
{
#if defined(_WIN32) || defined(_WIN64)   
    char_to_char(char_str, ch_out, FILE_PATH_MAX_LEN, CP_ACP, CP_UTF8);
#elif defined(__linux__)
    sprintf_s(ch_out, FILE_PATH_MAX_LEN, "%s", char_str);
#else

#endif
}