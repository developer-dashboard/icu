/*
********************************************************************************
*   Copyright (C) 1997-2004, International Business Machines
*   Corporation and others.  All Rights Reserved.
********************************************************************************
*
* File brkiter.h
*
* Modification History:
*
*   Date        Name        Description
*   02/18/97    aliu        Added typedef for TextCount.  Made DONE const.
*   05/07/97    aliu        Fixed DLL declaration.
*   07/09/97    jfitz       Renamed BreakIterator and interface synced with JDK
*   08/11/98    helena      Sync-up JDK1.2.
*   01/13/2000  helena      Added UErrorCode parameter to createXXXInstance methods.
********************************************************************************
*/

#ifndef BRKITER_H
#define BRKITER_H

#include "unicode/utypes.h"

#if UCONFIG_NO_BREAK_ITERATION

U_NAMESPACE_BEGIN

/*
 * Allow the declaration of APIs with pointers to BreakIterator
 * even when break iteration is removed from the build.
 */
class BreakIterator;

U_NAMESPACE_END

#else

#include "unicode/uobject.h"
#include "unicode/unistr.h"
#include "unicode/chariter.h"
#include "unicode/locid.h"
#include "unicode/ubrk.h"
#include "unicode/strenum.h"

U_NAMESPACE_BEGIN

typedef const void* URegistryKey;

/**
 * The BreakIterator class implements methods for finding the location
 * of boundaries in text. BreakIterator is an abstract base class.
 * Instances of BreakIterator maintain a current position and scan over
 * text returning the index of characters where boundaries occur.
 * <P>
 * Line boundary analysis determines where a text string can be broken
 * when line-wrapping. The mechanism correctly handles punctuation and
 * hyphenated words.
 * <P>
 * Sentence boundary analysis allows selection with correct
 * interpretation of periods within numbers and abbreviations, and
 * trailing punctuation marks such as quotation marks and parentheses.
 * <P>
 * Word boundary analysis is used by search and replace functions, as
 * well as within text editing applications that allow the user to
 * select words with a double click. Word selection provides correct
 * interpretation of punctuation marks within and following
 * words. Characters that are not part of a word, such as symbols or
 * punctuation marks, have word-breaks on both sides.
 * <P>
 * Character boundary analysis allows users to interact with
 * characters as they expect to, for example, when moving the cursor
 * through a text string. Character boundary analysis provides correct
 * navigation of through character strings, regardless of how the
 * character is stored.  For example, an accented character might be
 * stored as a base character and a diacritical mark. What users
 * consider to be a character can differ between languages.
 * <P>
 * This is the interface for all text boundaries.
 * <P>
 * Examples:
 * <P>
 * Helper function to output text
 * <pre>
 * \code
 *    void printTextRange( BreakIterator& iterator, int32_t start, int32_t end )
 *    {
 *        UnicodeString textBuffer, temp;
 *        CharacterIterator *strIter = iterator.createText();
 *        strIter->getText(temp);
 *        cout << " " << start << " " << end << " |"
 *             << temp.extractBetween(start, end, textBuffer)
 *             << "|" << endl;
 *        delete strIter;
 *    }
 * \endcode
 * </pre>
 * Print each element in order:
 * <pre>
 * \code
 *    void printEachForward( BreakIterator& boundary)
 *    {
 *       int32_t start = boundary.first();
 *       for (int32_t end = boundary.next();
 *         end != BreakIterator::DONE;
 *         start = end, end = boundary.next())
 *         {
 *             printTextRange( boundary, start, end );
 *         }
 *    }
 * \code
 * </pre>
 * Print each element in reverse order:
 * <pre>
 * \code
 *    void printEachBackward( BreakIterator& boundary)
 *    {
 *       int32_t end = boundary.last();
 *       for (int32_t start = boundary.previous();
 *         start != BreakIterator::DONE;
 *         end = start, start = boundary.previous())
 *         {
 *             printTextRange( boundary, start, end );
 *         }
 *    }
 * \endcode
 * </pre>
 * Print first element
 * <pre>
 * \code
 *    void printFirst(BreakIterator& boundary)
 *    {
 *        int32_t start = boundary.first();
 *        int32_t end = boundary.next();
 *        printTextRange( boundary, start, end );
 *    }
 * \endcode
 * </pre>
 * Print last element
 * <pre>
 *  \code
 *    void printLast(BreakIterator& boundary)
 *    {
 *        int32_t end = boundary.last();
 *        int32_t start = boundary.previous();
 *        printTextRange( boundary, start, end );
 *    }
 * \endcode
 * </pre>
 * Print the element at a specified position
 * <pre>
 * \code
 *    void printAt(BreakIterator &boundary, int32_t pos )
 *    {
 *        int32_t end = boundary.following(pos);
 *        int32_t start = boundary.previous();
 *        printTextRange( boundary, start, end );
 *    }
 * \endcode
 * </pre>
 * Creating and using text boundaries
 * <pre>
 * \code
 *       void BreakIterator_Example( void )
 *       {
 *           BreakIterator* boundary;
 *           UnicodeString stringToExamine("Aaa bbb ccc. Ddd eee fff.");
 *           cout << "Examining: " << stringToExamine << endl;
 *
 *           //print each sentence in forward and reverse order
 *           boundary = BreakIterator::createSentenceInstance( Locale::US );
 *           boundary->setText(stringToExamine);
 *           cout << "----- forward: -----------" << endl;
 *           printEachForward(*boundary);
 *           cout << "----- backward: ----------" << endl;
 *           printEachBackward(*boundary);
 *           delete boundary;
 *
 *           //print each word in order
 *           boundary = BreakIterator::createWordInstance();
 *           boundary->setText(stringToExamine);
 *           cout << "----- forward: -----------" << endl;
 *           printEachForward(*boundary);
 *           //print first element
 *           cout << "----- first: -------------" << endl;
 *           printFirst(*boundary);
 *           //print last element
 *           cout << "----- last: --------------" << endl;
 *           printLast(*boundary);
 *           //print word at charpos 10
 *           cout << "----- at pos 10: ---------" << endl;
 *           printAt(*boundary, 10 );
 *
 *           delete boundary;
 *       }
 * \endcode
 * </pre>
 */
