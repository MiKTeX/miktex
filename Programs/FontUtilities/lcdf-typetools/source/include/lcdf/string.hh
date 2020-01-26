// -*- related-file-name: "../../liblcdf/string.cc" -*-
#ifndef LCDF_STRING_HH
#define LCDF_STRING_HH
#include <lcdf/hashcode.hh>
#ifdef HAVE_PERMSTRING
# include <lcdf/permstr.hh>
#endif
#include <assert.h>
class StringAccum;

class String { public:

    /** @brief Construct an empty String (with length 0). */
    inline String() {
	assign_memo(&null_data, 0, 0);
    }

    /** @brief Construct a copy of the String @a x. */
    inline String(const String &x) {
	assign(x);
    }

    /** @brief Construct a String containing the C string @a cstr.
     * @param cstr a null-terminated C string
     * @return A String containing the characters of @a cstr, up to but not
     * including the terminating null character.
     *
     * If @a cstr equals String::out_of_memory_data(), returns an
     * out-of-memory string. */
    inline String(const char *cstr) {
	assign(cstr, -1, false);
    }

    /** @brief Construct a String containing the first @a len characters of
     * string @a s.
     * @param s a string
     * @param len number of characters to take from @a s.  If @a len @< 0,
     * then takes @c strlen(@a s) characters.
     * @return A String containing @a len characters of @a s.
     *
     * If @a s equals String::out_of_memory_data(), returns an out-of-memory
     * string. */
    inline String(const char *s, int len) {
	assign(s, len, false);
    }
    /** @overload */
    inline String(const unsigned char *s, int len) {
	assign(reinterpret_cast<const char *>(s), len, false);
    }

    /** @brief Construct a String containing the characters from @a begin
     * to @a end.
     * @param begin first character in string (begin iterator)
     * @param end pointer one past last character in string (end iterator)
     * @return A String containing the characters from @a begin to @a end.
     *
     * Returns a null string if @a begin @>= @a end.  If @a begin equals
     * String::out_of_memory_data(), returns an out-of-memory string. */
    inline String(const char *begin, const char *end) {
	assign(begin, (end > begin ? end - begin : 0), false);
    }
    /** @overload */
    inline String(const unsigned char *begin, const unsigned char *end) {
	assign(reinterpret_cast<const char *>(begin),
	       (end > begin ? end - begin : 0), false);
    }

    /** @brief Construct a String equal to "true" or "false" depending on the
     * value of @a x. */
    explicit inline String(bool x) {
	assign_memo(bool_data + (x ? 0 : 5), x ? 4 : 5, 0);
    }

    /** @brief Construct a String containing the single character @a c. */
    explicit inline String(char c) {
	assign(&c, 1, false);
    }

    /** @overload */
    explicit inline String(unsigned char c) {
	assign(reinterpret_cast<char *>(&c), 1, false);
    }

    /** @brief Construct a base-10 string representation of @a x. */
    explicit String(int x);
    /** @overload */
    explicit String(unsigned x);
    /** @overload */
    explicit String(long x);
    /** @overload */
    explicit String(unsigned long x);
    /** @overload */
    explicit String(double x);
#if HAVE_PERMSTRING
    inline String(PermString x) {
	assign(x);
    }
#endif

    /** @brief Destroy a String, freeing memory if necessary. */
    inline ~String() {
	deref();
    }


    /** @brief Return a const reference to an empty String.
     *
     * May be quicker than String::String(). */
    static inline const String &make_empty() {
	return reinterpret_cast<const String &>(null_string_rep);
    }

    /** @brief Return a String containing @a len unknown characters. */
    static String make_uninitialized(int len) {
	String s;
	s.append_uninitialized(len);
	return s;
    }

    /** @brief Return a String that directly references the first @a len
     * characters of @a s.
     *
     * This function is suitable for static constant strings whose data is
     * known to stay around forever, such as C string constants.  If @a len @<
     * 0, treats @a s as a null-terminated C string.
     *
     * @warning The String implementation may access @a s[@a len], which
     * should remain constant even though it's not part of the String. */
    static String make_stable(const char *s, int len = -1);

