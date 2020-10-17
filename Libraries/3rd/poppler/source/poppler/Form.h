//========================================================================
//
// Form.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2006 Julien Rebetez <julienr@svn.gnome.org>
// Copyright 2007, 2008, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2007-2010, 2012, 2015-2020 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Mark Riedesel <mark@klowner.com>
// Copyright 2011 Pino Toscano <pino@kde.org>
// Copyright 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2017 Roland Hieber <r.hieber@pengutronix.de>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2018 Andre Heinecke <aheinecke@intevation.de>
// Copyright 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2019 João Netto <joaonetto901@gmail.com>
// Copyright 2020 Nelson Benítez León <nbenitezl@gmail.com>
// Copyright 2020 Marek Kasik <mkasik@redhat.com>
//
//========================================================================

#ifndef FORM_H
#define FORM_H

#include "Object.h"
#include "Annot.h"

#include <ctime>

#include <set>
#include <vector>

class GooString;
class Array;
class Dict;
class Annot;
class AnnotWidget;
class Annots;
class LinkAction;
class GfxResources;
class PDFDoc;
class SignatureInfo;
class SignatureHandler;

enum FormFieldType
{
    formButton,
    formText,
    formChoice,
    formSignature,
    formUndef
};

enum FormButtonType
{
    formButtonCheck,
    formButtonPush,
    formButtonRadio
};

enum VariableTextQuadding
{
    quaddingLeftJustified,
    quaddingCentered,
    quaddingRightJustified
};

enum FormSignatureType
{
    adbe_pkcs7_sha1,
    adbe_pkcs7_detached,
    ETSI_CAdES_detached,
    unknown_signature_type
};

enum FillValueType
{
    fillValue,
    fillDefaultValue
};

class Form;
class FormField;
class FormFieldButton;
class FormFieldText;
class FormFieldSignature;
class FormFieldChoice;

//------------------------------------------------------------------------
// FormWidget
// A FormWidget represents the graphical part of a field and is "attached"
// to a page.
//------------------------------------------------------------------------

class FormWidget
{
public:
    virtual ~FormWidget();

    // Check if point is inside the field bounding rect
    bool inRect(double x, double y) const;

    // Get the field bounding rect
    void getRect(double *x1, double *y1, double *x2, double *y2) const;

    unsigned getID() { return ID; }
    void setID(unsigned int i) { ID = i; }

    FormField *getField() { return field; }
    FormFieldType getType() { return type; }

    Object *getObj() { return &obj; }
    Ref getRef() { return ref; }

    void setChildNum(unsigned i) { childNum = i; }
    unsigned getChildNum() { return childNum; }

    const GooString *getPartialName() const;
    void setPartialName(const GooString &name);
    const GooString *getAlternateUiName() const;
    const GooString *getMappingName() const;
    GooString *getFullyQualifiedName();

    bool isModified() const;

    bool isReadOnly() const;
    void setReadOnly(bool value);

    LinkAction *getActivationAction(); // The caller should not delete the result
    std::unique_ptr<LinkAction> getAdditionalAction(Annot::FormAdditionalActionsType type);
    bool setAdditionalAction(Annot::FormAdditionalActionsType t, const GooString &js);

    // return the unique ID corresponding to pageNum/fieldNum
    static int encodeID(unsigned pageNum, unsigned fieldNum);
    // decode id and retrieve pageNum and fieldNum
    static void decodeID(unsigned id, unsigned *pageNum, unsigned *fieldNum);

    void createWidgetAnnotation();
    AnnotWidget *getWidgetAnnotation() const { return widget; }

    virtual void updateWidgetAppearance() = 0;

    void print(int indent = 0);

protected:
    FormWidget(PDFDoc *docA, Object *aobj, unsigned num, Ref aref, FormField *fieldA);

    AnnotWidget *widget;
    FormField *field;
    FormFieldType type;
    Object obj;
    Ref ref;
    PDFDoc *doc;
    XRef *xref;

    // index of this field in the parent's child list
    unsigned childNum;

