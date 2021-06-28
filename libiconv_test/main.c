#include <stdio.h>
#include <string.h>
#include <iconv.h>

#define OUT_BUFFER_SIZE (180U)

static int gbk2utf8(const char* src, char* dest, size_t inlen)
{
    int result = 0;
    iconv_t cd;
    size_t ret_size;
    char *inbuf = (char*)src;
    size_t outlen = OUT_BUFFER_SIZE;
    char *outbuf = dest;
    memset(outbuf, 0, outlen);
    cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)(-1)) {
        printf("%s: iconv_open(UTF-8,GBK) failed!\n", __func__);
        return 0;
    }
    ret_size = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
    iconv_close(cd);
    if (ret_size < 0){
        printf("%s: iconv() failed!\n", __func__);
        result = 0;
    }
    else{
        result = 1;
    }
    return result;
}

char CODE[]="0123456789ABCDEF";

static void print_bin(const char *cp, int len)
{
    unsigned char	ch;
    int high,low;
    unsigned char str[5] = {0};

    if (len < 0)
        len = strlen(cp);

    while (len--) {
        ch = *cp++;
        if (ch != '"') {
            high = ch >> 4;
            low = ch & 0xF;
            str[0] = '0';
            str[1] = 'x';
            str[2] = CODE[high];
            str[3] = CODE[low];
            fputc(str[0], stdout);
            fputc(str[1], stdout);
            fputc(str[2], stdout);
            fputc(str[3], stdout);
            fputc(' ', stdout);
        }
    }
}

int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    int result = 0;
    const char gb_str[12] = {0xB2, 0xE2, 0xCA, 0xD4, 0xD6, 0xD0, 0xCE, 0xC4, 0x55, 0xC5, 0xCC, 0x00};
    char utf8_str[OUT_BUFFER_SIZE];
    int str_len = strlen(gb_str);
    memset(utf8_str, 0x00, sizeof(utf8_str));
    result = gbk2utf8(gb_str, utf8_str, str_len);
    printf("gbk_str:");
    print_bin(gb_str, -1);
    printf("\n");
    printf("gbk2utf8:%d:", result);
    print_bin(utf8_str, -1);
    printf("\n");
    return 0;
}