    /** @brief Return a String that directly references the character data in
     * [@a begin, @a end).
     * @param begin pointer to the first character in the character data
     * @param end pointer one beyond the last character in the character data
     *  (but see the warning)
     *
     * This function is suitable for static constant strings whose data is
     * known to stay around forever, such as C string constants.  Returns an
     * empty string if @a begin @>= @a end.
     *
     * @warning The String implementation may access *@a end, which should
     * remain constant even though it's not part of the String. */
    static inline String make_stable(const char *begin, const char *end) {
	if (begin < end)
	    return String::make_stable(begin, end - begin);
	else
	    return String();
    }
    static String make_fill(int c, int n); // n copies of c


    /** @brief Return the string's length. */
    inline int length() const {
	return _r.length;
    }

    /** @brief Return a pointer to the string's data.
     *
     * Only the first length() characters are valid, and the string data
     * might not be null-terminated. */
    inline const char *data() const {
	return _r.data;
    }

    /** @brief Return a pointer to the string's data as unsigned chars.
     *
     * Only the first length() characters are valid, and the string data
     * might not be null-terminated.  @sa data() */
    inline const unsigned char *udata() const {
	return reinterpret_cast<const unsigned char *>(_r.data);
    }


    typedef const char *const_iterator;
    typedef const_iterator iterator;

    /** @brief Return an iterator for the first character in the string.
     *
     * String iterators are simply pointers into string data, so they are
     * quite efficient.  @sa String::data */
    inline const_iterator begin() const {
	return _r.data;
    }

    /** @brief Return an iterator for the end of the string.
     *
     * The return value points one character beyond the last character in the
     * string. */
    inline const_iterator end() const {
	return _r.data + _r.length;
    }

    inline const unsigned char* ubegin() const {
        return reinterpret_cast<const unsigned char*>(_r.data);
    }

    inline const unsigned char* uend() const {
        return reinterpret_cast<const unsigned char*>(_r.data + _r.length);
    }


    typedef int (String::*unspecified_bool_type)() const;
    /** @brief Return true iff the string is nonempty. */
    inline operator unspecified_bool_type() const {
	return _r.length != 0 ? &String::length : 0;
    }

    /** @brief Return true iff the string is empty. */
    inline bool empty() const {
	return _r.length == 0;
    }

    /** @brief Return true iff the string is empty. */
    inline bool operator!() const {
	return empty();
    }


    /** @brief Return the @a i th character in the string.
     *
     * Does not check bounds.  @sa String::at */
    inline char operator[](int i) const {
	return _r.data[i];
    }

    /** @brief Return the @a i th character in the string.
     *
     * Checks bounds: an assertion will fail if @a i is less than 0 or not
     * less than length().  @sa String::operator[] */
    inline char at(int i) const {
	assert((unsigned) i < (unsigned) _r.length);
	return _r.data[i];
    }

    /** @brief Return the first character in the string.
     *
     * Does not check bounds.  Same as (*this)[0]. */
    inline char front() const {
	return _r.data[0];
    }

    /** @brief Return the last character in the string.
     *
     * Does not check bounds.  Same as (*this)[length() - 1]. */
    inline char back() const {
	return _r.data[_r.length - 1];
    }


    /** @brief Null-terminate the string.
     *
     * The terminating null character isn't considered part of the string, so
     * this->length() doesn't change.  Returns a corresponding C string
     * pointer.  The returned pointer is semi-temporary; it will persist until
     * the string is destroyed or appended to. */
    inline const char *c_str() const {
	// We may already have a '\0' in the right place.  If _memo has no
	// capacity, then this is one of the special strings (null or
	// stable). We are guaranteed, in these strings, that _data[_length]
	// exists. Otherwise must check that _data[_length] exists.
	const char *end_data = _r.data + _r.length;
	if ((_r.memo && end_data >= _r.memo->real_data + _r.memo->dirty)
	    || *end_data != '\0') {
	    if (char *x = const_cast<String *>(this)->append_uninitialized(1)) {
		*x = '\0';
		--_r.length;
	    }
	}
	return _r.data;
    }

#if HAVE_PERMSTRING
    operator PermString() const	{
	return PermString(_r.data, _r.length);
    }
#endif


    /** @brief Return a 32-bit hash function of the characters in [begin, end).
     *
     * Uses Paul Hsieh's "SuperFastHash" algorithm, described at
     * http://www.azillionmonkeys.com/qed/hash.html
     * This hash function uses all characters in the string.
     *
     * @invariant If end1 - begin1 == end2 - begin2 and memcmp(begin1, begin2,
     * end1 - begin1) == 0, then hashcode(begin1, end1) == hashcode(begin2,
     * end2). */
    static hashcode_t hashcode(const char *begin, const char *end);

