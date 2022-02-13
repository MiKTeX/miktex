
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <stdarg.h>
#include<functional>

#ifndef _WIN32
#include <cstring>
#endif
namespace std
{

	/**
	 * An extension of STL's string providing additional functionality that is often availiable in
	 * higher-level languages such as Python.
	 */
	class string_ref : public string
	{
	public:

		//static unsigned GetAutoSenseRadix(string_ref& Str) {
		//	if (Str.empty())
		//		return 10;

		//	if (Str.start_with("0x") || Str.start_with("0X")) {
		//		Str = Str.substr(2);
		//		return 16;
		//	}

		//	if (Str.start_with("0b") || Str.start_with("0B")) {
		//		Str = Str.substr(2);
		//		return 2;
		//	}

		//	if (Str.start_with("0o")) {
		//		Str = Str.substr(2);
		//		return 8;
		//	}

		//	if (Str[0] == '0' && Str.size() > 1 && std::isdigit(Str[1])) {
		//		Str = Str.substr(1);
		//		return 8;
		//	}

		//	return 10;
		//}

		//static bool consumeUnsignedInteger(string_ref& Str, unsigned Radix,
		//	unsigned long long& Result) {
		//	// Autosense radix if not specified.
		//	if (Radix == 0)
		//		Radix = GetAutoSenseRadix(Str);

		//	// Empty strings (after the radix autosense) are invalid.
		//	if (Str.empty()) return true;

		//	// Parse all the bytes of the string given this radix.  Watch for overflow.
		//	string_ref Str2 = Str;
		//	Result = 0;
		//	while (!Str2.empty()) {
		//		unsigned CharVal;
		//		if (Str2[0] >= '0' && Str2[0] <= '9')
		//			CharVal = Str2[0] - '0';
		//		else if (Str2[0] >= 'a' && Str2[0] <= 'z')
		//			CharVal = Str2[0] - 'a' + 10;
		//		else if (Str2[0] >= 'A' && Str2[0] <= 'Z')
		//			CharVal = Str2[0] - 'A' + 10;
		//		else
		//			break;

		//		// If the parsed value is larger than the integer radix, we cannot
		//		// consume any more characters.
		//		if (CharVal >= Radix)
		//			break;

		//		// Add in this character.
		//		unsigned long long PrevResult = Result;
		//		Result = Result * Radix + CharVal;

		//		// Check for overflow by shifting back and seeing if bits were lost.
		//		if (Result / Radix < PrevResult)
		//			return true;

		//		Str2 = Str2.substr(1);
		//	}

		//	// We consider the operation a failure if no characters were consumed
		//	// successfully.
		//	if (Str.size() == Str2.size())
		//		return true;

		//	Str = Str2;
		//	return false;
		//}

		//static bool consumeSignedInteger(string_ref& Str, unsigned Radix,
		//	long long& Result) {
		//	unsigned long long ULLVal;

		//	// Handle positive strings first.
		//	if (Str.empty() || Str.front() != '-') {
		//		if (consumeUnsignedInteger(Str, Radix, ULLVal) ||
		//			// Check for value so large it overflows a signed value.
		//			(long long)ULLVal < 0)
		//			return true;
		//		Result = ULLVal;
		//		return false;
		//	}

		//	// Get the positive part of the value.
		//	string_ref Str2 = Str.drop_front(1);
		//	if (consumeUnsignedInteger(Str2, Radix, ULLVal) ||
		//		// Reject values so large they'd overflow as negative signed, but allow
		//		// "-0".  This negates the unsigned so that the negative isn't undefined
		//		// on signed overflow.
		//		(long long)-ULLVal > 0)
		//		return true;

		//	Str = Str2;
		//	Result = -ULLVal;
		//	return false;
		//}

