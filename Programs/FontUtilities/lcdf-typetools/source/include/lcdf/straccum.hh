// -*- related-file-name: "../../liblcdf/straccum.cc" -*-
#ifndef LCDF_STRACCUM_HH
#define LCDF_STRACCUM_HH
#include <string.h>
#include <assert.h>
#include <lcdf/string.hh>
#if HAVE_PERMSTRING
# include <lcdf/permstr.hh>
#endif
#if __GNUC__ > 4
# define LCDF_SNPRINTF_ATTR __attribute__((__format__(__printf__, 3, 4)))
#else
# define LCDF_SNPRINTF_ATTR /* nothing */
#endif
template<typename T> class Vector;

/** @file <lcdf/straccum.hh>
    @brief Click's StringAccum class, used to construct Strings efficiently from pieces.
*/

class StringAccum { public:

    typedef const char* const_iterator;
    typedef char* iterator;

    typedef int (StringAccum::*unspecified_bool_type)() const;

    inline StringAccum();
    explicit inline StringAccum(int capacity);
    explicit inline StringAccum(const char *cstr);
    inline StringAccum(const char* s, int len);
    inline StringAccum(const String& str);
    inline StringAccum(const StringAccum& x);
    inline ~StringAccum();

    inline const char* data() const;
    inline char* data();
    inline const unsigned char* udata() const;
    inline unsigned char* udata();
    inline int length() const;
    inline int capacity() const;

    inline bool empty() const;
    inline bool operator!() const;
    inline bool out_of_memory() const;

    inline const_iterator begin() const;
    inline iterator begin();
    inline const_iterator end() const;
    inline iterator end();


    /** @brief Return true iff the StringAccum contains characters.
     *
     * Returns false for empty and out-of-memory StringAccums. */
    inline operator unspecified_bool_type() const {
        return _len != 0 ? &StringAccum::capacity : 0;
    }


    /** @brief Null-terminate this StringAccum and return its data.
     *
     * Note that the null character does not contribute to the StringAccum's
     * length(), and later append() and similar operations can overwrite it.
     * If appending the null character fails, the StringAccum becomes
     * out-of-memory and the returned value is a null string. */
    const char *c_str();


    inline char operator[](int i) const;
    inline char& operator[](int i);
    inline char front() const;
    inline char& front();
    inline char back() const;
    inline char& back();

    inline void clear();

    inline char* reserve(int n);
    inline void set_length(int len);

    /** @brief Set the StringAccum's length to @a len.
     * @pre @a len >= 0
     * @return 0 on success, -ENOMEM on failure */
    int resize(int len);

    inline void adjust_length(int delta);
    inline char* extend(int nadjust, int nreserve = 0);

    inline void pop_back(int n = 1);


    inline void append(char c);
    inline void append(unsigned char c);

    /** @brief Append @a len copies of character @a c to the StringAccum. */
    void append_fill(int c, int len);

    inline void append_utf8(unsigned ch);

    /** @brief Append the null-terminated C string @a s to this StringAccum.
     * @param s data to append */
    void append(const char* s);
    inline void append(const char* s, int len);
    inline void append(const unsigned char* s, int len);
    inline void append(const char* begin, const char* end);
    inline void append(const unsigned char* begin, const unsigned char* end);

    // word joining
    void append_break_lines(const String &text, int linelen, const String &leftmargin = String());

    /** @brief Append result of snprintf() to this StringAccum.
     * @param n maximum number of characters to print
     * @param format format argument to snprintf()
     *
     * The terminating null character is not appended to the string.
     *
     * @note The safe vsnprintf() variant is called if it exists.  It does in
     * the Linux kernel, and on modern Unix variants.  However, if it does not
     * exist on your machine, then this function is actually unsafe, and you
     * should make sure that the printf() invocation represented by your
     * arguments will never write more than @a n characters, not including the
     * terminating null. */
    StringAccum &snprintf(int n, const char *format, ...) LCDF_SNPRINTF_ATTR;


    /** @brief Return a String object with this StringAccum's contents.
     *
     * This operation donates the StringAccum's memory to the returned String.
     * After a call to take_string(), the StringAccum object becomes empty,
     * and any future append() operations may cause memory allocations.  If
     * the StringAccum is out-of-memory, the returned String is also
     * out-of-memory, but the StringAccum's out-of-memory state is reset. */
    String take_string();


    inline StringAccum& operator=(const StringAccum& x);

    /** @brief Swap this StringAccum's contents with @a x. */
    void swap(StringAccum &x);

    // see also operator<< declarations below

  private:

    enum {
        MEMO_SPACE = String::MEMO_SPACE
    };

    unsigned char *_s;
    int _len;
    int _cap;

    char *grow(int);
    void assign_out_of_memory();

