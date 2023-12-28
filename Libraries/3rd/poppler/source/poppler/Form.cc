//========================================================================
//
// Form.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2006-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright 2007-2012, 2015-2023 Albert Astals Cid <aacid@kde.org>
// Copyright 2007-2008, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2007, 2013, 2016, 2019, 2022 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2007 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright 2009 Matthias Drochner <M.Drochner@fz-juelich.de>
// Copyright 2009 KDAB via Guillermo Amaral <gamaral@amaral.com.mx>
// Copyright 2010, 2012 Mark Riedesel <mark@klowner.com>
// Copyright 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2017 Bernd Kuhls <berndkuhls@hotmail.com>
// Copyright 2018 Andre Heinecke <aheinecke@intevation.de>
// Copyright 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright 2018-2022 Nelson Benítez León <nbenitezl@gmail.com>
// Copyright 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2019 Tomoyuki Kubota <himajin100000@gmail.com>
// Copyright 2019 João Netto <joaonetto901@gmail.com>
// Copyright 2020-2022 Marek Kasik <mkasik@redhat.com>
// Copyright 2020 Thorsten Behrens <Thorsten.Behrens@CIB.de>
// Copyright 2020, 2023 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by Technische Universität Dresden
// Copyright 2021 Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
// Copyright 2021 Theofilos Intzoglou <int.teo@gmail.com>
// Copyright 2021 Even Rouault <even.rouault@spatialys.com>
// Copyright 2022 Alexander Sulfrian <asulfrian@zedat.fu-berlin.de>
// Copyright 2022 Erich E. Hoover <erich.e.hoover@gmail.com>
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
//========================================================================

#include <config.h>

#include <array>
#include <set>
#include <limits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "goo/ft_utils.h"
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "goo/GooString.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "CharCodeToUnicode.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Gfx.h"
#include "GfxFont.h"
#include "GlobalParams.h"
#include "Form.h"
#include "PDFDoc.h"
#include "DateInfo.h"
#include "CryptoSignBackend.h"
#include "SignatureInfo.h"
#include "CertificateInfo.h"
#include "XRef.h"
#include "PDFDocEncoding.h"
#include "Annot.h"
#include "Link.h"
#include "Lexer.h"
#include "Parser.h"
#include "CIDFontsWidthsBuilder.h"

#include "fofi/FoFiTrueType.h"
#include "fofi/FoFiIdentifier.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_set>

// helper for using std::visit to get a dependent false for static_asserts
// to help get compile errors if one ever extends variants
template<class>
inline constexpr bool always_false_v = false;

// return a newly allocated char* containing an UTF16BE string of size length
char *pdfDocEncodingToUTF16(const std::string &orig, int *length)
{
    // double size, a unicode char takes 2 char, add 2 for the unicode marker
    *length = 2 + 2 * orig.size();
    char *result = new char[(*length)];
    const char *cstring = orig.c_str();
    // unicode marker
    result[0] = '\xfe';
    result[1] = '\xff';
    // convert to utf16
    for (int i = 2, j = 0; i < (*length); i += 2, j++) {
        Unicode u = pdfDocEncoding[(unsigned int)((unsigned char)cstring[j])] & 0xffff;
        result[i] = (u >> 8) & 0xff;
        result[i + 1] = u & 0xff;
    }
    return result;
}

static GooString *convertToUtf16(GooString *pdfDocEncodingString)
{
    int tmp_length;
    char *tmp_str = pdfDocEncodingToUTF16(pdfDocEncodingString->toStr(), &tmp_length);
    delete pdfDocEncodingString;
    pdfDocEncodingString = new GooString(tmp_str + 2, tmp_length - 2); // Remove the unicode BOM
    delete[] tmp_str;
    return pdfDocEncodingString;
}

FormWidget::FormWidget(PDFDoc *docA, Object *aobj, unsigned num, Ref aref, FormField *fieldA)
{
    ref = aref;
    ID = 0;
    childNum = num;
    doc = docA;
    xref = doc->getXRef();
    obj = aobj->copy();
    type = formUndef;
    field = fieldA;
    widget = nullptr;
}

FormWidget::~FormWidget()
{
    if (widget) {
        widget->decRefCnt();
    }
}

void FormWidget::print(int indent)
{
    printf("%*s+ (%d %d): [widget]\n", indent, "", ref.num, ref.gen);
}

void FormWidget::createWidgetAnnotation()
{
    if (widget) {
        return;
    }

    Object obj1(ref);
    widget = new AnnotWidget(doc, &obj, &obj1, field);
}

bool FormWidget::inRect(double x, double y) const
{
    return widget ? widget->inRect(x, y) : false;
}

void FormWidget::getRect(double *x1, double *y1, double *x2, double *y2) const
{
    if (widget) {
        widget->getRect(x1, y1, x2, y2);
    }
}

bool FormWidget::isReadOnly() const
{
    return field->isReadOnly();
}

void FormWidget::setReadOnly(bool value)
{
    field->setReadOnly(value);
}

int FormWidget::encodeID(unsigned pageNum, unsigned fieldNum)
{
    return (pageNum << 4 * sizeof(unsigned)) + fieldNum;
}

void FormWidget::decodeID(unsigned id, unsigned *pageNum, unsigned *fieldNum)
{
    *pageNum = id >> 4 * sizeof(unsigned);
    *fieldNum = (id << 4 * sizeof(unsigned)) >> 4 * sizeof(unsigned);
}

const GooString *FormWidget::getPartialName() const
{
    return field->getPartialName();
}

void FormWidget::setPartialName(const GooString &name)
{
    field->setPartialName(name);
}

const GooString *FormWidget::getAlternateUiName() const
{
    return field->getAlternateUiName();
}

const GooString *FormWidget::getMappingName() const
{
    return field->getMappingName();
}

GooString *FormWidget::getFullyQualifiedName()
{
    return field->getFullyQualifiedName();
}

LinkAction *FormWidget::getActivationAction()
{
    return widget ? widget->getAction() : nullptr;
}

std::unique_ptr<LinkAction> FormWidget::getAdditionalAction(Annot::FormAdditionalActionsType t)
{
    return widget ? widget->getFormAdditionalAction(t) : nullptr;
}

bool FormWidget::setAdditionalAction(Annot::FormAdditionalActionsType t, const std::string &js)
{
    if (!widget) {
        return false;
    }

    return widget->setFormAdditionalAction(t, js);
}

FormWidgetButton::FormWidgetButton(PDFDoc *docA, Object *dictObj, unsigned num, Ref refA, FormField *p) : FormWidget(docA, dictObj, num, refA, p)
{
    type = formButton;
    onStr = nullptr;

    // Find the name of the ON state in the AP dictionary
    // The reference say the Off state, if it exists, _must_ be stored in the AP dict under the name /Off
    // The "on" state may be stored under any other name
    Object obj1 = obj.dictLookup("AP");
    if (obj1.isDict()) {
        Object obj2 = obj1.dictLookup("N");
        if (obj2.isDict()) {
            for (int i = 0; i < obj2.dictGetLength(); i++) {
                const char *key = obj2.dictGetKey(i);
                if (strcmp(key, "Off") != 0) {
                    onStr = new GooString(key);
                    break;
                }
            }
        }
    }
}

const char *FormWidgetButton::getOnStr() const
{
    if (onStr) {
        return onStr->c_str();
    }

    // 12.7.4.2.3 Check Boxes
    //  Yes should be used as the name for the on state
    return parent()->getButtonType() == formButtonCheck ? "Yes" : nullptr;
}

FormWidgetButton::~FormWidgetButton()
{
    delete onStr;
}

FormButtonType FormWidgetButton::getButtonType() const
{
    return parent()->getButtonType();
}

void FormWidgetButton::setAppearanceState(const char *state)
{
    if (!widget) {
        return;
    }
    widget->setAppearanceState(state);
}

void FormWidgetButton::updateWidgetAppearance()
{
    // The appearance stream must NOT be regenerated for this widget type
}

void FormWidgetButton::setState(bool astate)
{
    // pushButtons don't have state
    if (parent()->getButtonType() == formButtonPush) {
        return;
    }

    // Silently return if can't set ON state
    if (astate && !getOnStr()) {
        return;
    }

    parent()->setState(astate ? getOnStr() : (char *)"Off");
    // Parent will call setAppearanceState()

    // Now handle standAlone fields which are related to this one by having the same
    // fully qualified name. This is *partially* by spec, as seen in "Field names"
    // section inside "8.6.2 Field Dictionaries" in 1.7 PDF spec. Issue #1034

    if (!astate) { // We're only interested when this field is being set to ON,
        return; // to check if it has related fields and then set them OFF
    }

    unsigned this_page_num, this_field_num;
    decodeID(getID(), &this_page_num, &this_field_num);
    Page *this_page = doc->getCatalog()->getPage(this_page_num);
    const FormField *this_field = getField();
    if (!this_page->hasStandaloneFields() || this_field == nullptr) {
        return;
    }

    auto this_page_widgets = this_page->getFormWidgets();
    const FormButtonType this_button_type = getButtonType();

    const int tot = this_page_widgets->getNumWidgets();
    for (int i = 0; i < tot; i++) {
        bool found_related = false;
        FormWidget *wid = this_page_widgets->getWidget(i);
        const bool same_fqn = wid->getFullyQualifiedName()->cmp(getFullyQualifiedName()) == 0;
        const bool same_button_type = wid->getType() == formButton && static_cast<const FormWidgetButton *>(wid)->getButtonType() == this_button_type;

        if (same_fqn && same_button_type) {
            if (this_field->isStandAlone()) {
                //'this_field' is standAlone, so we need to search in both standAlone fields and normal fields
                if (this_field != wid->getField()) { // so take care to not choose our same field
                    found_related = true;
                }
            } else {
                //'this_field' is not standAlone, so we just need to search in standAlone fields
                if (wid->getField()->isStandAlone()) {
                    found_related = true;
                }
            }
        }

        if (found_related) {
            FormFieldButton *ffb = static_cast<FormFieldButton *>(wid->getField());
            if (ffb == nullptr) {
                error(errInternal, -1, "FormWidgetButton::setState : FormFieldButton expected\n");
                continue;
            }
            ffb->setState((char *)"Off", true);
        }
    }
}

bool FormWidgetButton::getState() const
{
    return getOnStr() ? parent()->getState(getOnStr()) : false;
}

FormFieldButton *FormWidgetButton::parent() const
{
    return static_cast<FormFieldButton *>(field);
}

FormWidgetText::FormWidgetText(PDFDoc *docA, Object *dictObj, unsigned num, Ref refA, FormField *p) : FormWidget(docA, dictObj, num, refA, p)
{
    type = formText;
}

const GooString *FormWidgetText::getContent() const
{
    return parent()->getContent();
}

void FormWidgetText::updateWidgetAppearance()
{
    if (widget) {
        widget->updateAppearanceStream();
    }
}

bool FormWidgetText::isMultiline() const
{
    return parent()->isMultiline();
}

bool FormWidgetText::isPassword() const
{
    return parent()->isPassword();
}

bool FormWidgetText::isFileSelect() const
{
    return parent()->isFileSelect();
}

bool FormWidgetText::noSpellCheck() const
{
    return parent()->noSpellCheck();
}

bool FormWidgetText::noScroll() const
{
    return parent()->noScroll();
}

bool FormWidgetText::isComb() const
{
    return parent()->isComb();
}

bool FormWidgetText::isRichText() const
{
    return parent()->isRichText();
}

int FormWidgetText::getMaxLen() const
{
    return parent()->getMaxLen();
}

double FormWidgetText::getTextFontSize()
{
    return parent()->getTextFontSize();
}

void FormWidgetText::setTextFontSize(int fontSize)
{
    parent()->setTextFontSize(fontSize);
}

void FormWidgetText::setContent(const GooString *new_content)
{
    parent()->setContentCopy(new_content);
}

void FormWidgetText::setAppearanceContent(const GooString *new_content)
{
    parent()->setAppearanceContentCopy(new_content);
}

FormFieldText *FormWidgetText::parent() const
{
    return static_cast<FormFieldText *>(field);
}

FormWidgetChoice::FormWidgetChoice(PDFDoc *docA, Object *dictObj, unsigned num, Ref refA, FormField *p) : FormWidget(docA, dictObj, num, refA, p)
{
    type = formChoice;
}

FormWidgetChoice::~FormWidgetChoice() { }

bool FormWidgetChoice::_checkRange(int i) const
{
    if (i < 0 || i >= parent()->getNumChoices()) {
        error(errInternal, -1, "FormWidgetChoice::_checkRange i out of range : {0:d}", i);
        return false;
    }
    return true;
}

void FormWidgetChoice::select(int i)
{
    if (!_checkRange(i)) {
        return;
    }
    parent()->select(i);
}

void FormWidgetChoice::toggle(int i)
{
    if (!_checkRange(i)) {
        return;
    }
    parent()->toggle(i);
}

void FormWidgetChoice::deselectAll()
{
    parent()->deselectAll();
}

const GooString *FormWidgetChoice::getEditChoice() const
{
    if (!hasEdit()) {
        error(errInternal, -1, "FormFieldChoice::getEditChoice called on a non-editable choice\n");
        return nullptr;
    }
    return parent()->getEditChoice();
}

