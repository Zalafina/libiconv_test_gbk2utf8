#include <stdio.h>
#include <string.h>
#include <iconv.h>

#define UNUSED(x) (void)x;

#define UTF8_STR_NONE			(0)
#define UTF8_STR_ALLASCII		(1)
#define UTF8_STR_YES			(2)

#define GBK_STR_NONE			(0)
#define GBK_STR_ALLASCII		(1)
#define GBK_STR_YES				(2)

#define SHIFTJIS_STR_NONE		(0)
#define SHIFTJIS_STR_ALLASCII	(1)
#define SHIFTJIS_STR_YES		(2)

#define STR_TYPE_NONE			(0)
#define STR_TYPE_ASCII			(1)
#define STR_TYPE_UTF8			(2)
#define STR_TYPE_GBK			(3)
#define STR_TYPE_SHIFTJIS		(4)
#define STR_TYPE_OTHER			(5)

#define CONVERT_BUFFSIZE		(180U)

#define CONVERT_RESULT_INIT				(-10)
#define CONVERT_RESULT_OPEN_FAILED		(-5)
#define CONVERT_RESULT_ERROR			(-1)

static int is_str_shiftjis(const char* str)
{
    unsigned int nBytes = 0;
    unsigned char chr = *str;
    int bAllAscii = 1;
    int result = SHIFTJIS_STR_ALLASCII;

    for (unsigned int i = 0; str[i] != '\0'; ++i){
        chr = *(str + i);
        if ((chr & 0x80) != 0 && nBytes == 0){
            bAllAscii = 0;
        }

        if (nBytes == 0) {
            if (chr >= 0x80) {
                if ((chr >= 0xA1)&&(chr <= 0xDF)){
                    nBytes = +1;
                }
                else if ((chr >= 0x81 && chr <= 0x9F)
                    || (chr >= 0xE0 && chr <= 0xEF)){
                    nBytes = +2;
                }
                else{
                    return SHIFTJIS_STR_NONE;
                }
                nBytes--;
            }
        }
        else{
            if (chr < 0x40 || chr> 0xFC || chr == 0x7F){
                return SHIFTJIS_STR_NONE;
            }
            nBytes--;
        }
    }

    if (nBytes != 0){
        return SHIFTJIS_STR_NONE;
    }

    if (bAllAscii){
        result = SHIFTJIS_STR_ALLASCII;
    }

    return result;
}

static int is_str_utf8(const char* str)
{
    unsigned int nBytes = 0;
    unsigned char chr = *str;
    int bAllAscii = 1;
    int result = UTF8_STR_YES;

    for (unsigned int i = 0; str[i] != '\0'; ++i){
        chr = *(str + i);

        if (nBytes == 0 && (chr & 0x80) != 0){
            bAllAscii = 0;
        }

        if (nBytes == 0) {
            if (chr >= 0x80) {

                if (chr >= 0xFC && chr <= 0xFD){
                    nBytes = 6;
                }
                else if (chr >= 0xF8){
                    nBytes = 5;
                }
                else if (chr >= 0xF0){
                    nBytes = 4;
                }
                else if (chr >= 0xE0){
                    nBytes = 3;
                }
                else if (chr >= 0xC0){
                    nBytes = 2;
                }
                else{
                    return UTF8_STR_NONE;
                }

                nBytes--;
            }
        }
        else{
            if ((chr & 0xC0) != 0x80){
                return UTF8_STR_NONE;
            }
            nBytes--;
        }
    }

    if (nBytes != 0){
        return UTF8_STR_NONE;
    }

    if (bAllAscii){
        result = UTF8_STR_ALLASCII;
    }

    return result;
}

static int is_str_gbk(const char* str)
{
    unsigned int nBytes = 0;
    unsigned char chr = *str;
    int bAllAscii = 1;
    int result = GBK_STR_YES;

    for (unsigned int i = 0; str[i] != '\0'; ++i){
        chr = *(str + i);
        if ((chr & 0x80) != 0 && nBytes == 0){
            bAllAscii = 0;
        }

        if (nBytes == 0) {
            if (chr >= 0x80) {
                if (chr >= 0x81 && chr <= 0xFE){
                    nBytes = +2;
                }
                else{
                    return GBK_STR_NONE;
                }

                nBytes--;
            }
        }
        else{
            if (chr < 0x40 || chr>0xFE){
                return GBK_STR_NONE;
            }
            nBytes--;
        }
    }

    if (nBytes != 0){
        return GBK_STR_NONE;
    }

    if (bAllAscii){
        result = GBK_STR_ALLASCII;
    }

    return result;
}