class U_COMMON_API BreakIterator : public UObject {
public:
    /**
     *  destructor
     *  @stable ICU 2.0
     */
    virtual ~BreakIterator();

    /**
     * Return true if another object is semantically equal to this
     * one. The other object should be an instance of the same subclass of
     * BreakIterator. Objects of different subclasses are considered
     * unequal.
     * <P>
     * Return true if this BreakIterator is at the same position in the
     * same text, and is the same class and type (word, line, etc.) of
     * BreakIterator, as the argument.  Text is considered the same if
     * it contains the same characters, it need not be the same
     * object, and styles are not considered.
     * @stable ICU 2.0
     */
    virtual UBool operator==(const BreakIterator&) const = 0;

    /**
     * Returns the complement of the result of operator==
     * @param rhs The BreakIterator to be compared for inequality
     * @return the complement of the result of operator==
     * @stable ICU 2.0
     */
    UBool operator!=(const BreakIterator& rhs) const { return !operator==(rhs); }

    /**
     * Return a polymorphic copy of this object.  This is an abstract
     * method which subclasses implement.
     * @stable ICU 2.0
     */
    virtual BreakIterator* clone(void) const = 0;

    /**
     * Return a polymorphic class ID for this object. Different subclasses
     * will return distinct unequal values.
     * @stable ICU 2.0
     */
    virtual UClassID getDynamicClassID(void) const = 0;

    /**
     * Return a CharacterIterator over the text being analyzed.
     * Changing the state of the returned iterator can have undefined consequences
     * on the operation of the break iterator.  If you need to change it, clone it first.
     * @stable ICU 2.0
     */
    virtual const CharacterIterator& getText(void) const = 0;

    /**
     * Change the text over which this operates. The text boundary is
     * reset to the start.
     * @param text The UnicodeString used to change the text.
     * @stable ICU 2.0
     */
    virtual void  setText(const UnicodeString &text) = 0;

    /**
     * Change the text over which this operates. The text boundary is
     * reset to the start.
     * @param it The CharacterIterator used to change the text.
     * @stable ICU 2.0
     */
    virtual void  adoptText(CharacterIterator* it) = 0;

    /**
     * DONE is returned by previous() and next() after all valid
     * boundaries have been returned.
     * @stable ICU 2.0
     */
#ifdef U_CYGWIN
    static U_COMMON_API const int32_t DONE;
#else
    static const int32_t DONE;
#endif

