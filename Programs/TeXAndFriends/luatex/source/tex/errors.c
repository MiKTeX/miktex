/*

errors.w

Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>

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

#include "ptexlib.h"
#define edit_var "TEXEDIT"

/*tex

When something anomalous is detected, \TeX\ typically does something like this:
$$\vbox{\halign{#\hfil\cr |print_err("Something anomalous has been
detected");|\cr |help3("This is the first line of my offer to help.")|\cr |("This
is the second line. I'm trying to")|\cr |("explain the best way for you to
proceed.");|\cr |error;|\cr}}$$ A two-line help message would be given using
|help2|, etc.; these informal helps should use simple vocabulary that complements
the words used in the official error message that was printed. (Outside the
U.S.A., the help messages should preferably be translated into the local
vernacular. Each line of help is at most 60 characters long, in the present
implementation, so that |max_print_line| will not be exceeded.)

The |print_err| procedure supplies a `\.!' before the official message, and makes
sure that the terminal is awake if a stop is going to occur. The |error|
procedure supplies a `\..' after the official message, then it shows the location
of the error; and if |interaction=error_stop_mode|, it also enters into a dialog
with the user, during which time the help message may be printed. @^system
dependencies@>

*/

/*tex The current level of interaction: */

int interaction;

/*tex Set from the command line: */

int interactionoption;

char *last_error = NULL;
char *last_lua_error = NULL;
char *last_warning_tag = NULL;
char *last_warning_str = NULL;
char *last_error_context = NULL;

int err_old_setting = 0 ;
int in_error = 0 ;

void set_last_error_context(void)
{
    str_number str;
    int sel = selector;
    int saved_new_line_char;
    int saved_new_string_line;
    selector = new_string;
    saved_new_line_char = new_line_char_par;
    saved_new_string_line = new_string_line;
    new_line_char_par = 10;
    new_string_line = 10;
    show_context();
    xfree(last_error_context);
    str = make_string();
    last_error_context = makecstring(str);
    flush_str(str);
    selector = sel;
    new_line_char_par = saved_new_line_char;
    new_string_line = saved_new_string_line;
    return;
}

void flush_err(void)
{
    str_number s_error;
    char *s = NULL;
    int callback_id ;
    if (in_error) {
        selector = err_old_setting;
        str_room(1);
        s_error = make_string();
        s = makecstring(s_error);
        flush_str(s_error);
        if (interaction == error_stop_mode) {
            wake_up_terminal();
        }
        xfree(last_error);
        last_error = (string) xmalloc((unsigned) (strlen(s) + 1));
        strcpy(last_error,s);
        callback_id = callback_defined(show_error_message_callback);
        if (callback_id > 0) {
            run_callback(callback_id, "->");
        } else {
            tprint(s);
        }
        in_error = 0 ;
    }
}

void print_err(const char *s)
{
    int callback_id = callback_defined(show_error_message_callback);
    if (interaction == error_stop_mode) {
        wake_up_terminal();
    }
    if (callback_id > 0) {
        err_old_setting = selector;
        selector = new_string;
        in_error = 1 ;
    }
    if (filelineerrorstylep) {
        print_file_line();
    } else {
        tprint_nl("! ");
    }
    tprint(s);
    if (callback_id <= 0) {
        xfree(last_error);
        last_error = (string) xmalloc((unsigned) (strlen(s) + 1));
        strcpy(last_error,s);
    }
}

/*tex

\TeX\ is careful not to call |error| when the print |selector| setting might be
unusual. The only possible values of |selector| at the time of error messages are

|no_print| (when |interaction=batch_mode| and |log_file| not yet open);

|term_only| (when |interaction>batch_mode| and |log_file| not yet open);

|log_only| (when |interaction=batch_mode| and |log_file| is open);

|term_and_log| (when |interaction>batch_mode| and |log_file| is open).

*/

