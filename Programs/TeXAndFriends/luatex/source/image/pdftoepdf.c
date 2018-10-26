/*
pdftoepdf.w

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2015 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.
*/

#define __STDC_FORMAT_MACROS /* for PRId64 etc.  */

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#  include <inttypes.h>
#endif
#include "image/epdf.h"
#include "luatexcallbackids.h"

/* to be sorted out, we cannot include */

#define xfree(a) do { free(a); a = NULL; } while (0)

/* Conflict with pdfgen.h */

#ifndef pdf_out

#define pdf_out(pdf, A) do { pdf_room(pdf, 1); *(pdf->buf->p++) = A; } while (0)

#define pdf_check_space(pdf) do { \
    if (pdf->cave > 0) { \
        pdf_out(pdf, ' '); \
        pdf->cave = 0; \
    } \
} while (0)

#define pdf_set_space(pdf) \
    pdf->cave = 1;

#define pdf_reset_space(pdf) \
    pdf->cave = 0;

#endif

/* Maintain AVL tree of all PDF files for embedding */

static avl_table *PdfDocumentTree = NULL;

/* AVL sort PdfDocument into PdfDocumentTree by file_path */

static int CompPdfDocument(const void *pa, const void *pb, void *p )
{
    return strcmp(((const PdfDocument *) pa)->file_path, ((const PdfDocument *) pb)->file_path);
}

/* Returns pointer to PdfDocument structure for PDF file. */

static PdfDocument *findPdfDocument(char *file_path)
{
    PdfDocument *pdf_doc, tmp;
    if (file_path == NULL) {
        normal_error("pdf backend","empty filename when loading pdf file");
    } else if (PdfDocumentTree == NULL) {
        return NULL;
    }
    tmp.file_path = file_path;
    pdf_doc = (PdfDocument *) avl_find(PdfDocumentTree, &tmp);
    return pdf_doc;
}

#define PDF_CHECKSUM_SIZE 32

static char *get_file_checksum(const char *a, file_error_mode fe)
{
    struct stat finfo;
    char *ck = NULL;
    if (stat(a, &finfo) == 0) {
        off_t size = finfo.st_size;
        time_t mtime = finfo.st_mtime;
        ck = (char *) malloc(PDF_CHECKSUM_SIZE);
        if (ck == NULL)
            formatted_error("pdf inclusion","out of memory while processing '%s'", a);
        snprintf(ck, PDF_CHECKSUM_SIZE, "%" PRIu64 "_%" PRIu64, (uint64_t) size,(uint64_t) mtime);
    } else {
        switch (fe) {
            case FE_FAIL:
                formatted_error("pdf inclusion","could not stat() file '%s'", a);
                break;
            case FE_RETURN_NULL:
                if (ck != NULL)
                    free(ck);
                ck = NULL;
                break;
            default:
                assert(0);
        }
    }
    return ck;
}

static char *get_stream_checksum (const char *str, unsigned long long str_size){
    /* http://www.cse.yorku.ca/~oz/hash.html */
    /* djb2                                  */
    unsigned long hash ;
    char *ck = NULL;
    unsigned int i;
    hash = 5381;
    ck = (char *) malloc(STRSTREAM_CHECKSUM_SIZE+1);
    if (ck == NULL)
        normal_error("pdf inclusion","out of memory while processing a memstream");
    for(i=0; i<(unsigned int)(str_size); i++) {
        hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + str[i] */
    }
    snprintf(ck,STRSTREAM_CHECKSUM_SIZE+1,"%lx",hash);
    ck[STRSTREAM_CHECKSUM_SIZE]='\0';
    return ck;
}

/*
    Returns pointer to PdfDocument structure for PDF file.
    Creates a new PdfDocument structure if it doesn't exist yet.
    When fe = FE_RETURN_NULL, the function returns NULL in error case.
*/