void FormWidgetChoice::updateWidgetAppearance()
{
    if (widget) {
        widget->updateAppearanceStream();
    }
}

bool FormWidgetChoice::isSelected(int i) const
{
    if (!_checkRange(i)) {
        return false;
    }
    return parent()->isSelected(i);
}

void FormWidgetChoice::setEditChoice(const GooString *new_content)
{
    if (!hasEdit()) {
        error(errInternal, -1, "FormFieldChoice::setEditChoice : trying to edit an non-editable choice\n");
        return;
    }

    parent()->setEditChoice(new_content);
}

int FormWidgetChoice::getNumChoices() const
{
    return parent()->getNumChoices();
}

const GooString *FormWidgetChoice::getChoice(int i) const
{
    return parent()->getChoice(i);
}

const GooString *FormWidgetChoice::getExportVal(int i) const
{
    return parent()->getExportVal(i);
}

bool FormWidgetChoice::isCombo() const
{
    return parent()->isCombo();
}

bool FormWidgetChoice::hasEdit() const
{
    return parent()->hasEdit();
}

bool FormWidgetChoice::isMultiSelect() const
{
    return parent()->isMultiSelect();
}

bool FormWidgetChoice::noSpellCheck() const
{
    return parent()->noSpellCheck();
}

bool FormWidgetChoice::commitOnSelChange() const
{
    return parent()->commitOnSelChange();
}

bool FormWidgetChoice::isListBox() const
{
    return parent()->isListBox();
}

FormFieldChoice *FormWidgetChoice::parent() const
{
    return static_cast<FormFieldChoice *>(field);
}

FormWidgetSignature::FormWidgetSignature(PDFDoc *docA, Object *dictObj, unsigned num, Ref refA, FormField *p) : FormWidget(docA, dictObj, num, refA, p)
{
    type = formSignature;
}

const GooString *FormWidgetSignature::getSignature() const
{
    return static_cast<FormFieldSignature *>(field)->getSignature();
}

SignatureInfo *FormWidgetSignature::validateSignature(bool doVerifyCert, bool forceRevalidation, time_t validationTime, bool ocspRevocationCheck, bool enableAIA)
{
    return static_cast<FormFieldSignature *>(field)->validateSignature(doVerifyCert, forceRevalidation, validationTime, ocspRevocationCheck, enableAIA);
}

// update hash with the specified range of data from the file
static bool hashFileRange(FILE *f, CryptoSign::SigningInterface *handler, Goffset start, Goffset end)
{
    if (!handler) {
        return false;
    }
    const int BUF_SIZE = 65536;

    unsigned char *buf = new unsigned char[BUF_SIZE];

    while (start < end) {
        if (Gfseek(f, start, SEEK_SET) != 0) {
            delete[] buf;
            return false;
        }
        int len = BUF_SIZE;
        if (end - start < len) {
            len = static_cast<int>(end - start);
        }
        if (fread(buf, 1, len, f) != static_cast<size_t>(len)) {
            delete[] buf;
            return false;
        }
        handler->addData(buf, len);
        start += len;
    }
    delete[] buf;
    return true;
}

bool FormWidgetSignature::signDocument(const std::string &saveFilename, const std::string &certNickname, const std::string &password, const GooString *reason, const GooString *location, const std::optional<GooString> &ownerPassword,
                                       const std::optional<GooString> &userPassword)
{
    auto backend = CryptoSign::Factory::createActive();
    if (!backend) {
        return false;
    }
    if (certNickname.empty()) {
        fprintf(stderr, "signDocument: Empty nickname\n");
        return false;
    }

    auto sigHandler = backend->createSigningHandler(certNickname, HashAlgorithm::Sha256);

    FormFieldSignature *signatureField = static_cast<FormFieldSignature *>(field);
    std::unique_ptr<X509CertificateInfo> certInfo = sigHandler->getCertificateInfo();
    if (!certInfo) {
        fprintf(stderr, "signDocument: error getting signature info\n");
        return false;
    }
    const std::string signerName = certInfo->getSubjectInfo().commonName;
    signatureField->setCertificateInfo(certInfo);
    updateWidgetAppearance(); // add visible signing info to appearance

    Object vObj(new Dict(xref));
    Ref vref = xref->addIndirectObject(vObj);
    if (!createSignature(vObj, vref, GooString(signerName), CryptoSign::maxSupportedSignatureSize, reason, location)) {
        return false;
    }

    // Incremental save to avoid breaking any existing signatures
    const GooString fname(saveFilename);
    if (doc->saveAs(fname, writeForceIncremental) != errNone) {
        fprintf(stderr, "signDocument: error saving to file \"%s\"\n", saveFilename.c_str());
        return false;
    }

    // Get start/end offset of signature object in the saved PDF
    Goffset objStart, objEnd;
    if (!getObjectStartEnd(fname, vref.num, &objStart, &objEnd, ownerPassword, userPassword)) {
        fprintf(stderr, "signDocument: unable to get signature object offsets\n");
        return false;
    }

    // Update byte range of signature in the saved PDF
    Goffset sigStart, sigEnd, fileSize;
    FILE *file = openFile(saveFilename.c_str(), "r+b");
    if (!updateOffsets(file, objStart, objEnd, &sigStart, &sigEnd, &fileSize)) {
        fprintf(stderr, "signDocument: unable update byte range\n");
        fclose(file);
        return false;
    }

    // compute hash of byte ranges
    if (!hashFileRange(file, sigHandler.get(), 0LL, sigStart)) {
        fclose(file);
        return false;
    }
    if (!hashFileRange(file, sigHandler.get(), sigEnd, fileSize)) {
        fclose(file);
        return false;
    }

    // and sign it
    auto signature = sigHandler->signDetached(password);
    if (!signature) {
        fclose(file);
        return false;
    }

    if (signature->getLength() > CryptoSign::maxSupportedSignatureSize) {
        fclose(file);
        return false;
    }

    // pad with zeroes to placeholder length
    auto length = signature->getLength();
    signature->append(std::string(CryptoSign::maxSupportedSignatureSize - length, '\0'));

    // write signature to saved file
    if (!updateSignature(file, sigStart, sigEnd, signature.value())) {
        fprintf(stderr, "signDocument: unable update signature\n");
        fclose(file);
        return false;
    }
    signatureField->setSignature(*signature);

    fclose(file);

    return true;
}

bool FormWidgetSignature::signDocumentWithAppearance(const std::string &saveFilename, const std::string &certNickname, const std::string &password, const GooString *reason, const GooString *location,
                                                     const std::optional<GooString> &ownerPassword, const std::optional<GooString> &userPassword, const GooString &signatureText, const GooString &signatureTextLeft, double fontSize,
                                                     double leftFontSize, std::unique_ptr<AnnotColor> &&fontColor, double borderWidth, std::unique_ptr<AnnotColor> &&borderColor, std::unique_ptr<AnnotColor> &&backgroundColor)
{
    // Set the appearance
    GooString *aux = getField()->getDefaultAppearance();
    std::string originalDefaultAppearance = aux ? aux->toStr() : std::string();

    Form *form = doc->getCatalog()->getCreateForm();
    const std::string pdfFontName = form->findPdfFontNameToUseForSigning();
    if (pdfFontName.empty()) {
        return false;
    }

    const DefaultAppearance da { { objName, pdfFontName.c_str() }, fontSize, std::move(fontColor) };
    getField()->setDefaultAppearance(da.toAppearanceString());

    std::unique_ptr<AnnotAppearanceCharacs> origAppearCharacs = getWidgetAnnotation()->getAppearCharacs() ? getWidgetAnnotation()->getAppearCharacs()->copy() : nullptr;
    auto appearCharacs = std::make_unique<AnnotAppearanceCharacs>(nullptr);
    appearCharacs->setBorderColor(std::move(borderColor));
    appearCharacs->setBackColor(std::move(backgroundColor));
    getWidgetAnnotation()->setAppearCharacs(std::move(appearCharacs));

    std::unique_ptr<AnnotBorder> origBorderCopy = getWidgetAnnotation()->getBorder() ? getWidgetAnnotation()->getBorder()->copy() : nullptr;
    std::unique_ptr<AnnotBorder> border(new AnnotBorderArray());
    border->setWidth(borderWidth);
    getWidgetAnnotation()->setBorder(std::move(border));

    getWidgetAnnotation()->generateFieldAppearance();
    getWidgetAnnotation()->updateAppearanceStream();

    form->ensureFontsForAllCharacters(&signatureText, pdfFontName);
    form->ensureFontsForAllCharacters(&signatureTextLeft, pdfFontName);

    ::FormFieldSignature *ffs = static_cast<::FormFieldSignature *>(getField());
    ffs->setCustomAppearanceContent(signatureText);
    ffs->setCustomAppearanceLeftContent(signatureTextLeft);
    ffs->setCustomAppearanceLeftFontSize(leftFontSize);

    // say that there a now signatures and that we should append only
    doc->getCatalog()->getAcroForm()->dictSet("SigFlags", Object(3));

    const bool success = signDocument(saveFilename, certNickname, password, reason, location, ownerPassword, userPassword);

    // Now bring back the annotation appearance back to what it was
    ffs->setDefaultAppearance(originalDefaultAppearance);
    ffs->setCustomAppearanceContent({});
    ffs->setCustomAppearanceLeftContent({});
    getWidgetAnnotation()->setAppearCharacs(std::move(origAppearCharacs));
    getWidgetAnnotation()->setBorder(std::move(origBorderCopy));
    getWidgetAnnotation()->generateFieldAppearance();
    getWidgetAnnotation()->updateAppearanceStream();

    return success;
}

// Get start and end file position of objNum in the PDF named filename.
bool FormWidgetSignature::getObjectStartEnd(const GooString &filename, int objNum, Goffset *objStart, Goffset *objEnd, const std::optional<GooString> &ownerPassword, const std::optional<GooString> &userPassword)
{
    PDFDoc newDoc(std::unique_ptr<GooString>(filename.copy()), ownerPassword, userPassword);
    if (!newDoc.isOk()) {
        return false;
    }

    XRef *newXref = newDoc.getXRef();
    XRefEntry *entry = newXref->getEntry(objNum);
    if (entry->type != xrefEntryUncompressed) {
        return false;
    }

    *objStart = entry->offset;
    newXref->fetch(objNum, entry->gen, 0, objEnd);
    return true;
}

// find next offset containing the dummy offset '9999999999' and overwrite with offset
static char *setNextOffset(char *start, Goffset offset)
{
    char buf[50];
    sprintf(buf, "%lld", offset);
    strcat(buf, "                  "); // add some padding

    char *p = strstr(start, "9999999999");
    if (p) {
        memcpy(p, buf, 10); // overwrite exact size.
        p += 10;
    } else {
        return nullptr;
    }
    return p;
}

// Updates the ByteRange array of the signature object in the file.
// Returns start/end of signature string and file size.
bool FormWidgetSignature::updateOffsets(FILE *f, Goffset objStart, Goffset objEnd, Goffset *sigStart, Goffset *sigEnd, Goffset *fileSize)
{
    if (Gfseek(f, 0, SEEK_END) != 0) {
        return false;
    }
    *fileSize = Gftell(f);

    if (objEnd > *fileSize) {
        objEnd = *fileSize;
    }

    // sanity check object offsets
    if (objEnd <= objStart || (objEnd - objStart >= INT_MAX)) {
        return false;
    }

    const size_t bufSize = static_cast<size_t>(objEnd - objStart);
    if (Gfseek(f, objStart, SEEK_SET) != 0) {
        return false;
    }
    std::vector<char> buf(bufSize + 1);
    if (fread(buf.data(), 1, bufSize, f) != bufSize) {
        return false;
    }
    buf[bufSize] = 0; // prevent string functions from searching past the end

    // search for the Contents field which contains the signature placeholder
    // which always must start with hex digits 000
    *sigStart = -1;
    *sigEnd = -1;
    for (size_t i = 0; i < bufSize - 14; i++) {
        if (buf[i] == '/' && strncmp(&buf[i], "/Contents <000", 14) == 0) {
            *sigStart = objStart + i + 10;
            char *p = strchr(&buf[i], '>');
            if (p) {
                *sigEnd = objStart + (p - buf.data()) + 1;
            }
            break;
        }
    }

    if (*sigStart == -1 || *sigEnd == -1) {
        return false;
    }

    // Search for ByteRange array and update offsets
    for (size_t i = 0; i < bufSize - 10; i++) {
        if (buf[i] == '/' && strncmp(&buf[i], "/ByteRange", 10) == 0) {
            // update range
            char *p = setNextOffset(&buf[i], *sigStart);
            if (!p) {
                return false;
            }
            p = setNextOffset(p, *sigEnd);
            if (!p) {
                return false;
            }
            p = setNextOffset(p, *fileSize - *sigEnd);
            if (!p) {
                return false;
            }
            break;
        }
    }

    // write buffer back to disk
    if (Gfseek(f, objStart, SEEK_SET) != 0) {
        return false;
    }
    fwrite(buf.data(), bufSize, 1, f);
    return true;
}