void fixup_selector(boolean logopened)
{
    if (interaction == batch_mode)
        selector = no_print;
    else
        selector = term_only;
    if (logopened)
        selector = selector + 2;
}

/*tex

A global variable |deletions_allowed| is set |false| if the |get_next| routine is
active when |error| is called; this ensures that |get_next| and related routines
like |get_token| will never be called recursively. A similar interlock is
provided by |set_box_allowed|. @^recursion@>

The global variable |history| records the worst level of error that has been
detected. It has four possible values: |spotless|, |warning_issued|,
|error_message_issued|, and |fatal_error_stop|.

Another global variable, |error_count|, is increased by one when an |error|
occurs without an interactive dialog, and it is reset to zero at the end of every
paragraph. If |error_count| reaches 100, \TeX\ decides that there is no point in
continuing further.

*/

/*tex Is it safe for |error| to call |get_token|? */

boolean deletions_allowed;

/*tex Is it safe to do a \.{\\setbox} assignment? */

boolean set_box_allowed;
/*tex Has the source input been clean so far? */

int history;

/*tex The number of scrolled errors since the last paragraph ended. */

int error_count;

/*tex Should \TeX\ pause for instructions? */

int interrupt;

/*tex Should interrupts be observed? */

boolean OK_to_interrupt;

/*tex

The value of |history| is initially |fatal_error_stop|, but it will be changed to
|spotless| if \TeX\ survives the initialization process.

*/

void initialize_errors(void)
{
    if (interactionoption == unspecified_mode)
        interaction = error_stop_mode;
    else
        interaction = interactionoption;
    deletions_allowed = true;
    set_box_allowed = true;
    OK_to_interrupt = true;
}

/*tex

It is possible for |error| to be called recursively if some error arises when
|get_token| is being used to delete a token, and/or if some fatal error occurs
while \TeX\ is trying to fix a non-fatal one. But such recursion @^recursion@> is
never more than two levels deep.

Individual lines of help are recorded in the array |help_line|.

*/

const char *help_line[7];

/*tex
    Should the |err_help| list be shown?
*/

boolean use_err_help;

/*tex

The |jump_out| procedure just cuts across all active procedure levels and exits
the program. It is used when there is no recovery from a particular error. The
exit code can be overloaded.

*/

int defaultexitcode = 0;

__attribute__ ((noreturn))
void do_final_end(void)
{
    update_terminal();
    ready_already = 0;
    lua_close(Luas);
    if ((history != spotless) && (history != warning_issued))
        uexit(1);
    else
        uexit(defaultexitcode);
}

__attribute__ ((noreturn))
void jump_out(void)
{
    close_files_and_terminate();
    do_final_end();
}

/*tex

Here is the function that calls the editor, if one is defined. This is loosely
based on a similar function in kpathsea, but the calling convention is quite
different.

*/

static const_string edit_value = EDITOR;

#if defined(WIN32)

static int Isspace (char c)
{
    return (c == ' ' || c == '\t');
}

#endif /* WIN32 */