PdfDocument *refPdfDocument(const char *file_path, file_error_mode fe, const char *userpassword, const char *ownerpassword)
{
    char *checksum, *path_copy;
    PdfDocument *pdf_doc;
    ppdoc *pdfe = NULL;
    int new_flag = 0;
    if ((checksum = get_file_checksum(file_path, fe)) == NULL) {
        return (PdfDocument *) NULL;
    }
    path_copy = xstrdup(file_path);
    if ((pdf_doc = findPdfDocument(path_copy)) == NULL) {
        new_flag = 1;
        pdf_doc = (PdfDocument*) xmalloc(sizeof( PdfDocument));
        pdf_doc->file_path = path_copy;
        pdf_doc->checksum = checksum;
        pdf_doc->pdfe = NULL;
        pdf_doc->inObjList = NULL;
        pdf_doc->ObjMapTree = NULL;
        pdf_doc->occurences = 0; /* 0 = unreferenced */
        pdf_doc->pc = 0;
        pdf_doc->is_mem = 0;
    } else {
        if (strncmp(pdf_doc->checksum, checksum, PDF_CHECKSUM_SIZE) != 0) {
            formatted_error("pdf inclusion","file has changed '%s'", file_path);
        }
        free(checksum);
        free(path_copy);
    }
    if (pdf_doc->pdfe == NULL) {
        pdfe = ppdoc_load(file_path);
        pdf_doc->pc++;
        /* todo: check if we might print the document */
        if (pdfe == NULL) {
            switch (fe) {
                case FE_FAIL:
                    normal_error("pdf inclusion","reading image failed");
                    break;
                case FE_RETURN_NULL:
                    if (pdf_doc->pdfe != NULL) {
                        ppdoc_free(pdfe);
                        pdf_doc->pdfe = NULL;
                    }
                    /* delete docName */
                    if (new_flag == 1) {
                        if (pdf_doc->file_path != NULL)
                            free(pdf_doc->file_path);
                        if (pdf_doc->checksum != NULL)
                            free(pdf_doc->checksum);
                        free(pdf_doc);
                    }
                    return (PdfDocument *) NULL;
                    break;
                default:
                    assert(0);
            }
        }
        if (pdfe != NULL) {
            if (ppdoc_crypt_status(pdfe) < 0) {
                ppdoc_crypt_pass(pdfe,userpassword,strlen(userpassword),NULL,0);
            }
            if (ppdoc_crypt_status(pdfe) < 0) {
                ppdoc_crypt_pass(pdfe,NULL,0,ownerpassword,strlen(ownerpassword));
            }
            if (ppdoc_crypt_status(pdfe) < 0) {
                formatted_error("pdf inclusion","the pdf file '%s' is encrypted, provide proper passwords",file_path);
            }
        }
        pdf_doc->pdfe = pdfe;
    }
    /* PDF file could be opened without problems, checksum ok. */
    if (PdfDocumentTree == NULL)
        PdfDocumentTree = avl_create(CompPdfDocument, NULL, &avl_xallocator);
    if ((PdfDocument *) avl_find(PdfDocumentTree, pdf_doc) == NULL) {
        avl_probe(PdfDocumentTree, pdf_doc);
    }
    pdf_doc->occurences++;
    return pdf_doc;
}

/*
    Returns pointer to PdfDocument structure for a PDF stream in memory of streamsize
    dimension. As before, creates a new PdfDocument structure if it doesn't exist yet
    with file_path = file_id
*/

PdfDocument *refMemStreamPdfDocument(char *docstream, unsigned long long streamsize,const char *file_id)
{
    char *checksum;
    char *file_path;
    PdfDocument *pdf_doc;
    ppdoc *pdfe = NULL;
    size_t  cnt = 0;
    checksum = get_stream_checksum(docstream, streamsize);
    cnt = strlen(file_id);
    file_path = (char *) malloc(cnt+STREAM_URI_LEN+STRSTREAM_CHECKSUM_SIZE+1); /* 1 for \0 */
    strcpy(file_path,STREAM_URI);
    strcat(file_path,file_id);
    strcat(file_path,checksum);
    file_path[cnt+STREAM_URI_LEN+STRSTREAM_CHECKSUM_SIZE]='\0';
    if ((pdf_doc = findPdfDocument(file_path)) == NULL) {
        /*new_flag = 1;*/
        pdf_doc = (PdfDocument*) xmalloc(sizeof( PdfDocument));
        pdf_doc->file_path = file_path;
        pdf_doc->checksum = checksum;
        pdf_doc->pdfe = NULL;
        pdf_doc->inObjList = NULL;
        pdf_doc->ObjMapTree = NULL;
        pdf_doc->occurences = 0; /* 0 = unreferenced */
        pdf_doc->pc = 0;
        pdf_doc->is_mem = 1;
        pdf_doc->memstream = docstream;
    } else {
        /* As is now, checksum is in file_path, so this check should be useless. */
        if (strncmp(pdf_doc->checksum, checksum, STRSTREAM_CHECKSUM_SIZE) != 0) {
            formatted_error("pdf inclusion","stream has changed '%s'", file_path);
        }
        free(file_path);
        free(checksum);
    }
    if (pdf_doc->pdfe == NULL) {
        pdfe = ppdoc_mem(docstream, streamsize);
        pdf_doc->pc++;
        if (pdfe == NULL) {
            normal_error("pdf inclusion","reading pdf Stream failed");
        }
        pdf_doc->pdfe = pdfe;
    }
    /* PDF file could be opened without problems, checksum ok. */
    if (PdfDocumentTree == NULL)
        PdfDocumentTree = avl_create(CompPdfDocument, NULL, &avl_xallocator);
    if ((PdfDocument *) avl_find(PdfDocumentTree, pdf_doc) == NULL) {
        avl_probe(PdfDocumentTree, pdf_doc);
    }
    pdf_doc->occurences++;
    return pdf_doc;
}