    /**
     * Return the index of the first character in the text being scanned.
     * @stable ICU 2.0
     */
    virtual int32_t first(void) = 0;

    /**
     * Return the index immediately BEYOND the last character in the text being scanned.
     * @stable ICU 2.0
     */
    virtual int32_t last(void) = 0;

    /**
     * Return the boundary preceding the current boundary.
     * @return The character index of the previous text boundary or DONE if all
     * boundaries have been returned.
     * @stable ICU 2.0
     */
    virtual int32_t previous(void) = 0;

    /**
     * Return the boundary following the current boundary.
     * @return The character index of the next text boundary or DONE if all
     * boundaries have been returned.
     * @stable ICU 2.0
     */
    virtual int32_t next(void) = 0;

    /**
     * Return character index of the current interator position within the text.
     * @return The boundary most recently returned.
     * @stable ICU 2.0
     */
    virtual int32_t current(void) const = 0;

    /**
     * Return the first boundary following the specified offset.
     * The value returned is always greater than the offset or
     * the value BreakIterator.DONE
     * @param offset the offset to begin scanning.
     * @return The first boundary after the specified offset.
     * @stable ICU 2.0
     */
    virtual int32_t following(int32_t offset) = 0;

    /**
     * Return the first boundary preceding the specified offset.
     * The value returned is always smaller than the offset or
     * the value BreakIterator.DONE
     * @param offset the offset to begin scanning.
     * @return The first boundary before the specified offset.
     * @stable ICU 2.0
     */
    virtual int32_t preceding(int32_t offset) = 0;

    /**
     * Return true if the specfied position is a boundary position.
     * As a side effect, the current position of the iterator is set
     * to the first boundary position at or following the specified offset.
     * @param offset the offset to check.
     * @return True if "offset" is a boundary position.
     * @stable ICU 2.0
     */
    virtual UBool isBoundary(int32_t offset) = 0;

    /**
     * Return the nth boundary from the current boundary
     * @param n which boundary to return.  A value of 0
     * does nothing.  Negative values move to previous boundaries
     * and positive values move to later boundaries.
     * @return The index of the nth boundary from the current position, or
     * DONE if there are fewer than |n| boundaries in the specfied direction.
     * @stable ICU 2.0
     */
    virtual int32_t next(int32_t n) = 0;

    /**
     * Create BreakIterator for word-breaks using the given locale.
     * Returns an instance of a BreakIterator implementing word breaks.
     * WordBreak is useful for word selection (ex. double click)
     * @param where the locale.
     * @param status the error code
     * @return A BreakIterator for word-breaks.  The UErrorCode& status
     * parameter is used to return status information to the user.
     * To check whether the construction succeeded or not, you should check
     * the value of U_SUCCESS(err).  If you wish more detailed information, you
     * can check for informational error results which still indicate success.
     * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
     * example, 'de_CH' was requested, but nothing was found there, so 'de' was
     * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
     * used; neither the requested locale nor any of its fall back locales
     * could be found.
     * The caller owns the returned object and is responsible for deleting it.
     * @stable ICU 2.0
     */
    static BreakIterator* createWordInstance(const Locale& where,
                                                   UErrorCode& status);

    /**
     * Create BreakIterator for line-breaks using specified locale.
     * Returns an instance of a BreakIterator implementing line breaks. Line
     * breaks are logically possible line breaks, actual line breaks are
     * usually determined based on display width.
     * LineBreak is useful for word wrapping text.
     * @param where the locale.
     * @param status The error code.
     * @return A BreakIterator for line-breaks.  The UErrorCode& status
     * parameter is used to return status information to the user.
     * To check whether the construction succeeded or not, you should check
     * the value of U_SUCCESS(err).  If you wish more detailed information, you
     * can check for informational error results which still indicate success.
     * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
     * example, 'de_CH' was requested, but nothing was found there, so 'de' was
     * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
     * used; neither the requested locale nor any of its fall back locales
     * could be found.
     * The caller owns the returned object and is responsible for deleting it.
     * @stable ICU 2.0
     */
    static BreakIterator* createLineInstance(const Locale& where,
                                                   UErrorCode& status);