		///// GetAsUnsignedInteger - Workhorse method that converts a integer character
		///// sequence of radix up to 36 to an unsigned long long value.
		//static	bool getAsUnsignedInteger(string_ref Str, unsigned Radix,
		//	unsigned long long& Result) {
		//	if (consumeUnsignedInteger(Str, Radix, Result))
		//		return true;

		//	// For getAsUnsignedInteger, we require the whole string to be consumed or
		//	// else we consider it a failure.
		//	return !Str.empty();
		//}

		//static  bool getAsSignedInteger(string_ref Str, unsigned Radix,
		//	long long& Result) {
		//	if (consumeSignedInteger(Str, Radix, Result))
		//		return true;

		//	// For getAsSignedInteger, we require the whole string to be consumed or else
		//	// we consider it a failure.
		//	return !Str.empty();
		//}


		///// Parse the current string as an integer of the specified radix.  If
		///// \p Radix is specified as zero, this does radix autosensing using
		///// extended C rules: 0 is octal, 0x is hex, 0b is binary.
		/////
		///// If the string is invalid or if only a subset of the string is valid,
		///// this returns true to signify the error.  The string is considered
		///// erroneous if empty or if it overflows T.
		//template <typename T>
		//std::enable_if_t<std::numeric_limits<T>::is_signed, bool>
		//	getAsInteger(unsigned Radix, T& Result) const {
		//	long long LLVal;
		//	if (getAsSignedInteger(*this, Radix, LLVal) ||
		//		static_cast<T>(LLVal) != LLVal)
		//		return true;
		//	Result = LLVal;
		//	return false;
		//}

		//template <typename T>
		//std::enable_if_t<!std::numeric_limits<T>::is_signed, bool>
		//	getAsInteger(unsigned Radix, T& Result) const {
		//	unsigned long long ULLVal;
		//	// The additional cast to unsigned long long is required to avoid the
		//	// Visual C++ warning C4805: '!=' : unsafe mix of type 'bool' and type
		//	// 'unsigned __int64' when instantiating getAsInteger with T = bool.
		//	if (getAsUnsignedInteger(*this, Radix, ULLVal) ||
		//		static_cast<unsigned long long>(static_cast<T>(ULLVal)) != ULLVal)
		//		return true;
		//	Result = ULLVal;
		//	return false;
		//}


		
		/**`
		 * Default constructor
		 *
		 * Constructs an empty string_ref ("")
		 */
		string_ref() : string() { }

		/**
		 * Duplicate the STL string copy constructor
		 *
		 * @param[in] s   The string to copy
		 * @param[in] pos The starting position in the string to copy from
		 * @param[in] n   The number of characters to copy
		 */
		string_ref(const string &s, size_type pos = 0, size_type n = npos) : string(s, pos, npos) { }

		/**
		 * Construct an string_ref from a null-terminated character array
		 *
		 * @param[in] s The character array to copy into the new string
		 */
		string_ref(const value_type *s) : string(s) { }

		/**
		 * Construct an string_ref from a character array and a length
		 *
		 * @param[in] s The character array to copy into the new string
		 * @param[in] n The number of characters to copy
		 */
		string_ref(const value_type *s, size_type n) : string(s, n) { }

		/**
		 * Create an string_ref with @p n copies of @p c
		 *
		 * @param[in] n The number of copies
		 * @param[in] c The character to copy @p n times
		 */
		string_ref(size_type n, value_type c) : string(n, c) { }

		/**
		 * Create a string from a range
		 *
		 * @param[in] first The first element to copy in
		 * @param[in] last  The last element to copy in
		 */
		template <class InputIterator>
			string_ref(InputIterator first, InputIterator last) : string(first, last) { }

		/**
		 * The destructor
		 */
		~string_ref() { }