    char *hard_extend(int nadjust, int nreserve);
    void hard_append(const char *s, int len);
    void append_utf8_hard(unsigned ch);

    friend StringAccum &operator<<(StringAccum &sa, const char *s);
    friend StringAccum &operator<<(StringAccum &sa, const String &str);
#if HAVE_PERMSTRING
    friend StringAccum &operator<<(StringAccum &sa, PermString s);
#endif

};

inline StringAccum &operator<<(StringAccum &, char);
inline StringAccum &operator<<(StringAccum &, unsigned char);
inline StringAccum &operator<<(StringAccum &, const char *);
inline StringAccum &operator<<(StringAccum &, const String &);
inline StringAccum &operator<<(StringAccum &, const StringAccum &);
#ifdef HAVE_PERMSTRING
inline StringAccum &operator<<(StringAccum &, PermString);
#endif

inline StringAccum &operator<<(StringAccum &, bool);
inline StringAccum &operator<<(StringAccum &, short);
inline StringAccum &operator<<(StringAccum &, unsigned short);
inline StringAccum &operator<<(StringAccum &, int);
inline StringAccum &operator<<(StringAccum &, unsigned);
StringAccum &operator<<(StringAccum &, long);
StringAccum &operator<<(StringAccum &, unsigned long);
StringAccum &operator<<(StringAccum &, double);


/** @brief Construct an empty StringAccum (with length 0). */
inline StringAccum::StringAccum()
    : _s(0), _len(0), _cap(0) {
}

/** @brief Construct a StringAccum with room for at least @a capacity
 * characters.
 * @param capacity initial capacity
 *
 * If @a capacity <= 0, the StringAccum is created empty.  If @a capacity
 * is too large (so that @a capacity bytes of memory can't be allocated),
 * the StringAccum falls back to a smaller capacity (possibly zero). */
inline StringAccum::StringAccum(int capacity)
    : _len(0) {
    assert(capacity >= 0);
    if (capacity
        && (_s = new unsigned char[capacity + MEMO_SPACE])) {
        _s += MEMO_SPACE;
        _cap = capacity;
    } else {
        _s = 0;
        _cap = 0;
    }
}

/** @brief Construct a StringAccum containing the characters in @a s. */
inline StringAccum::StringAccum(const char *cstr)
    : _s(0), _len(0), _cap(0) {
    append(cstr);
}

/** @brief Construct a StringAccum containing the characters in @a s. */
inline StringAccum::StringAccum(const char *s, int len)
    : _s(0), _len(0), _cap(0) {
    append(s, len);
}

/** @brief Construct a StringAccum containing the characters in @a str. */
inline StringAccum::StringAccum(const String &str)
    : _s(0), _len(0), _cap(0) {
    append(str.begin(), str.end());
}

/** @brief Construct a StringAccum containing a copy of @a x. */
inline StringAccum::StringAccum(const StringAccum &x)
    : _s(0), _len(0), _cap(0) {
    append(x.data(), x.length());
}

/** @brief Destroy a StringAccum, freeing its memory. */
inline StringAccum::~StringAccum() {
    if (_cap > 0)
        delete[] (_s - MEMO_SPACE);
}

/** @brief Return the contents of the StringAccum.
 * @return The StringAccum's contents.
 *
 * The return value is null if the StringAccum is empty or out-of-memory.
 * The returned data() value points to writable memory (unless the
 * StringAccum itself is const). */
inline const char* StringAccum::data() const {
    return reinterpret_cast<const char *>(_s);
}

/** @overload */
inline char* StringAccum::data() {
    return reinterpret_cast<char *>(_s);
}

inline const unsigned char* StringAccum::udata() const {
    return _s;
}

inline unsigned char* StringAccum::udata() {
    return _s;
}

/** @brief Return true iff the StringAccum is empty or out-of-memory. */
inline bool StringAccum::empty() const {
    return _len == 0;
}

/** @brief Return the length of the StringAccum. */
inline int StringAccum::length() const {
    return _len;
}

/** @brief Return the StringAccum's current capacity.
 *
 * The capacity is the maximum length the StringAccum can hold without
 * incurring a memory allocation.  Returns -1 for out-of-memory
 * StringAccums. */
inline int StringAccum::capacity() const {
    return _cap;
}

/** @brief Return an iterator for the first character in the StringAccum.
 *
 * StringAccum iterators are simply pointers into string data, so they are
 * quite efficient.  @sa StringAccum::data */
inline StringAccum::const_iterator StringAccum::begin() const {
    return reinterpret_cast<char*>(_s);
}

/** @overload */
inline StringAccum::iterator StringAccum::begin() {
    return reinterpret_cast<char*>(_s);
}