    /** @overload */
    static inline hashcode_t hashcode(const unsigned char *begin,
				      const unsigned char *end) {
	return hashcode(reinterpret_cast<const char *>(begin),
			reinterpret_cast<const char *>(end));
    }

    /** @brief Returns a 32-bit hash function of this string's characters.
     *
     * Equivalent to String::hashcode(begin(), end()).  Uses Paul Hsieh's
     * "SuperFastHash."
     *
     * @invariant  If s1 == s2, then s1.hashcode() == s2.hashcode(). */
    inline hashcode_t hashcode() const {
	return length() ? hashcode(begin(), end()) : 0;
    }


    /** @brief Return true iff this string is equal to the data in @a s.
     * @param s string data to compare to
     * @param len length of @a s
     *
     * Same as String::compare(*this, String(s, len)) == 0.  If @a len @< 0,
     * then treats @a s as a null-terminated C string.
     *
     * @sa String::compare(const String &a, const String &b) */
    bool equals(const char *s, int len) const;

    // bool operator==(const String &, const String &);
    // bool operator==(const String &, const char *);
    // bool operator==(const char *, const String &);
    // bool operator!=(const String &, const String &);
    // bool operator!=(const String &, const char *);
    // bool operator!=(const char *, const String &);


    /** @brief Compare two strings.
     * @param a first string to compare
     * @param b second string to compare
     *
     * Returns 0 if @a a == @a b, negative if @a a @< @a b in lexicographic
     * order, and positive if @a a @> @a b in lexicographic order.  The
     * lexicographic order treats all characters as unsigned. */
    static inline int compare(const String &a, const String &b) {
	return a.compare(b);
    }

    /** @brief Compare this string with string @a x.
     *
     * Same as String::compare(*this, @a x).
     * @sa String::compare(const String &a, const String &b) */
    inline int compare(const String &x) const {
	return compare(x._r.data, x._r.length);
    }

    /** @brief Compare this string with the data in @a s.
     * @param s string data to compare to
     * @param len length of @a s
     *
     * Same as String::compare(*this, String(s, len)).  If @a len @< 0, then
     * treats @a s as a null-terminated C string.
     *
     * @sa String::compare(const String &a, const String &b) */
    int compare(const char *s, int len) const;

    // bool operator<(const String &, const String &);
    // bool operator<=(const String &, const String &);
    // bool operator>(const String &, const String &);
    // bool operator>=(const String &, const String &);


    /** @brief Return a substring of the current string starting at @a begin
     * and ending before @a end.
     * @param begin pointer to the first substring character
     * @param end pointer one beyond the last substring character
     *
     * Returns an empty string if @a begin @>= @a end.  Also returns an empty
     * string if @a begin or @a end is out of range (i.e., either less than
     * this->begin() or greater than this->end()), but this should be
     * considered a programming error; a future version may generate a warning
     * for this case. */
    inline String substring(const char *begin, const char *end) const {
	if (begin < end && begin >= _r.data && end <= _r.data + _r.length)
	    return String(begin, end - begin, _r.memo);
	else
	    return String();
    }

    /** @overload */
    inline String substring(const unsigned char* first, const unsigned char* last) const {
	return substring(reinterpret_cast<const char*>(first),
			 reinterpret_cast<const char*>(last));
    }

    /** @brief Return a substring of this string, consisting of the @a len
     * characters starting at index @a pos.
     * @param pos substring's first position relative to the string
     * @param len length of substring
     *
     * If @a pos is negative, starts that far from the end of the string.  If
     * @a len is negative, leaves that many characters off the end of the
     * string.  If @a pos and @a len specify a substring that is partly
     * outside the string, only the part within the string is returned.  If
     * the substring is beyond either end of the string, returns an empty
     * string (but this should be considered a programming error; a future
     * version may generate a warning for this case).
     *
     * @note String::substring() is intended to behave like Perl's substr(). */
    String substring(int pos, int len) const;