__attribute__ ((noreturn))
static void luatex_calledit (int baseptr, int linenumber)
{
#if defined(MIKTEX)
  close_files_and_terminate();
  miktex_invoke_editor(makecstring(input_stack[base_ptr].name_field), linenumber);
  do_final_end();
#else
    char *temp, *command;
    char *fullcmd = NULL; /* avoid compiler warning */
    char c;
    int sdone, ddone, i;
    char *filename = makecstring(input_stack[base_ptr].name_field);
    int fnlength = strlen(filename);
#ifdef WIN32
    char *fp, *ffp, *env, editorname[256], buffer[256];
    int cnt = 0;
    int dontchange = 0;
#endif
    sdone = ddone = 0;
    /*tex
        Close any open input files, since we're going to kill the job.
    */
    close_files_and_terminate();
    /*tex
        Replace the default with the value of the appropriate environment
        variable or config file value, if it's set.
    */
    temp = kpse_var_value (edit_var);
    if (temp != NULL)
        edit_value = temp;
    /*tex
        Construct the command string.  The `11' is the maximum length an
        integer might be.
    */
    command = xmalloc (strlen (edit_value) + fnlength + 11);
    /*tex
        So we can construct it as we go.
    */
    temp = command;
#ifdef WIN32
    fp = editorname;
    if ((isalpha(*edit_value) && *(edit_value + 1) == ':' && IS_DIR_SEP (*(edit_value + 2)))
            || (*edit_value == '"' && isalpha(*(edit_value + 1))
            && *(edit_value + 2) == ':' && IS_DIR_SEP (*(edit_value + 3)))) {
        dontchange = 1;
    }
#endif
    while ((c = *edit_value++) != 0) {
        if (c == '%') {
            switch (c = *edit_value++) {
                case 'd':
                    if (ddone)
                        FATAL1 ("call_edit: `%%d' appears twice in editor command: `%s'", edit_value);
                    sprintf (temp, "%ld", (long int)linenumber);
                    while (*temp != '\0')
                        temp++;
                    ddone = 1;
                    break;
                case 's':
                    if (sdone)
                        FATAL1 ("call_edit: `%%s' appears twice in editor command: `%s'", edit_value);
                    for (i =0; i < fnlength; i++)
                        *temp++ = filename[i];
                    sdone = 1;
                    break;
                case '\0':
                    *temp++ = '%';
                    /*tex
                        Back up to the null to force termination.
                    */
                    edit_value--;
                    break;
                default:
                    *temp++ = '%';
                    *temp++ = c;
                    break;
            }
        } else {
#ifdef WIN32
            if (dontchange) {
                *temp++ = c;
            } else if(Isspace(c) && cnt == 0) {
                cnt++;
                temp = command;
                *temp++ = c;
                *fp = '\0';
            } else if(!Isspace(c) && cnt == 0) {
                *fp++ = c;
            } else {
                *temp++ = c;
            }
#else
            *temp++ = c;
#endif
        }
    }
    *temp = 0;
#ifdef WIN32
    if (dontchange == 0) {
        if(editorname[0] == '.' || editorname[0] == '/' || editorname[0] == '\\') {
            fprintf(stderr, "%s is not allowed to execute.\n", editorname);
            do_final_end();
        }
        env = (char *)getenv("PATH");
        if(SearchPath(env, editorname, ".exe", 256, buffer, &ffp)==0) {
            if(SearchPath(env, editorname, ".bat", 256, buffer, &ffp)==0) {
                fprintf(stderr, "I cannot find %s in the PATH.\n", editorname);
                do_final_end();
            }
        }
        fullcmd = (char *)xmalloc(strlen(buffer)+strlen(command)+5);
        strcpy(fullcmd, "\"");
        strcat(fullcmd, buffer);
        strcat(fullcmd, "\"");
        strcat(fullcmd, command);
    }
#else
    fullcmd = command;
#endif
    /*tex Execute the command. */
    if (system (fullcmd) != 0) {
        fprintf (stderr, "! Trouble executing `%s'.\n", command);
    }
    /*tex Quit, since we found an error.  */
    do_final_end ();
#endif
}

/*tex

  This completes the job of error reporting.

*/

