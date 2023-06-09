#pragma once

#include <string.h>
#include <stdint.h>
#include <string>
#include <iconv.h>

namespace xi {
namespace utils {

const static int32_t BUFFER_LENGTH = 256;

int codingConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) {
        return -1;
	}
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1) {
        return -1;
	}
    iconv_close(cd);
    *pout = nullptr;

    return 0;
}

int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    return codingConvert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    return codingConvert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

std::string utf8ToGBK(const std::string& ustr) {
	char buf[BUFFER_LENGTH] = {0};
	u2g((char *)ustr.c_str(), ustr.size(), buf, BUFFER_LENGTH);
	return std::string(buf);
}

std::string gbkToUtf8(const std::string& gstr) {
	char buf[BUFFER_LENGTH] = {0};
	g2u((char *)gstr.c_str(), gstr.size(), buf, BUFFER_LENGTH);
	return std::string(buf);
}

}
}