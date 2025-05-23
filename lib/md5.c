#include "md5.h"

/* Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
void* md5_buffer(const char* buffer, size_t len, void* resblock)
{
    struct md5_ctx ctx;

    /* Initialize the computation context.  */
    md5_init_ctx(&ctx);

    /* Process whole buffer but last len % 64 bytes.  */
    md5_process_bytes(buffer, len, &ctx);

    /* Put result in desired memory area.  */
    return md5_finish_ctx(&ctx, resblock);
}

/* Compute MD5 message digest for bytes read from STREAM.  The
   resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
int md5_stream(FILE* stream, void* resblock)
{
    //char* buffer = malloc(BLOCKSIZE + 72);
    char buffer[BLOCKSIZE + 72];
    /*
    if (!buffer)
        return 1;
    */
    struct md5_ctx ctx;
    md5_init_ctx(&ctx);
    size_t sum;

    /* Iterate over full file contents.  */
    while (1)
    {
        /* We read the file in blocks of BLOCKSIZE bytes.  One call of the
           computation function processes the whole buffer so that with the
           next round of the loop another block can be read.  */
        size_t n;
        sum = 0;

        /* Read block.  Take care for partial reads.  */
        while (1)
        {
            /* Either process a partial fread() from this loop,
               or the fread() in afalg_stream may have gotten EOF.
               We need to avoid a subsequent fread() as EOF may
               not be sticky.  For details of such systems, see:
               https://sourceware.org/bugzilla/show_bug.cgi?id=1190  */
            if (feof(stream))
                goto process_partial_block;

            n = fread(buffer + sum, 1, BLOCKSIZE - sum, stream);

            sum += n;

            if (sum == BLOCKSIZE)
                break;

            if (n == 0)
            {
                /* Check for the error flag IFF N == 0, so that we don't
                   exit the loop after a partial read due to e.g., EAGAIN
                   or EWOULDBLOCK.  */
                if (ferror(stream))
                {
                    // free(buffer);
                    return 1;
                }
                goto process_partial_block;
            }
        }

        /* Process buffer with BLOCKSIZE bytes.  Note that
           BLOCKSIZE % 64 == 0
         */
        md5_process_block(buffer, BLOCKSIZE, &ctx);
    }

process_partial_block:

    /* Process any remaining bytes.  */
    if (sum > 0)
        md5_process_bytes(buffer, sum, &ctx);

    /* Construct result in desired memory.  */
    md5_finish_ctx(&ctx, resblock);
    // free(buffer);
    return 0;
}

/* Initialize structure containing state of computation.
   (RFC 1321, 3.3: Step 3)  */
void md5_init_ctx(struct md5_ctx* ctx)
{
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;

    ctx->total[0] = ctx->total[1] = 0;
    ctx->buflen = 0;
}

/* Put result from CTX in first 16 bytes following RESBUF.  The result
   must be in little endian byte order.  */
void* md5_read_ctx(const struct md5_ctx* ctx, void* resbuf)
{
    char* r = resbuf;
    set_uint32(r + 0 * sizeof ctx->A, SWAP(ctx->A));
    set_uint32(r + 1 * sizeof ctx->B, SWAP(ctx->B));
    set_uint32(r + 2 * sizeof ctx->C, SWAP(ctx->C));
    set_uint32(r + 3 * sizeof ctx->D, SWAP(ctx->D));

    return resbuf;
}

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.  */
void* md5_finish_ctx(struct md5_ctx* ctx, void* resbuf)
{
    /* Take yet unprocessed bytes into account.  */
    uint32_t bytes = ctx->buflen;
    size_t size = (bytes < 56) ? 64 / 4 : 64 * 2 / 4;

    /* Now count remaining bytes.  */
    ctx->total[0] += bytes;
    if (ctx->total[0] < bytes)
        ++ctx->total[1];

    /* Put the 64-bit file length in *bits* at the end of the buffer.  */
    ctx->buffer[size - 2] = SWAP(ctx->total[0] << 3);
    ctx->buffer[size - 1] = SWAP((ctx->total[1] << 3) | (ctx->total[0] >> 29));

    memcpy(&((char*)ctx->buffer)[bytes], fillbuf, (size - 2) * 4 - bytes);

    /* Process last bytes.  */
    md5_process_block(ctx->buffer, size * 4, ctx);

    return md5_read_ctx(ctx, resbuf);
}

/* Process LEN bytes of BUFFER, accumulating context into CTX.
   It is assumed that LEN % 64 == 0.  */

