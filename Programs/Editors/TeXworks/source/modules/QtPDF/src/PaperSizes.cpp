#include "PaperSizes.h"
#include <QList>

class StandardPaperSizes : public QList<PaperSize>
{
public:
  StandardPaperSizes() {
    // Sizes from the package "beamer"
    append(PaperSize(PaperSize::tr("Beamer 16:10"), QSizeF(160, 100), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));
    append(PaperSize(PaperSize::tr("Beamer 16:9"), QSizeF(160, 90), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));
    append(PaperSize(PaperSize::tr("Beamer 14:9"), QSizeF(140, 90), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));
    append(PaperSize(PaperSize::tr("Beamer 1.41:1"), QSizeF(148.5, 105), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));
    append(PaperSize(PaperSize::tr("Beamer 5:4"), QSizeF(125, 100), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));
    append(PaperSize(PaperSize::tr("Beamer 4:3"), QSizeF(128, 96), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));
    append(PaperSize(PaperSize::tr("Beamer 3:2"), QSizeF(135, 90), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));

    // Special sizes from the package "geometry"
    append(PaperSize(PaperSize::tr("Screen"), QSizeF(225, 180), PaperSize::Millimeters, true, QSizeF(1e-3, 1e-3)));

    // Standard paper sizes (from https://en.wikipedia.org/wiki/Paper_size)
    append(PaperSize(PaperSize::tr("DIN 4A0"), QSizeF(1682, 2378), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN 2A0"), QSizeF(1189, 1682), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A0"), QSizeF(841, 1189), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A1"), QSizeF(594, 841), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A2"), QSizeF(420, 594), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A3"), QSizeF(297, 420), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A4"), QSizeF(210, 297), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A5"), QSizeF(148, 210), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A6"), QSizeF(105, 148), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A7"), QSizeF(74, 105), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A8"), QSizeF(52, 74), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A9"), QSizeF(37, 52), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN A10"), QSizeF(26, 37), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B0"), QSizeF(1000, 1414), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B1"), QSizeF(707, 1000), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B2"), QSizeF(500, 707), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B3"), QSizeF(353, 500), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B4"), QSizeF(250, 353), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B5"), QSizeF(176, 250), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B6"), QSizeF(125, 176), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B7"), QSizeF(88, 125), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B8"), QSizeF(62, 88), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B9"), QSizeF(44, 62), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN B10"), QSizeF(31, 44), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C0"), QSizeF(917, 1297), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C1"), QSizeF(648, 917), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C2"), QSizeF(458, 648), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C3"), QSizeF(324, 458), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C4"), QSizeF(229, 324), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C5"), QSizeF(162, 229), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C6"), QSizeF(114, 162), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C7"), QSizeF(81, 114), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C8"), QSizeF(57, 81), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C9"), QSizeF(40, 57), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C10"), QSizeF(28, 40), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN DL"), QSizeF(99, 210), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN DLE"), QSizeF(110, 220), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN C6/C5"), QSizeF(114, 229), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN RA0"), QSizeF(860, 1220), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN RA1"), QSizeF(610, 860), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN RA2"), QSizeF(430, 610), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN RA3"), QSizeF(305, 430), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN RA4"), QSizeF(215, 305), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN SRA0"), QSizeF(900, 1280), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN SRA1"), QSizeF(640, 900), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN SRA2"), QSizeF(450, 640), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN SRA3"), QSizeF(320, 450), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("DIN SRA4"), QSizeF(225, 320), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D0"), QSizeF(1091, 1542), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D1"), QSizeF(771, 1091), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D2"), QSizeF(545, 771), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D3"), QSizeF(386, 545), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D4"), QSizeF(273, 386), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D5"), QSizeF(193, 273), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D6"), QSizeF(136, 193), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D7"), QSizeF(96, 136), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D8"), QSizeF(68, 96), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS D9"), QSizeF(48, 68), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E0"), QSizeF(878, 1242), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E1"), QSizeF(621, 878), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E2"), QSizeF(439, 621), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E3"), QSizeF(310, 439), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E4"), QSizeF(220, 310), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E5"), QSizeF(155, 220), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E6"), QSizeF(110, 155), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E7"), QSizeF(78, 110), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E8"), QSizeF(55, 78), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS E9"), QSizeF(39, 55), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F0"), QSizeF(1044, 1477), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F1"), QSizeF(738, 1044), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F2"), QSizeF(522, 738), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F3"), QSizeF(369, 522), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F4"), QSizeF(261, 369), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F5"), QSizeF(185, 261), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F6"), QSizeF(131, 185), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F7"), QSizeF(92, 131), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F8"), QSizeF(65, 92), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS F9"), QSizeF(46, 65), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G0"), QSizeF(958, 1354), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G1"), QSizeF(677, 958), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G2"), QSizeF(479, 677), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G3"), QSizeF(339, 479), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G4"), QSizeF(239, 339), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G5"), QSizeF(169, 239), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G6"), QSizeF(120, 169), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G7"), QSizeF(85, 120), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G8"), QSizeF(60, 85), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SIS G9"), QSizeF(42, 60), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B0"), QSizeF(1030, 1456), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B1"), QSizeF(728, 1030), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B2"), QSizeF(515, 728), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B3"), QSizeF(364, 515), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B4"), QSizeF(257, 364), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B5"), QSizeF(182, 257), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B6"), QSizeF(128, 182), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B7"), QSizeF(91, 128), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B8"), QSizeF(64, 91), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B9"), QSizeF(45, 64), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B10"), QSizeF(32, 45), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B11"), QSizeF(22, 32), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS B12"), QSizeF(16, 22), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS Shiroku ban 4"), QSizeF(264, 379), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS Shiroku ban 5"), QSizeF(189, 262), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS Shiroku ban 6"), QSizeF(127, 188), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS Kiku 4"), QSizeF(227, 306), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("JIS Kiku 5"), QSizeF(151, 227), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D0"), QSizeF(764, 1064), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D1"), QSizeF(532, 760), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D2"), QSizeF(380, 528), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D3"), QSizeF(264, 376), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D4"), QSizeF(188, 260), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D5"), QSizeF(130, 184), PaperSize::Millimeters));
    append(PaperSize(PaperSize::tr("SAC D6"), QSizeF(92, 126), PaperSize::Millimeters));

    append(PaperSize(PaperSize::tr("Letter (ANSI A)"), QSizeF(8.5, 11), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Legal"), QSizeF(8.5, 14), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Tabloid (ANSI B)"), QSizeF(11, 17), PaperSize::Inches, false));
    append(PaperSize(PaperSize::tr("Ledger"), QSizeF(17, 11), PaperSize::Inches, false));
    append(PaperSize(PaperSize::tr("Junior Legal"), QSizeF(5, 8), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Half Letter"), QSizeF(5.5, 8.5), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Government Letter"), QSizeF(8, 10.5), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Government Legal"), QSizeF(8.5, 13), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("ANSI C"), QSizeF(17, 22), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("ANSI D"), QSizeF(22, 34), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("ANSI E"), QSizeF(34, 44), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch A"), QSizeF(9, 12), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch B"), QSizeF(12, 18), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch C"), QSizeF(18, 24), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch D"), QSizeF(24, 36), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch E1"), QSizeF(30, 42), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch E"), QSizeF(36, 48), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch E2"), QSizeF(26, 38), PaperSize::Inches));
    append(PaperSize(PaperSize::tr("Arch E3"), QSizeF(27, 39), PaperSize::Inches));
  }
} standardPaperSizes;