		/**
		 * Split a string by whitespace
		 *
		 * @return A vector of strings, each of which is a substring of the string
		 */
		vector<string_ref> split(size_type limit = npos) const
		{
			vector<string_ref> v;

			const_iterator 
				i = begin(),
				  last = i;
			for (; i != end(); i++)
			{
				if (*i == ' ' || *i == '\n' || *i == '\t' || *i == '\r')
				{
					if (i + 1 != end() && (i[1] == ' ' || i[1] == '\n' || i[1] == '\t' || i[1] == '\r'))
						continue;
					v.push_back(string_ref(last, i));
					last = i + 1;
					if (v.size() >= limit - 1)
					{
						v.push_back(string_ref(last, end()));
						return v;
					}
				}
			}

			if (last != i)
				v.push_back(string_ref(last, i));

			return v;
		}

		/**
		 * Split a string by a character
		 *
		 * Returns a vector of ext_strings, each of which is a substring of the string formed by splitting
		 * it on boundaries formed by the character @p separator.  If @p limit is set, the returned vector
		 * will contain a maximum of @p limit elements with the last element containing the rest of
		 * the string.
		 *
		 * If @p separator is not found in the string, a single element will be returned in the vector
		 * containing the entire string.
		 *
		 * The separators are removed from the output
		 *
		 * @param[in] separator The character separator to split the string on
		 * @param[in] limit     The maximum number of output elements
		 * @return A vector of strings, each of which is a substring of the string
		 *
		 * @section split_ex Example
		 * @code
		 * std::string_ref s("This|is|a|test.");
		 * std::vector<std::string_ref> v = s.split('|');
		 * std::copy(v.begin(), v.end(), std::ostream_iterator<std::string_ref>(std::cout, "\n"));
		 *
		 * This
		 * is
		 * a
		 * test.
		 * @endcode
		 */
		vector<string_ref> split(value_type separator, size_type limit = npos) const
		{
			vector<string_ref> v;

			const_iterator 
				i = begin(),
				last = i;
			for (; i != end(); i++)
			{
				if (*i == separator)
				{
					v.push_back(string_ref(last, i));
					last = i + 1;
					if (v.size() >= limit - 1)
					{
						v.push_back(string_ref(last, end()));
						return v;
					}
				}
			}

			if (last != i)
				v.push_back(string_ref(last, i));

			return v;
		}

		/**
		 * Split a string by another string
		 *
		 * Returns a vector of ext_strings, each of which is a substring of the string formed by
		 * splitting it on boundaries formed by the string @p separator.  If @p limit is set, the
		 * returned vector will contain a maximum of @p limit elements with the last element
		 * containing the rest of the string.
		 *
		 * If @p separator is not found in the string, a single element will be returned in the
		 * vector containing the entire string.
		 *
		 * The separators are removed from the output
		 *
		 * @param[in] separator The string separator to split the string on
		 * @param[in] limit     The maximum number of output elements
		 * @return A vector of strings, each of which is a substring of the string
		 *
		 * @ref split_ex
		 */
		vector<string_ref> split(const string &separator, size_type limit = npos) const
		{
			vector<string_ref> v;

			const_iterator
				i = begin(),
				last = i;
			for (; i != end(); i++)
			{
				if (string(i, i + separator.length()) == separator)
				{
					v.push_back(string_ref(last, i));
					last = i + separator.length();

					if (v.size() >= limit - 1)
					{
						v.push_back(string_ref(last, end()));
						return v;
					}
				}
			}

			if (last != i)
				v.push_back(string_ref(last, i));

			return v;
		}

		/**
		 * Convert a string into an integer
		 *
		 * Convert the initial portion of a string into a signed integer.  Once a non-numeric
		 * character is reached, the remainder of @p string is ignored and the integer that was
		 * read returned.
		 *
		 * @param s The string to convert
		 * @return The integer converted from @p string
		 */
		static long int integer(const string &s)
		{
			long int retval = 0;
			bool neg = false;

			for (const_iterator i = s.begin(); i != s.end(); i++)
			{
				if (i == s.begin())
				{
					if (*i == '-')
					{
						neg = true;
						continue;
					}
					else if (*i == '+')
						continue;
				}
				if (*i >= '0' && *i <= '9')
				{
					retval *= 10;
					retval += *i - '0';
				}
				else
					break;
			}

			if (neg)
				retval *= -1;

			return retval;
		}

