#define HZIDXSIZE   215

#define HANZI_UNKNOWN -1
#define HANZI_UNIHAN   1
#define HANZI_STROKE   2
#define HANZI_PINYIN   3
#define HANZI_ZHUYIN   4

extern struct hanzi_index hz_index[];
extern int hz_index_len;
extern int hanzi_mode;

extern struct hanzi_index HZ_RADICAL[];
extern struct hanzi_index HZ_STROKE[];
extern struct hanzi_index HZ_PINYIN[];
extern struct hanzi_index HZ_ZHUYIN[];
extern struct hanzi_index HZ_UNKNOWN[];
