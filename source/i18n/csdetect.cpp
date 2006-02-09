/*
 **********************************************************************
 *   Copyright (C) 2005-2006, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 **********************************************************************
 */

#include "unicode/utypes.h"
#include "unicode/ucsdet.h"

#include "csdetect.h"
#include "csmatch.h"
#include "uenumimp.h"

#include "cmemory.h"
#include "cstring.h"
#include "umutex.h"
#include "ucln_in.h"
#include "inputext.h"
#include "csrsbcs.h"
#include "csrmbcs.h"
#include "csrutf8.h"
#include "csrucode.h"
#include "csr2022.h"

#define ARRAY_SIZE(array) (sizeof array / sizeof array[0])

#define NEW_ARRAY(type,count) (type *) uprv_malloc((count) * sizeof(type))
#define DELETE_ARRAY(array) uprv_free((void *) (array))

U_CDECL_BEGIN
static CharsetRecognizer **fCSRecognizers = NULL;

static int32_t fCSRecognizers_size = 0;

static UBool U_CALLCONV csdet_cleanup(void)
{
    if (fCSRecognizers != NULL) {
        for(int32_t r = 0; r < fCSRecognizers_size; r += 1) {
            delete fCSRecognizers[r];
            fCSRecognizers[r] = NULL;
        }

        DELETE_ARRAY(fCSRecognizers);
        fCSRecognizers = NULL;
        fCSRecognizers_size = 0;
    }

    return TRUE;
}
U_CDECL_END

U_NAMESPACE_BEGIN

void CharsetDetector::setRecognizers()
{
    UBool needsInit;
    CharsetRecognizer **recognizers;

    umtx_lock(NULL);
    needsInit = (UBool) (fCSRecognizers == NULL);
    umtx_unlock(NULL);

    if (needsInit) {
        CharsetRecognizer *tempArray[] = {
            new CharsetRecog_UTF8(),

            new CharsetRecog_UTF_16_BE(),
            new CharsetRecog_UTF_16_LE(),
            new CharsetRecog_UTF_32_BE(),
            new CharsetRecog_UTF_32_LE(),

            new CharsetRecog_8859_1_en(),
            new CharsetRecog_8859_1_da(),
            new CharsetRecog_8859_1_de(),
            new CharsetRecog_8859_1_es(),
            new CharsetRecog_8859_1_fr(),
            new CharsetRecog_8859_1_it(),
            new CharsetRecog_8859_1_nl(),
            new CharsetRecog_8859_1_no(),
            new CharsetRecog_8859_1_pt(),
            new CharsetRecog_8859_1_sv(),
            new CharsetRecog_8859_2_cs(),
            new CharsetRecog_8859_2_hu(),
            new CharsetRecog_8859_2_pl(),
            new CharsetRecog_8859_2_ro(),
            new CharsetRecog_8859_5_ru(),
            new CharsetRecog_8859_6_ar(),
            new CharsetRecog_8859_7_el(),
            new CharsetRecog_8859_8_I_he(),
            new CharsetRecog_8859_8_he(),
            new CharsetRecog_windows_1251(),
            new CharsetRecog_windows_1256(),
            new CharsetRecog_KOI8_R(),
            new CharsetRecog_8859_9_tr(),
            new CharsetRecog_sjis(),
            new CharsetRecog_gb_18030(),
            new CharsetRecog_euc_jp(),
            new CharsetRecog_euc_kr(),

            new CharsetRecog_2022JP(),
            new CharsetRecog_2022KR(),
            new CharsetRecog_2022CN()
        };
        int32_t rCount = ARRAY_SIZE(tempArray);
        int32_t r;

        recognizers = NEW_ARRAY(CharsetRecognizer *, rCount);
        for (r = 0; r < rCount; r += 1) {
           recognizers[r] = tempArray[r];
        }

        umtx_lock(NULL);
        if (fCSRecognizers == NULL) {
            fCSRecognizers = recognizers;
            fCSRecognizers_size = rCount;
        }
        umtx_unlock(NULL);

        if (fCSRecognizers != recognizers) {
            for (r = 0; r < rCount; r += 1) {
                delete recognizers[r];
                recognizers[r] = NULL;
            }

            DELETE_ARRAY(recognizers);
        }

        recognizers = NULL;
        ucln_i18n_registerCleanup(UCLN_I18N_CSDET, csdet_cleanup);
    }
}

CharsetDetector::CharsetDetector()
  : textIn(new InputText()), resultCount(0), fStripTags(FALSE), fFreshTextSet(FALSE)
{
    setRecognizers();

    resultArray = new CharsetMatch *[fCSRecognizers_size];

    for(int32_t i = 0; i < fCSRecognizers_size; i += 1) {
        resultArray[i] = new CharsetMatch();
    }
}

CharsetDetector::~CharsetDetector()
{
    delete textIn;

    for(int32_t i = 0; i < fCSRecognizers_size; i += 1) {
        delete resultArray[i];
    }

    delete [] resultArray;
}

void CharsetDetector::setText(const char *in, int32_t len)
{
    textIn->setText(in, len);
    fFreshTextSet = TRUE;
}

