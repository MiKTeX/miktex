#ifndef EFONT_MAKET1FONT_HH
#define EFONT_MAKET1FONT_HH
#include <efont/cff.hh>
namespace Efont {
class Type1Font;
}

Efont::Type1Font *create_type1_font(Efont::Cff::Font *, ErrorHandler *);

#endif