/*
    AVL sort ObjMap into ObjMapTree by object number and generation keep the ObjMap
    struct small, as these are accumulated until the end
*/

typedef struct ObjMap ObjMap ;

struct ObjMap {
    ppref * in;
    int out_num;
};

static int CompObjMap(const void *pa, const void *pb, void *p)
{
    const ppref *a = (((const ObjMap *) pa)->in);
    const ppref *b = (((const ObjMap *) pb)->in);
    if (a->number > b->number)
        return 1;
    else if (a->number < b->number)
        return -1;
    else if (a->version == b->version)
        return 0;
    else if (a->version < b->version)
        return -1;
    return 1;
}

static ObjMap *findObjMap(PdfDocument * pdf_doc, ppref * in)
{
    ObjMap *obj_map, tmp;
    if (pdf_doc->ObjMapTree == NULL)
        return NULL;
    tmp.in = in;
    obj_map = (ObjMap *) avl_find(pdf_doc->ObjMapTree, &tmp);
    return obj_map;
}

static void addObjMap(PdfDocument * pdf_doc, ppref * in, int out_num)
{
    ObjMap *obj_map = NULL;
    if (pdf_doc->ObjMapTree == NULL)
        pdf_doc->ObjMapTree = avl_create(CompObjMap, NULL, &avl_xallocator);
    obj_map = (ObjMap*)xmalloc(sizeof(ObjMap));
    obj_map->in = in;
    obj_map->out_num = out_num;
    avl_probe(pdf_doc->ObjMapTree, obj_map);
}

/*
    When copying the Resources of the selected page, all objects are
    copied recursively top-down.  The findObjMap() function checks if an
    object has already been copied; if so, instead of copying just the
    new object number will be referenced.  The ObjMapTree guarantees,
    that during the entire LuaTeX run any object from any embedded PDF
    file will end up max. once in the output PDF file.  Indirect objects
    are not fetched during copying, but get a new object number from
    LuaTeX and then will be appended into a linked list.
*/

static int addInObj(PDF pdf, PdfDocument * pdf_doc, ppref * ref)
{
    ObjMap *obj_map;
    InObj *p, *q, *n;
    if (ref->number == 0) {
        normal_error("pdf inclusion","reference to invalid object (broken pdf)");
    }
    if ((obj_map = findObjMap(pdf_doc, ref)) != NULL) {
        return obj_map->out_num;
    }
    n = (InObj*)xmalloc(sizeof(InObj));
    n->ref = ref;
    n->next = NULL;
    n->num = pdf_create_obj(pdf, obj_type_others, 0);
    addObjMap(pdf_doc, ref, n->num);
    if (pdf_doc->inObjList == NULL) {
        pdf_doc->inObjList = n;
    } else {
        /*
            It is important to add new objects at the end of the list,
            because new objects are being added while the list is being
            written out by writeRefs().
        */
        for (p = pdf_doc->inObjList; p != NULL; p = p->next)
            q = p;
        q->next = n;
    }
    return n->num;
}

static void copyObject(PDF, PdfDocument *, ppobj *);

static void copyString(PDF pdf, ppstring str)
{
    pdf_check_space(pdf);
    switch (ppstring_type((void *)(str))) {
        case PPSTRING_PLAIN:
            pdf_out(pdf, '(');
            pdf_out_block(pdf, (const char *) str, ppstring_size((void *)(str)));
            pdf_out(pdf, ')');
            break;
        case PPSTRING_BASE16:
            pdf_out(pdf, '<');
            pdf_out_block(pdf, (const char *) str, ppstring_size((void *)(str)));
            pdf_out(pdf, '>');
            break;
        case PPSTRING_BASE85:
            pdf_out(pdf, '<');
            pdf_out(pdf, '~');
            pdf_out_block(pdf, (const char *) str, ppstring_size((void *)(str)));
            pdf_out(pdf, '~');
            pdf_out(pdf, '>');
            break;
    }
    pdf_set_space(pdf);
}

/*
static void copyName(PDF pdf, ppname *name)
{
    pdf_add_name(pdf, (const char *) name);
}
*/

static void copyArray(PDF pdf, PdfDocument * pdf_doc, pparray * array)
{
    int i;
    int n = array->size;
    pdf_begin_array(pdf);
    for (i=0; i<n; ++i) {
        copyObject(pdf, pdf_doc, pparray_at(array,i));
    }
    pdf_end_array(pdf);
}

static void copyDict(PDF pdf, PdfDocument * pdf_doc, ppdict *dict)
{
    int i;
    int n = dict->size;
    pdf_begin_dict(pdf);
    for (i=0; i<n; ++i) {
        pdf_add_name(pdf, (const char *) ppdict_key(dict,i));
        copyObject(pdf, pdf_doc, ppdict_at(dict,i));
    }
    pdf_end_dict(pdf);
}