UBool CharsetDetector::setStripTagsFlag(UBool flag)
{
    UBool temp = fStripTags;
    fStripTags = flag;
    fFreshTextSet = TRUE;
    return temp;
}

UBool CharsetDetector::getStripTagsFlag() const
{
    return fStripTags;
}

void CharsetDetector::setDeclaredEncoding(const char *encoding, int32_t len) const
{
    textIn->setDeclaredEncoding(encoding,len);
}

int32_t CharsetDetector::getDetectableCount()
{
    setRecognizers();

    return fCSRecognizers_size; 
}

const CharsetMatch *CharsetDetector::detect(UErrorCode &status)
{
    int32_t maxMatchesFound = 0;

    detectAll(maxMatchesFound, status);

    if(maxMatchesFound > 0) {
        return resultArray[0];
    } else {
        return NULL;
    }
}

// this sort of conversion is explicitly mentioned in the C++ standard
// in section 4.4:

// [Note: if a program could assign a pointer of type T** to a pointer of type
//  const T** (that is, if line //1 below was allowed), a program could
// 	    inadvertently modify a const object (as it is done on line //2).  For
// 						 example,

// 	 int32_t main() {
// 		const char c = 'c';
// 		char* pc;
// 		const char** pcc = &pc;//1: not allowed
// 		*pcc = &c;
// 		*pc = 'C';//2: modifies a const object
// 	    }
const CharsetMatch * const *CharsetDetector::detectAll(int32_t &maxMatchesFound, UErrorCode &status)
{
    if(!textIn->isSet()) {
        status = U_MISSING_RESOURCE_ERROR;// TODO:  Need to set proper status code for input text not set

        return NULL;
    } else if(fFreshTextSet) {
        CharsetRecognizer *csr;
        int32_t            detectResults;
        int32_t            confidence;

        textIn->MungeInput(fStripTags);

        // Iterate over all possible charsets, remember all that
        // give a match quality > 0.
        resultCount = 0;
        for (int32_t i = 0; i < fCSRecognizers_size; i += 1) {
            csr = fCSRecognizers[i];
            detectResults = csr->match(textIn);
            confidence = detectResults;

            if (confidence > 0)  {
                resultArray[resultCount++]->set(textIn, csr, confidence);
            }
        }

        for(int32_t i = resultCount; i < fCSRecognizers_size; i += 1) {
            resultArray[i]->set(textIn, 0, 0);
        }

        //Bubble sort
        for(int32_t i = resultCount; i > 1; i -= 1) {
            for(int32_t j = 0; j < i-1; j += 1) {
                if(resultArray[j]->getConfidence() < resultArray[j+1]->getConfidence()) {
                    CharsetMatch *temp = resultArray[j];
                    resultArray[j] = resultArray[j+1];
                    resultArray[j+1] = temp;
                }
            }
        }

        fFreshTextSet = FALSE;
    }

    maxMatchesFound = resultCount;

    return resultArray;
}

const char *CharsetDetector::getCharsetName(int32_t index, UErrorCode &status) const
{
    if( index > fCSRecognizers_size-1 || index < 0) {
        status = U_INDEX_OUTOFBOUNDS_ERROR;

        return 0;
    } else {
        return fCSRecognizers[index]->getName();
    }
}

U_NAMESPACE_END

U_CDECL_BEGIN
typedef struct {
    int32_t currIndex;
} Context;



static void U_CALLCONV
enumClose(UEnumeration *en) {
    if(en->context != NULL) {
        DELETE_ARRAY(en->context);
    }

    DELETE_ARRAY(en);
}

static int32_t U_CALLCONV
enumCount(UEnumeration *en, UErrorCode *status) {
    return fCSRecognizers_size;
}

static const char* U_CALLCONV
enumNext(UEnumeration *en, int32_t *resultLength, UErrorCode *status) {
    if(((Context *)en->context)->currIndex >= fCSRecognizers_size) {
        return NULL;
    }
    const char *currName = fCSRecognizers[((Context *)en->context)->currIndex]->getName();
    *resultLength = (int32_t)uprv_strlen(currName);
    ((Context *)en->context)->currIndex++;

    return currName;
}

static void U_CALLCONV
enumReset(UEnumeration *en, UErrorCode *status) {
    ((Context *)en->context)->currIndex = 0;
}

UEnumeration enumeration = {
    NULL,
    NULL,
    enumClose,
    enumCount,
    uenum_unextDefault,
    enumNext,
    enumReset
};

U_DRAFT  UEnumeration * U_EXPORT2
ucsdet_getAllDetectableCharsets(const UCharsetDetector *ucsd,  UErrorCode *status)
{
    if(U_FAILURE(*status)) {
        return 0;
    }

    /* Initialize recognized charsets. */
    CharsetDetector::getDetectableCount();

    UEnumeration *en = NEW_ARRAY(UEnumeration, 1);
    memcpy(en, &enumeration, sizeof(UEnumeration));
    en->context = (void*)NEW_ARRAY(Context, 1);
    uprv_memset(en->context, 0, sizeof(Context));
    return en;
}
U_CDECL_END