// Overwrite signature string in the file with new signature
bool FormWidgetSignature::updateSignature(FILE *f, Goffset sigStart, Goffset sigEnd, const GooString &signature)
{
    if (signature.getLength() * 2 + 2 != sigEnd - sigStart) {
        return false;
    }

    if (Gfseek(f, sigStart, SEEK_SET) != 0) {
        return false;
    }
    const char *c = signature.c_str();
    fprintf(f, "<");
    for (int i = 0; i < signature.getLength(); i++) {
        unsigned char value = *(c + i) & 0x000000ff;
        fprintf(f, "%2.2x", value);
    }
    fprintf(f, "> ");
    return true;
}

bool FormWidgetSignature::createSignature(Object &vObj, Ref vRef, const GooString &name, int placeholderLength, const GooString *reason, const GooString *location)
{
    vObj.dictAdd("Type", Object(objName, "Sig"));
    vObj.dictAdd("Filter", Object(objName, "Adobe.PPKLite"));
    vObj.dictAdd("SubFilter", Object(objName, "adbe.pkcs7.detached"));
    vObj.dictAdd("Name", Object(name.copy()));
    GooString *date = timeToDateString(nullptr);
    vObj.dictAdd("M", Object(date));
    if (reason && (reason->getLength() > 0)) {
        vObj.dictAdd("Reason", Object(reason->copy()));
    }
    if (location && (location->getLength() > 0)) {
        vObj.dictAdd("Location", Object(location->copy()));
    }

    vObj.dictAdd("Contents", Object(objHexString, new GooString(std::string(placeholderLength, '\0'))));
    Object bObj(new Array(xref));
    // reserve space in byte range for maximum number of bytes
    bObj.arrayAdd(Object(static_cast<long long>(0LL)));
    bObj.arrayAdd(Object(static_cast<long long>(9999999999LL)));
    bObj.arrayAdd(Object(static_cast<long long>(9999999999LL)));
    bObj.arrayAdd(Object(static_cast<long long>(9999999999LL)));
    vObj.dictAdd("ByteRange", bObj.copy());
    obj.dictSet("V", Object(vRef));
    xref->setModifiedObject(&obj, ref);
    return true;
}

std::vector<Goffset> FormWidgetSignature::getSignedRangeBounds() const
{
    return static_cast<FormFieldSignature *>(field)->getSignedRangeBounds();
}

std::optional<GooString> FormWidgetSignature::getCheckedSignature(Goffset *checkedFileSize)
{
    return static_cast<FormFieldSignature *>(field)->getCheckedSignature(checkedFileSize);
}

void FormWidgetSignature::updateWidgetAppearance()
{
    if (widget) {
        widget->updateAppearanceStream();
    }
}

//========================================================================
// FormField
//========================================================================

FormField::FormField(PDFDoc *docA, Object &&aobj, const Ref aref, FormField *parentA, std::set<int> *usedParents, FormFieldType ty)
{
    doc = docA;
    xref = doc->getXRef();
    obj = std::move(aobj);
    Dict *dict = obj.getDict();
    ref = aref;
    type = ty;
    parent = parentA;
    numChildren = 0;
    children = nullptr;
    terminal = false;
    widgets = nullptr;
    readOnly = false;
    defaultAppearance = nullptr;
    fullyQualifiedName = nullptr;
    quadding = VariableTextQuadding::leftJustified;
    hasQuadding = false;
    standAlone = false;

    // childs
    Object obj1 = dict->lookup("Kids");
    if (obj1.isArray()) {
        // Load children
        for (int i = 0; i < obj1.arrayGetLength(); i++) {
            Ref childRef;
            Object childObj = obj1.getArray()->get(i, &childRef);
            if (childRef == Ref::INVALID()) {
                error(errSyntaxError, -1, "Invalid form field renference");
                continue;
            }
            if (!childObj.isDict()) {
                error(errSyntaxError, -1, "Form field child is not a dictionary");
                continue;
            }

            if (usedParents->find(childRef.num) == usedParents->end()) {
                // Field child: it could be a form field or a widget or composed dict
                const Object &objParent = childObj.dictLookupNF("Parent");
                Object obj3 = childObj.dictLookup("Parent");
                if (objParent.isRef() || obj3.isDict()) {
                    // Child is a form field or composed dict
                    // We create the field, if it's composed
                    // it will create the widget as a child
                    std::set<int> usedParentsAux = *usedParents;
                    usedParentsAux.insert(childRef.num);

                    if (terminal) {
                        error(errSyntaxWarning, -1, "Field can't have both Widget AND Field as kids\n");
                        continue;
                    }

                    numChildren++;
                    children = (FormField **)greallocn(children, numChildren, sizeof(FormField *));
                    children[numChildren - 1] = Form::createFieldFromDict(std::move(childObj), doc, childRef, this, &usedParentsAux);
                } else {
                    Object obj2 = childObj.dictLookup("Subtype");
                    if (obj2.isName("Widget")) {
                        // Child is a widget annotation
                        if (!terminal && numChildren > 0) {
                            error(errSyntaxWarning, -1, "Field can't have both Widget AND Field as kids\n");
                            continue;
                        }
                        _createWidget(&childObj, childRef);
                    }
                }
            }
        }
    } else {
        // No children, if it's a composed dict, create the child widget
        obj1 = dict->lookup("Subtype");
        if (obj1.isName("Widget")) {
            _createWidget(&obj, ref);
        }
    }

    // flags
    obj1 = Form::fieldLookup(dict, "Ff");
    if (obj1.isInt()) {
        int flags = obj1.getInt();
        if (flags & 0x1) { // 1 -> ReadOnly
            readOnly = true;
        }
        if (flags & 0x2) { // 2 -> Required
            // TODO
        }
        if (flags & 0x4) { // 3 -> NoExport
            // TODO
        }
    }

    // Variable Text
    obj1 = Form::fieldLookup(dict, "DA");
    if (obj1.isString()) {
        defaultAppearance = obj1.getString()->copy();
    }

    obj1 = Form::fieldLookup(dict, "Q");
    if (obj1.isInt()) {
        const VariableTextQuadding aux = static_cast<VariableTextQuadding>(obj1.getInt());
        hasQuadding = aux == VariableTextQuadding::leftJustified || aux == VariableTextQuadding::centered || aux == VariableTextQuadding::rightJustified;
        if (likely(hasQuadding)) {
            quadding = static_cast<VariableTextQuadding>(aux);
        }
    }

    obj1 = dict->lookup("T");
    if (obj1.isString()) {
        partialName = obj1.getString()->copy();
    } else {
        partialName = nullptr;
    }

    obj1 = dict->lookup("TU");
    if (obj1.isString()) {
        alternateUiName = obj1.getString()->copy();
    } else {
        alternateUiName = nullptr;
    }

    obj1 = dict->lookup("TM");
    if (obj1.isString()) {
        mappingName = obj1.getString()->copy();
    } else {
        mappingName = nullptr;
    }
}

void FormField::setDefaultAppearance(const std::string &appearance)
{
    delete defaultAppearance;
    defaultAppearance = new GooString(appearance);
}

void FormField::setPartialName(const GooString &name)
{
    delete partialName;
    partialName = name.copy();

    obj.getDict()->set("T", Object(name.copy()));
    xref->setModifiedObject(&obj, ref);
}

FormField::~FormField()
{
    if (!terminal) {
        if (children) {
            for (int i = 0; i < numChildren; i++) {
                delete children[i];
            }
            gfree(children);
        }
    } else {
        for (int i = 0; i < numChildren; ++i) {
            delete widgets[i];
        }
        gfree(widgets);
    }

    delete defaultAppearance;
    delete partialName;
    delete alternateUiName;
    delete mappingName;
    delete fullyQualifiedName;
}

void FormField::print(int indent)
{
    printf("%*s- (%d %d): [container] terminal: %s children: %d\n", indent, "", ref.num, ref.gen, terminal ? "Yes" : "No", numChildren);
}

void FormField::printTree(int indent)
{
    print(indent);
    if (terminal) {
        for (int i = 0; i < numChildren; i++) {
            widgets[i]->print(indent + 4);
        }
    } else {
        for (int i = 0; i < numChildren; i++) {
            children[i]->printTree(indent + 4);
        }
    }
}

void FormField::fillChildrenSiblingsID()
{
    if (terminal) {
        return;
    }
    for (int i = 0; i < numChildren; i++) {
        children[i]->fillChildrenSiblingsID();
    }
}

void FormField::createWidgetAnnotations()
{
    if (terminal) {
        for (int i = 0; i < numChildren; i++) {
            widgets[i]->createWidgetAnnotation();
        }
    } else {
        for (int i = 0; i < numChildren; i++) {
            children[i]->createWidgetAnnotations();
        }
    }
}

void FormField::_createWidget(Object *objA, Ref aref)
{
    terminal = true;
    numChildren++;
    widgets = (FormWidget **)greallocn(widgets, numChildren, sizeof(FormWidget *));
    // ID = index in "widgets" table
    switch (type) {
    case formButton:
        widgets[numChildren - 1] = new FormWidgetButton(doc, objA, numChildren - 1, aref, this);
        break;
    case formText:
        widgets[numChildren - 1] = new FormWidgetText(doc, objA, numChildren - 1, aref, this);
        break;
    case formChoice:
        widgets[numChildren - 1] = new FormWidgetChoice(doc, objA, numChildren - 1, aref, this);
        break;
    case formSignature:
        widgets[numChildren - 1] = new FormWidgetSignature(doc, objA, numChildren - 1, aref, this);
        break;
    default:
        error(errSyntaxWarning, -1, "SubType on non-terminal field, invalid document?");
        numChildren--;
    }
}

FormWidget *FormField::findWidgetByRef(Ref aref)
{
    if (terminal) {
        for (int i = 0; i < numChildren; i++) {
            if (widgets[i]->getRef() == aref) {
                return widgets[i];
            }
        }
    } else {
        for (int i = 0; i < numChildren; i++) {
            FormWidget *result = children[i]->findWidgetByRef(aref);
            if (result) {
                return result;
            }
        }
    }
    return nullptr;
}

GooString *FormField::getFullyQualifiedName()
{
    Object parentObj;
    const GooString *parent_name;
    bool unicode_encoded = false;

    if (fullyQualifiedName) {
        return fullyQualifiedName;
    }

    fullyQualifiedName = new GooString();

    std::set<int> parsedRefs;
    Ref parentRef;
    parentObj = obj.getDict()->lookup("Parent", &parentRef);
    if (parentRef != Ref::INVALID()) {
        parsedRefs.insert(parentRef.num);
    }
    while (parentObj.isDict()) {
        Object obj2 = parentObj.dictLookup("T");
        if (obj2.isString()) {
            parent_name = obj2.getString();

            if (unicode_encoded) {
                fullyQualifiedName->insert(0, "\0.", 2); // 2-byte unicode period
                if (parent_name->hasUnicodeMarker()) {
                    fullyQualifiedName->insert(0, parent_name->c_str() + 2, parent_name->getLength() - 2); // Remove the unicode BOM
                } else {
                    int tmp_length;
                    char *tmp_str = pdfDocEncodingToUTF16(parent_name->toStr(), &tmp_length);
                    fullyQualifiedName->insert(0, tmp_str + 2, tmp_length - 2); // Remove the unicode BOM
                    delete[] tmp_str;
                }
            } else {
                fullyQualifiedName->insert(0, '.'); // 1-byte ascii period
                if (parent_name->hasUnicodeMarker()) {
                    unicode_encoded = true;
                    fullyQualifiedName = convertToUtf16(fullyQualifiedName);
                    fullyQualifiedName->insert(0, parent_name->c_str() + 2, parent_name->getLength() - 2); // Remove the unicode BOM
                } else {
                    fullyQualifiedName->insert(0, parent_name);
                }
            }
        }
        parentObj = parentObj.getDict()->lookup("Parent", &parentRef);
        if (parentRef != Ref::INVALID() && !parsedRefs.insert(parentRef.num).second) {
            error(errSyntaxError, -1, "FormField: Loop while trying to look for Parents\n");
            return fullyQualifiedName;
        }
    }

    if (partialName) {
        if (unicode_encoded) {
            if (partialName->hasUnicodeMarker()) {
                fullyQualifiedName->append(partialName->c_str() + 2, partialName->getLength() - 2); // Remove the unicode BOM
            } else {
                int tmp_length;
                char *tmp_str = pdfDocEncodingToUTF16(partialName->toStr(), &tmp_length);
                fullyQualifiedName->append(tmp_str + 2, tmp_length - 2); // Remove the unicode BOM
                delete[] tmp_str;
            }
        } else {
            if (partialName->hasUnicodeMarker()) {
                unicode_encoded = true;
                fullyQualifiedName = convertToUtf16(fullyQualifiedName);
                fullyQualifiedName->append(partialName->c_str() + 2, partialName->getLength() - 2); // Remove the unicode BOM
            } else {
                fullyQualifiedName->append(partialName);
            }
        }
    } else {
        int len = fullyQualifiedName->getLength();
        // Remove the last period
        if (unicode_encoded) {
            if (len > 1) {
                fullyQualifiedName->del(len - 2, 2);
            }
        } else {
            if (len > 0) {
                fullyQualifiedName->del(len - 1, 1);
            }
        }
    }

    if (unicode_encoded) {
        fullyQualifiedName->prependUnicodeMarker();
    }

    return fullyQualifiedName;
}

