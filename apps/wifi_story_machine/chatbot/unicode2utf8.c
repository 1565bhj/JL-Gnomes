int unicode2utf8_one(unsigned int unic, unsigned char *utf)
{
    int usize = 0;
    if (unic <= 0x7F) { //1字节
        *(utf + usize++) = unic & 0x7F;
    } else if (unic >= 0x80 && unic <= 0x7FF) { //2字节
        *(utf + usize++) = (unsigned char)((((unic >> 6) & 0x1F) | 0xC0) & 0xFF);
        *(utf + usize++) = (unsigned char)(((unic & 0x3F) | 0x80) & 0xFF);
    } else if (unic >= 0x800 && unic <= 0xFFFF) { //3字节
        *(utf + usize++) = (unsigned char)((((unic >> 12) & 0x0F) | 0xE0) & 0xFF);
        *(utf + usize++) = (unsigned char)((((unic >> 6)  & 0x3F) | 0x80) & 0xFF);
        *(utf + usize++) = (unsigned char)(((unic & 0x3F) | 0x80) & 0xFF);
    } else if (unic >= 0x10000 && unic <= 0x10FFFF) { //4字节
        *(utf + usize++) = (unsigned char)((((unic >> 18) & 0x07) | 0xF0) & 0xFF);
        *(utf + usize++) = (unsigned char)((((unic >> 12) & 0x3F) | 0x80) & 0xFF);
        *(utf + usize++) = (unsigned char)((((unic >>  6) & 0x3F) | 0x80) & 0xFF);
        *(utf + usize++) = (unsigned char)(((unic & 0x3F) | 0x80) & 0xFF);
    } else {
        printf("Error : unknow uncode\n");
        return -1;
    }
    return usize;
}
int utf82unicode_one(unsigned int utf, unsigned char *unicode)
{
    int size = 0;
    if (utf <= 0x7F) { //1字节
        *(unicode + size++) = utf & 0x7F;
    } else if (utf >= 0xC080 && utf <= 0xCFBF) { //2字节
        *(unicode + size++) = (unsigned char)((utf & 0x3F) | (((utf >> 8) & 0x03) << 6)) & 0xFF;
        *(unicode + size++) = (unsigned char)(((utf >> 10) & 0x07)) & 0xFF;
    } else if (utf >= 0xE08080 && utf <= 0xEFBFBF) { //3字节
        *(unicode + size++) = (unsigned char)((utf & 0x3F) | (((utf >> 8) & 0x03) << 6)) & 0xFF;
        *(unicode + size++) = (unsigned char)(((utf >> 10) & 0x0F) | ((utf >> 16) & 0x0F) << 4) & 0xFF;
    } else if (utf >= 0xF0808080 && utf <= 0xF7BFBFBF) { //4字节
        *(unicode + size++) = (unsigned char)((utf & 0x3F) | ((utf >> 8) & 0x03) << 6) & 0xFF;
        *(unicode + size++) = (unsigned char)(((utf >> 10) & 0x0F) | (((utf >> 16) & 0x0F) << 4)) & 0xFF;
        *(unicode + size++) = (unsigned char)(((utf >> 20) & 0x03) | (((utf >> 24) & 0x07) << 2)) & 0xFF;
    } else {
        printf("Error : unknow UTF8 code\n");
        return -1;
    }
    return size;
}

int unicode2utf8(unsigned char *unicode, int unicode_size, unsigned char *utf8, int utf8size)
{
    unsigned short unic;
    int i = 0;
    int ret;
    int wlen = 0;
    while (unicode_size > 0) {
        unic = unicode[0] | (unicode[1] << 8);
        ret = unicode2utf8_one(unic, utf8);
        utf8 += ret;
        wlen += ret;
        unicode += 2;
        unicode_size -= 2;

        if (wlen >= utf8size) {
            return wlen;
        }
    }
    return wlen;
}
