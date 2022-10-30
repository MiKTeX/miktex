extern UChar AKASATANA[];
extern UChar *akasatana;
extern UChar AIUEO[];
extern UChar *aiueo;
extern UChar kanatable[];
extern UChar extkanatable[];
extern int   kana_ye_mode;

#define SPACE    0x3000    /* 全角スペース */
#define ALPHATOP 0xff01    /* ！ */
#define ALPHAEND 0xff5a    /* ｚ */
#define YENSIGN  0xffe5    /* ￥ */
#define HIRATOP  0x3041    /* ぁ */
#define HIRAEND  0x3096    /* ゖ */
#define KATATOP  0x30a1    /* ァ */
#define KATAEND  0x30fa    /* ヺ */
#define EXKANATOP  0x31F0  /* ㇰ */
#define EXKANAEND  0x31FF  /* ㇿ */
#define CRKANATOP  0x32D0  /* ㋐ */
#define CRKANAEND  0x32FE  /* ㋾ */
#define HANKANATOP 0xFF71  /* ｱ */
#define HANKANAEND 0xFF9D  /* ﾝ */
#define HANKANAWO  0xFF66  /* ｦ */
#define HANKANATU  0xFF6F  /* ｯ */
#define SQKANATOP  0x3300  /* ㌀ */
#define SQKANAEND  0x3357  /* ㍗ */
#define CRLATNTOP  0x24B6  /* Ⓐ */
#define CRLATNEND  0x24E9  /* ⓩ */

#define is_katakana(a)  ((a)>=KATATOP && (a)<=KATAEND)
#define is_hiragana(a)  ((a)>=HIRATOP && (a)<=HIRAEND)
#define is_extkana(a)   ((a)>=EXKANATOP && (a)<=EXKANAEND)
#define is_circkana(a)  ((a)>=CRKANATOP && (a)<=CRKANAEND)
#define is_hankana(a)   ((a)>=HANKANATOP && (a)<=HANKANAEND || (a)>=HANKANAWO && (a)<=HANKANATU)
#define is_sqkana(a)    ((a)>=SQKANATOP && (a)<=SQKANAEND)
#define is_circlatin(a) ((a)>=CRLATNTOP && (a)<=CRLATNEND)