		/**
		 * Convert the string to an integer
		 *
		 * Convert the initial portion of the string into a signed integer.  Once a non-numeric
		 * character is reached, the remainder of the string is ignored and the integer that had
		 * been read thus far is returned.
		 *
		 * @return The integer converted from the string
		 */
		long int integer() const
		{
			return integer(*this);
		}

		/**
		 * Split a string into chunks of size @p chunklen.  Returns a vector of strings.
		 *
		 * Splits a string into chunks of the given size.  The final chunk may not fill its
		 * entire allocated number of characters.
		 *
		 * @param[in] chunklen The number of characters per chunk
		 * @return A vector of strings, each of length <= chunklen
		 *
		 * @section chunk_split-ex Example
		 * @code
		 * std::string_ref s("abcdefghijk");
		 * std::vector<std::string_ref> v = s.chunk_split(3);
		 * std::copy(v.begin(), v.end(), ostream_iterator<std::string_ref>(cout, " "));
		 *
		 * abc def ghi jk
		 * @endcode
		 */
		vector<string_ref> chunk_split(size_type chunklen) const
		{
			vector<string_ref> retval;
			retval.reserve(size() / chunklen + 1);

			size_type count = 0;
			const_iterator
				i = begin(),
				last = i;
			for (; i != end(); i++, count++)
			{
				if (count == chunklen)
				{
					count = 0;
					retval.push_back(string_ref(last, i));
					last = i;
				}
			}
			
			if (last != i)
				retval.push_back(string_ref(last, i));

			return retval;
		}

		/**
		 * Join a sequence of strings by some glue to create a new string
		 *
		 * Glue is not added to the end of the string.
		 *
		 * @pre [first, last) is a valid range
		 * @pre InputIterator is a model of STL's Input Iterator
		 * @pre InputIterator must point to a string type (std::string, std::string_ref, char *)
		 *
		 * @param[in] glue  The glue to join strings with
		 * @param[in] first The beginning of the range to join
		 * @param[in] last  The end of the range to join
		 * @return A string constructed of each element of the range connected together with @p glue
		 *
		 * @section join_ex Example
		 * @code
		 * std::vector<std::string_ref> v;
		 * v.push_back("This");
		 * v.push_back("is");
		 * v.push_back("a");
		 * v.push_back("test.");
		 * std::cout << std::string_ref::join("|", v.begin(), v.end()) << std::endl;
		 *
		 * This|is|a|test.
		 * @endcode
		 */
		template <class InputIterator>
			static string_ref join(const string &glue, InputIterator first, InputIterator last)
			{
				string_ref retval;

				for (; first != last; ++first)
				{
					retval.append(*first);
					retval.append(glue);
				}
				retval.erase(retval.length() - glue.length());

				return retval;
			}

		/**
		 * Join a sequence of strings by some glue to create a new string
		 *
		 * @copydoc join
		 * @ref join_ex
		 */
		template <class InputIterator>
			static string_ref join(value_type glue, InputIterator first, InputIterator last)
			{
				string_ref retval;

				for (; first != last; ++first)
				{
					retval.append(*first);
					retval.append(1, glue);
				}
				retval.erase(retval.length() - 1);

				return retval;
			}