void FormField::updateChildrenAppearance()
{
    // Recursively update each child's appearance
    if (terminal) {
        for (int i = 0; i < numChildren; i++) {
            widgets[i]->updateWidgetAppearance();
        }
    } else {
        for (int i = 0; i < numChildren; i++) {
            children[i]->updateChildrenAppearance();
        }
    }
}

void FormField::setReadOnly(bool value)
{
    if (value == readOnly) {
        return;
    }

    readOnly = value;

    Dict *dict = obj.getDict();

    const Object obj1 = Form::fieldLookup(dict, "Ff");
    int flags = 0;
    if (obj1.isInt()) {
        flags = obj1.getInt();
    }
    if (value) {
        flags |= 1;
    } else {
        flags &= ~1;
    }

    dict->set("Ff", Object(flags));
    xref->setModifiedObject(&obj, ref);
    updateChildrenAppearance();
}

void FormField::reset(const std::vector<std::string> &excludedFields)
{
    resetChildren(excludedFields);
}

void FormField::resetChildren(const std::vector<std::string> &excludedFields)
{
    if (!terminal) {
        for (int i = 0; i < numChildren; i++) {
            children[i]->reset(excludedFields);
        }
    }
}

bool FormField::isAmongExcludedFields(const std::vector<std::string> &excludedFields)
{
    Ref fieldRef;

    for (const std::string &field : excludedFields) {
        if (field.compare(field.size() - 2, 2, " R") == 0) {
            if (sscanf(field.c_str(), "%d %d R", &fieldRef.num, &fieldRef.gen) == 2 && fieldRef == getRef()) {
                return true;
            }
        } else {
            if (field == getFullyQualifiedName()->toStr()) {
                return true;
            }
        }
    }

    return false;
}

FormField *FormField::findFieldByRef(Ref aref)
{
    if (terminal) {
        if (this->getRef() == aref) {
            return this;
        }
    } else {
        for (int i = 0; i < numChildren; i++) {
            FormField *result = children[i]->findFieldByRef(aref);
            if (result) {
                return result;
            }
        }
    }
    return nullptr;
}

FormField *FormField::findFieldByFullyQualifiedName(const std::string &name)
{
    if (terminal) {
        if (getFullyQualifiedName()->cmp(name.c_str()) == 0) {
            return this;
        }
    } else {
        for (int i = 0; i < numChildren; i++) {
            FormField *result = children[i]->findFieldByFullyQualifiedName(name);
            if (result) {
                return result;
            }
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------
// FormFieldButton
//------------------------------------------------------------------------
FormFieldButton::FormFieldButton(PDFDoc *docA, Object &&dictObj, const Ref refA, FormField *parentA, std::set<int> *usedParents) : FormField(docA, std::move(dictObj), refA, parentA, usedParents, formButton)
{
    Dict *dict = obj.getDict();
    active_child = -1;
    noAllOff = false;
    siblings = nullptr;
    numSiblings = 0;
    appearanceState.setToNull();
    defaultAppearanceState.setToNull();

    btype = formButtonCheck;
    Object obj1 = Form::fieldLookup(dict, "Ff");
    if (obj1.isInt()) {
        int flags = obj1.getInt();

        if (flags & 0x10000) { // 17 -> push button
            btype = formButtonPush;
        } else if (flags & 0x8000) { // 16 -> radio button
            btype = formButtonRadio;
            if (flags & 0x4000) { // 15 -> noToggleToOff
                noAllOff = true;
            }
        }
        if (flags & 0x1000000) { // 26 -> radiosInUnison
            error(errUnimplemented, -1, "FormFieldButton:: radiosInUnison flag unimplemented, please report a bug with a testcase\n");
        }
    }

    bool isChildRadiobutton = btype == formButtonRadio && terminal && parent && parent->getType() == formButton;
    // Ignore "V" for child radiobuttons, so FormFieldButton::getState() does not use it and instead uses the
    // "V" of the parent, which is the real value indicating the active field in the radio group. Issue #159
    if (btype != formButtonPush && !isChildRadiobutton) {
        // Even though V is inheritable we are interested in the value of this
        // field, if not present it's probably because it's a button in a set.
        appearanceState = dict->lookup("V");
        defaultAppearanceState = Form::fieldLookup(dict, "DV");
    }
}

static const char *_getButtonType(FormButtonType type)
{
    switch (type) {
    case formButtonPush:
        return "push";
    case formButtonCheck:
        return "check";
    case formButtonRadio:
        return "radio";
    default:
        break;
    }
    return "unknown";
}

void FormFieldButton::print(int indent)
{
    printf("%*s- (%d %d): [%s] terminal: %s children: %d\n", indent, "", ref.num, ref.gen, _getButtonType(btype), terminal ? "Yes" : "No", numChildren);
}

void FormFieldButton::setNumSiblings(int num)
{
    numSiblings = num;
    siblings = (FormFieldButton **)greallocn(siblings, numSiblings, sizeof(FormFieldButton *));
}

void FormFieldButton::fillChildrenSiblingsID()
{
    if (!terminal) {
        for (int i = 0; i < numChildren; i++) {
            FormFieldButton *child = dynamic_cast<FormFieldButton *>(children[i]);
            if (child != nullptr) {
                // Fill the siblings of this node childs
                child->setNumSiblings(numChildren - 1);
                for (int j = 0, counter = 0; j < numChildren; j++) {
                    FormFieldButton *otherChild = dynamic_cast<FormFieldButton *>(children[j]);
                    if (i == j) {
                        continue;
                    }
                    if (child == otherChild) {
                        continue;
                    }
                    child->setSibling(counter, otherChild);
                    counter++;
                }

                // now call ourselves on the child
                // to fill its children data
                child->fillChildrenSiblingsID();
            }
        }
    }
}

bool FormFieldButton::setState(const char *state, bool ignoreToggleOff)
{
    // A check button could behave as a radio button
    // when it's in a set of more than 1 buttons
    if (btype != formButtonRadio && btype != formButtonCheck) {
        return false;
    }

    if (terminal && parent && parent->getType() == formButton && appearanceState.isNull()) {
        // It's button in a set, set state on parent
        if (static_cast<FormFieldButton *>(parent)->setState(state)) {
            return true;
        }
        return false;
    }

    bool isOn = strcmp(state, "Off") != 0;

    if (!isOn && noAllOff && !ignoreToggleOff) {
        return false; // Don't allow to set all radio to off
    }

    const char *current = getAppearanceState();
    bool currentFound = false, newFound = false;

    for (int i = 0; i < numChildren; i++) {
        FormWidgetButton *widget;

        // If radio button is a terminal field we want the widget at i, but
        // if it's not terminal, the child widget is a composed dict, so
        // we want the ony child widget of the children at i
        if (terminal) {
            widget = static_cast<FormWidgetButton *>(widgets[i]);
        } else {
            widget = static_cast<FormWidgetButton *>(children[i]->getWidget(0));
        }

        if (!widget->getOnStr()) {
            continue;
        }

        const char *onStr = widget->getOnStr();
        if (current && strcmp(current, onStr) == 0) {
            widget->setAppearanceState("Off");
            if (!isOn) {
                break;
            }
            currentFound = true;
        }

        if (isOn && strcmp(state, onStr) == 0) {
            widget->setAppearanceState(state);
            newFound = true;
        }

        if (currentFound && newFound) {
            break;
        }
    }

    updateState(state);

    return true;
}

bool FormFieldButton::getState(const char *state) const
{
    if (appearanceState.isName(state)) {
        return true;
    }

    return (parent && parent->getType() == formButton) ? static_cast<FormFieldButton *>(parent)->getState(state) : false;
}

void FormFieldButton::updateState(const char *state)
{
    appearanceState = Object(objName, state);
    obj.getDict()->set("V", appearanceState.copy());
    xref->setModifiedObject(&obj, ref);
}

FormFieldButton::~FormFieldButton()
{
    if (siblings) {
        gfree(siblings);
    }
}

void FormFieldButton::reset(const std::vector<std::string> &excludedFields)
{
    if (!isAmongExcludedFields(excludedFields)) {
        if (getDefaultAppearanceState()) {
            setState(getDefaultAppearanceState());
        } else {
            obj.getDict()->remove("V");

            // Clear check button if it doesn't have default value.
            // This behaviour is what Adobe Reader does, it is not written in specification.
            if (btype == formButtonCheck) {
                setState("Off");
            }
        }
    }

    resetChildren(excludedFields);
}

//------------------------------------------------------------------------
// FormFieldText
//------------------------------------------------------------------------
FormFieldText::FormFieldText(PDFDoc *docA, Object &&dictObj, const Ref refA, FormField *parentA, std::set<int> *usedParents) : FormField(docA, std::move(dictObj), refA, parentA, usedParents, formText)
{
    Dict *dict = obj.getDict();
    Object obj1;
    content = nullptr;
    internalContent = nullptr;
    defaultContent = nullptr;
    multiline = password = fileSelect = doNotSpellCheck = doNotScroll = comb = richText = false;
    maxLen = 0;

    obj1 = Form::fieldLookup(dict, "Ff");
    if (obj1.isInt()) {
        int flags = obj1.getInt();
        if (flags & 0x1000) { // 13 -> Multiline
            multiline = true;
        }
        if (flags & 0x2000) { // 14 -> Password
            password = true;
        }
        if (flags & 0x100000) { // 21 -> FileSelect
            fileSelect = true;
        }
        if (flags & 0x400000) { // 23 -> DoNotSpellCheck
            doNotSpellCheck = true;
        }
        if (flags & 0x800000) { // 24 -> DoNotScroll
            doNotScroll = true;
        }
        if (flags & 0x1000000) { // 25 -> Comb
            comb = true;
        }
        if (flags & 0x2000000) { // 26 -> RichText
            richText = true;
        }
    }

    obj1 = Form::fieldLookup(dict, "MaxLen");
    if (obj1.isInt()) {
        maxLen = obj1.getInt();
    }

    fillContent(fillDefaultValue);
    fillContent(fillValue);
}

void FormFieldText::fillContent(FillValueType fillType)
{
    Dict *dict = obj.getDict();
    Object obj1;

    obj1 = Form::fieldLookup(dict, fillType == fillDefaultValue ? "DV" : "V");
    if (obj1.isString()) {
        if (obj1.getString()->hasUnicodeMarker()) {
            if (obj1.getString()->getLength() > 2) {
                if (fillType == fillDefaultValue) {
                    defaultContent = obj1.getString()->copy();
                } else {
                    content = obj1.getString()->copy();
                }
            }
        } else if (obj1.getString()->getLength() > 0) {
            // non-unicode string -- assume pdfDocEncoding and try to convert to UTF16BE
            int tmp_length;
            char *tmp_str = pdfDocEncodingToUTF16(obj1.getString()->toStr(), &tmp_length);

            if (fillType == fillDefaultValue) {
                defaultContent = new GooString(tmp_str, tmp_length);
            } else {
                content = new GooString(tmp_str, tmp_length);
            }

            delete[] tmp_str;
        }
    }
}

void FormFieldText::print(int indent)
{
    printf("%*s- (%d %d): [text] terminal: %s children: %d\n", indent, "", ref.num, ref.gen, terminal ? "Yes" : "No", numChildren);
}

void FormFieldText::setContentCopy(const GooString *new_content)
{
    delete content;
    content = nullptr;

    if (new_content) {
        content = new_content->copy();

        // append the unicode marker <FE FF> if needed
        if (!content->hasUnicodeMarker()) {
            content->prependUnicodeMarker();
        }
        Form *form = doc->getCatalog()->getForm();
        if (form) {
            DefaultAppearance da(defaultAppearance);
            if (da.getFontName().isName()) {
                const std::string fontName = da.getFontName().getName();
                if (!fontName.empty()) {
                    // Use the field resource dictionary if it exists
                    Object fieldResourcesDictObj = obj.dictLookup("DR");
                    if (fieldResourcesDictObj.isDict()) {
                        GfxResources fieldResources(doc->getXRef(), fieldResourcesDictObj.getDict(), form->getDefaultResources());
                        const std::vector<Form::AddFontResult> newFonts = form->ensureFontsForAllCharacters(content, fontName, &fieldResources);
                        // If we added new fonts to the Form object default resuources we also need to add them (we only add the ref so this is cheap)
                        // to the field DR dictionary
                        for (const Form::AddFontResult &afr : newFonts) {
                            fieldResourcesDictObj.dictLookup("Font").dictAdd(afr.fontName.c_str(), Object(afr.ref));
                        }
                    } else {
                        form->ensureFontsForAllCharacters(content, fontName);
                    }
                }
            } else {
                // This is wrong, there has to be a Tf in DA
            }
        }
    }

    obj.getDict()->set("V", Object(content ? content->copy() : new GooString("")));
    xref->setModifiedObject(&obj, ref);
    updateChildrenAppearance();
}

void FormFieldText::setAppearanceContentCopy(const GooString *new_content)
{
    delete internalContent;
    internalContent = nullptr;

    if (new_content) {
        internalContent = new_content->copy();
    }
    updateChildrenAppearance();
}

FormFieldText::~FormFieldText()
{
    delete content;
    delete internalContent;
    delete defaultContent;
}

void FormFieldText::reset(const std::vector<std::string> &excludedFields)
{
    if (!isAmongExcludedFields(excludedFields)) {
        setContentCopy(defaultContent);
        if (defaultContent == nullptr) {
            obj.getDict()->remove("V");
        }
    }

    resetChildren(excludedFields);
}

double FormFieldText::getTextFontSize()
{
    std::vector<std::string> daToks;
    int idx = parseDA(&daToks);
    double fontSize = -1;
    if (idx >= 0) {
        char *p = nullptr;
        fontSize = strtod(daToks[idx].c_str(), &p);
        if (!p || *p) {
            fontSize = -1;
        }
    }
    return fontSize;
}

void FormFieldText::setTextFontSize(int fontSize)
{
    if (fontSize > 0 && obj.isDict()) {
        std::vector<std::string> daToks;
        int idx = parseDA(&daToks);
        if (idx == -1) {
            error(errSyntaxError, -1, "FormFieldText:: invalid DA object\n");
            return;
        }
        if (defaultAppearance) {
            delete defaultAppearance;
        }
        defaultAppearance = new GooString;
        for (std::size_t i = 0; i < daToks.size(); ++i) {
            if (i > 0) {
                defaultAppearance->append(' ');
            }
            if (i == (std::size_t)idx) {
                defaultAppearance->appendf("{0:d}", fontSize);
            } else {
                defaultAppearance->append(daToks[i]);
            }
        }
        obj.dictSet("DA", Object(defaultAppearance->copy()));
        xref->setModifiedObject(&obj, ref);
        updateChildrenAppearance();
    }
}

int FormFieldText::tokenizeDA(const std::string &da, std::vector<std::string> *daToks, const char *searchTok)
{
    int idx = -1;
    if (daToks) {
        size_t i = 0;
        size_t j = 0;
        while (i < da.size()) {
            while (i < da.size() && Lexer::isSpace(da[i])) {
                ++i;
            }
            if (i < da.size()) {
                for (j = i + 1; j < da.size() && !Lexer::isSpace(da[j]); ++j) { }
                std::string tok(da, i, j - i);
                if (searchTok && tok == searchTok) {
                    idx = daToks->size();
                }
                daToks->emplace_back(std::move(tok));
                i = j;
            }
        }
    }
    return idx;
}

int FormFieldText::parseDA(std::vector<std::string> *daToks)
{
    int idx = -1;
    if (obj.isDict()) {
        Object objDA(obj.dictLookup("DA"));
        if (objDA.isString()) {
            const GooString *da = objDA.getString();
            idx = tokenizeDA(da->toStr(), daToks, "Tf") - 1;
        }
    }
    return idx;
}

//------------------------------------------------------------------------
// FormFieldChoice
//------------------------------------------------------------------------
FormFieldChoice::FormFieldChoice(PDFDoc *docA, Object &&aobj, const Ref refA, FormField *parentA, std::set<int> *usedParents) : FormField(docA, std::move(aobj), refA, parentA, usedParents, formChoice)
{
    numChoices = 0;
    choices = nullptr;
    defaultChoices = nullptr;
    editedChoice = nullptr;
    topIdx = 0;

    Dict *dict = obj.getDict();
    Object obj1;

    combo = edit = multiselect = doNotSpellCheck = doCommitOnSelChange = false;

    obj1 = Form::fieldLookup(dict, "Ff");
    if (obj1.isInt()) {
        int flags = obj1.getInt();
        if (flags & 0x20000) { // 18 -> Combo
            combo = true;
        }
        if (flags & 0x40000) { // 19 -> Edit
            edit = true;
        }
        if (flags & 0x200000) { // 22 -> MultiSelect
            multiselect = true;
        }
        if (flags & 0x400000) { // 23 -> DoNotSpellCheck
            doNotSpellCheck = true;
        }
        if (flags & 0x4000000) { // 27 -> CommitOnSelChange
            doCommitOnSelChange = true;
        }
    }

    obj1 = dict->lookup("TI");
    if (obj1.isInt()) {
        topIdx = obj1.getInt();
        if (topIdx < 0) {
            error(errSyntaxError, -1, "FormFieldChoice:: invalid topIdx entry\n");
            topIdx = 0;
        }
    }

    obj1 = Form::fieldLookup(dict, "Opt");
    if (obj1.isArray()) {
        numChoices = obj1.arrayGetLength();
        choices = new ChoiceOpt[numChoices];
        memset(choices, 0, sizeof(ChoiceOpt) * numChoices);

        for (int i = 0; i < numChoices; i++) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isString()) {
                choices[i].optionName = obj2.getString()->copy();
            } else if (obj2.isArray()) { // [Export_value, Displayed_text]
                if (obj2.arrayGetLength() < 2) {
                    error(errSyntaxError, -1, "FormWidgetChoice:: invalid Opt entry -- array's length < 2\n");
                    continue;
                }
                Object obj3 = obj2.arrayGet(0);
                if (obj3.isString()) {
                    choices[i].exportVal = obj3.getString()->copy();
                } else {
                    error(errSyntaxError, -1, "FormWidgetChoice:: invalid Opt entry -- exported value not a string\n");
                }

                obj3 = obj2.arrayGet(1);
                if (obj3.isString()) {
                    choices[i].optionName = obj3.getString()->copy();
                } else {
                    error(errSyntaxError, -1, "FormWidgetChoice:: invalid Opt entry -- choice name not a string\n");
                }
            } else {
                error(errSyntaxError, -1, "FormWidgetChoice:: invalid {0:d} Opt entry\n", i);
            }
        }
    } else {
        // empty choice
    }

    // Find selected items
    // Note: PDF specs say that /V has precedence over /I, but acroread seems to
    // do the opposite. We do the same.
    obj1 = Form::fieldLookup(dict, "I");
    if (obj1.isArray()) {
        for (int i = 0; i < obj1.arrayGetLength(); i++) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isInt() && obj2.getInt() >= 0 && obj2.getInt() < numChoices) {
                choices[obj2.getInt()].selected = true;
            }
        }
    } else {
        // Note: According to PDF specs, /V should *never* contain the exportVal.
        // However, if /Opt is an array of (exportVal,optionName) pairs, acroread
        // seems to expect the exportVal instead of the optionName and so we do too.
        fillChoices(fillValue);
    }

    fillChoices(fillDefaultValue);
}