void error(void)
{
    /*tex What the user types :*/
    ASCII_code c;
    int callback_id;
    /*tex Used to save global variables when deleting tokens: */
    int s1, s2, s3, s4;
    int i;
    flush_err();
    if (history < error_message_issued)
        history = error_message_issued;
    callback_id = callback_defined(show_error_hook_callback);
    if (callback_id > 0) {
        set_last_error_context();
        run_callback(callback_id, "->");
    } else {
        print_char('.');
        show_context();
    }
    if (haltonerrorp) {
        /*tex If |close_files_and_terminate| generates an error, we'll end up back
          here; just give up in that case. If files are truncated, too bad. */
        if (haltingonerrorp) {
          /*tex  quit immediately */
          do_final_end ();
        }
        haltingonerrorp=true;
        history = fatal_error_stop;
        jump_out();
    }
    if (interaction == error_stop_mode) {
        /*tex Get user's advice and |return|. */
        while (1) {
          CONTINUE:
            /*tex
             Original reports:

             https://tex.stackexchange.com/questions/551313/
             https://tug.org/pipermail/tex-live/2020-June/045876.html

            This will probably be fixed by DEK in the 2021 tuneup in a different
            way (so we'll have to remove or alter this change), but the interaction
            sequence in the reports above causes a segmentation fault in web2c -
            writing to the closed \write15 stream because we wrongly decrement
            selector from 16 to 15 in term_input, due to the lack of this check in
            recursive error() call.
            */
            if (interaction !=error_stop_mode)
                return;
            clear_for_error_prompt();
            prompt_input("? ");
            if (last == first)
                return;
            c = buffer[first];
            if (c >= 'a')
                c = c + 'A' - 'a';
                /*tex
                    Interpret code |c| and |return| if done. It is desirable to
                    provide an `\.E' option here that gives the user an easy way
                    to return from \TeX\ to the system editor, with the offending
                    line ready to be edited. But such an extension requires some
                    system wizardry, so the present implementation simply types
                    out the name of the file that should be edited and the
                    relevant line number.
                */
            switch (c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (deletions_allowed) {
                    /*tex
                        Delete |c-"0"| tokens and |goto continue|. We allow
                        deletion of up to 99 tokens at a time.
                    */
                    s1 = cur_tok;
                    s2 = cur_cmd;
                    s3 = cur_chr;
                    s4 = align_state;
                    align_state = 1000000;
                    OK_to_interrupt = false;
                    if ((last > first + 1) && (buffer[first + 1] >= '0')
                        && (buffer[first + 1] <= '9'))
                        c = c * 10 + buffer[first + 1] - '0' * 11;
                    else
                        c = c - '0';
                    while (c > 0) {
                        /*tex One-level recursive call of |error| is possible. */
                        get_token();
                        decr(c);
                    }
                    cur_tok = s1;
                    cur_cmd = s2;
                    cur_chr = s3;
                    align_state = s4;
                    OK_to_interrupt = true;
                    help2(
                        "I have just deleted some text, as you asked.",
                        "You can now delete more, or insert, or whatever."
                    );
                    show_context();
                    goto CONTINUE;
                }
                break;
            case 'E':
                if (base_ptr > 0) {
                    int callback_id = callback_defined(call_edit_callback);
                    if (callback_id>0) {
                        (void)run_callback(callback_id, "Sd->", makecstring(input_stack[base_ptr].name_field), line);
                        /*tex This should not be reached. */
                        jump_out();
                    } else {
                        tprint_nl("You want to edit file ");
                        print(input_stack[base_ptr].name_field);
                        tprint(" at line ");
                        print_int(line);
                        interaction = scroll_mode;
                        if (kpse_init) {
                            luatex_calledit(base_ptr, line);
                        } else {
                            /*tex This should not be reached. */
                            tprint_nl("There is no valid callback defined.");
                            jump_out();
                        }
                    }
                }
                break;
            case 'H':
                /*tex Print the help information and |goto continue| */
                if (use_err_help) {
                    give_err_help();
                } else {
                    if (help_line[0] == NULL) {
                        help2(
                            "Sorry, I don't know how to help in this situation.",
                            "Maybe you should try asking a human?"
                        );
                    }
                    i = 0;
                    while (help_line[i] != NULL)
                        tprint_nl(help_line[i++]);
                    help4(
                        "Sorry, I already gave what help I could...",
                        "Maybe you should try asking a human?",
                        "An error might have occurred before I noticed any problems.",
                        "``If all else fails, read the instructions.''"
                    );
                    goto CONTINUE;
                }
                break;
            case 'I':
                /*tex

                    Introduce new material from the terminal and |return|. When
                    the following code is executed, |buffer[(first+1)..(last-1)]|
                    may contain the material inserted by the user; otherwise
                    another prompt will be given. In order to understand this
                    part of the program fully, you need to be familiar with
                    \TeX's input stacks.

                    We enter a new syntactic level for terminal input:

                */
                begin_file_reading();
                /*tex
                    Now |state=mid_line|, so an initial blank space will count as
                    a blank.
                */
                if (last > first + 1) {
                    iloc = first + 1;
                    buffer[first] = ' ';
                } else {
                    prompt_input("insert>");
                    iloc = first;
                }
                first = last;
                /*tex No |end_line_char| ends this line. */
                ilimit = last - 1;
                return;
                break;
            case 'Q':
            case 'R':
            case 'S':
                /*tex

                    Change the interaction level and |return|. Here the author of
                    \TeX\ apologizes for making use of the numerical relation
                    between |"Q"|, |"R"|, |"S"|, and the desired interaction
                    settings |batch_mode|, |nonstop_mode|, |scroll_mode|.

                */
                error_count = 0;
                interaction = batch_mode + c - 'Q';
                tprint("OK, entering ");
                switch (c) {
                case 'Q':
                    tprint_esc("batchmode");
                    decr(selector);
                    break;
                case 'R':
                    tprint_esc("nonstopmode");
                    break;
                case 'S':
                    tprint_esc("scrollmode");
                    break;
                }
                tprint("...");
                print_ln();
                update_terminal();
                return;
                break;
            case 'X':
                interaction = scroll_mode;
                jump_out();
                break;
            default:
                break;
            }
            if (!use_err_help) {
                /* Print the menu of available options */
                tprint("Type <return> to proceed, S to scroll future error messages,");
                tprint_nl("R to run without stopping, Q to run quietly,");
                tprint_nl("I to insert something, ");
                if (base_ptr > 0)
                    tprint("E to edit your file,");
                if (deletions_allowed)
                    tprint_nl("1 or ... or 9 to ignore the next 1 to 9 tokens of input,");
                tprint_nl("H for help, X to quit.");
            }
            use_err_help = false;
        }

    }
    incr(error_count);
    if (error_count == 100) {
        tprint_nl("(That makes 100 errors; please try again.)");
        history = fatal_error_stop;
        jump_out();
    }
    /*tex Put help message on the transcript file. */
    if (interaction > batch_mode) {
        /*tex Avoid terminal output: */
        decr(selector);
    }
    if (use_err_help) {
        print_ln();
        give_err_help();
    } else {
        int i1 = 0;
        while (help_line[i1] != NULL)
            tprint_nl(help_line[i1++]);
    }
    print_ln();
    if (interaction > batch_mode) {
        /*tex Re-enable terminal output: */
        incr(selector);
    }
    print_ln();
}