    /** @brief Return the suffix of the current string starting at index @a pos.
     *
     * If @a pos is negative, starts that far from the end of the string.  If
     * @a pos is so negative that the suffix starts outside the string, then
     * the entire string is returned.  If the substring is beyond the end of
     * the string (@a pos > length()), returns an empty string (but this
     * should be considered a programming error; a future version may generate
     * a warning for this case).
     *
     * @note String::substring() is intended to behave like Perl's substr(). */
    inline String substring(int pos) const {
	return substring((pos <= -_r.length ? 0 : pos), _r.length);
    }


    /** @brief Search for a character in a string.
     * @param c character to search for
     * @param start initial search position
     *
     * Return the index of the leftmost occurence of @a c, starting at index
     * @a start and working up to the end of the string.  Returns -1 if @a c
     * is not found. */
    int find_left(char c, int start = 0) const;

    /** @brief Search for a substring in a string.
     * @param x substring to search for
     * @param start initial search position
     *
     * Return the index of the leftmost occurence of the substring @a str,
     * starting at index @a start and working up to the end of the string.
     * Returns -1 if @a str is not found. */
    int find_left(const String &x, int start = 0) const;

    /** @brief Search for a character in a string.
     * @param c character to search for
     * @param start initial search position
     *
     * Return the index of the rightmost occurence of the character @a c,
     * starting at index @a start and working back to the beginning of the
     * string.  Returns -1 if @a c is not found.  @a start may start beyond
     * the end of the string. */
    int find_right(char c, int start = 0x7FFFFFFF) const;

    /** @brief Return true iff this string begins with prefix @a x.
     *
     * Same as String::starts_with(@a x.data(), @a x.length()). */
    inline bool starts_with(const String &x) const {
	return starts_with(x._r.data, x._r.length);
    }

    /** @brief Return true iff this string begins with the data in @a s.
     * @param s string data to compare to
     * @param len length of @a s
     *
     * If @a len @< 0, then treats @a s as a null-terminated C string.
     *
     * @sa String::compare(const String &a, const String &b) */
    bool starts_with(const char *s, int len) const;

    String lower() const;
    String upper() const;
    String printable(int type = 0) const;


    /** @brief Assign this string to @a x. */
    inline String &operator=(const String &x) {
	if (&x != this) {
	    deref();
	    assign(x);
	}
	return *this;
    }

    /** @brief Assign this string to the C string @a cstr. */
    inline String &operator=(const char *cstr) {
	assign(cstr, -1, true);
	return *this;
    }

#ifdef HAVE_PERMSTRING
    inline String &operator=(PermString p) {
	deref();
	assign(p);
	return *this;
    }
#endif

    /** @brief Append the null-terminated C string @a cstr to this string.
     * @param cstr data to append */
    inline void append(const char *cstr) {
	append(cstr, -1, 0);
    }

    /** @brief Append the first @a len characters of @a s to this string.
     * @param s data to append
     * @param len length of data
     * @pre @a len @>= 0 */
    inline void append(const char *s, int len) {
	append(s, len, 0);
    }

    /** @brief Appends the data from @a begin to @a end to the end of this
     * string.
     *
     * Does nothing if @a begin @>= @a end. */
    inline void append(const char *begin, const char *end) {
	if (begin < end)
	    append(begin, end - begin);
    }

    /** @brief Append @a len copies of character @a c to this string. */
    void append_fill(int c, int len);

    /** @brief Append @a len unknown characters to this string.
     * @return Modifiable pointer to the appended characters.
     *
     * The caller may safely modify the returned memory.  Null is returned if
     * the string becomes out-of-memory. */
    char *append_uninitialized(int len);


    /** @brief Append a copy of @a x to the end of this string.
     *
     * Returns the result. */
    inline String &operator+=(const String &x) {
	append(x._r.data, x._r.length, x._r.memo);
	return *this;
    }

    /** @brief Append a copy of the C string @a cstr to the end of this string.
     *
     * Returns the result. */
    inline String &operator+=(const char *cstr) {
	append(cstr);
	return *this;
    }

    /** @brief Append the character @a c to the end of this string.
     *
     * Returns the result. */
    inline String &operator+=(char c) {
	append(&c, 1);
	return *this;
    }

#if HAVE_PERMSTRING
    inline String &operator+=(PermString p) {
	append(p.c_str(), p.length());
	return *this;
    }
#endif


    // String operator+(String, const String &);
    // String operator+(String, const char *);
    // String operator+(const char *, const String &);
    // String operator+(String, PermString);
    // String operator+(PermString, String);
    // String operator+(PermString, const char *);
    // String operator+(const char *, PermString);
    // String operator+(PermString, PermString);
    // String operator+(String, char);