		/**
		 * Search for any instances of @p needle and replace them with @p s
		 *
		 * @param[in] needle The string to replace
		 * @param[in] s      The replacement string
		 * @return				*this
		 * @post					All instances of @p needle in the string are replaced with @p s
		 *
		 * @section replace-ex Example
		 * @code
		 * std::string_ref s("This is a test.");
		 * s.replace("is", "ere");
		 * std::cout << s << std::endl;
		 *
		 * There ere a test.
		 * @endcode
		 */
		string_ref &replace(const string &needle, const string &s)
		{
			size_type
				lastpos = 0,
				thispos;

			while ((thispos = find(needle, lastpos)) != npos)
			{
				string::replace(thispos, needle.length(), s);
				lastpos = thispos + 1;
			}
			return *this;
		}
		string_ref &replace_first(const string &needle, const string &s)
		{
			size_type
				lastpos = 0,
				thispos;

			if ((thispos = find(needle, lastpos)) != npos)
			{
				string::replace(thispos, needle.length(), s);
				lastpos = thispos + 1;
			}
			return *this;
		}
		/**
		 * Search of any instances of @p needle and replace them with @p c
		 *
		 * @param[in] needle The character to replace
		 * @param[in] c      The replacement character
		 * @return           *this
		 * @post             All instances of @p needle in the string are replaced with @p c
		 *
		 * @ref replace-ex
		 */
		string_ref &replace(value_type needle, value_type c)
		{
			for (iterator i = begin(); i != end(); i++)
				if (*i == needle)
					*i = c;

			return *this;
		}

		/**
		 * Repeat a string @p n times
		 *
		 * @param[in] n The number of times to repeat the string
		 * @return string_ref containing @p n copies of the string
		 *
		 * @section repeat-ex Example
		 * @code
		 * std::string_ref s("123");
		 * s = s * 3;
		 * std::cout << s << std::endl;
		 *
		 * 123123123
		 * @endcode
		 */
		string_ref operator*(size_type n)
		{
			string_ref retval;
			for (size_type i = 0; i < n; i++)
				retval.append(*this);

			return retval;
		}

		/**
		 * Convert the string to lowercase
		 *
		 * @return *this
		 * @post The string is converted to lowercase
		 */
		string_ref &tolower()
		{
			for (iterator i = begin(); i != end(); i++)
				if (*i >= 'A' && *i <= 'Z')
					*i = (*i) + ('a' - 'A');
			return *this;
		}

		/**
		 * Convert the string to uppercase
		 *
		 * @return *this
		 * @post The string is converted to uppercase
		 */
		string_ref &toupper()
		{
			for (iterator i = begin(); i != end(); i++)
				if (*i >= 'a' && *i <= 'z')
					*i = (*i) - ('a' - 'A');
			return *this;
		}

		/**
		 * Count the occurances of @p str in the string.
		 *
		 * @return The count of substrings @p str in the string
		 */
		size_type count(const string &str) const
		{
			size_type
				count = 0,
				last = 0,
				cur = 0;

			while ((cur = find(str, last + 1)) != npos)
			{
				count++;
				last = cur;
			}

			return count;
		}

		/**
		 * Determine if the string is alphanumeric
		 *
		 * @return true if the string contains only characters between a-z, A-Z and 0-9 and
		 * contains at least one character, else false
		 */
		bool is_alnum() const
		{
			if (length() == 0)
				return false;

			for (const_iterator i = begin(); i != end(); i++)
			{
				if (*i < 'A' || *i > 'Z')
					if (*i < '0' || *i > '9')
						if (*i < 'a' || *i > 'z')
							return false;
			}

			return true;
		}

		/**
		 * Determine if the string is alphabetic only
		 *
		 * @return true of the string contains only characters between a-z and A-Z and contains at
		 * least one character, else false
		 */
		bool is_alpha() const
		{
			if (length() == 0)
				return false;

			for (const_iterator i = begin(); i != end(); i++)
				if (*i < 'A' || (*i > 'Z' && (*i < 'a' || *i > 'z')))
					return false;

			return true;
		}

		/**
		 * Determine if the string is numeric only
		 *
		 * @return true if the string contains only characters between 0-9 and contains at least
		 * one character, else false
		 */
		bool is_numeric() const
		{
			if (length() == 0)
				return false;

			for (const_iterator i = begin(); i != end(); i++)
				if (*i < '0' || *i > '9')
					return false;

			return true;
		}