void FormFieldChoice::fillChoices(FillValueType fillType)
{
    const char *key = fillType == fillDefaultValue ? "DV" : "V";
    Dict *dict = obj.getDict();
    Object obj1;

    obj1 = Form::fieldLookup(dict, key);
    if (obj1.isString() || obj1.isArray()) {
        if (fillType == fillDefaultValue) {
            defaultChoices = new bool[numChoices];
            memset(defaultChoices, 0, sizeof(bool) * numChoices);
        }

        if (obj1.isString()) {
            bool optionFound = false;

            for (int i = 0; i < numChoices; i++) {
                if (choices[i].exportVal) {
                    if (choices[i].exportVal->cmp(obj1.getString()) == 0) {
                        optionFound = true;
                    }
                } else if (choices[i].optionName) {
                    if (choices[i].optionName->cmp(obj1.getString()) == 0) {
                        optionFound = true;
                    }
                }

                if (optionFound) {
                    if (fillType == fillDefaultValue) {
                        defaultChoices[i] = true;
                    } else {
                        choices[i].selected = true;
                    }
                    break; // We've determined that this option is selected. No need to keep on scanning
                }
            }

            // Set custom value if /V doesn't refer to any predefined option and the field is user-editable
            if (fillType == fillValue && !optionFound && edit) {
                editedChoice = obj1.getString()->copy();
            }
        } else if (obj1.isArray()) {
            for (int i = 0; i < numChoices; i++) {
                for (int j = 0; j < obj1.arrayGetLength(); j++) {
                    const Object obj2 = obj1.arrayGet(j);
                    if (!obj2.isString()) {
                        error(errSyntaxError, -1, "FormWidgetChoice:: {0:s} array contains a non string object", key);
                        continue;
                    }

                    bool matches = false;

                    if (choices[i].exportVal) {
                        if (choices[i].exportVal->cmp(obj2.getString()) == 0) {
                            matches = true;
                        }
                    } else if (choices[i].optionName) {
                        if (choices[i].optionName->cmp(obj2.getString()) == 0) {
                            matches = true;
                        }
                    }

                    if (matches) {
                        if (fillType == fillDefaultValue) {
                            defaultChoices[i] = true;
                        } else {
                            choices[i].selected = true;
                        }
                        break; // We've determined that this option is selected. No need to keep on scanning
                    }
                }
            }
        }
    }
}

FormFieldChoice::~FormFieldChoice()
{
    for (int i = 0; i < numChoices; i++) {
        delete choices[i].exportVal;
        delete choices[i].optionName;
    }
    delete[] choices;
    delete[] defaultChoices;
    delete editedChoice;
}

void FormFieldChoice::print(int indent)
{
    printf("%*s- (%d %d): [choice] terminal: %s children: %d\n", indent, "", ref.num, ref.gen, terminal ? "Yes" : "No", numChildren);
}

void FormFieldChoice::updateSelection()
{
    Object objV;
    Object objI(objNull);

    if (edit && editedChoice) {
        // This is an editable combo-box with user-entered text
        objV = Object(editedChoice->copy());
    } else {
        const int numSelected = getNumSelected();

        // Create /I array only if multiple selection is allowed (as per PDF spec)
        if (multiselect) {
            objI = Object(new Array(xref));
        }

        if (numSelected == 0) {
            // No options are selected
            objV = Object(new GooString(""));
        } else if (numSelected == 1) {
            // Only one option is selected
            for (int i = 0; i < numChoices; i++) {
                if (choices[i].selected) {
                    if (multiselect) {
                        objI.arrayAdd(Object(i));
                    }

                    if (choices[i].exportVal) {
                        objV = Object(choices[i].exportVal->copy());
                    } else if (choices[i].optionName) {
                        objV = Object(choices[i].optionName->copy());
                    }

                    break; // We've just written the selected option. No need to keep on scanning
                }
            }
        } else {
            // More than one option is selected
            objV = Object(new Array(xref));
            for (int i = 0; i < numChoices; i++) {
                if (choices[i].selected) {
                    if (multiselect) {
                        objI.arrayAdd(Object(i));
                    }

                    if (choices[i].exportVal) {
                        objV.arrayAdd(Object(choices[i].exportVal->copy()));
                    } else if (choices[i].optionName) {
                        objV.arrayAdd(Object(choices[i].optionName->copy()));
                    }
                }
            }
        }
    }

    obj.getDict()->set("V", std::move(objV));
    obj.getDict()->set("I", std::move(objI));
    xref->setModifiedObject(&obj, ref);
    updateChildrenAppearance();
}

void FormFieldChoice::unselectAll()
{
    for (int i = 0; i < numChoices; i++) {
        choices[i].selected = false;
    }
}

void FormFieldChoice::deselectAll()
{
    delete editedChoice;
    editedChoice = nullptr;

    unselectAll();
    updateSelection();
}

void FormFieldChoice::toggle(int i)
{
    delete editedChoice;
    editedChoice = nullptr;

    choices[i].selected = !choices[i].selected;
    updateSelection();
}

void FormFieldChoice::select(int i)
{
    delete editedChoice;
    editedChoice = nullptr;

    if (!multiselect) {
        unselectAll();
    }

    choices[i].selected = true;
    updateSelection();
}

void FormFieldChoice::setEditChoice(const GooString *new_content)
{
    delete editedChoice;
    editedChoice = nullptr;

    unselectAll();

    if (new_content) {
        editedChoice = new_content->copy();

        // append the unicode marker <FE FF> if needed
        if (!editedChoice->hasUnicodeMarker()) {
            editedChoice->prependUnicodeMarker();
        }
    }
    updateSelection();
}

const GooString *FormFieldChoice::getEditChoice() const
{
    return editedChoice;
}

int FormFieldChoice::getNumSelected()
{
    int cnt = 0;
    for (int i = 0; i < numChoices; i++) {
        if (choices[i].selected) {
            cnt++;
        }
    }
    return cnt;
}

const GooString *FormFieldChoice::getSelectedChoice() const
{
    if (edit && editedChoice) {
        return editedChoice;
    }

    for (int i = 0; i < numChoices; i++) {
        if (choices[i].optionName && choices[i].selected) {
            return choices[i].optionName;
        }
    }

    return nullptr;
}

void FormFieldChoice::reset(const std::vector<std::string> &excludedFields)
{
    if (!isAmongExcludedFields(excludedFields)) {
        delete editedChoice;
        editedChoice = nullptr;

        if (defaultChoices) {
            for (int i = 0; i < numChoices; i++) {
                choices[i].selected = defaultChoices[i];
            }
        } else {
            unselectAll();
        }
    }

    resetChildren(excludedFields);

    updateSelection();
}

