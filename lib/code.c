#include "code.h"

void TCHARToChar(const TCHAR* tcharStr, char* charStr, size_t charStrSize)
{
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, tcharStr, -1, charStr, (int)charStrSize, NULL, NULL);
#else
    strncpy(charStr, tcharStr, charStrSize - 1);
    charStr[charStrSize - 1] = '\0';
#endif
}

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


char* gen_node_id()
{
    char* ch_node_id = (char*)malloc(sizeof(char) * 39);
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);
    if (SUCCEEDED(hr))
    {
        memset(ch_node_id, 0, 39);
        WCHAR guidString[39];
        hr = StringFromGUID2(&guid, guidString, sizeof(guidString) / sizeof(guidString[0]));
        if (FAILED(hr) || hr == 0)
        {
            fprintf(stderr, "Failed to convert GUID to string\n");
        }
        TCHARToChar(guidString, ch_node_id, 39);
    }
    // clear {}
    char* ch_node_id_without_edge = (char*)malloc(sizeof(char) * 39);
    memset(ch_node_id_without_edge, 0, 39);
    memcpy(ch_node_id_without_edge, &ch_node_id[1], strlen(ch_node_id) - 2);
    return ch_node_id_without_edge;
}

void format_path(char* path)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int len = strlen(path);
    char* swap = (char*)malloc(len * sizeof(char));
    int exist_counts=0;
    for (i = 0;i < len - 1;i++)
    {
        if ((path[i] == '\\') && (path[i + 1] == '\\'))
        {
            path[i] = 0x00;
            exist_counts++;
        }
        swap[j++] = path[i];
    }
    if(exist_counts>0)
    {        
        for(i=0;i<j;i++)
        {
            path[i] = swap[i];
        }
        for(i=j;i<len;i++)
        {
            path[i]=0x00;
        }
    }
   

}