		/**
		 * Determine if a string is all lower case
		 *
		 * @return true if there is at least one character, and all characters are lowercase
		 * letters, else false
		 */
		bool is_lower() const
		{
			if (length() == 0)
				return false;

			for (const_iterator i = begin(); i != end(); i++)
				if (*i < 'a' || *i < 'z')
					return false;

			return true;
		}

		/**
		 * Determine if a string is all upper case
		 *
		 * @return true if there is at least one character, and all characters are uppercase
		 * letters, else false
		 */
		bool is_upper() const
		{
			if (length() == 0)
				return false;

			for (const_iterator i = begin(); i != end(); i++)
				if (*i < 'A' || *i > 'Z')
					return false;

			return true;
		}

		/**
		 * Swap the case of a string
		 *
		 * @post Converts all uppercase to lowercase, and all lowercase to uppercase in the string
		 * @return *this
		 */
		string_ref &swapcase()
		{
			for (iterator i = begin(); i != end(); i++)
				if (*i >= 'A' && *i <= 'Z')
					*i += ('a' - 'A');
				else if (*i >= 'a' && *i <= 'z')
					*i -= ('a' - 'A');
			
			return *this;
		}

		/*******************************************************************************
			Function:	std::string_ref::start_with
			Access:		public 
			Qualifier:	const
			Parameter:	const string & str			
			Returns:	bool		
		
			Purpose:	is the string start with str
		*******************************************************************************/
		bool start_with(const string& str) const
		{
			return ( this->find(str) == 0 );
		}

		/// Return a string_ref equal to 'this' but with only the last \p N
		/// elements remaining.  If \p N is greater than the length of the
		/// string, the entire string is returned.
		
		string_ref take_back(size_t N = 1) const {
			if (N >= size())
				return *this;
			return drop_front(size() - N);
		}
		/// Return a string_ref equal to 'this' but with the first \p N elements
		/// dropped.
		
		string_ref drop_front(size_t N = 1) const {
			//assert(size() >= N && "Dropping more elements than exist");
			return substr(N);
		}



		/// Return a string_ref equal to 'this' but with the last \p N elements
		/// dropped.
		
		string_ref drop_back(size_t N = 1) const {
		
			return substr(0, size() - N);
		}

		/// Return a string_ref equal to 'this', but with all characters satisfying
		/// the given predicate dropped from the beginning of the string.
		
		string_ref drop_while(std::function<bool(char)> F) const {
			return substr(std::find_if_not(begin(),end(),F)-begin());
		}

		/// Return a string_ref equal to 'this', but with all characters not
		/// satisfying the given predicate dropped from the beginning of the string.
		
			string_ref drop_until(std::function<bool(char)> F) const {
			return substr(std::find_if(begin(), end(), F) - begin());
		}

		/// Returns true if this string_ref has the given prefix and removes that
		/// prefix.
		bool consume_front(string_ref Prefix) {
			if (!start_with(Prefix))
				return false;

			*this = drop_front(Prefix.size());
			return true;
		}

		/// Returns true if this string_ref has the given suffix and removes that
		/// suffix.
		bool consume_back(string_ref Suffix) {
			if (!end_with(Suffix))
				return false;

			*this = drop_back(Suffix.size());
			return true;
		}