/*tex

A dozen or so error messages end with a parenthesized integer, so we save a teeny
bit of program space by declaring the following procedure:

*/

void int_error(int n)
{
    tprint(" (");
    print_int(n);
    print_char(')');
    error();
}

/*tex

In anomalous cases, the print selector might be in an unknown state; the
following subroutine is called to fix things just enough to keep running a bit
longer.

*/

void normalize_selector(void)
{
    if (log_opened_global)
        selector = term_and_log;
    else
        selector = term_only;
    if (job_name == 0)
        open_log_file();
    if (interaction == batch_mode)
        decr(selector);
}

/*tex

The following procedure prints \TeX's last words before dying.

*/

void succumb(void)
{
    if (interaction == error_stop_mode) {
        /*tex No more interaction: */
        interaction = scroll_mode;
    }
    if (log_opened_global) {
        error();
    }
    history = fatal_error_stop;
    /*tex Irrecoverable error: */
    jump_out();
}

/*tex

This prints |s|, and that's it.

*/

void fatal_error(const char *s)
{
    normalize_selector();
    print_err("Emergency stop");
    help1(s);
    succumb();
}

/*tex

Here is the most dreaded error message. We stop due to finiteness.

*/

void overflow(const char *s, unsigned int n)
{
    normalize_selector();
    print_err("TeX capacity exceeded, sorry [");
    tprint(s);
    print_char('=');
    print_int((int) n);
    print_char(']');
    if (varmem == NULL) {
      print_err("Sorry, I ran out of memory.");
      print_ln();
      exit(EXIT_FAILURE);
    }
    help2(
        "If you really absolutely need more capacity,",
        "you can ask a wizard to enlarge me."
    );
    succumb();
}