static void copyStreamStream(PDF pdf, ppstream * stream, int decode, int callback_id)
{
    uint8_t *data = NULL;
    size_t size = 0;
    if (0) {
        for (data = ppstream_first(stream, &size, decode); data != NULL; data = ppstream_next(stream, &size)) {
            pdf_out_block(pdf, (const char *) data, size);
        }
    } else {
        data = ppstream_all(stream,&size,decode);
        if (data != NULL) {
            /*tex We only do this when we recompress in which case we fetch the whole stream. */
            if (callback_id == 1) {
                callback_id = callback_defined(process_pdf_image_content_callback);
            }
            if (callback_id) {
                char *result = NULL;
                run_callback(callback_id, "S->S",(char *) data,&result);
                pdf_out_block(pdf, (const char *) (uint8_t *) result, size);
                xfree(result);
            } else {
                pdf_out_block(pdf, (const char *) data, size);
            }
        }
    }
    ppstream_done(stream);
}

static void copyStream(PDF pdf, PdfDocument * pdf_doc, ppstream * stream)
{
    ppdict *dict = stream->dict; /* bug in: stream_dict(stream) */
    if (pdf->compress_level == 0 || pdf->recompress) {
        ppobj * obj = ppdict_get_obj (dict, "Filter");
        int known = 0;
        if (obj != NULL && obj->type == PPNAME) {
            const char *codecs[] = {
                "ASCIIHexDecode", "ASCII85Decode", "RunLengthDecode",
                "FlateDecode", "LZWDecode", NULL
            };
            int k;
            const char *val = ppobj_get_name(obj);
            for (k = 0; codecs[k] != NULL; k++) {
                if (strcmp(val,codecs[k]) == 0) {
                    known = 1;
                    break;
                }
            }
        }
        if (known) {
            /*tex recompress or keep uncompressed */
            const char *ignoredkeys[] = {
                "Filter", "DecodeParms", "Length", "DL", NULL
            };
            int i;
            pdf_begin_dict(pdf);
            for (i=0; i<dict->size; ++i) {
                const char *key = ppdict_key(dict,i);
                int copy = 1;
                int k;
                for (k = 0; ignoredkeys[k] != NULL; k++) {
                    if (strcmp(key,ignoredkeys[k]) == 0) {
                        copy = 0;
                        break;
                    }
                }
                if (copy) {
                    pdf_add_name(pdf, key);
                    copyObject(pdf, pdf_doc, ppdict_at(dict,i));
                }
            }
            pdf_dict_add_streaminfo(pdf);
            pdf_end_dict(pdf);
            pdf_begin_stream(pdf);
            copyStreamStream(pdf, stream, 1, 0);
            pdf_end_stream(pdf);
            return ;
        }
    }
    /* copy as-is */
    copyDict(pdf, pdf_doc, dict);
    pdf_begin_stream(pdf);
    copyStreamStream(pdf, stream, 0, 0);
    pdf_end_stream(pdf);
}

static void copyObject(PDF pdf, PdfDocument * pdf_doc, ppobj * obj)
{
    switch (obj->type) {
        case PPNULL:
            pdf_add_null(pdf);
            break;
        case PPBOOL:
            pdf_add_bool(pdf,obj->integer);                     /* ppobj_get_bool_value(obj) */
            break;
        case PPINT:
            pdf_add_int(pdf,obj->integer);                      /* ppobj_get_int_value(obj) */
            break;
        case PPNUM:
            pdf_add_real(pdf,obj->number);                      /* ppobj_get_num_value(obj) */
            break;
        case PPNAME:
            pdf_add_name(pdf, (const char *) obj->name);        /* ppobj_get_name(obj) */
            break;
        case PPSTRING:
            copyString(pdf, obj->string);                       /* ppobj_get_string(obj) */
            break;
        case PPARRAY:
            copyArray(pdf, pdf_doc, obj->array);                /* ppobj_get_array(obj) */
            break;
        case PPDICT:
            copyDict(pdf, pdf_doc, obj->dict);                  /* ppobj_get_dict(obj) */
            break;
        case PPSTREAM:
            copyStream(pdf, pdf_doc, obj->stream);              /* ppobj_get_stream(obj) */
            break;
        case PPREF:
            pdf_add_ref(pdf, addInObj(pdf, pdf_doc, obj->ref)); /* ppobj_get_ref(obj) */
            break;
        default:
            break;
    }
}

static void writeRefs(PDF pdf, PdfDocument * pdf_doc)
{
    InObj *r, *n;
    ppobj * obj;
    for (r = pdf_doc->inObjList; r != NULL;) {
        obj = ppref_obj(r->ref);
        if (obj->type == PPSTREAM)
            pdf_begin_obj(pdf, r->num, OBJSTM_NEVER);
        else
            pdf_begin_obj(pdf, r->num, 2);
        copyObject(pdf, pdf_doc, obj);
        pdf_end_obj(pdf);
        n = r->next;
        free(r);
        r = n;
        pdf_doc->inObjList = n;
    }
}

