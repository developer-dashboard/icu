/*
******************************************************************************
*
*   Copyright (C) 1996-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*
* File resbund.h
*
*   CREATED BY
*       Richard Gillam
*
* Modification History:
*
*   Date        Name        Description
*   2/5/97      aliu        Added scanForLocaleInFile.  Added
*                           constructor which attempts to read resource bundle
*                           from a specific file, without searching other files.
*   2/11/97     aliu        Added UErrorCode return values to constructors.  Fixed
*                           infinite loops in scanForFile and scanForLocale.
*                           Modified getRawResourceData to not delete storage
*                           in localeData and resourceData which it doesn't own.
*                           Added Mac compatibility #ifdefs for tellp() and
*                           ios::nocreate.
*   2/18/97     helena      Updated with 100% documentation coverage.
*   3/13/97     aliu        Rewrote to load in entire resource bundle and store
*                           it as a Hashtable of ResourceBundleData objects.
*                           Added state table to govern parsing of files.
*                           Modified to load locale index out of new file
*                           distinct from default.txt.
*   3/25/97     aliu        Modified to support 2-d arrays, needed for timezone
*                           data. Added support for custom file suffixes.  Again,
*                           needed to support timezone data.
*   4/7/97      aliu        Cleaned up.
* 03/02/99      stephen     Removed dependency on FILE*.
* 03/29/99      helena      Merged Bertrand and Stephen's changes.
* 06/11/99      stephen     Removed parsing of .txt files.
*                           Reworked to use new binary format.
*                           Cleaned up.
* 06/14/99      stephen     Removed methods taking a filename suffix.
* 11/09/99      weiv        Added getLocale(), fRealLocale, removed fRealLocaleID
******************************************************************************
*/

#ifndef RESBUND_H
#define RESBUND_H
  
#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/ures.h"
#include "unicode/unistr.h"
#include "unicode/locid.h"

U_NAMESPACE_BEGIN

/**
 * A class representing a collection of resource information pertaining to a given
 * locale. A resource bundle provides a way of accessing locale- specfic information in
 * a data file. You create a resource bundle that manages the resources for a given
 * locale and then ask it for individual resources.
 * <P>
 * Resource bundles in ICU4C are currently defined using text files which conform to the following
 * <a href="http://oss.software.ibm.com/cvs/icu/~checkout~/icuhtml/design/bnf_rb.txt">BNF definition</a>.
 * More on resource bundle concepts and syntax can be found in the 
 * <a href="http://oss.software.ibm.com/icu/userguide/Fallbackmechanism.html">users guide</a>.
 * <P>
 *
 * The ResourceBundle class is not suitable for subclassing.
 *
 * @stable ICU 2.0
 */
class U_COMMON_API ResourceBundle : public UObject {
public:
    /**
     * Constructor
     *
     * @param path    This is a full pathname in the platform-specific format for the
     *                directory containing the resource data files we want to load
     *                resources from. We use locale IDs to generate filenames, and the
     *                filenames have this string prepended to them before being passed
     *                to the C++ I/O functions. Therefore, this string must always end
     *                with a directory delimiter (whatever that is for the target OS)
     *                for this class to work correctly.
     * @param locale  This is the locale this resource bundle is for. To get resources
     *                for the French locale, for example, you would create a
     *                ResourceBundle passing Locale::FRENCH for the "locale" parameter,
     *                and all subsequent calls to that resource bundle will return
     *                resources that pertain to the French locale. If the caller doesn't
     *                pass a locale parameter, the default locale for the system (as
     *                returned by Locale::getDefault()) will be used.
     * @param err     The Error Code.
     * The UErrorCode& err parameter is used to return status information to the user. To
     * check whether the construction succeeded or not, you should check the value of
     * U_SUCCESS(err). If you wish more detailed information, you can check for
     * informational error results which still indicate success. U_USING_FALLBACK_ERROR
     * indicates that a fall back locale was used. For example, 'de_CH' was requested,
     * but nothing was found there, so 'de' was used. U_USING_DEFAULT_ERROR indicates that
     * the default locale data was used; neither the requested locale nor any of its
     * fall back locales could be found.
     * @stable ICU 2.0
     */
    ResourceBundle(const UnicodeString&    path,
                   const Locale&           locale,
                   UErrorCode&              err);

    /**
     * Construct a resource bundle for the root bundle in the specified path.
     *
     * @param path A path/basename for the data file(s) containing the bundle.
     * @param err A UErrorCode value
     * @stable ICU 2.0
     */
    ResourceBundle(const UnicodeString&    path,
                   UErrorCode&              err);

    /**
     * Construct a resource bundle for the ICU root bundle.
     *
     * @param err A UErrorCode value
     * @stable ICU 2.0
     */
    ResourceBundle(UErrorCode &err);

#ifdef ICU_RESOURCEBUNDLE_USE_DEPRECATES
    /**
     * Constructs a ResourceBundle
     *
     * @obsolete ICU 2.4. Use const char * pathnames instead since this API will be removed in that release.
     */
    ResourceBundle(const wchar_t* path,
                   const Locale& locale,
                   UErrorCode& err);
#endif /* ICU_RESOURCEBUNDLE_USE_DEPRECATES */