static int convert2utf8(const char* src, char* dest, size_t inlen, const char* fromcode)
{
    char *inbuf = (char*)src;
    size_t outlen = inlen * 4;
    if (outlen > CONVERT_BUFFSIZE){
        outlen = CONVERT_BUFFSIZE;
    }
    char *outbuf = dest;
    memset(outbuf, 0, outlen);
    iconv_t cd = iconv_open("UTF-8", fromcode);
    if (cd == (iconv_t)(-1)) {
        fputs("open_failed:", stdout);
        return -5;
    }
    int ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
    iconv_close(cd);
    return ret;
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

int convert_str(const char *value)
{
    int				check_utf8 = UTF8_STR_NONE;
    int				check_gbk = GBK_STR_NONE;
    int				check_shiftjis = SHIFTJIS_STR_NONE;
    int				conv_len = 0, conv_result = CONVERT_RESULT_INIT;
    int				string_type = STR_TYPE_NONE;
    char			*conv_value;
    char conv_buffer[CONVERT_BUFFSIZE+4];

    conv_value = (char*)value;
    conv_len = strlen(value);
    if (conv_len > 0){
        check_utf8 = is_str_utf8(value);
        if (UTF8_STR_NONE == check_utf8){
            check_gbk = is_str_gbk(value);
            if (GBK_STR_YES == check_gbk){
                string_type = STR_TYPE_GBK;
                memset(conv_buffer, 0x00, sizeof(conv_buffer));
                //conv_result = convert2utf8(value, conv_buffer, conv_len, "GBK");
                conv_result = convert2utf8(value, conv_buffer, conv_len, "GB18030");
                if (conv_result < 0){
                    /* GBK to UTF-8 convert Failed. */
                }
                else{
                    /* GBK to UTF-8 convert Success. */
                    conv_value = conv_buffer;
                }
            }
            else if (GBK_STR_ALLASCII == check_gbk){
                string_type = STR_TYPE_ASCII;
            }
            else{
                check_shiftjis = is_str_shiftjis(value);
                if (SHIFTJIS_STR_YES == check_shiftjis){
                    string_type = STR_TYPE_SHIFTJIS;
                    memset(conv_buffer, 0x00, sizeof(conv_buffer));
                    conv_result = convert2utf8(value, conv_buffer, conv_len, "SHIFT_JIS");
                    if (conv_result < 0){
                        /* Shift-JIS to UTF-8 convert Failed. */
                    }
                    else{
                        /* Shift-JIS to UTF-8 convert Success. */
                        conv_value = conv_buffer;
                    }
                }
                else if (SHIFTJIS_STR_ALLASCII == check_shiftjis){
                    string_type = STR_TYPE_ASCII;
                }
                else{
                    string_type = STR_TYPE_OTHER;
                }
            }
        }
        else if (UTF8_STR_YES == check_utf8){
            string_type = STR_TYPE_UTF8;
        }
        else{
            string_type = STR_TYPE_ASCII;
        }
    }
    else{
        string_type = STR_TYPE_NONE;
        return conv_result;
    }

    fputs("ConvertStr_HEX", stdout);
    fputs("=\"", stdout);
    print_bin(conv_value, -1);
    fputs("\" ", stdout);
    fputs("\n", stdout);
    if (STR_TYPE_ASCII == string_type){
        printf("ASCII=%s\n",conv_value);
    }
    else if (STR_TYPE_UTF8 == string_type){
        printf("UTF8=%s\n",conv_value);
    }
    else if (STR_TYPE_GBK == string_type){
        printf("GBK2UTF8:%d=%s\n",conv_len,conv_value);
    }
    else if (STR_TYPE_SHIFTJIS == string_type){
        printf("SHIFT-JIS2UTF8:%d=%s\n",conv_len,conv_value);
    }
    else if (STR_TYPE_OTHER == string_type){
        printf("OTHER=%s\n",conv_value);
    }

    if (conv_result != CONVERT_RESULT_INIT){
        if (conv_result >= 0){
            if (STR_TYPE_GBK == string_type){
                fputs("GBK2UTF8_OK\n", stdout);
            }
            else if (STR_TYPE_SHIFTJIS == string_type){
                fputs("SHIFT-JIS2UTF8_OK\n", stdout);
            }
        }
        else if (conv_result == CONVERT_RESULT_OPEN_FAILED){
            fputs("ICONV_OPEN_NG\n", stdout);
        }
        else{
            if (STR_TYPE_GBK == string_type){
                fputs("GBK2UTF8_NG\n", stdout);
            }
            else if (STR_TYPE_SHIFTJIS == string_type){
                fputs("SHIFT-JIS2UTF8_NG\n", stdout);
            }
        }
    }

    return conv_result;
}

int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    int result = 0;
    const char gb_str[12] = {0xB2, 0xE2, 0xCA, 0xD4, 0xD6, 0xD0, 0xCE, 0xC4, 0x55, 0xC5, 0xCC, 0x00};
    const char shiftjis_str[18] = {0x93, 0xFA, 0x96, 0x7B, 0x8C, 0xEA, 0x82, 0xCC, 0x41, 0x41, 0x41, 0x83, 0x65, 0x83, 0x58, 0x83, 0x67, 0x00};

//    const char input_utf8_str[19] = {
//        0xB3, 0xAC, 0xB3, 0xA4, 0xB3, 0xAC, 0xB3, 0xA4, 0xD6, 0xD0, 0xCE, 0xC4, 0xB2, 0xE2, 0xCA, 0xD4,
//        0xC6, 0xE6, 0x00
//    };

    const char input_utf8_str[9] = {
        0xFE, 0x9D, 0xFE, 0x9E, 0xFE, 0x9F, 0xFE, 0xA0, 0x00
    };

    UNUSED(gb_str);
    UNUSED(shiftjis_str);

    printf("input_str:\"");
    print_bin(input_utf8_str, -1);
    printf("\"\n");
    result = convert_str(input_utf8_str);
    printf("conv_restul:%d\n", result);
    return 0;
}
