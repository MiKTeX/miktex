#pragma once
#include <mutex>
#include <string>
namespace lsp
{
	class stream
	{
	public:
		virtual ~stream() = default;
		virtual  bool fail() = 0;
		virtual  bool bad() = 0;
		virtual  bool eof() = 0;
		virtual  bool good() = 0;
		virtual  void clear() = 0;
		virtual  std::string  what() = 0;
		virtual  bool need_to_clear_the_state()
		{
			return false;
		}

		bool  operator!()
		{
			return bad();
		}
	};
	class istream : public  stream
	{
	public:
		virtual  int get() = 0;
		virtual ~istream() = default;
		virtual  istream& read(char* str, std::streamsize count) = 0;
	};
	template <class T >
	class base_istream : public istream
	{
	public:
		explicit  base_istream(T& _t) :_impl(_t)
		{

		}

		int get() override
		{
			return  _impl.get();
		}
		bool fail() override
		{
			return  _impl.fail();
		}
		bool bad() override
		{
			return  _impl.bad();
		}
		bool eof() override
		{
			return  _impl.eof();
		}
		bool good() override
		{
			return  _impl.good();
		}
		istream& read(char* str, std::streamsize count) override
		{
			_impl.read(str, count);
			return *this;
		}

		void clear() override
		{
			_impl.clear();
		}
		T& _impl;
	};
	class ostream : public  stream
	{
	public:
		virtual ~ostream() = default;

		virtual  ostream& write(const std::string&) = 0;
		virtual  ostream& write(std::streamsize) = 0;
		virtual  ostream& flush() = 0;

	};
	template <class T >
	class base_ostream : public ostream
	{
	public:
		explicit  base_ostream(T& _t) :_impl(_t)
		{

		}

		bool fail() override
		{
			return  _impl.fail();
		}
		bool good() override
		{
			return  _impl.good();
		}
		bool bad() override
		{
			return  _impl.bad();
		}
		bool eof() override
		{
			return  _impl.eof();
		}

		ostream& write(const std::string& c) override
		{
			_impl << c;
			return *this;
		}

		ostream& write(std::streamsize _s) override
		{

			_impl << std::to_string(_s);
			return *this;
		}

		ostream& flush() override
		{
			_impl.flush();
			return *this;
		}

		void clear() override
		{
			_impl.clear();
		}
	protected:
		T& _impl;
	};

	template <class T >
	class base_iostream : public istream, public ostream
	{
	public:
		explicit  base_iostream(T& _t) :_impl(_t)
		{

		}

		int get() override
		{
			return  _impl.get();
		}
		bool fail() override
		{
			return  _impl.fail();
		}
		bool bad() override
		{
			return  _impl.bad();
		}
		bool eof() override
		{
			return  _impl.eof();
		}
		bool good() override
		{
			return  _impl.good();
		}
		istream& read(char* str, std::streamsize count) override
		{
			_impl.read(str, count);
			return *this;
		}
		ostream& write(const std::string& c) override
		{
			_impl << c;
			return *this;
		}

		ostream& write(std::streamsize _s) override
		{
			_impl << std::to_string(_s);
			return *this;
		}

		ostream& flush() override
		{
			_impl.flush();
			return *this;
		}

		void clear() override
		{
			_impl.clear();
		}
	protected:
		T& _impl;
	};
}