PaperSize::PaperSize(const QString & name, const QSizeF & size, const LengthUnit lengthUnit, const bool rotatable /* = true */, const QSizeF & tolerances /* = QSizeF() */)
  : _name(name)
  , _size(size)
  , _tolerances(tolerances)
  , _rotatable(rotatable)
  , _landscape(false)
  , _lengthUnit(lengthUnit)
{
  if (_lengthUnit == Inches)
    _size *= 25.4;

  if (!_tolerances.isValid()) {
    // Use DIN/ISO tolerances
    if (_size.width() <= 150) _tolerances.setWidth(1.5);
    else if (_size.width() <= 600) _tolerances.setWidth(2);
    else _tolerances.setWidth(3);

    if (_size.height() <= 150) _tolerances.setHeight(1.5);
    else if (_size.height() <= 600) _tolerances.setHeight(2);
    else _tolerances.setHeight(3);
  }
}

QString PaperSize::label() const
{
  float w = _size.width();
  float h = _size.height();
  //: Length unit: millimeters
  QString unit = tr("mm");

  if (_lengthUnit == Inches) {
    w /= 25.4;
    h /= 25.4;
    //: Length unit: inches
    unit = tr("in");
  }

  if (_landscape)
    qSwap(w, h);

  if (_name.isEmpty())
    return QString::fromUtf8("%1 × %2 %3").arg(w).arg(h).arg(unit);
  else
    return QString::fromUtf8("%1 [%2 × %3 %4]").arg(_name).arg(w).arg(h).arg(unit);
}

//static
PaperSize PaperSize::findForMillimeter(const QSizeF & paperSize)
{
  float w = paperSize.width();
  float h = paperSize.height();
  bool ls = false;

  if (w > h) {
    qSwap(w, h);
    ls = true;
  }

  PaperSize ps(QString(), QSizeF(w, h));
  ps.setLandscape(ls);

  int idx = standardPaperSizes.indexOf(ps);
  if (idx < 0) return ps;

  ps = standardPaperSizes[idx];
  if (ls) ps.setLandscape();
  return ps;
}

bool PaperSize::operator ==(const PaperSize & other) const
{
  float wdiff = qAbs(_size.width() - other._size.width());
  float hdiff = qAbs(_size.height() - other._size.height());
  float dw = 1e-3, dh = 1e-3;
  if (_tolerances.isValid()) {
    if (other._tolerances.isValid()) {
      dw = qMin(_tolerances.width(), other._tolerances.width());
      dh = qMin(_tolerances.height(), other._tolerances.height());
    }
    else {
      dw = _tolerances.width();
      dh = _tolerances.height();
    }
  }
  else if (other._tolerances.isValid()) {
    dw = other._tolerances.width();
    dh = other._tolerances.height();
  }

  if (wdiff <= dw && hdiff <= dh) return true;

  if (_rotatable || other._rotatable) {
    wdiff = qAbs(_size.height() - other._size.width());
    hdiff = qAbs(_size.width() - other._size.height());
    dw = 1e-3, dh = 1e-3;
    if (_tolerances.isValid()) {
      if (other._tolerances.isValid()) {
        dw = qMin(_tolerances.height(), other._tolerances.width());
        dh = qMin(_tolerances.width(), other._tolerances.height());
      }
      else {
        dw = _tolerances.width();
        dh = _tolerances.height();
      }
    }
    else if (other._tolerances.isValid()) {
      dw = other._tolerances.width();
      dh = other._tolerances.height();
    }
    if (wdiff <= dw && hdiff <= dh) return true;
  }

  return false;
}