/*tex

The program might sometime run completely amok, at which point there is no choice
but to stop. If no previous error has been detected, that's bad news; a message
is printed that is really intended for the \TeX\ maintenance person instead of
the user (unless the user has been particularly diabolical). The index entries
for `this can't happen' may help to pinpoint the problem. @^dry rot@>

*/

void confusion(const char *s)
{                               /* consistency check violated; |s| tells where */
    normalize_selector();
    if (history < error_message_issued) {
        print_err("This can't happen (");
        tprint(s);
        print_char(')');
        help1(
            "I'm broken. Please show this to someone who can fix"
        );
    } else {
        print_err("I can't go on meeting you like this");
        help2(
            "One of your faux pas seems to have wounded me deeply...",
            "in fact, I'm barely conscious. Please fix it and try again."
        );
    }
    succumb();
}

/*tex

Users occasionally want to interrupt \TeX\ while it's running. If the runtime
system allows this, one can implement a routine that sets the global variable
|interrupt| to some nonzero value when such an interrupt is signalled. Otherwise
there is probably at least a way to make |interrupt| nonzero using the debugger.
@^system dependencies@> @^debugging@>

*/

void check_interrupt(void)
{
    if (interrupt != 0)
        pause_for_instructions();
}

/*tex

When an interrupt has been detected, the program goes into its highest
interaction level and lets the user have nearly the full flexibility of the
|error| routine. \TeX\ checks for interrupts only at times when it is safe to do
this.

*/

void pause_for_instructions(void)
{
    if (OK_to_interrupt) {
        interaction = error_stop_mode;
        if ((selector == log_only) || (selector == no_print))
            incr(selector);
        print_err("Interruption");
        help3(
            "You rang?",
            "Try to insert some instructions for me (e.g.,`I\\showlists'),",
            "unless you just want to quit by typing `X'."
        );
        deletions_allowed = false;
        error();
        deletions_allowed = true;
        interrupt = 0;
    }
}

void tex_error(const char *msg, const char **hlp)
{
    print_err(msg);
    if (hlp != NULL) {
        int i;
        for (i = 0; (hlp[i] != NULL && i <= 5); i++) {
            help_line[i] = hlp[i];
        }
        help_line[i] = NULL;
    } else {
        help_line[0] = NULL;
    }
    error();
}

/*tex

The |back_error| routine is used when we want to replace an offending token just
before issuing an error message. This routine, like |back_input|, requires that
|cur_tok| has been set. We disable interrupts during the call of |back_input| so
that the help message won't be lost.

*/

void back_error(void)
{
    OK_to_interrupt = false;
    back_input();
    OK_to_interrupt = true;
    error();
}

/*tex

    Back up one inserted token and call |error|.
*/

void ins_error(void)
{
    OK_to_interrupt = false;
    back_input();
    token_type = inserted;
    OK_to_interrupt = true;
    error();
}

/*tex

When \TeX\ wants to typeset a character that doesn't exist, the character node is
not created; thus the output routine can assume that characters exist when it
sees them. The following procedure prints a warning message unless the user has
suppressed it.
If |tracing_lost_chars_par| (i.e. \.{\\tracinglostchar})  is  greater than 2,
it's considered as an error.

*/

