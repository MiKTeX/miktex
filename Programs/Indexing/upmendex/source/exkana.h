extern UChar AKASATANA[];
extern UChar *akasatana;
extern UChar AIUEO[];
extern UChar *aiueo;
extern UChar kanatable[];
extern UChar extkanatable[];

extern UChar GANADA[];

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

#define is_katakana(a)  ((a)>=KATATOP && (a)<=KATAEND)
#define is_hiragana(a)  ((a)>=HIRATOP && (a)<=HIRAEND)
#define is_extkana(a)   ((a)>=EXKANATOP && (a)<=EXKANAEND)
