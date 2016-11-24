#pragma once

extern unsigned char *hexify_block(unsigned char *resultbuf, int length);
extern unsigned char *unhexify_block(unsigned char *str, int length);
extern unsigned int unhexnib(unsigned char hexchar);