//------------------------------------------------------------------------
// FormFieldSignature
//------------------------------------------------------------------------
FormFieldSignature::FormFieldSignature(PDFDoc *docA, Object &&dict, const Ref refA, FormField *parentA, std::set<int> *usedParents)
    : FormField(docA, std::move(dict), refA, parentA, usedParents, formSignature), signature_type(unsigned_signature_field), signature(nullptr)
{
    signature_info = new SignatureInfo();
    parseInfo();
}

FormFieldSignature::~FormFieldSignature()
{
    delete signature_info;
    delete signature;
}

void FormFieldSignature::setSignature(const GooString &sig)
{
    delete signature;
    signature = sig.copy();
}

const GooString &FormFieldSignature::getCustomAppearanceContent() const
{
    return customAppearanceContent;
}

void FormFieldSignature::setCustomAppearanceContent(const GooString &s)
{
    customAppearanceContent = GooString(s.toStr());
}

const GooString &FormFieldSignature::getCustomAppearanceLeftContent() const
{
    return customAppearanceLeftContent;
}

void FormFieldSignature::setCustomAppearanceLeftContent(const GooString &s)
{
    customAppearanceLeftContent = GooString(s.toStr());
}

double FormFieldSignature::getCustomAppearanceLeftFontSize() const
{
    return customAppearanceLeftFontSize;
}

void FormFieldSignature::setCustomAppearanceLeftFontSize(double size)
{
    customAppearanceLeftFontSize = size;
}

Ref FormFieldSignature::getImageResource() const
{
    return imageResource;
}

void FormFieldSignature::setImageResource(const Ref imageResourceA)
{
    imageResource = imageResourceA;
}

void FormFieldSignature::setCertificateInfo(std::unique_ptr<X509CertificateInfo> &certInfo)
{
    certificate_info.swap(certInfo);
}

FormWidget *FormFieldSignature::getCreateWidget()
{
    ::FormWidget *fw = getWidget(0);
    if (!fw) {
        error(errSyntaxError, 0, "FormFieldSignature: was asked for widget and didn't had one, creating it");
        _createWidget(&obj, ref);
        fw = getWidget(0);
        fw->createWidgetAnnotation();
    }
    return fw;
}

void FormFieldSignature::parseInfo()
{
    if (!obj.isDict()) {
        return;
    }

    // retrieve PKCS#7
    Object sig_dict = obj.dictLookup("V");
    if (!sig_dict.isDict()) {
        return;
    }

    Object contents_obj = sig_dict.dictLookup("Contents");
    if (contents_obj.isString()) {
        signature = contents_obj.getString()->copy();
    }

    byte_range = sig_dict.dictLookup("ByteRange");

    const Object location_obj = sig_dict.dictLookup("Location");
    if (location_obj.isString()) {
        signature_info->setLocation(location_obj.getString());
    }

    const Object reason_obj = sig_dict.dictLookup("Reason");
    if (reason_obj.isString()) {
        signature_info->setReason(reason_obj.getString());
    }

    // retrieve SigningTime
    Object time_of_signing = sig_dict.dictLookup("M");
    if (time_of_signing.isString()) {
        const GooString *time_str = time_of_signing.getString();
        signature_info->setSigningTime(dateStringToTime(time_str)); // Put this information directly in SignatureInfo object
    }

    // check if subfilter is supported for signature validation, only detached signatures work for now
    Object subfilterName = sig_dict.dictLookup("SubFilter");
    if (subfilterName.isName("adbe.pkcs7.sha1")) {
        signature_type = adbe_pkcs7_sha1;
        signature_info->setSubFilterSupport(true);
    } else if (subfilterName.isName("adbe.pkcs7.detached")) {
        signature_type = adbe_pkcs7_detached;
        signature_info->setSubFilterSupport(true);
    } else if (subfilterName.isName("ETSI.CAdES.detached")) {
        signature_type = ETSI_CAdES_detached;
        signature_info->setSubFilterSupport(true);
    } else {
        signature_type = unknown_signature_type;
    }
}

void FormFieldSignature::hashSignedDataBlock(CryptoSign::VerificationInterface *handler, Goffset block_len)
{
    if (!handler) {
        return;
    }
    const int BLOCK_SIZE = 4096;
    unsigned char signed_data_buffer[BLOCK_SIZE];

    Goffset i = 0;
    while (i < block_len) {
        Goffset bytes_left = block_len - i;
        if (bytes_left < BLOCK_SIZE) {
            doc->getBaseStream()->doGetChars(static_cast<int>(bytes_left), signed_data_buffer);
            handler->addData(signed_data_buffer, static_cast<int>(bytes_left));
            i = block_len;
        } else {
            doc->getBaseStream()->doGetChars(BLOCK_SIZE, signed_data_buffer);
            handler->addData(signed_data_buffer, BLOCK_SIZE);
            i += BLOCK_SIZE;
        }
    }
}

FormSignatureType FormWidgetSignature::signatureType() const
{
    return static_cast<FormFieldSignature *>(field)->getSignatureType();
}

void FormWidgetSignature::setSignatureType(FormSignatureType fst)
{
    static_cast<FormFieldSignature *>(field)->setSignatureType(fst);
}

SignatureInfo *FormFieldSignature::validateSignature(bool doVerifyCert, bool forceRevalidation, time_t validationTime, bool ocspRevocationCheck, bool enableAIA)
{
    auto backend = CryptoSign::Factory::createActive();
    if (!backend) {
        return signature_info;
    }

    if (signature_info->getSignatureValStatus() != SIGNATURE_NOT_VERIFIED && !forceRevalidation) {
        return signature_info;
    }

    if (signature == nullptr) {
        error(errSyntaxError, 0, "Invalid or missing Signature string");
        return signature_info;
    }

    if (!byte_range.isArray()) {
        error(errSyntaxError, 0, "Invalid or missing ByteRange array");
        return signature_info;
    }

    int arrayLen = byte_range.arrayGetLength();
    if (arrayLen < 2) {
        error(errSyntaxError, 0, "Too few elements in ByteRange array");
        return signature_info;
    }

    const int signature_len = signature->getLength();
    std::vector<unsigned char> signatureData(signature_len);
    memcpy(signatureData.data(), signature->c_str(), signature_len);
    auto signature_handler = backend->createVerificationHandler(std::move(signatureData));

    Goffset fileLength = doc->getBaseStream()->getLength();
    for (int i = 0; i < arrayLen / 2; i++) {
        Object offsetObj = byte_range.arrayGet(i * 2);
        Object lenObj = byte_range.arrayGet(i * 2 + 1);

        if (!offsetObj.isIntOrInt64() || !lenObj.isIntOrInt64()) {
            error(errSyntaxError, 0, "Illegal values in ByteRange array");
            return signature_info;
        }

        Goffset offset = offsetObj.getIntOrInt64();
        Goffset len = lenObj.getIntOrInt64();

        if (offset < 0 || offset >= fileLength || len < 0 || len > fileLength || offset + len > fileLength) {
            error(errSyntaxError, 0, "Illegal values in ByteRange array");
            return signature_info;
        }

        doc->getBaseStream()->setPos(offset);
        hashSignedDataBlock(signature_handler.get(), len);
    }

    if (!signature_info->isSubfilterSupported()) {
        error(errUnimplemented, 0, "Unable to validate this type of signature");
        return signature_info;
    }
    const SignatureValidationStatus sig_val_state = signature_handler->validateSignature();
    signature_info->setSignatureValStatus(sig_val_state);
    signature_info->setSignerName(signature_handler->getSignerName());
    signature_info->setSubjectDN(signature_handler->getSignerSubjectDN());
    signature_info->setHashAlgorithm(signature_handler->getHashAlgorithm());

    // verify if signature contains a 'signing time' attribute
    if (signature_handler->getSigningTime() != std::chrono::system_clock::time_point {}) {
        signature_info->setSigningTime(std::chrono::system_clock::to_time_t(signature_handler->getSigningTime()));
    }

    signature_info->setCertificateInfo(signature_handler->getCertificateInfo());

    if (sig_val_state != SIGNATURE_VALID || !doVerifyCert) {
        return signature_info;
    }

    const CertificateValidationStatus cert_val_state = signature_handler->validateCertificate(std::chrono::system_clock::from_time_t(validationTime), ocspRevocationCheck, enableAIA);
    signature_info->setCertificateValStatus(cert_val_state);

    return signature_info;
}

std::vector<Goffset> FormFieldSignature::getSignedRangeBounds() const
{
    std::vector<Goffset> range_vec;
    if (byte_range.isArray()) {
        if (byte_range.arrayGetLength() == 4) {
            for (int i = 0; i < 2; ++i) {
                const Object offsetObj(byte_range.arrayGet(2 * i));
                const Object lenObj(byte_range.arrayGet(2 * i + 1));
                if (offsetObj.isIntOrInt64() && lenObj.isIntOrInt64()) {
                    const Goffset offset = offsetObj.getIntOrInt64();
                    const Goffset len = lenObj.getIntOrInt64();
                    range_vec.push_back(offset);
                    range_vec.push_back(offset + len);
                }
            }
        }
    }
    return range_vec;
}

std::optional<GooString> FormFieldSignature::getCheckedSignature(Goffset *checkedFileSize)
{
    Goffset start = 0;
    Goffset end = 0;
    const std::vector<Goffset> ranges = getSignedRangeBounds();
    if (ranges.size() == 4) {
        start = ranges[1];
        end = ranges[2];
    }
    if (end >= start + 6) {
        BaseStream *stream = doc->getBaseStream();
        *checkedFileSize = stream->getLength();
        Goffset len = end - start;
        stream->setPos(end - 1);
        int c2 = stream->lookChar();
        stream->setPos(start);
        int c1 = stream->getChar();
        // PDF signatures are first ASN1 DER, then hex encoded PKCS#7 structures,
        // possibly padded with 0 characters and enclosed in '<' and '>'.
        // The ASN1 DER encoding of a PKCS#7 structure must start with the tag 0x30
        // for SEQUENCE. The next byte must be 0x80 for ASN1 DER indefinite length
        // encoding or (0x80 + n) for ASN1 DER definite length encoding
        // where n is the number of subsequent "length bytes" which big-endian
        // encode the length of the content of the SEQUENCE following them.
        if (len <= std::numeric_limits<int>::max() && *checkedFileSize > end && c1 == '<' && c2 == '>') {
            GooString gstr;
            ++start;
            --end;
            len = end - start;
            Goffset pos = 0;
            do {
                c1 = stream->getChar();
                if (c1 == EOF) {
                    return {};
                }
                gstr.append(static_cast<char>(c1));
            } while (++pos < len);
            if (gstr.getChar(0) == '3' && gstr.getChar(1) == '0') {
                if (gstr.getChar(2) == '8' && gstr.getChar(3) == '0') {
                    // ASN1 DER indefinite length encoding:
                    // We only check that all characters up to the enclosing '>'
                    // are hex characters and that there are two hex encoded 0 bytes
                    // just before the enclosing '>' marking the end of the indefinite
                    // length encoding.
                    int paddingCount = 0;
                    while (gstr.getChar(len - 1) == '0' && gstr.getChar(len - 2) == '0') {
                        ++paddingCount;
                        len -= 2;
                    }
                    if (paddingCount < 2 || len % 2 == 1) {
                        len = 0;
                    }
                } else if (gstr.getChar(2) == '8') {
                    // ASN1 DER definite length encoding:
                    // We calculate the length of the following bytes from the length bytes and
                    // check that after the length bytes and the following calculated number of
                    // bytes all bytes up to the enclosing '>' character are hex encoded 0 bytes.
                    int lenBytes = gstr.getChar(3) - '0';
                    if (lenBytes > 0 && lenBytes <= 4) {
                        int sigLen = 0;
                        for (int i = 0; i < 2 * lenBytes; ++i) {
                            sigLen <<= 4;
                            char c = gstr.getChar(i + 4);
                            if (isdigit(c)) {
                                sigLen += c - '0';
                            } else if (isxdigit(c) && c >= 'a') {
                                sigLen += c - 'a' + 10;
                            } else if (isxdigit(c) && c >= 'A') {
                                sigLen += c - 'A' + 10;
                            } else {
                                len = 0;
                                break;
                            }
                        }
                        if (sigLen > 0 && 2 * (sigLen + lenBytes) <= len - 4) {
                            for (Goffset i = 2 * (sigLen + lenBytes) + 4; i < len; ++i) {
                                if (gstr.getChar(i) != '0') {
                                    len = 0;
                                    break;
                                }
                            }
                        } else {
                            len = 0;
                        }
                    }
                }
                for (const char c : gstr.toStr()) {
                    if (!isxdigit(c)) {
                        len = 0;
                    }
                }
                if (len > 0) {
                    return GooString(&gstr, 0, len);
                }
            }
        }
    }
    return {};
}

void FormFieldSignature::print(int indent)
{
    printf("%*s- (%d %d): [signature] terminal: %s children: %d\n", indent, "", ref.num, ref.gen, terminal ? "Yes" : "No", numChildren);
}

