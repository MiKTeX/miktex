/* Copyright (C) 2007,2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*			   Python Interface to FontForge		      */


#ifdef HAVE_CONFIG_H
# include <w2c/config.h>
#endif

#include <stdio.h>

#include "uiinterface.h"

typedef struct flaglist {
  const char *name;
  int flag;
} flaglist;

/* These don't get translated. They are a copy of a similar list in fontinfo.c */
static struct flaglist sfnt_name_str_ids[] = {
    { "SubFamily", 2},
    { "Copyright", 0},
    { "Family", 1},
    { "Fullname", 4},
    { "UniqueID", 3},
    { "Version", 5},
    { "PostscriptName", 6},
    { "Trademark", 7},
    { "Manufacturer", 8},
    { "Designer", 9},
    { "Descriptor", 10},
    { "Vendor URL", 11},
    { "Designer URL", 12},
    { "License", 13},
    { "License URL", 14},
/* slot 15 is reserved */
    { "Preferred Family", 16},
    { "Preferred Styles", 17},
    { "Compatible Full", 18},
    { "Sample Text", 19},
    { "CID findfont Name", 20},
    { "WWS Family", 21},
    { "WWS Subfamily", 22},
    { NULL, 0 }
};
/* These don't get translated. They are a copy of a similar list in fontinfo.c */
static struct flaglist sfnt_name_mslangs[] = {
    { "Afrikaans", 0x436},
    { "Albanian", 0x41c},
    { "Amharic", 0x45e},
    { "Arabic (Saudi Arabia)", 0x401},
    { "Arabic (Iraq)", 0x801},
    { "Arabic (Egypt)", 0xc01},
    { "Arabic (Libya)", 0x1001},
    { "Arabic (Algeria)", 0x1401},
    { "Arabic (Morocco)", 0x1801},
    { "Arabic (Tunisia)", 0x1C01},
    { "Arabic (Oman)", 0x2001},
    { "Arabic (Yemen)", 0x2401},
    { "Arabic (Syria)", 0x2801},
    { "Arabic (Jordan)", 0x2c01},
    { "Arabic (Lebanon)", 0x3001},
    { "Arabic (Kuwait)", 0x3401},
    { "Arabic (U.A.E.)", 0x3801},
    { "Arabic (Bahrain)", 0x3c01},
    { "Arabic (Qatar)", 0x4001},
    { "Armenian", 0x42b},
    { "Assamese", 0x44d},
    { "Azeri (Latin)", 0x42c},
    { "Azeri (Cyrillic)", 0x82c},
    { "Basque", 0x42d},
    { "Byelorussian", 0x423},
    { "Bengali", 0x445},
    { "Bengali Bangladesh", 0x845},
    { "Bulgarian", 0x402},
    { "Burmese", 0x455},
    { "Catalan", 0x403},
    { "Cambodian", 0x453},
    { "Cherokee", 0x45c},
    { "Chinese (Taiwan)", 0x404},
    { "Chinese (PRC)", 0x804},
    { "Chinese (Hong Kong)", 0xc04},
    { "Chinese (Singapore)", 0x1004},
    { "Chinese (Macau)", 0x1404},
    { "Croatian", 0x41a},
    { "Croatian Bosnia/Herzegovina", 0x101a},
    { "Czech", 0x405},
    { "Danish", 0x406},
    { "Divehi", 0x465},
    { "Dutch", 0x413},
    { "Flemish (Belgian Dutch)", 0x813},
    { "Edo", 0x466},
    { "English (British)", 0x809},
    { "English (US)", 0x409},
    { "English (Canada)", 0x1009},
    { "English (Australian)", 0xc09},
    { "English (New Zealand)", 0x1409},
    { "English (Irish)", 0x1809},
    { "English (South Africa)", 0x1c09},
    { "English (Jamaica)", 0x2009},
    { "English (Caribbean)", 0x2409},
    { "English (Belize)", 0x2809},
    { "English (Trinidad)", 0x2c09},
    { "English (Zimbabwe)", 0x3009},
    { "English (Philippines)", 0x3409},
    { "English (Indonesia)", 0x3809},
    { "English (Hong Kong)", 0x3c09},
    { "English (India)", 0x4009},
    { "English (Malaysia)", 0x4409},
    { "Estonian", 0x425},
    { "Faeroese", 0x438},
    { "Farsi", 0x429},
    { "Filipino", 0x464},
    { "Finnish", 0x40b},
    { "French French", 0x40c},
    { "French Belgium", 0x80c},
    { "French Canadian", 0xc0c},
    { "French Swiss", 0x100c},
    { "French Luxembourg", 0x140c},
    { "French Monaco", 0x180c},
    { "French West Indies", 0x1c0c},
    { "French Réunion", 0x200c},
    { "French D.R. Congo", 0x240c},
    { "French Senegal", 0x280c},
    { "French Camaroon", 0x2c0c},
    { "French Côte d'Ivoire", 0x300c},
    { "French Mali", 0x340c},
    { "French Morocco", 0x380c},
    { "French Haiti", 0x3c0c},
    { "French North Africa", 0xe40c},
    { "Frisian", 0x462},
    { "Fulfulde", 0x467},
    { "Gaelic (Scottish)", 0x43c},
    { "Gaelic (Irish)", 0x83c},
    { "Galician", 0x467},
    { "Georgian", 0x437},
    { "German German", 0x407},
    { "German Swiss", 0x807},
    { "German Austrian", 0xc07},
    { "German Luxembourg", 0x1007},
    { "German Liechtenstein", 0x1407},
    { "Greek", 0x408},
    { "Guarani", 0x474},
    { "Gujarati", 0x447},
    { "Hausa", 0x468},
    { "Hawaiian", 0x475},
    { "Hebrew", 0x40d},
    { "Hindi", 0x439},
    { "Hungarian", 0x40e},
    { "Ibibio", 0x469},
    { "Icelandic", 0x40f},
    { "Igbo", 0x470},
    { "Indonesian", 0x421},
    { "Inuktitut", 0x45d},
    { "Italian", 0x410},
    { "Italian Swiss", 0x810},
    { "Japanese", 0x411},
    { "Kannada", 0x44b},
    { "Kanuri", 0x471},
    { "Kashmiri (India)", 0x860},
    { "Kazakh", 0x43f},
    { "Khmer", 0x453},
    { "Kirghiz", 0x440},
    { "Konkani", 0x457},
    { "Korean", 0x412},
    { "Korean (Johab)", 0x812},
    { "Lao", 0x454},
    { "Latvian", 0x426},
    { "Latin", 0x476},
    { "Lithuanian", 0x427},
    { "Lithuanian (Classic)", 0x827},
    { "Macedonian", 0x42f},
    { "Malay", 0x43e},
    { "Malay (Brunei)", 0x83e},
    { "Malayalam", 0x44c},
    { "Maltese", 0x43a},
    { "Manipuri", 0x458},
    { "Maori", 0x481},
    { "Marathi", 0x44e},
    { "Mongolian (Cyrillic)", 0x450},
    { "Mongolian (Mongolian)", 0x850},
    { "Nepali", 0x461},
    { "Nepali (India)", 0x861},
    { "Norwegian (Bokmal)", 0x414},
    { "Norwegian (Nynorsk)", 0x814},
    { "Oriya", 0x448},
    { "Oromo", 0x472},
    { "Papiamentu", 0x479},
    { "Pashto", 0x463},
    { "Polish", 0x415},
    { "Portugese (Portugal)", 0x416},
    { "Portuguese (Brasil)", 0x816},
    { "Punjabi (India)", 0x446},
    { "Punjabi (Pakistan)", 0x846},
    { "Quecha (Bolivia)", 0x46b},
    { "Quecha (Ecuador)", 0x86b},
    { "Quecha (Peru)", 0xc6b},
    { "Rhaeto-Romanic", 0x417},
    { "Romanian", 0x418},
    { "Romanian (Moldova)", 0x818},
    { "Russian", 0x419},
    { "Russian (Moldova)", 0x819},
    { "Sami (Lappish)", 0x43b},
    { "Sanskrit", 0x43b},
    { "Sepedi", 0x46c},
    { "Serbian (Cyrillic)", 0xc1a},
    { "Serbian (Latin)", 0x81a},
    { "Sindhi India", 0x459},
    { "Sindhi Pakistan", 0x859},
    { "Sinhalese", 0x45b},
    { "Slovak", 0x41b},
    { "Slovenian", 0x424},
    { "Sorbian", 0x42e},
    { "Spanish (Traditional)", 0x40a},
    { "Spanish Mexico", 0x80a},
    { "Spanish (Modern)", 0xc0a},
    { "Spanish (Guatemala)", 0x100a},
    { "Spanish (Costa Rica)", 0x140a},
    { "Spanish (Panama)", 0x180a},
    { "Spanish (Dominican Republic)", 0x1c0a},
    { "Spanish (Venezuela)", 0x200a},
    { "Spanish (Colombia)", 0x240a},
    { "Spanish (Peru)", 0x280a},
    { "Spanish (Argentina)", 0x2c0a},
    { "Spanish (Ecuador)", 0x300a},
    { "Spanish (Chile)", 0x340a},
    { "Spanish (Uruguay)", 0x380a},
    { "Spanish (Paraguay)", 0x3c0a},
    { "Spanish (Bolivia)", 0x400a},
    { "Spanish (El Salvador)", 0x440a},
    { "Spanish (Honduras)", 0x480a},
    { "Spanish (Nicaragua)", 0x4c0a},
    { "Spanish (Puerto Rico)", 0x500a},
    { "Spanish (United States)", 0x540a},
    { "Spanish (Latin America)", 0xe40a},
    { "Sutu", 0x430},
    { "Swahili (Kenyan)", 0x441},
    { "Swedish (Sweden)", 0x41d},
    { "Swedish (Finland)", 0x81d},
    { "Syriac", 0x45a},
    { "Tagalog", 0x464},
    { "Tajik", 0x428},
    { "Tamazight (Arabic)", 0x45f},
    { "Tamazight (Latin)", 0x85f},
    { "Tamil", 0x449},
    { "Tatar (Tatarstan)", 0x444},
    { "Telugu", 0x44a},
    { "Thai", 0x41e},
    { "Tibetan (PRC)", 0x451},
    { "Tibetan Bhutan", 0x851},
    { "Tigrinya Ethiopia", 0x473},
    { "Tigrinyan Eritrea", 0x873},
    { "Tsonga", 0x431},
    { "Tswana", 0x432},
    { "Turkish", 0x41f},
    { "Turkmen", 0x442},
    { "Uighur", 0x480},
    { "Ukrainian", 0x422},
    { "Urdu (Pakistan)", 0x420},
    { "Urdu (India)", 0x820},
    { "Uzbek (Latin)", 0x443},
    { "Uzbek (Cyrillic)", 0x843},
    { "Venda", 0x433},
    { "Vietnamese", 0x42a},
    { "Welsh", 0x452},
    { "Xhosa", 0x434},
    { "Yi", 0x478},
    { "Yiddish", 0x43d},
    { "Yoruba", 0x46a},
    { "Zulu", 0x435},
    { NULL, 0 }};

const char *NOUI_TTFNameIds(int id) {
    int i;

    for ( i=0; sfnt_name_str_ids[i].name!=NULL; ++i )
	if ( sfnt_name_str_ids[i].flag == id )
return( (char *) sfnt_name_str_ids[i].name );

return( "Unknown" );
}

const char *NOUI_MSLangString(int language) {
    int i;

    for ( i=0; sfnt_name_mslangs[i].name!=NULL; ++i )
	if ( sfnt_name_mslangs[i].flag == language )
return( (char *) sfnt_name_mslangs[i].name );

    language &= 0xff;
    for ( i=0; sfnt_name_mslangs[i].name!=NULL; ++i )
	if ( sfnt_name_mslangs[i].flag == language )
return( (char *) sfnt_name_mslangs[i].name );

return( "Unknown" );
}