/* get the pagebox coordinates according to the pagebox_spec */

static void somebox(ppdict *page, const char * key, pprect * box)
{
    pprect * r = ppdict_get_box(page, key, box);
    if (r != NULL) {
        box->lx = r->lx;
        box->ly = r->ly;
        box->rx = r->rx;
        box->ry = r->ry;
    }
}

static void get_pagebox(ppdict * page, int pagebox_spec, pprect * box)
{
    box->lx = box->rx = box->ly = box->ry = 0;
    somebox(page,"MediaBox",box);
    if (pagebox_spec == PDF_BOX_SPEC_MEDIA) {
        return;
    }
    somebox(page,"CropBox",box);
    if (pagebox_spec == PDF_BOX_SPEC_CROP) {
        return;
    }
    switch (pagebox_spec) {
        case PDF_BOX_SPEC_BLEED:
            somebox(page,"BleedBox",box);
            break;
        case PDF_BOX_SPEC_TRIM:
            somebox(page,"TrimBox",box);
            break;
        case PDF_BOX_SPEC_ART:
            somebox(page,"ArtBox",box);
            break;
        default:
            break;
    }
}

/*
    Reads various information about the PDF and sets it up for later inclusion.
    This will fail if the PDF version of the PDF is higher than minor_pdf_version_wanted
    or page_name is given and can not be found. It makes no sense to give page_name and
    page_num. Returns the page number.
*/

static ppdict * get_pdf_page_dict(ppdoc *pdfe, int n)
{
    ppref *r;
    int i;
    for (r=ppdoc_first_page(pdfe), i=1; r != NULL; r = ppdoc_next_page(pdfe), ++i) {
        if (i == n) {
            return ppref_obj(r)->dict;
        }
    }
    return NULL;
}

// static ppdict * get_pdf_page_dict(ppdoc *pdfe, int n)
// {
//     return ppref_obj(ppdoc_page(pdfe,n))->dict;
// }