/** @brief Return an iterator for the end of the StringAccum.
 *
 * The return value points one character beyond the last character in the
 * StringAccum. */
inline StringAccum::const_iterator StringAccum::end() const {
    return reinterpret_cast<char*>(_s + _len);
}

/** @overload */
inline StringAccum::iterator StringAccum::end() {
    return reinterpret_cast<char*>(_s + _len);
}

/** @brief Return true iff the StringAccum does not contain characters.
 *
 * Returns true for empty and out-of-memory StringAccums. */
inline bool StringAccum::operator!() const {
    return _len == 0;
}

/** @brief Return true iff the StringAccum is out-of-memory. */
inline bool StringAccum::out_of_memory() const {
    return _cap < 0;
}

/** @brief Return the <a>i</a>th character in the string.
 * @param i character index
 * @pre 0 <= @a i < length() */
inline char StringAccum::operator[](int i) const {
    assert((unsigned) i < (unsigned) _len);
    return static_cast<char>(_s[i]);
}

/** @brief Return a reference to the <a>i</a>th character in the string.
 * @param i character index
 * @pre 0 <= @a i < length() */
inline char& StringAccum::operator[](int i) {
    assert((unsigned) i < (unsigned) _len);
    return reinterpret_cast<char &>(_s[i]);
}

/** @brief Return the first character in the string.
 * @pre length() > 0 */
inline char StringAccum::front() const {
    assert(_len > 0);
    return static_cast<char>(_s[0]);
}

/** @brief Return a reference to the first character in the string.
 * @pre length() > 0 */
inline char& StringAccum::front() {
    assert(_len > 0);
    return reinterpret_cast<char &>(_s[0]);
}

/** @brief Return the last character in the string.
 * @pre length() > 0 */
inline char StringAccum::back() const {
    assert(_len > 0);
    return static_cast<char>(_s[_len - 1]);
}

/** @brief Return a reference to the last character in the string.
 * @pre length() > 0 */
inline char& StringAccum::back() {
    assert(_len > 0);
    return reinterpret_cast<char &>(_s[_len - 1]);
}

/** @brief Clear the StringAccum's comments.
 *
 * All characters in the StringAccum are erased.  Also resets the
 * StringAccum's out-of-memory status. */
inline void StringAccum::clear() {
    if (_cap < 0) {
        _cap = 0;
        _s = 0;
    }
    _len = 0;
}

/** @brief Reserve space for at least @a n characters.
 * @return a pointer to at least @a n characters, or null if allocation
 * fails
 * @pre @a n >= 0
 *
 * reserve() does not change the string's length(), only its capacity().
 * In a frequent usage pattern, code calls reserve(), passing an upper
 * bound on the characters that could be written by a series of
 * operations.  After writing into the returned buffer, adjust_length() is
 * called to account for the number of characters actually written.
 *
 * On failure, null is returned and errno is set to ENOMEM. */
inline char* StringAccum::reserve(int n) {
    assert(n >= 0);
    if (_len + n <= _cap)
        return reinterpret_cast<char *>(_s + _len);
    else
        return grow(_len + n);
}

/** @brief Set the StringAccum's length to @a len.
 * @param len new length in characters
 * @pre 0 <= @a len <= capacity()
 * @sa adjust_length */
inline void StringAccum::set_length(int len) {
    assert(len >= 0 && _len <= _cap);
    _len = len;
}

/** @brief Adjust the StringAccum's length.
 * @param delta  length adjustment
 * @pre If @a delta > 0, then length() + @a delta <= capacity().
 *      If @a delta < 0, then length() + delta >= 0.
 *
 * The StringAccum's length after adjust_length(@a delta) equals its old
 * length plus @a delta.  Generally adjust_length() is used after a call
 * to reserve().  @sa set_length */
inline void StringAccum::adjust_length(int delta) {
    assert(_len + delta >= 0 && _len + delta <= _cap);
    _len += delta;
}

/** @brief Reserve space and adjust length in one operation.
 * @param nadjust number of characters to reserve and adjust length
 * @param nreserve additional characters to reserve
 * @pre @a nadjust >= 0 and @a nreserve >= 0
 *
 * This operation combines the effects of reserve(@a nadjust + @a
 * nreserve) and adjust_length(@a nadjust).  Returns the result of the
 * reserve() call. */
inline char* StringAccum::extend(int nadjust, int nreserve) {
    assert(nadjust >= 0 && nreserve >= 0);
    if (_len + nadjust + nreserve <= _cap) {
        char *x = reinterpret_cast<char *>(_s + _len);
        _len += nadjust;
        return x;
    } else
        return hard_extend(nadjust, nreserve);
}

/** @brief Remove characters from the end of the StringAccum.
 * @param n number of characters to remove
 * @pre @a n >= 0 and @a n <= length()
 *
 * Same as adjust_length(-@a n). */