    /*
    Field ID is an (unsigned) integer, calculated as follow :
    the first sizeof/2 bits are the field number, relative to the page
    the last sizeof/2 bits are the page number
    [page number | field number]
    (encoding) id = (pageNum << 4*sizeof(unsigned)) + fieldNum;
    (decoding) pageNum = id >> 4*sizeof(unsigned); fieldNum = (id << 4*sizeof(unsigned)) >> 4*sizeof(unsigned);
    */
    unsigned ID;
};

//------------------------------------------------------------------------
// FormWidgetButton
//------------------------------------------------------------------------

class FormWidgetButton : public FormWidget
{
public:
    FormWidgetButton(PDFDoc *docA, Object *dictObj, unsigned num, Ref ref, FormField *p);
    ~FormWidgetButton() override;

    FormButtonType getButtonType() const;

    void setState(bool state);
    bool getState() const;

    const char *getOnStr() const;
    void setAppearanceState(const char *state);
    void updateWidgetAppearance() override;

protected:
    FormFieldButton *parent() const;
    GooString *onStr;
};

//------------------------------------------------------------------------
// FormWidgetText
//------------------------------------------------------------------------

class FormWidgetText : public FormWidget
{
public:
    FormWidgetText(PDFDoc *docA, Object *dictObj, unsigned num, Ref ref, FormField *p);
    // return the field's content (UTF16BE)
    const GooString *getContent() const;

    // expects a UTF16BE string
    void setContent(const GooString *new_content);
    // sets the text inside the field appearance stream
    void setAppearanceContent(const GooString *new_content);

    void updateWidgetAppearance() override;

    bool isMultiline() const;
    bool isPassword() const;
    bool isFileSelect() const;
    bool noSpellCheck() const;
    bool noScroll() const;
    bool isComb() const;
    bool isRichText() const;
    int getMaxLen() const;
    // return the font size of the field's text
    double getTextFontSize();
    // set the font size of the field's text (currently only integer values)
    void setTextFontSize(int fontSize);

protected:
    FormFieldText *parent() const;
};

//------------------------------------------------------------------------
// FormWidgetChoice
//------------------------------------------------------------------------

class FormWidgetChoice : public FormWidget
{
public:
    FormWidgetChoice(PDFDoc *docA, Object *dictObj, unsigned num, Ref ref, FormField *p);
    ~FormWidgetChoice() override;

    int getNumChoices() const;
    // return the display name of the i-th choice (UTF16BE)
    const GooString *getChoice(int i) const;
    const GooString *getExportVal(int i) const;
    // select the i-th choice
    void select(int i);

    // toggle selection of the i-th choice
    void toggle(int i);

    // deselect everything
    void deselectAll();

    // except a UTF16BE string
    // only work for editable combo box, set the user-entered text as the current choice
    void setEditChoice(const GooString *new_content);

    const GooString *getEditChoice() const;

    void updateWidgetAppearance() override;
    bool isSelected(int i) const;

    bool isCombo() const;
    bool hasEdit() const;
    bool isMultiSelect() const;
    bool noSpellCheck() const;
    bool commitOnSelChange() const;
    bool isListBox() const;

protected:
    bool _checkRange(int i) const;
    FormFieldChoice *parent() const;
};

//------------------------------------------------------------------------
// FormWidgetSignature
//------------------------------------------------------------------------

class FormWidgetSignature : public FormWidget
{
public:
    FormWidgetSignature(PDFDoc *docA, Object *dictObj, unsigned num, Ref ref, FormField *p);
    void updateWidgetAppearance() override;

    FormSignatureType signatureType() const;
    // Use -1 for now as validationTime
    SignatureInfo *validateSignature(bool doVerifyCert, bool forceRevalidation, time_t validationTime);

    // returns a list with the boundaries of the signed ranges
    // the elements of the list are of type Goffset
    std::vector<Goffset> getSignedRangeBounds() const;

    // checks the length encoding of the signature and returns the hex encoded signature
    // if the check passed (and the checked file size as output parameter in checkedFileSize)
    // otherwise a nullptr is returned
    GooString *getCheckedSignature(Goffset *checkedFileSize);