void read_pdf_info(image_dict * idict)
{
    PdfDocument *pdf_doc = NULL;
    ppdoc * pdfe = NULL;
    ppdict *pageDict, *groupDict;
    pprect pagebox;
    ppint rotate = 0;
    int pdf_major_version_found = 1;
    int pdf_minor_version_found = 3;
    double xsize, ysize, xorig, yorig;
    if (img_type(idict) == IMG_TYPE_PDF) {
        pdf_doc = refPdfDocument(img_filepath(idict), FE_FAIL, img_userpassword(idict), img_ownerpassword(idict));
    } else if (img_type(idict) == IMG_TYPE_PDFMEMSTREAM) {
        pdf_doc = findPdfDocument(img_filepath(idict)) ;
        if (pdf_doc == NULL )
           normal_error("pdf inclusion", "memstream not initialized");
        if (pdf_doc->pdfe == NULL)
           normal_error("pdf inclusion", "memstream document is empty");
        pdf_doc->occurences++;
    } else {
        normal_error("pdf inclusion","unknown document");
    }
    pdfe = pdf_doc->pdfe;
    /*
        Check PDF version. This works only for PDF 1.x but since any versions of
        PDF newer than 1.x will not be backwards compatible to PDF 1.x, we will
        then have to changed drastically anyway.
    */
    pdf_major_version_found = ppdoc_version_number(pdfe,&pdf_minor_version_found);
    if ((100 * pdf_major_version_found + pdf_major_version_found) > (100 * img_pdfmajorversion(idict) + img_pdfminorversion(idict))) {
        const char *msg = "PDF inclusion: found PDF version '%d.%d', but at most version '%d.%d' allowed";
        if (img_errorlevel(idict) > 0) {
            formatted_error("pdf inclusion",msg, pdf_major_version_found, pdf_minor_version_found, img_pdfmajorversion(idict), img_pdfminorversion(idict));
        } else {
            formatted_warning("pdf inclusion",msg, pdf_major_version_found, pdf_minor_version_found, img_pdfmajorversion(idict), img_pdfminorversion(idict));
        }
    }
    img_totalpages(idict) = ppdoc_page_count(pdfe);
    if (img_pagename(idict)) {
        /*
            get page by name is obsolete
        */
        normal_error("pdf inclusion","named pages are not supported");
    } else {
        /*
            get page by number
        */
        if (img_pagenum(idict) <= 0
            || img_pagenum(idict) > img_totalpages(idict))
            formatted_error("pdf inclusion","required page '%i' does not exist",(int) img_pagenum(idict));
    }
    /*
        get the required page
    */
    pageDict = get_pdf_page_dict(pdfe,img_pagenum(idict));
    /*
        get the pagebox coordinates (media, crop,...) to use
    */
    get_pagebox(pageDict, img_pagebox(idict), &pagebox);
    if (pagebox.rx > pagebox.lx) {
        xorig = pagebox.lx;
        xsize = pagebox.rx - pagebox.lx;
    } else {
        xorig = pagebox.rx;
        xsize = pagebox.lx - pagebox.rx;
    }
    if (pagebox.ry > pagebox.ly) {
        yorig = pagebox.ly;
        ysize = pagebox.ry - pagebox.ly;
    } else {
        yorig = pagebox.ry;
        ysize = pagebox.ly - pagebox.ry;
    }
    /*
        The following 4 parameters are raw. Do _not_ modify by /Rotate!
    */
    img_xsize(idict) = bp2sp(xsize);
    img_ysize(idict) = bp2sp(ysize);
    img_xorig(idict) = bp2sp(xorig);
    img_yorig(idict) = bp2sp(yorig);
    /*
        Handle /Rotate parameter. Only multiples of 90 deg. are allowed (PDF Ref. v1.3,
        p. 78). We also accept negative angles. Beware: PDF counts clockwise!
    */
    if (ppdict_get_int(pageDict, "Rotate", &rotate)) {
        switch ((((int)rotate % 360) + 360) % 360) {
            case 0:
                img_rotation(idict) = 0;
                break;
            case 90:
                img_rotation(idict) = 3;
                break;
            case 180:
                img_rotation(idict) = 2;
                break;
            case 270:
                img_rotation(idict) = 1;
                break;
            default:
                formatted_warning("pdf inclusion","/Rotate parameter in PDF file not multiple of 90 degrees");
        }
    }
    /*
        currently unused info whether PDF contains a /Group
    */
    groupDict = ppdict_get_dict(pageDict, "Group");
    if (groupDict != NULL) {
         img_set_group(idict);
    }
    /*
        LuaTeX pre 0.85 versions did this:

        if (readtype == IMG_CLOSEINBETWEEN) {
            unrefPdfDocument(img_filepath(idict));
        }

        and also unref'd in the finalizer so we got an extra unrefs when garbage was
        collected. However it is more efficient to keep the file open so we do that
        now. The (slower) alternative is to unref here (which in most cases forcing a
        close of the file) but then we must not call flush_pdf_info.

        A close (unref) can be forced by nilling the dict object at the lua end and
        forcing a collectgarbage("collect") after that.

    */
    if (! img_keepopen(idict)) {
        unrefPdfDocument(img_filepath(idict));
    }
}

void flush_pdf_info(image_dict * idict)
{
    if (img_keepopen(idict)) {
        unrefPdfDocument(img_filepath(idict));
    }
}

/*
    Write the current epf_doc. Here the included PDF is copied, so most errors
    that can happen during PDF inclusion will arise here.
*/