    /**
     * Create BreakIterator for character-breaks using specified locale
     * Returns an instance of a BreakIterator implementing character breaks.
     * Character breaks are boundaries of combining character sequences.
     * @param where the locale.
     * @param status The error code.
     * @return A BreakIterator for character-breaks.  The UErrorCode& status
     * parameter is used to return status information to the user.
     * To check whether the construction succeeded or not, you should check
     * the value of U_SUCCESS(err).  If you wish more detailed information, you
     * can check for informational error results which still indicate success.
     * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
     * example, 'de_CH' was requested, but nothing was found there, so 'de' was
     * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
     * used; neither the requested locale nor any of its fall back locales
     * could be found.
     * The caller owns the returned object and is responsible for deleting it.
     * @stable ICU 2.0
     */
    static BreakIterator* createCharacterInstance(const Locale& where,
                                                        UErrorCode& status);

    /**
     * Create BreakIterator for sentence-breaks using specified locale
     * Returns an instance of a BreakIterator implementing sentence breaks.
     * @param where the locale.
     * @param status The error code.
     * @return A BreakIterator for sentence-breaks.  The UErrorCode& status
     * parameter is used to return status information to the user.
     * To check whether the construction succeeded or not, you should check
     * the value of U_SUCCESS(err).  If you wish more detailed information, you
     * can check for informational error results which still indicate success.
     * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
     * example, 'de_CH' was requested, but nothing was found there, so 'de' was
     * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
     * used; neither the requested locale nor any of its fall back locales
     * could be found.
     * The caller owns the returned object and is responsible for deleting it.
     * @stable ICU 2.0
     */
    static BreakIterator* createSentenceInstance(const Locale& where,
                                                       UErrorCode& status);

    /**
     * Create BreakIterator for title-casing breaks using the specified locale
     * Returns an instance of a BreakIterator implementing title breaks.
     * The iterator returned locates title boundaries as described for 
     * Unicode 3.2 only. For Unicode 4.0 and above title boundary iteration,
     * please use Word Boundary iterator.{@link #createWordInstance }
     *
     * @param where the locale.
     * @param status The error code.
     * @return A BreakIterator for title-breaks.  The UErrorCode& status
     * parameter is used to return status information to the user.
     * To check whether the construction succeeded or not, you should check
     * the value of U_SUCCESS(err).  If you wish more detailed information, you
     * can check for informational error results which still indicate success.
     * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
     * example, 'de_CH' was requested, but nothing was found there, so 'de' was
     * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
     * used; neither the requested locale nor any of its fall back locales
     * could be found.
     * The caller owns the returned object and is responsible for deleting it.
     * @stable ICU 2.1
     */
    static BreakIterator* createTitleInstance(const Locale& where,
                                                       UErrorCode& status);

    /**
     * Get the set of Locales for which TextBoundaries are installed.
     * <p><b>Note:</b> this will not return locales added through the register
     * call.</p>
     * @param count the output parameter of number of elements in the locale list
     * @return available locales
     * @stable ICU 2.0
     */
    static const Locale* getAvailableLocales(int32_t& count);

    /**
     * Get name of the object for the desired Locale, in the desired langauge.
     * @param objectLocale must be from getAvailableLocales.
     * @param displayLocale specifies the desired locale for output.
     * @param name the fill-in parameter of the return value
     * Uses best match.
     * @return user-displayable name
     * @stable ICU 2.0
     */
    static UnicodeString& getDisplayName(const Locale& objectLocale,
                                         const Locale& displayLocale,
                                         UnicodeString& name);

    /**
     * Get name of the object for the desired Locale, in the langauge of the
     * default locale.
     * @param objectLocale must be from getMatchingLocales
     * @param name the fill-in parameter of the return value
     * @return user-displayable name
     * @stable ICU 2.0
     */
    static UnicodeString& getDisplayName(const Locale& objectLocale,
                                         UnicodeString& name);