		/*******************************************************************************
			Function:	std::string_ref::end_with
			Access:		public 
			Qualifier:	const
			Parameter:	const string & str			
			Returns:	bool		
		
			Purpose:	is the string end with str
		*******************************************************************************/
		bool end_with(const string& str) const
		{
			if (str.length() > this->length())
			{
				return false;
			}
			size_type off = this->length() - str.length();
			return ( find(str, off) == off);
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::format
			Access:		public 
			Qualifier:	
			Parameter:	const char * format_string
			Parameter:	...
			Returns:	string_ref&
			
			Purpose:	format the string
		*******************************************************************************/
		string_ref& format(const char* format_string, ...)
		{
            if (format_string == 0)
            {
                return *this;
            }

            va_list argList;
            va_start( argList, format_string );

#ifdef _WIN32
            int len = _vscprintf( format_string, argList );
            char* pbuf = new char[len + 1];
            if (pbuf != 0)
            {
                vsprintf_s( pbuf, len + 1, format_string, argList );
                *this = pbuf;
                delete[] pbuf;
            }
#else
            const int INLINE_FORMAT_BUFFER_LEN =2048;
            char* buf = new char[INLINE_FORMAT_BUFFER_LEN + 1];
            if (buf != 0)
            {
                int len  =vsnprintf(buf,INLINE_FORMAT_BUFFER_LEN, format_string, argList);
                assign(buf,buf+len);
                delete[] buf;
            }
#endif
            va_end( argList );
            return *this;
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::trim_left
			Access:		public 
			Qualifier:	
			Parameter:	value_type ch
			Returns:	string_ref&
			
			Purpose:	delete all char which is ch at the left of the string
		*******************************************************************************/
		string_ref& trim_left(value_type ch = ' ')
		{
			size_type off = this->find_first_not_of(ch);
			if (off != string::npos)
			{
				this->erase(0, off);
			}
			return *this;
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::trim_right
			Access:		public 
			Qualifier:	
			Parameter:	value_type ch
			Returns:	string_ref&
			
			Purpose:	delete all char which is ch at the right of the string
		*******************************************************************************/
		string_ref& trim_right(value_type ch = ' ')
		{
			size_type off = this->find_last_not_of(ch);
			if (off == string::npos)
			{
				off = 0;
			}
			else
			{
				off++;
			}
			this->erase(off, length() - off);
			return *this;
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::trim
			Access:		public 
			Qualifier:	
			Parameter:	value_type ch
			Returns:	string_ref&
			
			Purpose:	delete all char which is ch at the left and right of the string
		*******************************************************************************/
		string_ref& trim(value_type ch = ' ')
		{
			trim_left(ch);
			trim_right(ch);
			return *this;
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::float_num
			Access:		public static 
			Qualifier:	
			Parameter:	const string & str
			Returns:	double
			
			Purpose:	parse str to a float number
		*******************************************************************************/
		static double float_num(const string& str)
		{
			return atof(str.c_str());
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::float_num
			Access:		public static 
			Qualifier:	
			Returns:	double
			
			Purpose:	parse this to a float number
		*******************************************************************************/
		double float_num() const
		{
			return float_num(*this);
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::compare_nocase
			Access:		public 
			Qualifier:	const
			Parameter:	const string & str
			Returns:	int
			
			Purpose:	compare string no case
		*******************************************************************************/
		int compare_nocase(const string& str) const
		{
#ifdef _WIN32
			return _stricmp(this->c_str(), str.c_str());
#else
		    return strcasecmp(this->c_str(), str.c_str());
#endif
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::compare_nocase
			Access:		public 
			Qualifier:	const
			Parameter:	size_type index
			Parameter:	size_type length
			Parameter:	const string & str
			Returns:	int
			
			Purpose:	compare substring no case
		*******************************************************************************/
		int compare_nocase( size_type index, size_type length, const string &str ) const
		{
			string_ref temp = this->substr(index, length);
			return temp.compare_nocase(str);
		}

		/*******************************************************************************
			Function:	hl_lib::string_ref::compare_nocase
			Access:		public 
			Qualifier:	const
			Parameter:	size_type index
			Parameter:	size_type length
			Parameter:	const string & str
			Parameter:	size_type index2
			Parameter:	size_type length2
			Returns:	int
			
			Purpose:	compare two substring no case
		*******************************************************************************/
		int compare_nocase( size_type index, size_type length, const string &str, size_type index2,	size_type length2) const
		{
			string_ref temp1 = this->substr(index, length);
			string_ref temp2 = str.substr(index2, length2);
			return temp1.compare_nocase(temp2);
		}

	};

}