    const GooString *getSignature() const;
};

//------------------------------------------------------------------------
// FormField
// A FormField implements the logical side of a field and is "attached" to
// the Catalog. This is an internal class and client applications should
// only interact with FormWidgets.
//------------------------------------------------------------------------

class FormField
{
public:
    FormField(PDFDoc *docA, Object &&aobj, const Ref aref, FormField *parent, std::set<int> *usedParents, FormFieldType t = formUndef);

    virtual ~FormField();

    // Accessors.
    FormFieldType getType() const { return type; }
    Object *getObj() { return &obj; }
    Ref getRef() { return ref; }

    void setReadOnly(bool value);
    bool isReadOnly() const { return readOnly; }
    void setStandAlone(bool value) { standAlone = value; }
    bool isStandAlone() const { return standAlone; }

    GooString *getDefaultAppearance() const { return defaultAppearance; }
    bool hasTextQuadding() const { return hasQuadding; }
    VariableTextQuadding getTextQuadding() const { return quadding; }

    const GooString *getPartialName() const { return partialName; }
    void setPartialName(const GooString &name);
    const GooString *getAlternateUiName() const { return alternateUiName; }
    const GooString *getMappingName() const { return mappingName; }
    GooString *getFullyQualifiedName();

    FormWidget *findWidgetByRef(Ref aref);
    int getNumWidgets() const { return terminal ? numChildren : 0; }
    FormWidget *getWidget(int i) const { return terminal ? widgets[i] : nullptr; }
    int getNumChildren() const { return !terminal ? numChildren : 0; }
    FormField *getChildren(int i) const { return children[i]; }

    // only implemented in FormFieldButton
    virtual void fillChildrenSiblingsID();

    void createWidgetAnnotations();

    void printTree(int indent = 0);
    virtual void print(int indent = 0);
    virtual void reset(const std::vector<std::string> &excludedFields);
    void resetChildren(const std::vector<std::string> &excludedFields);
    FormField *findFieldByRef(Ref aref);
    FormField *findFieldByFullyQualifiedName(const std::string &name);

protected:
    void _createWidget(Object *obj, Ref aref);
    void createChildren(std::set<int> *usedParents);
    void updateChildrenAppearance();
    bool isAmongExcludedFields(const std::vector<std::string> &excludedFields);

    FormFieldType type; // field type
    Ref ref;
    bool terminal;
    Object obj;
    PDFDoc *doc;
    XRef *xref;
    FormField **children;
    FormField *parent;
    int numChildren;
    FormWidget **widgets;
    bool readOnly;

    GooString *partialName; // T field
    GooString *alternateUiName; // TU field
    GooString *mappingName; // TM field
    GooString *fullyQualifiedName;

    // Variable Text
    GooString *defaultAppearance;
    bool hasQuadding;
    VariableTextQuadding quadding;

    // True when FormField is not part of Catalog's Field array (or there isn't one).
    bool standAlone;

private:
    FormField() { }
};

//------------------------------------------------------------------------
// FormFieldButton
//------------------------------------------------------------------------

class FormFieldButton : public FormField
{
public:
    FormFieldButton(PDFDoc *docA, Object &&dict, const Ref ref, FormField *parent, std::set<int> *usedParents);

    FormButtonType getButtonType() const { return btype; }

    bool noToggleToOff() const { return noAllOff; }

    // returns true if the state modification is accepted
    bool setState(const char *state);
    bool getState(const char *state) const;

    const char *getAppearanceState() const { return appearanceState.isName() ? appearanceState.getName() : nullptr; }
    const char *getDefaultAppearanceState() const { return defaultAppearanceState.isName() ? defaultAppearanceState.getName() : nullptr; }

    void fillChildrenSiblingsID() override;

    void setNumSiblings(int num);
    void setSibling(int i, FormFieldButton *id) { siblings[i] = id; }

    // For radio buttons, return the fields of the other radio buttons in the same group
    FormFieldButton *getSibling(int i) const { return siblings[i]; }
    int getNumSiblings() const { return numSiblings; }