void write_epdf(PDF pdf, image_dict * idict, int suppress_optional_info)
{
    PdfDocument *pdf_doc = NULL;
    ppdoc *pdfe = NULL;
    ppdict *pageDict, *infoDict;
    ppobj *obj, *content, *resources;
    pprect pagebox;
    int i;
    double bbox[4];
    const char *pagedictkeys[] = {
        "Group", "LastModified", "Metadata", "PieceInfo", "SeparationInfo", NULL
    };
    /*
        open PDF file
    */
    if (img_type(idict) == IMG_TYPE_PDF) {
        pdf_doc = refPdfDocument(img_filepath(idict), FE_FAIL, img_userpassword(idict), img_ownerpassword(idict));
    } else if (img_type(idict) == IMG_TYPE_PDFMEMSTREAM) {
        pdf_doc = findPdfDocument(img_filepath(idict)) ;
        pdf_doc->occurences++;
    } else {
        normal_error("pdf inclusion","unknown document");
    }
    pdfe = pdf_doc->pdfe;
    pageDict = get_pdf_page_dict(pdfe,img_pagenum(idict));
    /*
        write the Page header
    */
    pdf_begin_obj(pdf, img_objnum(idict), OBJSTM_NEVER);
    pdf_begin_dict(pdf);
    pdf_dict_add_name(pdf, "Type", "XObject");
    pdf_dict_add_name(pdf, "Subtype", "Form");
    pdf_dict_add_int(pdf, "FormType", 1);
    /*
        write additional information
    */
    pdf_dict_add_img_filename(pdf, idict);
    if ((suppress_optional_info & 4) == 0) {
        pdf_dict_add_int(pdf, "PTEX.PageNumber", (int) img_pagenum(idict));
    }
    if ((suppress_optional_info & 8) == 0) {
        infoDict = ppdoc_info(pdfe);
        if (infoDict != NULL) {
            /* todo : check this
                pdf_dict_add_ref(pdf, "PTEX.InfoDict", addInObj(pdf, pdf_doc, infoDict));
            */
            pdf_add_name(pdf, "PTEX.InfoDict");
            copyDict(pdf, pdf_doc, infoDict);
        }
    }
    if (img_is_bbox(idict)) {
        bbox[0] = sp2bp(img_bbox(idict)[0]);
        bbox[1] = sp2bp(img_bbox(idict)[1]);
        bbox[2] = sp2bp(img_bbox(idict)[2]);
        bbox[3] = sp2bp(img_bbox(idict)[3]);
    } else {
        /*
            get the pagebox coordinates (media, crop,...) to use.
        */
        get_pagebox(pageDict, img_pagebox(idict), &pagebox);
        bbox[0] = pagebox.lx;
        bbox[1] = pagebox.ly;
        bbox[2] = pagebox.rx;
        bbox[3] = pagebox.ry;
    }
    pdf_add_name(pdf, "BBox");
    pdf_begin_array(pdf);
    pdf_add_real(pdf, bbox[0]);
    pdf_add_real(pdf, bbox[1]);
    pdf_add_real(pdf, bbox[2]);
    pdf_add_real(pdf, bbox[3]);
    pdf_end_array(pdf);
    /*
        Now all relevant parts of the Page dictionary are copied. Metadata validity
        check is needed(as a stream it must be indirect).
    */
    obj = ppdict_get_obj(pageDict, "Metadata");
    if (obj != NULL && obj->type != PPREF) {
        formatted_warning("pdf inclusion","/Metadata must be indirect object");
    }
    /*
        copy selected items in Page dictionary
    */
    for (i = 0; pagedictkeys[i] != NULL; i++) {
        obj = ppdict_rget_obj(pageDict, pagedictkeys[i]);
        if (obj != NULL) {
            pdf_add_name(pdf, pagedictkeys[i]);
            /*
                preserves indirection
            */
            copyObject(pdf, pdf_doc, obj);
        }
    }
    resources = ppdict_rget_obj(pageDict, "Resources");
    if (resources == NULL) {
        /*
            If there are no Resources in the Page dict of the embedded page,
            try to inherit the Resources from the Pages tree of the embedded
            PDF file, climbing up the tree until the Resources are found.
            (This fixes a problem with Scribus 1.3.3.14.)
        */
            obj = ppdict_rget_obj(pageDict, "Parent");
            while (obj != NULL && obj->type == PPDICT) {
                resources = ppdict_rget_obj(obj->dict, "Resources");
                if (resources != NULL) {
                    break;
                }
                obj = ppdict_get_obj(obj->dict, "Parent");
            }
    }
    if (resources != NULL) {
        pdf_add_name(pdf, "Resources");
        copyObject(pdf, pdf_doc, resources);
    } else {
        formatted_warning("pdf inclusion","Page /Resources missing");
    }
    /*
        User supplied entries.
    */
    if (img_attr(idict) != NULL && strlen(img_attr(idict)) > 0) {
        pdf_printf(pdf, "\n%s\n", img_attr(idict));
    }
    /*
        Write the Page contents.
    */
    content = ppdict_rget_obj(pageDict, "Contents");
    if (content && content->type == PPSTREAM) {
        if (pdf->compress_level == 0 || pdf->recompress) {
            pdf_dict_add_streaminfo(pdf);
            pdf_end_dict(pdf);
            pdf_begin_stream(pdf);
            copyStreamStream(pdf, content->stream, 1, 1); /* decompress */
        } else {
            /* copies compressed stream */
            ppstream * stream = content->stream;
            ppdict *streamDict = stream->dict; /* */
            obj = ppdict_rget_obj(streamDict, "Length");
            if (obj != NULL) {
                pdf_add_name(pdf, "Length");
                copyObject(pdf, pdf_doc, obj);
                obj = ppdict_rget_obj(streamDict, "Filter");
                if (obj != NULL) {
                    pdf_add_name(pdf, "Filter");
                    copyObject(pdf, pdf_doc, obj);
                    /* the next one is irrelevant, only for inline images: */
                    /*
                    obj = ppdict_rget_obj(streamDict, "DecodeParms");
                    if (obj != NULL) {
                        pdf_add_name(pdf, "DecodeParms");
                        copyObject(pdf, pdf_doc, obj);
                    }
                    */
                }
               pdf_end_dict(pdf);
                pdf_begin_stream(pdf);
                copyStreamStream(pdf, stream, 0, 0);
            } else {
                pdf_dict_add_streaminfo(pdf);
                pdf_end_dict(pdf);
                pdf_begin_stream(pdf);
                copyStreamStream(pdf, stream, 1, 0);
            }
        }
        pdf_end_stream(pdf);
    } else if (content && content->type == PPARRAY) {
        /* listens to compresslevel */
        pdf_dict_add_streaminfo(pdf);
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
        {
            int i;
            int b = 0;
            int n = content->array->size;
            for (i=0; i<n; ++i) {
                ppobj *o = pparray_at(content->array,i);
                while (o != NULL && o->type == PPREF) {
                    o = ppref_obj((ppref *) o->ref);
                }
                if (o != NULL && o->type == PPSTREAM) {
                    if (b) {
                        /*
                            Put a space between streams to be on the safe side (streams
                            should have a trailing space here, but one never knows)
                        */
                        pdf_out(pdf, ' ');
                    } else {
                        b = 1;
                    }
                    copyStreamStream(pdf, (ppstream *) o->stream, 1, 0);
                }
            }
        }
        pdf_end_stream(pdf);
    } else {
        /*
            the contents are optional, but we need to include an empty stream
        */
        pdf_dict_add_streaminfo(pdf);
        pdf_end_dict(pdf);
        pdf_begin_stream(pdf);
        pdf_end_stream(pdf);
    }
    pdf_end_obj(pdf);
    /*
        write out all indirect objects
    */
    writeRefs(pdf, pdf_doc);
    /*
        unrefPdfDocument() must come after freeing whatever is used

    */
    if (! img_keepopen(idict)) {
        unrefPdfDocument(img_filepath(idict));
    }
}

