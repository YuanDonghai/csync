#include "diff.h"
/*void rdiff_usage(const char* error, ...)
{
    va_list va = NULL;
    char buf[256];

    va_start(va, error);
    vsnprintf(buf, sizeof(buf), error, va);
    va_end(va);
    fprintf(stderr, "rdiff: %s\n\nTry `rdiff --help' for more information.\n",
        buf);
}*/
rs_result rdiff_sig(const char* basis_name, const char* sig_name)
{
    FILE* basis_file, * sig_file;
    rs_stats_t stats;
    rs_result result;
    rs_magic_number sig_magic;

    char* rs_hash_name = NULL;
    char* rs_rollsum_name = NULL;
    int block_len = 0;
    int strong_len = 0;
    int show_stats = 0;
    int file_force = 0;
    __int64 basis_file_len = diff_get_file_length(basis_name);
    basis_file = rs_file_open(basis_name, "rb", file_force);
    sig_file = rs_file_open(sig_name, "wb", file_force);

    // set block size
   // fseek(basis_file, 0, SEEK_END);

    //fseek(basis_file, 0, SEEK_SET);

    if (basis_file_len < LEVEL_FILE_SIZE_4K)
    {
        block_len = 128 * 1024;
    }
    else
    {
        if (basis_file_len < LEVEL_FILE_SIZE_4M)
        {
            block_len = 1024 * 1024;
        }
        else
        {
            if (basis_file_len < LEVEL_FILE_SIZE_64M)
            {
                block_len = 4 * 1024 * 1024;
            }
            else
            {
                if (basis_file_len < LEVEL_FILE_SIZE_512M)
                {
                    block_len = 8 * 1024 * 1024;
                }
                else
                {
                    if (basis_file_len < LEVEL_FILE_SIZE_1G)
                    {
                        block_len = 16 * 1024 * 1024;
                    }
                    else
                    {
                        block_len = 64 * 1024 * 1024;
                    }
                }
            }
        }
    }
    //block_len = 128 * 1024;
    if (!rs_hash_name || !strcmp(rs_hash_name, "blake2"))
    {
        sig_magic = RS_BLAKE2_SIG_MAGIC;
    }
    else if (!strcmp(rs_hash_name, "md4"))
    {
        sig_magic = RS_MD4_SIG_MAGIC;
    }
    else
    {
        s_log(LOG_ERROR, "Unknown hash algorithm '%s'.", rs_hash_name);
        rs_file_close(sig_file);
        rs_file_close(basis_file);
        return RS_SYNTAX_ERROR;
    }
    if (!rs_rollsum_name || !strcmp(rs_rollsum_name, "rabinkarp"))
    {
        /* The RabinKarp magics are 0x10 greater than the rollsum magics. */
        sig_magic += 0x10;
    }
    else if (strcmp(rs_rollsum_name, "rollsum"))
    {
        s_log(LOG_ERROR, "Unknown rollsum algorithm '%s'.", rs_rollsum_name);
        rs_file_close(sig_file);
        rs_file_close(basis_file);
        return RS_SYNTAX_ERROR;
    }

    result =
        rs_sig_file(basis_file, sig_file, block_len, strong_len, sig_magic, &stats);

    rs_file_close(sig_file);
    rs_file_close(basis_file);
    if (result != RS_DONE)
        return result;

    if (show_stats)
        rs_log_stats(&stats);

    return result;
}

rs_result rdiff_delta(const char* newfile_name, const char* sig_name, const char* delta_name)
{
    FILE* sig_file, * new_file, * delta_file;
    int show_stats = 0;
    int file_force = 0;
    rs_result result;
    rs_signature_t* sumset;
    rs_stats_t stats;

    sig_file = rs_file_open(sig_name, "rb", file_force);
    new_file = rs_file_open(newfile_name, "rb", file_force);
    delta_file = rs_file_open(delta_name, "wb", file_force);

    result = rs_loadsig_file(sig_file, &sumset, &stats);
    if (result != RS_DONE)
    {
        rs_file_close(delta_file);
        rs_file_close(new_file);
        rs_file_close(sig_file);
        return result;
    }

    if (show_stats)
        rs_log_stats(&stats);

    if ((result = rs_build_hash_table(sumset)) != RS_DONE)
    {
        rs_file_close(delta_file);
        rs_file_close(new_file);
        rs_file_close(sig_file);
        return result;
    }

    result = rs_delta_file(sumset, new_file, delta_file, &stats);

    rs_file_close(delta_file);
    rs_file_close(new_file);
    rs_file_close(sig_file);

    if (show_stats)
    {
        rs_signature_log_stats(sumset);
        rs_log_stats(&stats);
    }

    rs_free_sumset(sumset);

    return result;
}

rs_result rdiff_patch(const char* basis_name, const char* delta_name, const char* new_filename)
{
    /* patch BASIS [DELTA [NEWFILE]] */
    FILE* basis_file, * delta_file, * new_file;
    rs_stats_t stats;
    rs_result result;
    int show_stats = 0;
    int file_force = 0;
    basis_file = rs_file_open(basis_name, "rb", file_force);
    delta_file = rs_file_open(delta_name, "rb", file_force);
    new_file = rs_file_open(new_filename, "wb", file_force);

    result = rs_patch_file(basis_file, delta_file, new_file, &stats);

    rs_file_close(new_file);
    rs_file_close(delta_file);
    rs_file_close(basis_file);

    if (show_stats)
        rs_log_stats(&stats);

    return result;
}


int64_t diff_get_file_length(char* fname)
{
    FILE* file = fopen(fname, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
#if defined(_WIN32) || defined(_WIN64)
    if (_fseeki64(file, 0, SEEK_END) != 0) {
        perror("Failed to seek file");
        fclose(file);
        return -1;
    }

    __int64 fileSize = _ftelli64(file);
    if (fileSize == -1) {
        perror("Failed to tell file size");
    }
#elif defined(__linux__)
    // linux
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek file");
        fclose(file);
        return -1;
    }

    __int64 fileSize = ftell(file);
    if (fileSize == -1) {
        perror("Failed to tell file size");
    }
#else
    //others
#endif


    fclose(file);
    return fileSize;
}