//------------------------------------------------------------------------
// Form
//------------------------------------------------------------------------

Form::Form(PDFDoc *docA) : doc(docA)
{
    Object obj1;

    XRef *xref = doc->getXRef();

    size = 0;
    numFields = 0;
    rootFields = nullptr;
    quadding = VariableTextQuadding::leftJustified;
    defaultAppearance = nullptr;
    defaultResources = nullptr;

    Object *acroForm = doc->getCatalog()->getAcroForm();

    needAppearances = acroForm->dictLookup("NeedAppearances").getBoolWithDefaultValue(false);

    obj1 = acroForm->dictLookup("DA");
    if (obj1.isString()) {
        defaultAppearance = obj1.getString()->copy();
    }

    obj1 = acroForm->dictLookup("Q");
    if (obj1.isInt()) {
        const VariableTextQuadding aux = static_cast<VariableTextQuadding>(obj1.getInt());
        if (aux == VariableTextQuadding::leftJustified || aux == VariableTextQuadding::centered || aux == VariableTextQuadding::rightJustified) {
            quadding = static_cast<VariableTextQuadding>(aux);
        }
    }

    resDict = acroForm->dictLookup("DR");
    if (resDict.isDict()) {
        // At a minimum, this dictionary shall contain a Font entry
        obj1 = resDict.dictLookup("Font");
        if (obj1.isDict()) {
            defaultResources = new GfxResources(xref, resDict.getDict(), nullptr);
        }
    }
    if (!defaultResources) {
        resDict.setToNull();
    }

    obj1 = acroForm->dictLookup("Fields");
    if (obj1.isArray()) {
        Array *array = obj1.getArray();
        std::set<Ref> alreadyReadRefs;
        for (int i = 0; i < array->getLength(); i++) {
            Object obj2 = array->get(i);
            const Object &oref = array->getNF(i);
            if (!oref.isRef()) {
                error(errSyntaxWarning, -1, "Direct object in rootFields");
                continue;
            }

            if (!obj2.isDict()) {
                error(errSyntaxWarning, -1, "Reference in Fields array to an invalid or non existent object");
                continue;
            }

            if (alreadyReadRefs.find(oref.getRef()) != alreadyReadRefs.end()) {
                continue;
            }
            alreadyReadRefs.insert(oref.getRef());

            if (numFields >= size) {
                size += 16;
                rootFields = (FormField **)greallocn(rootFields, size, sizeof(FormField *));
            }

            std::set<int> usedParents;
            rootFields[numFields++] = createFieldFromDict(std::move(obj2), doc, oref.getRef(), nullptr, &usedParents);
        }
    } else {
        error(errSyntaxError, -1, "Can't get Fields array\n");
    }

    obj1 = acroForm->dictLookup("CO");
    if (obj1.isArray()) {
        Array *array = obj1.getArray();
        calculateOrder.reserve(array->getLength());
        for (int i = 0; i < array->getLength(); i++) {
            const Object &oref = array->getNF(i);
            if (!oref.isRef()) {
                error(errSyntaxWarning, -1, "Direct object in CO");
                continue;
            }
            calculateOrder.push_back(oref.getRef());
        }
    }

    //   for (int i = 0; i < numFields; i++)
    //     rootFields[i]->printTree();
}

Form::~Form()
{
    int i;
    for (i = 0; i < numFields; ++i) {
        delete rootFields[i];
    }
    gfree(rootFields);
    delete defaultAppearance;
    delete defaultResources;
}

// Look up an inheritable field dictionary entry.
static Object fieldLookup(Dict *field, const char *key, std::set<int> *usedParents)
{
    Dict *dict = field;
    Object obj = dict->lookup(key);
    if (!obj.isNull()) {
        return obj;
    }
    const Object &parent = dict->lookupNF("Parent");
    if (parent.isRef()) {
        const Ref ref = parent.getRef();
        if (usedParents->find(ref.num) == usedParents->end()) {
            usedParents->insert(ref.num);

            Object obj2 = parent.fetch(dict->getXRef());
            if (obj2.isDict()) {
                return fieldLookup(obj2.getDict(), key, usedParents);
            }
        }
    } else if (parent.isDict()) {
        return fieldLookup(parent.getDict(), key, usedParents);
    }
    return Object(objNull);
}

Object Form::fieldLookup(Dict *field, const char *key)
{
    std::set<int> usedParents;
    return ::fieldLookup(field, key, &usedParents);
}

FormField *Form::createFieldFromDict(Object &&obj, PDFDoc *docA, const Ref aref, FormField *parent, std::set<int> *usedParents)
{
    FormField *field;

    const Object obj2 = Form::fieldLookup(obj.getDict(), "FT");
    if (obj2.isName("Btn")) {
        field = new FormFieldButton(docA, std::move(obj), aref, parent, usedParents);
    } else if (obj2.isName("Tx")) {
        field = new FormFieldText(docA, std::move(obj), aref, parent, usedParents);
    } else if (obj2.isName("Ch")) {
        field = new FormFieldChoice(docA, std::move(obj), aref, parent, usedParents);
    } else if (obj2.isName("Sig")) {
        field = new FormFieldSignature(docA, std::move(obj), aref, parent, usedParents);
    } else { // we don't have an FT entry => non-terminal field
        field = new FormField(docA, std::move(obj), aref, parent, usedParents);
    }

    return field;
}

static const std::string kOurDictFontNamePrefix = "popplerfont";

std::string Form::findFontInDefaultResources(const std::string &fontFamily, const std::string &fontStyle) const
{
    if (!resDict.isDict()) {
        return {};
    }

    const std::string fontFamilyAndStyle = fontStyle.empty() ? fontFamily : fontFamily + " " + fontStyle;

    Object fontDictObj = resDict.dictLookup("Font");
    assert(fontDictObj.isDict());

    const Dict *fontDict = fontDictObj.getDict();
    for (int i = 0; i < fontDict->getLength(); ++i) {
        const char *key = fontDict->getKey(i);
        if (GooString::startsWith(key, kOurDictFontNamePrefix)) {
            const Object fontObj = fontDict->getVal(i);
            if (fontObj.isDict() && fontObj.dictIs("Font")) {
                const Object fontBaseFontObj = fontObj.dictLookup("BaseFont");
                if (fontBaseFontObj.isName(fontFamilyAndStyle.c_str())) {
                    return key;
                }
            }
        }
    }

    return {};
}

Form::AddFontResult Form::addFontToDefaultResources(const std::string &fontFamily, const std::string &fontStyle, bool forceName)
{
    FamilyStyleFontSearchResult findFontRes = globalParams->findSystemFontFileForFamilyAndStyle(fontFamily, fontStyle);
    std::vector<std::string> filesToIgnore;
    while (!findFontRes.filepath.empty()) {
        Form::AddFontResult addFontRes = addFontToDefaultResources(findFontRes.filepath, findFontRes.faceIndex, fontFamily, fontStyle, forceName);
        if (!addFontRes.fontName.empty()) {
            return addFontRes;
        }
        filesToIgnore.emplace_back(findFontRes.filepath);
        findFontRes = globalParams->findSystemFontFileForFamilyAndStyle(fontFamily, fontStyle, filesToIgnore);
    }
    return {};
}

Form::AddFontResult Form::addFontToDefaultResources(const std::string &filepath, int faceIndex, const std::string &fontFamily, const std::string &fontStyle, bool forceName)
{
    if (!GooString::endsWith(filepath, ".ttf") && !GooString::endsWith(filepath, ".ttc") && !GooString::endsWith(filepath, ".otf")) {
        error(errIO, -1, "We only support embedding ttf/ttc/otf fonts for now. The font file for {0:s} {1:s} was {2:s}", fontFamily.c_str(), fontStyle.c_str(), filepath.c_str());
        return {};
    }

    const FoFiIdentifierType fontFoFiType = FoFiIdentifier::identifyFile(filepath.c_str());
    if (fontFoFiType != fofiIdTrueType && fontFoFiType != fofiIdTrueTypeCollection && fontFoFiType != fofiIdOpenTypeCFF8Bit && fontFoFiType != fofiIdOpenTypeCFFCID) {
        error(errIO, -1, "We only support embedding ttf/ttc/otf fonts for now. The font file for {0:s} {1:s} was {2:s} of type {3:d}", fontFamily.c_str(), fontStyle.c_str(), filepath.c_str(), fontFoFiType);
        return {};
    }

    const std::string fontFamilyAndStyle = fontStyle.empty() ? fontFamily : fontFamily + " " + fontStyle;

    if (forceName && defaultResources && defaultResources->lookupFont(fontFamilyAndStyle.c_str())) {
        error(errInternal, -1, "Form::addFontToDefaultResources: Asked to forceName but font name exists {0:s}", fontFamilyAndStyle.c_str());
        return {};
    }

    XRef *xref = doc->getXRef();
    Object fontDict(new Dict(xref));
    fontDict.dictSet("Type", Object(objName, "Font"));
    fontDict.dictSet("Subtype", Object(objName, "Type0"));
    fontDict.dictSet("BaseFont", Object(objName, fontFamilyAndStyle.c_str()));

    fontDict.dictSet("Encoding", Object(objName, "Identity-H"));

    {
        std::unique_ptr<Array> descendantFonts = std::make_unique<Array>(xref);

        const bool isTrueType = (fontFoFiType == fofiIdTrueType || fontFoFiType == fofiIdTrueTypeCollection);
        std::unique_ptr<Dict> descendantFont = std::make_unique<Dict>(xref);
        descendantFont->set("Type", Object(objName, "Font"));
        descendantFont->set("Subtype", Object(objName, isTrueType ? "CIDFontType2" : "CIDFontType0"));
        descendantFont->set("BaseFont", Object(objName, fontFamilyAndStyle.c_str()));

        {
            // We only support fonts with identity cmaps for now
            Dict *cidSystemInfo = new Dict(xref);
            cidSystemInfo->set("Registry", Object(new GooString("Adobe")));
            cidSystemInfo->set("Ordering", Object(new GooString("Identity")));
            cidSystemInfo->set("Supplement", Object(0));
            descendantFont->set("CIDSystemInfo", Object(cidSystemInfo));
        }

        FT_Library freetypeLib;
        if (FT_Init_FreeType(&freetypeLib)) {
            error(errIO, -1, "FT_Init_FreeType failed");
            return {};
        }
        const std::unique_ptr<FT_Library, void (*)(FT_Library *)> freetypeLibDeleter(&freetypeLib, [](FT_Library *l) { FT_Done_FreeType(*l); });

        FT_Face face;
        if (ft_new_face_from_file(freetypeLib, filepath.c_str(), faceIndex, &face)) {
            error(errIO, -1, "ft_new_face_from_file failed for {0:s}", filepath.c_str());
            return {};
        }
        const std::unique_ptr<FT_Face, void (*)(FT_Face *)> faceDeleter(&face, [](FT_Face *f) { FT_Done_Face(*f); });

        if (FT_Set_Char_Size(face, 1000, 1000, 0, 0)) {
            error(errIO, -1, "FT_Set_Char_Size failed for {0:s}", filepath.c_str());
            return {};
        }

        {
            std::unique_ptr<Dict> fontDescriptor = std::make_unique<Dict>(xref);
            fontDescriptor->set("Type", Object(objName, "FontDescriptor"));
            fontDescriptor->set("FontName", Object(objName, fontFamilyAndStyle.c_str()));

            // a bit arbirary but the Flags field is mandatory...
            const std::string lowerCaseFontFamily = GooString::toLowerCase(fontFamily);
            if (lowerCaseFontFamily.find("serif") != std::string::npos && lowerCaseFontFamily.find("sans") == std::string::npos) {
                fontDescriptor->set("Flags", Object(2)); // Serif
            } else {
                fontDescriptor->set("Flags", Object(0)); // Sans Serif
            }

            Array *fontBBox = new Array(xref);
            fontBBox->add(Object(static_cast<int>(face->bbox.xMin)));
            fontBBox->add(Object(static_cast<int>(face->bbox.yMin)));
            fontBBox->add(Object(static_cast<int>(face->bbox.xMax)));
            fontBBox->add(Object(static_cast<int>(face->bbox.yMax)));
            fontDescriptor->set("FontBBox", Object(fontBBox));

            fontDescriptor->set("Ascent", Object(static_cast<int>(face->ascender)));

            fontDescriptor->set("Descent", Object(static_cast<int>(face->descender)));

            {
                const std::unique_ptr<GooFile> file(GooFile::open(filepath));
                if (!file) {
                    error(errIO, -1, "Failed to open {0:s}", filepath.c_str());
                    return {};
                }
                const Goffset fileSize = file->size();
                if (fileSize < 0) {
                    error(errIO, -1, "Failed to get file size for {0:s}", filepath.c_str());
                    return {};
                }
                // GooFile::read only takes an integer so for now we don't support huge fonts
                if (fileSize > std::numeric_limits<int>::max()) {
                    error(errIO, -1, "Font size is too big {0:s}", filepath.c_str());
                    return {};
                }
                char *dataPtr = static_cast<char *>(gmalloc(fileSize));
                const Goffset bytesRead = file->read(dataPtr, static_cast<int>(fileSize), 0);
                if (bytesRead != fileSize) {
                    error(errIO, -1, "Failed to read contents of {0:s}", filepath.c_str());
                    gfree(dataPtr);
                    return {};
                }

                if (isTrueType) {
                    const Ref fontFile2Ref = xref->addStreamObject(new Dict(xref), dataPtr, fileSize, StreamCompression::Compress);
                    fontDescriptor->set("FontFile2", Object(fontFile2Ref));
                } else {
                    Dict *fontFileStreamDict = new Dict(xref);
                    fontFileStreamDict->set("Subtype", Object(objName, "OpenType"));
                    const Ref fontFile3Ref = xref->addStreamObject(fontFileStreamDict, dataPtr, fileSize, StreamCompression::Compress);
                    fontDescriptor->set("FontFile3", Object(fontFile3Ref));
                }
            }

            const Ref fontDescriptorRef = xref->addIndirectObject(Object(fontDescriptor.release()));
            descendantFont->set("FontDescriptor", Object(fontDescriptorRef));
        }

        static const int basicMultilingualMaxCode = 65535;

        const std::unique_ptr<FoFiTrueType> fft = FoFiTrueType::load(filepath.c_str());
        if (fft) {

            // Look for the Unicode BMP cmaps, which are 0/3 or 3/1
            int unicodeBMPCMap = fft->findCmap(0, 3);
            if (unicodeBMPCMap < 0) {
                unicodeBMPCMap = fft->findCmap(3, 1);
            }
            if (unicodeBMPCMap < 0) {
                error(errIO, -1, "Font does not have an unicode BMP cmap {0:s}", filepath.c_str());
                return {};
            }

            CIDFontsWidthsBuilder fontsWidths;

            for (int code = 0; code <= basicMultilingualMaxCode; ++code) {
                const int glyph = fft->mapCodeToGID(unicodeBMPCMap, code);
                if (FT_Load_Glyph(face, glyph, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING)) {
                    fontsWidths.addWidth(code, 0);
                } else {
                    fontsWidths.addWidth(code, static_cast<int>(face->glyph->metrics.horiAdvance));
                }
            }
            Array *widths = new Array(xref);
            for (const auto &segment : fontsWidths.takeSegments()) {
                std::visit(
                        [&widths, &xref](auto &&s) {
                            using T = std::decay_t<decltype(s)>;
                            if constexpr (std::is_same_v<T, CIDFontsWidthsBuilder::ListSegment>) {
                                widths->add(Object(s.first));
                                auto widthsInner = std::make_unique<Array>(xref);
                                for (const auto &w : s.widths) {
                                    widthsInner->add(Object(w));
                                }
                                widths->add(Object(widthsInner.release()));
                            } else if constexpr (std::is_same_v<T, CIDFontsWidthsBuilder::RangeSegment>) {
                                widths->add(Object(s.first));
                                widths->add(Object(s.last));
                                widths->add(Object(s.width));
                            } else {
                                static_assert(always_false_v<T>, "non-exhaustive visitor");
                            }
                        },
                        segment);
            }
            descendantFont->set("W", Object(widths));

            char *dataPtr = static_cast<char *>(gmalloc(2 * (basicMultilingualMaxCode + 1)));
            int i = 0;

            for (int code = 0; code <= basicMultilingualMaxCode; ++code) {
                const int glyph = fft->mapCodeToGID(unicodeBMPCMap, code);
                dataPtr[i++] = (unsigned char)(glyph >> 8);
                dataPtr[i++] = (unsigned char)(glyph & 0xff);
            }
            const Ref cidToGidMapStream = xref->addStreamObject(new Dict(xref), dataPtr, basicMultilingualMaxCode * 2, StreamCompression::Compress);
            descendantFont->set("CIDToGIDMap", Object(cidToGidMapStream));
        }

        descendantFonts->add(Object(descendantFont.release()));

        fontDict.dictSet("DescendantFonts", Object(descendantFonts.release()));
    }

    const Ref fontDictRef = xref->addIndirectObject(fontDict);

    std::string dictFontName = forceName ? fontFamilyAndStyle : kOurDictFontNamePrefix;
    Object *acroForm = doc->getCatalog()->getAcroForm();
    if (resDict.isDict()) {
        Ref fontDictObjRef;
        Object fontDictObj = resDict.getDict()->lookup("Font", &fontDictObjRef);
        assert(fontDictObj.isDict());
        dictFontName = fontDictObj.getDict()->findAvailableKey(dictFontName);
        fontDictObj.dictSet(dictFontName.c_str(), Object(fontDictRef));

        if (fontDictObjRef != Ref::INVALID()) {
            xref->setModifiedObject(&fontDictObj, fontDictObjRef);
        } else {
            Ref resDictRef;
            acroForm->getDict()->lookup("DR", &resDictRef);
            if (resDictRef != Ref::INVALID()) {
                xref->setModifiedObject(&resDict, resDictRef);
            } else {
                doc->getCatalog()->setAcroFormModified();
            }
        }

        // maybe we can do something to reuse the existing data instead of recreating from scratch?
        delete defaultResources;
        defaultResources = new GfxResources(xref, resDict.getDict(), nullptr);
    } else {
        Dict *fontsDict = new Dict(xref);
        fontsDict->set(dictFontName.c_str(), Object(fontDictRef));

        Dict *defaultResourcesDict = new Dict(xref);
        defaultResourcesDict->set("Font", Object(fontsDict));

        assert(!defaultResources);
        defaultResources = new GfxResources(xref, defaultResourcesDict, nullptr);
        resDict = Object(defaultResourcesDict);

        acroForm->dictSet("DR", resDict.copy());
        doc->getCatalog()->setAcroFormModified();
    }

    return { dictFontName, fontDictRef };
}