/* a special simple case of inclusion, e.g. an appearance stream */

int write_epdf_object(PDF pdf, image_dict * idict, int n)
{
    int num = 0 ;
    if (img_type(idict) != IMG_TYPE_PDF) {
        normal_error("pdf inclusion","unknown document");
    } else {
        PdfDocument * pdf_doc = refPdfDocument(img_filepath(idict), FE_FAIL, img_userpassword(idict), img_ownerpassword(idict));
        ppdoc * pdfe = pdf_doc->pdfe;
        ppref * ref = ppxref_find(ppdoc_xref(pdfe), (ppuint) n);
        if (ref != NULL) {
            ppobj *obj;
            num = pdf->obj_count++;
            obj = ppref_obj(ref);
            if (obj->type == PPSTREAM) {
                pdf_begin_obj(pdf, num, OBJSTM_NEVER);
            } else {
                pdf_begin_obj(pdf, num, 2);
            }
            copyObject(pdf, pdf_doc, obj);
            pdf_end_obj(pdf);
            writeRefs(pdf, pdf_doc);
        }
        if (! img_keepopen(idict)) {
            unrefPdfDocument(img_filepath(idict));
        }
    }
    return num;
}

/* Deallocate a PdfDocument with all its resources. */

static void deletePdfDocumentPdfDoc(PdfDocument * pdf_doc)
{
    InObj *r, *n;
    /* this may be probably needed for an emergency destroyPdfDocument() */
    for (r = pdf_doc->inObjList; r != NULL; r = n) {
        n = r->next;
        free(r);
    }
    if (pdf_doc->pdfe != NULL) {
        ppdoc_free(pdf_doc->pdfe);
        pdf_doc->pdfe = NULL;
    }
    if (pdf_doc->memstream != NULL) {
     /* pplib does this: free(pdf_doc->memstream); */
        pdf_doc->memstream = NULL;
    }
 /* pdf_doc->pc++; */
    pdf_doc->pc = 0;
}

static void destroyPdfDocument(void *pa, void * p)
{
    PdfDocument *pdf_doc = (PdfDocument *) pa;
    deletePdfDocumentPdfDoc(pdf_doc);
    /* TODO: delete rest of pdf_doc */
}

/*
    Called when an image has been written and its resources in image_tab are
    freed and it's not referenced anymore.
*/

void unrefPdfDocument(char *file_path)
{
    PdfDocument *pdf_doc = findPdfDocument(file_path);
    if (pdf_doc == NULL) {
        /* we're ok */
    } else if (pdf_doc->occurences > 0) {
        pdf_doc->occurences--;
        if (pdf_doc->occurences == 0) {
            deletePdfDocumentPdfDoc(pdf_doc);
        }
    } else {
        /*
            We either have a mismatch in ref and unref or we're somehow out of sync
            which can happen when we mess with the same file in lua and tex.
        */
        formatted_warning("pdf inclusion","there can be a mismatch in opening and closing file '%s'",file_path);
    }
}

/*
    For completeness, but it isn't currently used (unreferencing is done by mean
    of file_path.
*/

void unrefMemStreamPdfDocument(char *file_id)
{
  (void) unrefPdfDocument(file_id);

}

/*
    Called when PDF embedding system is finalized.  We now deallocate all remaining
    PdfDocuments.
*/

void epdf_free(void)
{
    if (PdfDocumentTree != NULL)
        avl_destroy(PdfDocumentTree, destroyPdfDocument);
    PdfDocumentTree = NULL;
}