    /** @brief Return true iff the String's data is shared or immutable. */
    inline bool data_shared() const {
	return !_r.memo || _r.memo->refcount != 1;
    }

    /** @brief Return a compact version of this String.
     *
     * The compact version shares no more than 256 bytes of data with any
     * other non-stable String. */
    inline String compact() const {
	if (!_r.memo || _r.memo->refcount == 1
	    || (uint32_t) _r.length + 256 >= _r.memo->capacity)
	    return *this;
	else
	    return String(_r.data, _r.data + _r.length);
    }

    /** @brief Ensure the string's data is unshared and return a mutable
     * pointer to it. */
    char *mutable_data();

    /** @brief Null-terminate the string and return a mutable pointer to its
     * data.
     * @sa String::c_str */
    char *mutable_c_str();

    unsigned char *mutable_udata() {
	return reinterpret_cast<unsigned char *>(mutable_data());
    }

    void align(int);


    /** @brief Return true iff this is an out-of-memory string. */
    inline bool out_of_memory() const {
	return _r.data == &oom_data;
    }

    /** @brief Return a const reference to an out-of-memory String. */
    static inline const String &make_out_of_memory() {
	return reinterpret_cast<const String &>(oom_string_rep);
    }

    /** @brief Return the data pointer used for out-of-memory strings.
     *
     * The returned value may be dereferenced; it points to a null
     * character. */
    static inline const char *out_of_memory_data() {
	return &oom_data;
    }

  private:

    /** @cond never */
    struct memo_t {
	volatile uint32_t refcount;
	uint32_t capacity;
	volatile uint32_t dirty;
#if HAVE_STRING_PROFILING > 1
	memo_t **pprev;
	memo_t *next;
#endif
	char real_data[8];	// but it might be more or less
    };

    enum {
	MEMO_SPACE = sizeof(memo_t) - 8
    };

    struct rep_t {
	const char *data;
	int length;
	memo_t *memo;
    };
    /** @endcond never */

    mutable rep_t _r;		// mutable for c_str()

#if HAVE_STRING_PROFILING
    static uint64_t live_memo_count;
    static uint64_t memo_sizes[55];
    static uint64_t live_memo_sizes[55];
    static uint64_t live_memo_bytes[55];
# if HAVE_STRING_PROFILING > 1
    static memo_t *live_memos[55];
# endif

    static inline int profile_memo_size_bucket(uint32_t dirty, uint32_t capacity) {
	if (capacity <= 16)
	    return dirty;
	else if (capacity <= 32)
	    return 17 + (capacity - 17) / 2;
	else if (capacity <= 64)
	    return 25 + (capacity - 33) / 8;
	else
	    return 29 + 26 - ffs_msb(capacity - 1);
    }

    static void profile_update_memo_dirty(memo_t *memo, uint32_t old_dirty, uint32_t new_dirty, uint32_t capacity) {
	if (capacity <= 16 && new_dirty != old_dirty) {
	    ++memo_sizes[new_dirty];
	    ++live_memo_sizes[new_dirty];
	    live_memo_bytes[new_dirty] += capacity;
	    --live_memo_sizes[old_dirty];
	    live_memo_bytes[old_dirty] -= capacity;
# if HAVE_STRING_PROFILING > 1
	    if ((*memo->pprev = memo->next))
		memo->next->pprev = memo->pprev;
	    memo->pprev = &live_memos[new_dirty];
	    if ((memo->next = *memo->pprev))
		memo->next->pprev = &memo->next;
	    *memo->pprev = memo;
# else
	    (void) memo;
# endif
	}
    }

    static void one_profile_report(StringAccum &sa, int i, int examples);
#endif

    inline void assign_memo(const char *data, int length, memo_t *memo) const {
	_r.data = data;
	_r.length = length;
	if ((_r.memo = memo))
	    ++memo->refcount;
    }

    inline String(const char *data, int length, memo_t *memo) {
	assign_memo(data, length, memo);
    }

    inline void assign(const String &x) const {
	assign_memo(x._r.data, x._r.length, x._r.memo);
    }

    inline void deref() const {
	if (_r.memo && --_r.memo->refcount == 0)
	    delete_memo(_r.memo);
    }