std::string Form::getFallbackFontForChar(Unicode uChar, const GfxFont &fontToEmulate) const
{
    const UCharFontSearchResult res = globalParams->findSystemFontFileForUChar(uChar, fontToEmulate);

    return findFontInDefaultResources(res.family, res.style);
}

std::vector<Form::AddFontResult> Form::ensureFontsForAllCharacters(const GooString *unicodeText, const std::string &pdfFontNameToEmulate, GfxResources *fieldResources)
{
    GfxResources *resources = fieldResources ? fieldResources : defaultResources;
    std::shared_ptr<GfxFont> f;
    if (!resources) {
        // There's no resources, so create one with the needed font name
        addFontToDefaultResources(pdfFontNameToEmulate, "", /*forceName*/ true);
        resources = defaultResources;
    }
    f = resources->lookupFont(pdfFontNameToEmulate.c_str());
    const CharCodeToUnicode *ccToUnicode = f ? f->getToUnicode() : nullptr;
    if (!ccToUnicode) {
        error(errInternal, -1, "Form::ensureFontsForAllCharacters: No ccToUnicode, this should not happen\n");
        return {}; // will never happen with current code
    }

    std::vector<AddFontResult> newFonts;

    // If the text has some characters that are not available in the font, try adding a font for those
    std::unordered_set<Unicode> seen;
    for (int i = 2; i < unicodeText->getLength(); i += 2) {
        Unicode uChar = (unsigned char)(unicodeText->getChar(i)) << 8;
        uChar += (unsigned char)(unicodeText->getChar(i + 1));

        if (uChar < 128 && !std::isprint(static_cast<unsigned char>(uChar))) {
            continue;
        }
        if (seen.find(uChar) != seen.end()) {
            continue;
        }
        seen.insert(uChar);

        CharCode c;
        bool addFont = false;
        if (ccToUnicode->mapToCharCode(&uChar, &c, 1)) {
            if (f->isCIDFont()) {
                auto cidFont = static_cast<const GfxCIDFont *>(f.get());
                if (c < cidFont->getCIDToGIDLen() && c != 0 && c != '\r' && c != '\n') {
                    const int glyph = cidFont->getCIDToGID()[c];
                    if (glyph == 0) {
                        addFont = true;
                    }
                }
            }
        } else {
            addFont = true;
        }

        if (addFont) {
            Form::AddFontResult res = doGetAddFontToDefaultResources(uChar, *f);
            if (res.ref != Ref::INVALID()) {
                newFonts.emplace_back(res);
            }
        }
    }

    return newFonts;
}

Form::AddFontResult Form::doGetAddFontToDefaultResources(Unicode uChar, const GfxFont &fontToEmulate)
{
    const UCharFontSearchResult res = globalParams->findSystemFontFileForUChar(uChar, fontToEmulate);

    std::string pdfFontName = findFontInDefaultResources(res.family, res.style);
    if (pdfFontName.empty()) {
        return addFontToDefaultResources(res.filepath, res.faceIndex, res.family, res.style);
    }
    return { pdfFontName, Ref::INVALID() };
}

void Form::postWidgetsLoad()
{
    // We create the widget annotations associated to
    // every form widget here, because the AnnotWidget constructor
    // needs the form object that gets from the catalog. When constructing
    // a FormWidget the Catalog is still creating the form object
    for (int i = 0; i < numFields; i++) {
        rootFields[i]->fillChildrenSiblingsID();
        rootFields[i]->createWidgetAnnotations();
    }
}

FormWidget *Form::findWidgetByRef(Ref aref)
{
    for (int i = 0; i < numFields; i++) {
        FormWidget *result = rootFields[i]->findWidgetByRef(aref);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

FormField *Form::findFieldByRef(Ref aref) const
{
    for (int i = 0; i < numFields; i++) {
        FormField *result = rootFields[i]->findFieldByRef(aref);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

FormField *Form::findFieldByFullyQualifiedName(const std::string &name) const
{
    for (int i = 0; i < numFields; i++) {
        FormField *result = rootFields[i]->findFieldByFullyQualifiedName(name);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

void Form::reset(const std::vector<std::string> &fields, bool excludeFields)
{
    FormField *foundField;
    const bool resetAllFields = fields.empty();

    if (resetAllFields) {
        for (int i = 0; i < numFields; i++) {
            rootFields[i]->reset(std::vector<std::string>());
        }
    } else {
        if (!excludeFields) {
            for (const std::string &field : fields) {
                Ref fieldRef;

                if (field.size() > 1 && field.compare(field.size() - 2, 2, " R") == 0 && sscanf(field.c_str(), "%d %d R", &fieldRef.num, &fieldRef.gen) == 2) {
                    foundField = findFieldByRef(fieldRef);
                } else {
                    foundField = findFieldByFullyQualifiedName(field);
                }

                if (foundField) {
                    foundField->reset(std::vector<std::string>());
                }
            }
        } else {
            for (int i = 0; i < numFields; i++) {
                rootFields[i]->reset(fields);
            }
        }
    }
}

std::string Form::findPdfFontNameToUseForSigning()
{
    static constexpr std::array<const char *, 2> fontsToUseToSign = { "Helvetica", "Arial" };
    for (const char *fontToUseToSign : fontsToUseToSign) {
        std::string pdfFontName = findFontInDefaultResources(fontToUseToSign, "");
        if (!pdfFontName.empty()) {
            return pdfFontName;
        }

        pdfFontName = addFontToDefaultResources(fontToUseToSign, "").fontName;
        if (!pdfFontName.empty()) {
            return pdfFontName;
        }
    }

    error(errInternal, -1, "Form::findPdfFontNameToUseForSigning: No suitable font found'\n");

    return {};
}

//------------------------------------------------------------------------
// FormPageWidgets
//------------------------------------------------------------------------

FormPageWidgets::FormPageWidgets(Annots *annots, unsigned int page, Form *form)
{
    numWidgets = 0;
    widgets = nullptr;
    size = 0;

    if (annots && !annots->getAnnots().empty() && form) {
        size = annots->getAnnots().size();
        widgets = (FormWidget **)greallocn(widgets, size, sizeof(FormWidget *));

        /* For each entry in the page 'Annots' dict, try to find
           a matching form field */
        for (Annot *annot : annots->getAnnots()) {

            if (annot->getType() != Annot::typeWidget) {
                continue;
            }

            if (!annot->getHasRef()) {
                /* Since all entry in a form field's kid dict needs to be
                   indirect references, if this annot isn't indirect, it isn't
                   related to a form field */
                continue;
            }

            Ref r = annot->getRef();

            /* Try to find a form field which either has this Annot in its Kids entry
                or  is merged with this Annot */
            FormWidget *tmp = form->findWidgetByRef(r);
            if (tmp) {
                // We've found a corresponding form field, link it
                tmp->setID(FormWidget::encodeID(page, numWidgets));
                widgets[numWidgets++] = tmp;
            }
        }
    }
}

void FormPageWidgets::addWidgets(const std::vector<FormField *> &addedWidgets, unsigned int page)
{
    if (addedWidgets.empty()) {
        return;
    }

    size += addedWidgets.size();
    widgets = (FormWidget **)greallocn(widgets, size, sizeof(FormWidget *));

    for (auto frmField : addedWidgets) {
        FormWidget *frmWidget = frmField->getWidget(0);
        frmWidget->setID(FormWidget::encodeID(page, numWidgets));
        widgets[numWidgets++] = frmWidget;
    }
}

FormPageWidgets::~FormPageWidgets()
{
    gfree(widgets);
}
