/**
 * @file output.cpp
 * @author Christian Schenk
 * @brief C4P output routines 
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is part of C4P.
 *
 * C4P is licensed under GNU General Public License version 2 or any later
 * version.
 */

class output
{
private:
    void flush_buf();

public:
    unsigned get_buf_mark()
    {
        return buf_ptr;
    }

public:
    void get_buf_text(std::string& str, unsigned buf_mark, unsigned count);

public:
    void out_buf_over(unsigned buf_mark, const char* s, unsigned count);

public:
    void open_file(unsigned filnum, const char* file_name);

public:
    void close_file(unsigned filnum);

public:
    void reopen_file(unsigned filnum, const char* file_name);

private:
    void put_char(int c);

private:
    void out_char(int c);

public:
    void out_stop()
    {
        halting = true;
    }

public:
    void out_resume()
    {
        halting = false;
    }

public:
    void out_s(const std::string& s);

public:
    void redir_file(int filnum);

public:
    void translate_char(int c);

public:
    void translate_string(const char* s);

private:
    bool halting;

private:
    FILE* file_table[3];

private:
    FILE* cur_file;

private:
    int cur_file_num;

private:
    char out_buf[BUF_SIZE];

private:
    unsigned buf_ptr;

private:
    unsigned chars_in_buf;

private:
    int last_char;
};

extern output cppout;