    void assign(const char *cstr, int len, bool need_deref);
#if HAVE_PERMSTRING
    inline void assign(PermString x) const {
	assign_memo(x.c_str(), x.length(), 0);
    }
#endif
    void assign_out_of_memory();
    void append(const char *s, int len, memo_t *memo);
    static memo_t *create_memo(char *space, int dirty, int capacity);
    static void delete_memo(memo_t *memo);

    static const char null_data;
    static const char oom_data;
    static const char bool_data[11];
    static const char int_data[20];
    static const rep_t null_string_rep;
    static const rep_t oom_string_rep;

    static String make_claim(char *, int, int); // claim memory

    friend struct rep_t;
    friend class StringAccum;

};


/** @relates String
 * @brief Compares two strings for equality.
 *
 * Returns true iff the two operands have the same lengths and the same
 * characters in the same order.  At most one of the operands can be a
 * null-terminated C string.
 * @sa String::compare
 */
inline bool operator==(const String &a, const String &b) {
    return a.equals(b.data(), b.length());
}

/** @relates String */
inline bool operator==(const char *a, const String &b) {
    return b.equals(a, -1);
}

/** @relates String */
inline bool operator==(const String &a, const char *b) {
    return a.equals(b, -1);
}

/** @relates String
 * @brief Compare two Strings for inequality.
 *
 * Returns true iff !(@a a == @a b).  At most one of the operands can be a
 * null-terminated C string. */
inline bool operator!=(const String &a, const String &b) {
    return !a.equals(b.data(), b.length());
}

/** @relates String */
inline bool operator!=(const char *a, const String &b) {
    return !b.equals(a, -1);
}

/** @relates String */
inline bool operator!=(const String &a, const char *b) {
    return !a.equals(b, -1);
}

/** @relates String
 * @brief Compare two Strings.
 *
 * Returns true iff @a a @< @a b in lexicographic order.
 * @sa String::compare
 */
inline bool operator<(const String &a, const String &b) {
    return a.compare(b.data(), b.length()) < 0;
}

/** @relates String
 * @brief Compare two Strings.
 *
 * Returns true iff @a a @<= @a b in lexicographic order.
 * @sa String::compare
 */
inline bool operator<=(const String &a, const String &b) {
    return a.compare(b.data(), b.length()) <= 0;
}

/** @relates String
 * @brief Compare two Strings.
 *
 * Returns true iff @a a @> @a b in lexicographic order.
 * @sa String::compare
 */
inline bool operator>(const String &a, const String &b) {
    return a.compare(b.data(), b.length()) > 0;
}

/** @relates String
 * @brief Compare two Strings.
 *
 * Returns true iff @a a @>= @a b in lexicographic order.
 * @sa String::compare
 */
inline bool operator>=(const String &a, const String &b) {
    return a.compare(b.data(), b.length()) >= 0;
}

/** @relates String
 * @brief Concatenate the operands and return the result.
 *
 * At most one of the two operands can be a null-terminated C string. */
inline String operator+(String a, const String &b) {
    a += b;
    return a;
}

/** @relates String */
inline String operator+(String a, const char *b) {
    a.append(b);
    return a;
}

/** @relates String */
inline String operator+(const char *a, const String &b) {
    String s1(a);
    s1 += b;
    return s1;
}

/** @relates String
 * @brief Concatenate the operands and return the result.
 *
 * The second operand is a single character. */
inline String operator+(String a, char b) {
    a.append(&b, 1);
    return a;
}

#if HAVE_PERMSTRING

inline bool operator==(PermString a, const String &b) {
    return b.equals(a.c_str(), a.length());
}

inline bool operator==(const String &a, PermString b) {
    return a.equals(b.c_str(), b.length());
}

inline bool operator!=(PermString a, const String &b) {
    return !b.equals(a.c_str(), a.length());
}

inline bool operator!=(const String &a, PermString b) {
    return !a.equals(b.c_str(), b.length());
}

inline String operator+(String a, PermString b) {
    a.append(b.c_str(), b.length());
    return a;
}

inline String operator+(PermString a, String b) {
    return String(a) + b;
}

inline String operator+(PermString a, const char *b) {
    return String(a) + b;
}

inline String operator+(const char *a, PermString b) {
    return String(a) + b;
}

inline String operator+(PermString a, PermString b) {
    return String(a) + b;
}

#endif

inline hashcode_t hashcode(const String &str) {
    return str.hashcode();
}

#endif