    void print(int indent) override;
    void reset(const std::vector<std::string> &excludedFields) override;

    ~FormFieldButton() override;

protected:
    void updateState(const char *state);

    FormFieldButton **siblings; // IDs of dependent buttons (each button of a radio field has all the others buttons
                                // of the same field in this array)
    int numSiblings;

    FormButtonType btype;
    int size;
    int active_child; // only used for combo box
    bool noAllOff;
    Object appearanceState; // V
    Object defaultAppearanceState; // DV
};

//------------------------------------------------------------------------
// FormFieldText
//------------------------------------------------------------------------

class FormFieldText : public FormField
{
public:
    FormFieldText(PDFDoc *docA, Object &&dictObj, const Ref ref, FormField *parent, std::set<int> *usedParents);

    const GooString *getContent() const { return content; }
    const GooString *getAppearanceContent() const { return internalContent ? internalContent : content; }
    void setContentCopy(const GooString *new_content);
    void setAppearanceContentCopy(const GooString *new_content);
    ~FormFieldText() override;

    bool isMultiline() const { return multiline; }
    bool isPassword() const { return password; }
    bool isFileSelect() const { return fileSelect; }
    bool noSpellCheck() const { return doNotSpellCheck; }
    bool noScroll() const { return doNotScroll; }
    bool isComb() const { return comb; }
    bool isRichText() const { return richText; }

    int getMaxLen() const { return maxLen; }

    // return the font size of the field's text
    double getTextFontSize();
    // set the font size of the field's text (currently only integer values)
    void setTextFontSize(int fontSize);

    void print(int indent) override;
    void reset(const std::vector<std::string> &excludedFields) override;

    static int tokenizeDA(const GooString *daString, std::vector<GooString *> *daToks, const char *searchTok);

protected:
    int parseDA(std::vector<GooString *> *daToks);
    void fillContent(FillValueType fillType);

    GooString *content;
    GooString *internalContent;
    GooString *defaultContent;
    bool multiline;
    bool password;
    bool fileSelect;
    bool doNotSpellCheck;
    bool doNotScroll;
    bool comb;
    bool richText;
    int maxLen;
};

//------------------------------------------------------------------------
// FormFieldChoice
//------------------------------------------------------------------------

class FormFieldChoice : public FormField
{
public:
    FormFieldChoice(PDFDoc *docA, Object &&aobj, const Ref ref, FormField *parent, std::set<int> *usedParents);

    ~FormFieldChoice() override;

    int getNumChoices() const { return numChoices; }
    const GooString *getChoice(int i) const { return choices ? choices[i].optionName : nullptr; }
    const GooString *getExportVal(int i) const { return choices ? choices[i].exportVal : nullptr; }
    // For multi-select choices it returns the first one
    const GooString *getSelectedChoice() const;

    // select the i-th choice
    void select(int i);

    // toggle selection of the i-th choice
    void toggle(int i);

    // deselect everything
    void deselectAll();

    // only work for editable combo box, set the user-entered text as the current choice
    void setEditChoice(const GooString *new_content);

    const GooString *getEditChoice() const;

    bool isSelected(int i) const { return choices[i].selected; }

    int getNumSelected();

    bool isCombo() const { return combo; }
    bool hasEdit() const { return edit; }
    bool isMultiSelect() const { return multiselect; }
    bool noSpellCheck() const { return doNotSpellCheck; }
    bool commitOnSelChange() const { return doCommitOnSelChange; }
    bool isListBox() const { return !combo; }

    int getTopIndex() const { return topIdx; }

    void print(int indent) override;
    void reset(const std::vector<std::string> &excludedFields) override;

protected:
    void unselectAll();
    void updateSelection();
    void fillChoices(FillValueType fillType);

    bool combo;
    bool edit;
    bool multiselect;
    bool doNotSpellCheck;
    bool doCommitOnSelChange;

    struct ChoiceOpt
    {
        GooString *exportVal; // the export value ("internal" name)
        GooString *optionName; // displayed name
        bool selected; // if this choice is selected
    };