void char_warning(internal_font_number f, int c)
{
    /*tex saved value of |tracing_online| */
    int old_setting;
    /* index to current digit; we assume that $0\L n<16^{22}$ */
    int k;
    if (tracing_lost_chars_par > 0) {
        old_setting = tracing_online_par;
        if (tracing_lost_chars_par > 1)
            tracing_online_par = 1;
        begin_diagnostic();
        tprint_nl("Missing character: There is no ");
        print(c);
        tprint(" (U+");
        k = 0;
        if (c < 16)
            print_char('0');
        if (c < 256)
            print_char('0');
        if (c < 4096)
            print_char('0');
        do {
            dig[k] = c % 16;
            c = c / 16;
            incr(k);
        } while (c != 0);
        print_the_digs((eight_bits) k);
        tprint(") in font ");
        print_font_name(f);
        print_char('!');
        end_diagnostic(false);
        tracing_online_par = old_setting;
    }
    if (tracing_lost_chars_par > 2) {
       error();
    }
}

void wrapup_backend(void) {
    ensure_output_state(static_pdf, ST_OMODE_FIX);
    if (output_mode_used == OMODE_NONE) {
        print_err(" ==> Fatal error occurred, no FMT file produced!");
    } else {
        backend_out_control[backend_control_finish_file](static_pdf,history == fatal_error_stop);
    }
}

void normal_error(const char *t, const char *p)
{
    normalize_selector();
    if (interaction == error_stop_mode) {
        wake_up_terminal();
    }
    if (filelineerrorstylep) {
        print_file_line();
    } else {
        tprint_nl("! ");
    }
    tprint("error: ");
    if (cur_file_name) {
        tprint(" (file ");
        tprint(cur_file_name);
        tprint(")");
    }
    if (t != NULL) {
        tprint(" (");
        tprint(t);
        tprint(")");
    }
    tprint(": ");
    if (p != NULL)
        tprint(p);
    history = fatal_error_stop;
    wrapup_backend();
    exit(EXIT_FAILURE);
}

void normal_warning(const char *t, const char *p)
{
    int report_id ;
    if (strcmp(t,"lua") == 0) {
        int saved_new_line_char;
        saved_new_line_char = new_line_char_par;
        new_line_char_par = 10;
        report_id = callback_defined(show_lua_error_hook_callback);
        if (report_id == 0) {
	    if (p != NULL)
	      tprint(p);
            help2(
                "The lua interpreter ran into a problem, so the",
                "remainder of this lua chunk will be ignored."
            );
        } else {
            (void) run_callback(report_id, "->");
        }
        error();
        new_line_char_par = saved_new_line_char;
    } else {
        report_id = callback_defined(show_warning_message_callback);
        if (report_id > 0) {
            /*tex Free the last ones, */
            xfree(last_warning_str);
            xfree(last_warning_tag);
            last_warning_str = (string) xmalloc(strlen(p) + 1);
            last_warning_tag = (string) xmalloc(strlen(t) + 1);
            strcpy(last_warning_str,p);
            strcpy(last_warning_tag,t);
            run_callback(report_id, "->");
        } else {
            print_ln();
            tprint("warning ");
            if (cur_file_name) {
                tprint(" (file ");
                tprint(cur_file_name);
                tprint(")");
            }
            if (t != NULL) {
                tprint(" (");
                tprint(t);
                tprint(")");
            }
            tprint(": ");
            if (p != NULL)
                tprint(p);
            print_ln();
        }
        if (history == spotless)
            history = warning_issued;
    }
}

static char print_buf[PRINTF_BUF_SIZE];

__attribute__ ((format(printf, 2,3)))
void formatted_error(const char *t, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(print_buf, PRINTF_BUF_SIZE, fmt, args);
    normal_error(t,print_buf);
    va_end(args);
}

__attribute__ ((format(printf, 2,3)))
void formatted_warning(const char *t, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(print_buf, PRINTF_BUF_SIZE, fmt, args);
    normal_warning(t,print_buf);
    va_end(args);
}