    /**
     * Standard constructor, onstructs a resource bundle for the locale-specific
     * bundle in the specified path.
     *
     * @param path A path/basename for the data file(s) containing the bundle.
     *             NULL is used for ICU data.
     * @param locale The locale for which to open a resource bundle.
     * @param err A UErrorCode value
     * @stable ICU 2.0
     */
    ResourceBundle(const char* path,
                   const Locale& locale,
                   UErrorCode& err);

    /**
     * Copy constructor.
     *
     * @param original The resource bundle to copy.
     * @stable ICU 2.0
     */
    ResourceBundle(const ResourceBundle &original);

    /**
     * Constructor from a C UResourceBundle.
     *
     * @param res A pointer to the C resource bundle.
     * @param status A UErrorCode value.
     * @stable ICU 2.0
     */
    ResourceBundle(UResourceBundle *res, 
                   UErrorCode &status);

    /**
     * Assignment operator.
     *
     * @param other The resource bundle to copy.
     * @stable ICU 2.0
     */
    ResourceBundle& 
      operator=(const ResourceBundle& other);

    /** Destructor. 
     * @stable ICU 2.0
     */
    ~ResourceBundle();

    /**
     * Returns the size of a resource. Size for scalar types is always 1, and for vector/table types is
     * the number of child resources.
     *
     * @return number of resources in a given resource.
     * @stable ICU 2.0
     */
    int32_t 
      getSize(void) const;

    /**
     * returns a string from a string resource type
     *
     * @param status: fills in the outgoing error code
     *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
     *                could be a warning 
     *                e.g.: <TT>U_USING_FALLBACK_WARNING</TT>,<TT>U_USING_DEFAULT_WARNING </TT>
     * @return a pointer to a zero-terminated UChar array which lives in a memory mapped/DLL file.
     * @stable ICU 2.0
     */
    UnicodeString 
      getString(UErrorCode& status) const;

    /**
     * returns a binary data from a resource. Can be used at most primitive resource types (binaries,
     * strings, ints)
     *
     * @param resourceBundle: a string resource
     * @param len:    fills in the length of resulting byte chunk
     * @param status: fills in the outgoing error code
     *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
     *                could be a warning 
     *                e.g.: <TT>U_USING_FALLBACK_WARNING</TT>,<TT>U_USING_DEFAULT_WARNING </TT>
     * @return a pointer to a chunk of unsigned bytes which live in a memory mapped/DLL file.
     * @stable ICU 2.0
     */
    const uint8_t*
      getBinary(int32_t& len, UErrorCode& status) const;


    /**
     * returns an integer vector from a resource. 
     *
     * @param resourceBundle: a string resource
     * @param len:    fills in the length of resulting integer vector
     * @param status: fills in the outgoing error code
     *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
     *                could be a warning 
     *                e.g.: <TT>U_USING_FALLBACK_WARNING</TT>,<TT>U_USING_DEFAULT_WARNING </TT>
     * @return a pointer to a vector of integers that lives in a memory mapped/DLL file.
     * @stable ICU 2.0
     */
    const int32_t*
      getIntVector(int32_t& len, UErrorCode& status) const;

    /**
     * returns an unsigned integer from a resource. 
     * This integer is originally 28 bits.
     *
     * @param status: fills in the outgoing error code
     *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
     *                could be a warning 
     *                e.g.: <TT>U_USING_FALLBACK_WARNING</TT>,<TT>U_USING_DEFAULT_WARNING </TT>
     * @return an unsigned integer value
     * @stable ICU 2.0
     */
    uint32_t 
      getUInt(UErrorCode& status) const;

    /**
     * returns a signed integer from a resource. 
     * This integer is originally 28 bit and the sign gets propagated.
     *
     * @param status: fills in the outgoing error code
     *                could be <TT>U_MISSING_RESOURCE_ERROR</T> if the key is not found
     *                could be a warning 
     *                e.g.: <TT>U_USING_FALLBACK_WARNING</TT>,<TT>U_USING_DEFAULT_WARNING </TT>
     * @return a signed integer value
     * @stable ICU 2.0
     */
    int32_t 
      getInt(UErrorCode& status) const;

    /**
     * Checks whether the resource has another element to iterate over.
     *
     * @return TRUE if there are more elements, FALSE if there is no more elements
     * @stable ICU 2.0
     */
    UBool 
      hasNext(void) const;

    /**
     * Resets the internal context of a resource so that iteration starts from the first element.
     *
     * @stable ICU 2.0
     */
    void 
      resetIterator(void);

    /**
     * Returns the key associated with this resource. Not all the resources have a key - only 
     * those that are members of a table.
     *
     * @return a key associated to this resource, or NULL if it doesn't have a key
     * @stable ICU 2.0
     */
    const char*
      getKey(void);