    int numChoices;
    ChoiceOpt *choices;
    bool *defaultChoices;
    GooString *editedChoice;
    int topIdx; // TI
};

//------------------------------------------------------------------------
// FormFieldSignature
//------------------------------------------------------------------------

class FormFieldSignature : public FormField
{
public:
    FormFieldSignature(PDFDoc *docA, Object &&dict, const Ref ref, FormField *parent, std::set<int> *usedParents);

    // Use -1 for now as validationTime
    SignatureInfo *validateSignature(bool doVerifyCert, bool forceRevalidation, time_t validationTime);

    // returns a list with the boundaries of the signed ranges
    // the elements of the list are of type Goffset
    std::vector<Goffset> getSignedRangeBounds() const;

    // checks the length encoding of the signature and returns the hex encoded signature
    // if the check passed (and the checked file size as output parameter in checkedFileSize)
    // otherwise a nullptr is returned
    GooString *getCheckedSignature(Goffset *checkedFileSize);

    ~FormFieldSignature() override;
    Object *getByteRange() { return &byte_range; }
    const GooString *getSignature() const { return signature; }
    FormSignatureType getSignatureType() const { return signature_type; }

private:
    void parseInfo();
    void hashSignedDataBlock(SignatureHandler *handler, Goffset block_len);

    FormSignatureType signature_type;
    Object byte_range;
    GooString *signature;
    SignatureInfo *signature_info;

    void print(int indent) override;
};

//------------------------------------------------------------------------
// Form
// This class handle the document-wide part of Form (things in the acroForm
// Catalog entry).
//------------------------------------------------------------------------

class Form
{
public:
    Form(PDFDoc *docA, Object *acroForm);

    ~Form();

    Form(const Form &) = delete;
    Form &operator=(const Form &) = delete;

    // Look up an inheritable field dictionary entry.
    static Object fieldLookup(Dict *field, const char *key);

    /* Creates a new Field of the type specified in obj's dict.
       used in Form::Form , FormField::FormField and
       Page::loadStandaloneFields */
    static FormField *createFieldFromDict(Object &&obj, PDFDoc *docA, const Ref aref, FormField *parent, std::set<int> *usedParents);

    Object *getObj() const { return acroForm; }
    bool getNeedAppearances() const { return needAppearances; }
    int getNumFields() const { return numFields; }
    FormField *getRootField(int i) const { return rootFields[i]; }
    const GooString *getDefaultAppearance() const { return defaultAppearance; }
    VariableTextQuadding getTextQuadding() const { return quadding; }
    GfxResources *getDefaultResources() const { return defaultResources; }
    Object *getDefaultResourcesObj() { return &resDict; }

    FormWidget *findWidgetByRef(Ref aref);
    FormField *findFieldByRef(Ref aref) const;
    FormField *findFieldByFullyQualifiedName(const std::string &name) const;

    void postWidgetsLoad();

    const std::vector<Ref> &getCalculateOrder() const { return calculateOrder; }

    void reset(const std::vector<std::string> &fields, bool excludeFields);

private:
    FormField **rootFields;
    int numFields;
    int size;
    PDFDoc *doc;
    XRef *xref;
    Object *acroForm;
    bool needAppearances;
    GfxResources *defaultResources;
    Object resDict;
    std::vector<Ref> calculateOrder;

    // Variable Text
    GooString *defaultAppearance;
    VariableTextQuadding quadding;
};

//------------------------------------------------------------------------
// FormPageWidgets
//------------------------------------------------------------------------

class FormPageWidgets
{
public:
    FormPageWidgets(Annots *annots, unsigned int page, Form *form);
    ~FormPageWidgets();

    FormPageWidgets(const FormPageWidgets &) = delete;
    FormPageWidgets &operator=(const FormPageWidgets &) = delete;

    int getNumWidgets() const { return numWidgets; }
    FormWidget *getWidget(int i) const { return widgets[i]; }
    void addWidgets(const std::vector<FormField *> &addedWidgets, unsigned int page);

private:
    FormWidget **widgets;
    int numWidgets;
    int size;
};

#endif