inline void StringAccum::pop_back(int n) {
    assert(n >= 0 && _len >= n);
    _len -= n;
}

/** @brief Append character @a c to the StringAccum.
 * @param c character to append */
inline void StringAccum::append(char c) {
    if (_len < _cap || grow(_len))
        _s[_len++] = c;
}

/** @overload */
inline void StringAccum::append(unsigned char c) {
    append(static_cast<char>(c));
}

/** @brief Append the UTF-8 encoding of Unicode character @a ch. */
inline void StringAccum::append_utf8(unsigned ch) {
    if (ch < 0x80)
        append((unsigned char) ch);
    else
        append_utf8_hard(ch);
}

/** @brief Append the first @a len characters of @a s to this StringAccum.
 * @param s data to append
 * @param len length of data
 * @pre @a len >= 0 */
inline void StringAccum::append(const char* s, int len) {
    assert(len >= 0);
    if (_len + len <= _cap) {
        memcpy(_s + _len, s, len);
        _len += len;
    } else
        hard_append(s, len);
}

/** @overload */
inline void StringAccum::append(const unsigned char* s, int len) {
    append(reinterpret_cast<const char *>(s), len);
}

/** @brief Append the data from @a begin to @a end to the end of this
 * StringAccum.
 *
 * Does nothing if @a begin >= @a end. */
inline void StringAccum::append(const char* begin, const char* end) {
    if (begin < end)
        append(begin, end - begin);
}

/** @overload */
inline void StringAccum::append(const unsigned char* begin, const unsigned char* end) {
    if (begin < end)
        append(begin, end - begin);
}

/** @brief Assign this StringAccum to @a x. */
inline StringAccum& StringAccum::operator=(const StringAccum& x) {
    if (&x != this) {
        if (out_of_memory())
            _s = 0, _cap = 0;
        _len = 0;
        append(x.data(), x.length());
    }
    return *this;
}

/** @relates StringAccum
    @brief Append character @a c to StringAccum @a sa.
    @return @a sa
    @note Same as @a sa.append(@a c). */
inline StringAccum &
operator<<(StringAccum &sa, char c)
{
    sa.append(c);
    return sa;
}

/** @relates StringAccum
    @brief Append character @a c to StringAccum @a sa.
    @return @a sa
    @note Same as @a sa.append(@a c). */
inline StringAccum &
operator<<(StringAccum &sa, unsigned char c)
{
    sa.append(c);
    return sa;
}

/** @relates StringAccum
    @brief Append null-terminated C string @a cstr to StringAccum @a sa.
    @return @a sa
    @note Same as @a sa.append(@a cstr). */
inline StringAccum &
operator<<(StringAccum &sa, const char *cstr)
{
    sa.append(cstr);
    return sa;
}

/** @relates StringAccum
    @brief Append "true" or "false" to @a sa, depending on @a b.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, bool b)
{
    static const char truefalse[] = "truefalse";
    if (b)
        sa.append(truefalse, 4);
    else
        sa.append(truefalse + 4, 5);
    return sa;
}

/** @relates StringAccum
    @brief Append decimal representation of @a i to @a sa.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, short i)
{
    return sa << static_cast<long>(i);
}

/** @relates StringAccum
    @brief Append decimal representation of @a u to @a sa.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, unsigned short u)
{
    return sa << static_cast<unsigned long>(u);
}

/** @relates StringAccum
    @brief Append decimal representation of @a i to @a sa.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, int i)
{
    return sa << static_cast<long>(i);
}

/** @relates StringAccum
    @brief Append decimal representation of @a u to @a sa.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, unsigned u)
{
    return sa << static_cast<unsigned long>(u);
}

/** @relates StringAccum
    @brief Append the contents of @a str to @a sa.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, const String &str)
{
    if (!str.out_of_memory())
        sa.hard_append(str.begin(), str.length());
    else
        sa.assign_out_of_memory();
    return sa;
}

#ifdef HAVE_PERMSTRING
inline StringAccum &
operator<<(StringAccum &sa, PermString s)
{
    sa.append(s.c_str(), s.length());
    return sa;
}
#endif

/** @relates StringAccum
    @brief Append the contents of @a sb to @a sa.
    @return @a sa */
inline StringAccum &
operator<<(StringAccum &sa, const StringAccum &sb)
{
    sa.append(sb.begin(), sb.end());
    return sa;
}

inline bool
operator==(StringAccum &sa, const char *s)
{
    return strcmp(sa.c_str(), s) == 0;
}

inline bool
operator!=(StringAccum &sa, const char *s)
{
    return strcmp(sa.c_str(), s) != 0;
}

#undef LCDF_SNPRINTF_ATTR
#endif