    /**
     * Thread safe client-buffer-based cloning operation
     *    Do NOT call delete on a safeclone, since 'new' is not used to create it.
     * @param stackBuffer user allocated space for the new clone. If NULL new memory will be allocated.
     * If buffer is not large enough, new memory will be allocated.
     * @param BufferSize reference to size of allocated space.
     * If BufferSize == 0, a sufficient size for use in cloning will
     * be returned ('pre-flighting')
     * If BufferSize is not enough for a stack-based safe clone,
     * new memory will be allocated.
     * @param status to indicate whether the operation went on smoothly or there were errors
     *  An informational status value, U_SAFECLONE_ALLOCATED_ERROR, is used if any allocations were
     *  necessary.
     * @return pointer to the new clone
     *
     * @stable ICU 2.0
     */
    virtual BreakIterator *  createBufferClone(void *stackBuffer,
                                               int32_t &BufferSize,
                                               UErrorCode &status) = 0;

    /**
     *   Determine whether the BreakIterator was created in user memory by
     *   createBufferClone(), and thus should not be deleted.  Such objects
     *   must be closed by an explicit call to the destructor (not delete).
     *  @stable ICU 2.0
     */
    inline UBool isBufferClone(void);

    /**
     * Register a new break iterator of the indicated kind, to use in the given locale.
     * The break iterator will be adopted.  Clones of the iterator will be returned
     * if a request for a break iterator of the given kind matches or falls back to
     * this locale.
     * @param toAdopt the BreakIterator instance to be adopted
     * @param locale the Locale for which this instance is to be registered
     * @param kind the type of iterator for which this instance is to be registered
     * @param status the in/out status code, no special meanings are assigned
     * @return a registry key that can be used to unregister this instance
     * @stable ICU 2.4
     */
    static URegistryKey registerInstance(BreakIterator* toAdopt, const Locale& locale, UBreakIteratorType kind, UErrorCode& status);

    /**
     * Unregister a previously-registered BreakIterator using the key returned from the
     * register call.  Key becomes invalid after a successful call and should not be used again.
     * The BreakIterator corresponding to the key will be deleted.
     * @param key the registry key returned by a previous call to registerInstance
     * @param status the in/out status code, no special meanings are assigned
     * @return TRUE if the iterator for the key was successfully unregistered
     * @stable ICU 2.4
     */
    static UBool unregister(URegistryKey key, UErrorCode& status);

    /**
     * Return a StringEnumeration over the locales available at the time of the call, 
     * including registered locales.
     * @return a StringEnumeration over the locales available at the time of the call
     * @stable ICU 2.4
     */
    static StringEnumeration* getAvailableLocales(void);

    /**
     * Returns the locale for this break iterator. Two flavors are available: valid and 
     * actual locale. 
     * @draft ICU 2.8 likely to change in ICU 3.0, based on feedback
     */
    Locale getLocale(ULocDataLocaleType type, UErrorCode& status) const;

    /** Get the locale for this break iterator object. You can choose between valid and actual locale.
     *  @param type type of the locale we're looking for (valid or actual) 
     *  @param status error code for the operation
     *  @return the locale
     *  @internal
     */
    const char *getLocaleID(ULocDataLocaleType type, UErrorCode& status) const;

 private:
    static BreakIterator* makeCharacterInstance(const Locale& loc, UErrorCode& status);
    static BreakIterator* makeWordInstance(const Locale& loc, UErrorCode& status);
    static BreakIterator* makeLineInstance(const Locale& loc, UErrorCode& status);
    static BreakIterator* makeSentenceInstance(const Locale& loc, UErrorCode& status);
    static BreakIterator* makeTitleInstance(const Locale& loc, UErrorCode& status);

    static BreakIterator* createInstance(const Locale& loc, UBreakIteratorType kind, UErrorCode& status);
    static BreakIterator* makeInstance(const Locale& loc, int32_t kind, UErrorCode& status);

    friend class ICUBreakIteratorFactory;
    friend class ICUBreakIteratorService;

protected:
    /** @internal */
    BreakIterator();
    /** @internal */
    UBool fBufferClone;
    /** @internal */
    BreakIterator (const BreakIterator &other) : UObject(other), fBufferClone(FALSE) {}

private:

    /** @internal */
    char actualLocale[ULOC_FULLNAME_CAPACITY];
    char validLocale[ULOC_FULLNAME_CAPACITY];

    /**
     * The assignment operator has no real implementation.
     * It's provided to make the compiler happy. Do not call.
     */
    BreakIterator& operator=(const BreakIterator&) { return *this; }
};

inline UBool BreakIterator::isBufferClone()
{
    return fBufferClone;
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */

#endif // _BRKITER
//eof