void md5_process_block(const void* buffer, size_t len, struct md5_ctx* ctx)
{
    uint32_t correct_words[16];
    const uint32_t* words = buffer;
    size_t nwords = len / sizeof(uint32_t);
    const uint32_t* endp = words + nwords;
    uint32_t A = ctx->A;
    uint32_t B = ctx->B;
    uint32_t C = ctx->C;
    uint32_t D = ctx->D;
    uint32_t lolen = len;

    /* First increment the byte count.  RFC 1321 specifies the possible
       length of the file up to 2^64 bits.  Here we only compute the
       number of bytes.  Do a double word increment.  */
    ctx->total[0] += lolen;
    ctx->total[1] += (len >> 31 >> 1) + (ctx->total[0] < lolen);

    /* Process all bytes in the buffer with 64 bytes in each round of
       the loop.  */
    while (words < endp)
    {
        uint32_t* cwp = correct_words;
        uint32_t A_save = A;
        uint32_t B_save = B;
        uint32_t C_save = C;
        uint32_t D_save = D;

        /* First round: using the given function, the context and a constant
           the next context is computed.  Because the algorithms processing
           unit is a 32-bit word and it is determined to work on words in
           little endian byte order we perhaps have to change the byte order
           before the computation.  To reduce the work for the next steps
           we store the swapped words in the array CORRECT_WORDS.  */

#define OP(a, b, c, d, s, T)                                            \
      do                                                                \
        {                                                               \
          a += FF (b, c, d) + (*cwp++ = SWAP (*words)) + T;             \
          ++words;                                                      \
          CYCLIC (a, s);                                                \
          a += b;                                                       \
        }                                                               \
      while (0)

           /* It is unfortunate that C does not provide an operator for
              cyclic rotation.  Hope the C compiler is smart enough.  */
#define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

              /* Before we start, one word to the strange constants.
                 They are defined in RFC 1321 as

                 T[i] = (int) (4294967296.0 * fabs (sin (i))), i=1..64

                 Here is an equivalent invocation using Perl:

                 perl -e 'foreach(1..64){printf "0x%08x\n", int (4294967296 * abs (sin $_))}'
               */

               /* Round 1.  */
        OP(A, B, C, D, 7, 0xd76aa478);
        OP(D, A, B, C, 12, 0xe8c7b756);
        OP(C, D, A, B, 17, 0x242070db);
        OP(B, C, D, A, 22, 0xc1bdceee);
        OP(A, B, C, D, 7, 0xf57c0faf);
        OP(D, A, B, C, 12, 0x4787c62a);
        OP(C, D, A, B, 17, 0xa8304613);
        OP(B, C, D, A, 22, 0xfd469501);
        OP(A, B, C, D, 7, 0x698098d8);
        OP(D, A, B, C, 12, 0x8b44f7af);
        OP(C, D, A, B, 17, 0xffff5bb1);
        OP(B, C, D, A, 22, 0x895cd7be);
        OP(A, B, C, D, 7, 0x6b901122);
        OP(D, A, B, C, 12, 0xfd987193);
        OP(C, D, A, B, 17, 0xa679438e);
        OP(B, C, D, A, 22, 0x49b40821);

        /* For the second to fourth round we have the possibly swapped words
           in CORRECT_WORDS.  Redefine the macro to take an additional first
           argument specifying the function to use.  */
#undef OP
#define OP(f, a, b, c, d, k, s, T)                                      \
      do                                                                \
        {                                                               \
          a += f (b, c, d) + correct_words[k] + T;                      \
          CYCLIC (a, s);                                                \
          a += b;                                                       \
        }                                                               \
      while (0)

           /* Round 2.  */
        OP(FG, A, B, C, D, 1, 5, 0xf61e2562);
        OP(FG, D, A, B, C, 6, 9, 0xc040b340);
        OP(FG, C, D, A, B, 11, 14, 0x265e5a51);
        OP(FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
        OP(FG, A, B, C, D, 5, 5, 0xd62f105d);
        OP(FG, D, A, B, C, 10, 9, 0x02441453);
        OP(FG, C, D, A, B, 15, 14, 0xd8a1e681);
        OP(FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
        OP(FG, A, B, C, D, 9, 5, 0x21e1cde6);
        OP(FG, D, A, B, C, 14, 9, 0xc33707d6);
        OP(FG, C, D, A, B, 3, 14, 0xf4d50d87);
        OP(FG, B, C, D, A, 8, 20, 0x455a14ed);
        OP(FG, A, B, C, D, 13, 5, 0xa9e3e905);
        OP(FG, D, A, B, C, 2, 9, 0xfcefa3f8);
        OP(FG, C, D, A, B, 7, 14, 0x676f02d9);
        OP(FG, B, C, D, A, 12, 20, 0x8d2a4c8a);

        /* Round 3.  */
        OP(FH, A, B, C, D, 5, 4, 0xfffa3942);
        OP(FH, D, A, B, C, 8, 11, 0x8771f681);
        OP(FH, C, D, A, B, 11, 16, 0x6d9d6122);
        OP(FH, B, C, D, A, 14, 23, 0xfde5380c);
        OP(FH, A, B, C, D, 1, 4, 0xa4beea44);
        OP(FH, D, A, B, C, 4, 11, 0x4bdecfa9);
        OP(FH, C, D, A, B, 7, 16, 0xf6bb4b60);
        OP(FH, B, C, D, A, 10, 23, 0xbebfbc70);
        OP(FH, A, B, C, D, 13, 4, 0x289b7ec6);
        OP(FH, D, A, B, C, 0, 11, 0xeaa127fa);
        OP(FH, C, D, A, B, 3, 16, 0xd4ef3085);
        OP(FH, B, C, D, A, 6, 23, 0x04881d05);
        OP(FH, A, B, C, D, 9, 4, 0xd9d4d039);
        OP(FH, D, A, B, C, 12, 11, 0xe6db99e5);
        OP(FH, C, D, A, B, 15, 16, 0x1fa27cf8);
        OP(FH, B, C, D, A, 2, 23, 0xc4ac5665);

        /* Round 4.  */
        OP(FI, A, B, C, D, 0, 6, 0xf4292244);
        OP(FI, D, A, B, C, 7, 10, 0x432aff97);
        OP(FI, C, D, A, B, 14, 15, 0xab9423a7);
        OP(FI, B, C, D, A, 5, 21, 0xfc93a039);
        OP(FI, A, B, C, D, 12, 6, 0x655b59c3);
        OP(FI, D, A, B, C, 3, 10, 0x8f0ccc92);
        OP(FI, C, D, A, B, 10, 15, 0xffeff47d);
        OP(FI, B, C, D, A, 1, 21, 0x85845dd1);
        OP(FI, A, B, C, D, 8, 6, 0x6fa87e4f);
        OP(FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
        OP(FI, C, D, A, B, 6, 15, 0xa3014314);
        OP(FI, B, C, D, A, 13, 21, 0x4e0811a1);
        OP(FI, A, B, C, D, 4, 6, 0xf7537e82);
        OP(FI, D, A, B, C, 11, 10, 0xbd3af235);
        OP(FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
        OP(FI, B, C, D, A, 9, 21, 0xeb86d391);

        /* Add the starting values of the context.  */
        A += A_save;
        B += B_save;
        C += C_save;
        D += D_save;
    }

    /* Put checksum in context given as argument.  */
    ctx->A = A;
    ctx->B = B;
    ctx->C = C;
    ctx->D = D;
}

void md5_process_bytes(const void* buffer, size_t len, struct md5_ctx* ctx)
{
    /* When we already have some bits in our internal buffer concatenate
       both inputs first.  */
    if (ctx->buflen != 0)
    {
        size_t left_over = ctx->buflen;
        size_t add = 128 - left_over > len ? len : 128 - left_over;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, add);
        ctx->buflen += add;

        if (ctx->buflen > 64)
        {
            md5_process_block(ctx->buffer, ctx->buflen & ~63, ctx);

            ctx->buflen &= 63;
            /* The regions in the following copy operation cannot overlap,
               because ctx->buflen < 64 ≤ (left_over + add) & ~63.  */
            memcpy(ctx->buffer,
                &((char*)ctx->buffer)[(left_over + add) & ~63],
                ctx->buflen);
        }

        buffer = (const char*)buffer + add;
        len -= add;
    }

    /* Process available complete blocks.  */
    if (len >= 64)
    {
        {
            md5_process_block(buffer, len & ~63, ctx);
            buffer = (const char*)buffer + (len & ~63);
            len &= 63;
        }
    }

    /* Move remaining bytes in internal buffer.  */
    if (len > 0)
    {
        size_t left_over = ctx->buflen;

        memcpy(&((char*)ctx->buffer)[left_over], buffer, len);
        left_over += len;
        if (left_over >= 64)
        {
            md5_process_block(ctx->buffer, 64, ctx);
            left_over -= 64;
            /* The regions in the following copy operation cannot overlap,
               because left_over ≤ 64.  */
            memcpy(ctx->buffer, &ctx->buffer[16], left_over);
        }
        ctx->buflen = left_over;
    }
}

/* Copy the 4 byte value from v into the memory location pointed to by *cp,
   If your architecture allows unaligned access this is equivalent to
   * (uint32_t *) cp = v  */
static void set_uint32(char* cp, uint32_t v)
{
    memcpy(cp, &v, sizeof v);
}


int check_file_with_md5(const char* file_name, char* md5_buf)
{
    // check sum
    FILE* file_checksum = fopen(file_name, "rb");
    if (!file_checksum)
    {
        return -1;
    }
    char ch_out[32];
    char ch_out_hex[64];
    memset(ch_out, 0, 32);
    md5_stream(file_checksum, ch_out);
    fclose(file_checksum);
    trans_ascii_to_hex(ch_out, 32, ch_out_hex);
    ch_out_hex[32] = 0;
    if (0 != strcmp(md5_buf, ch_out_hex))
    {
       
        return -1;
    }
    return 0;
}