    /**
     * Gets the locale ID of the resource bundle as a string.
     * Same as getLocale().getName() .
     *
     * @return the locale ID of the resource bundle as a string
     * @stable ICU 2.0
     */
    const char*
      getName(void);


    /**
     * Returns the type of a resource. Available types are defined in enum UResType
     *
     * @return type of the given resource.
     * @stable ICU 2.0
     */
    UResType 
      getType(void);

    /**
     * Returns the next resource in a given resource or NULL if there are no more resources 
     *
     * @param status            fills in the outgoing error code
     * @return                  ResourceBundle object.
     * @stable ICU 2.0
     */
    ResourceBundle 
      getNext(UErrorCode& status);

    /**
     * Returns the next string in a resource or NULL if there are no more resources 
     * to iterate over. 
     *
     * @param status            fills in the outgoing error code
     * @return an UnicodeString object.
     * @stable ICU 2.0
     */
    UnicodeString 
      getNextString(UErrorCode& status);

    /**
     * Returns the next string in a resource or NULL if there are no more resources 
     * to iterate over. 
     *
     * @param key               fill in for key associated with this string
     * @param status            fills in the outgoing error code
     * @return an UnicodeString object.
     * @stable ICU 2.0
     */
    UnicodeString 
      getNextString(const char ** key, 
                    UErrorCode& status);

    /**
     * Returns the resource in a resource at the specified index. 
     *
     * @param index             an index to the wanted resource.
     * @param status            fills in the outgoing error code
     * @return                  ResourceBundle object. If there is an error, resource is invalid.
     * @stable ICU 2.0
     */
    ResourceBundle 
      get(int32_t index, 
          UErrorCode& status) const;

    /**
     * Returns the string in a given resource at the specified index.
     *
     * @param index             an index to the wanted string.
     * @param status            fills in the outgoing error code
     * @return                  an UnicodeString object. If there is an error, string is bogus
     * @stable ICU 2.0
     */
    UnicodeString 
      getStringEx(int32_t index, 
                  UErrorCode& status) const;

    /**
     * Returns a resource in a resource that has a given key. This procedure works only with table
     * resources. 
     *
     * @param key               a key associated with the wanted resource
     * @param status            fills in the outgoing error code.
     * @return                  ResourceBundle object. If there is an error, resource is invalid.
     * @stable ICU 2.0
     */
    ResourceBundle 
      get(const char* key, 
          UErrorCode& status) const;

    /**
     * Returns a string in a resource that has a given key. This procedure works only with table
     * resources. 
     *
     * @param key               a key associated with the wanted string
     * @param status            fills in the outgoing error code
     * @return                  an UnicodeString object. If there is an error, string is bogus
     * @stable ICU 2.0
     */
    UnicodeString 
      getStringEx(const char* key, 
                  UErrorCode& status) const;
    
    /**
     * Return the version number associated with this ResourceBundle as a string. Please
     * use getVersion, as this method is going to be deprecated.
     *
     * @return  A version number string as specified in the resource bundle or its parent.
     *          The caller does not own this string.
     * @see getVersion
     * @stable ICU 2.0
     */
    const char*   
      getVersionNumber(void) const;

    /**
     * Return the version number associated with this ResourceBundle as a UVersionInfo array.
     *
     * @param versionInfo A UVersionInfo array that is filled with the version number
     *                    as specified in the resource bundle or its parent.
     * @stable ICU 2.0
     */
    void 
      getVersion(UVersionInfo versionInfo) const;

    /**
     * Return the Locale associated with this ResourceBundle. 
     *
     * @return a Locale object
     * @stable ICU 2.0
     */
    const Locale&
      getLocale(void) const;

    /**
     * ICU "poor man's RTTI", returns a UClassID for the actual class.
     *
     * @draft ICU 2.2
     */
    virtual inline UClassID 
      getDynamicClassID() const 
    { return getStaticClassID(); }

    /**
     * ICU "poor man's RTTI", returns a UClassID for this class.
     *
     * @draft ICU 2.2
     */
    static inline UClassID 
      getStaticClassID() 
    { return (UClassID)&fgClassID; }

private:
    UResourceBundle *resource;
    void constructForLocale(const UnicodeString& path, const Locale& locale, UErrorCode& error);
#ifdef ICU_RESOURCEBUNDLE_USE_DEPRECATES
    /**
     * @obsolete ICU 2.4. Use const char * pathnames instead since this API will be removed in that release.
     */
    void constructForLocale(const wchar_t* path, const Locale& locale, UErrorCode& error);
#endif /* ICU_RESOURCEBUNDLE_USE_DEPRECATES */
    Locale *locName;

    /**
     * The address of this static class variable serves as this class's ID
     * for ICU "poor man's RTTI".
     */
    static const char fgClassID;
};

U_NAMESPACE_END
#endif
