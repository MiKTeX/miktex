/**
 * @file gram.y
 * @author Christian Schenk
 * @brief Parser specification
 *
 * @copyright Copyright © 1991-2022 Christian Schenk
 *
 * This file is part of C4P.
 *
 * C4P is licensed under GNU General Public License version 2 or any later
 * version.
 */

%{
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "output.h"

unsigned block_level;
symbol_t* prog_symbol;

namespace
{
    pascal_type last_type;
    void* last_type_ptr;
    value_t last_value;
    array_node* last_array_node;
    std::stack<array_node*> array_type_stack;
    std::stack<parameter_node*> parameter_node_stack;
    declarator_node* last_declarator;
    parameter_node* last_parameter;
    bool by_reference;
    record_section_node* last_record_section;
    variant_node* last_variant;
    std::string file_spec;
    bool file_specified;
    pascal_type file_type;
    size_t buf_mark;
    size_t buf_mark2;
    bool to_eoln;
    unsigned nglobalvars;
    unsigned routine_handle;
}

#define push_array_type(arr)                 \
    array_type_stack.push (last_array_node); \
    last_array_node = arr

#define pop_array_type()                      \
    last_array_node = array_type_stack.top(); \
    array_type_stack.pop ()

#define push_parameter_node(par)    \
    parameter_node_stack.push (par)

#define pop_parameter_node(par)       \
    par = parameter_node_stack.top(); \
    parameter_node_stack.pop ()

#define get_array_offset(arr) \
    arr->lower_bound
%}

%union {
    symbol_t* sym;
    C4P_integer value;
    pascal_type type;
    C4P_real fvalue;
    void* type_ptr;
    unsigned buf_mark;
}

%token AND ARRAY BEGIn CASE CONST DIV DO DOWNTO ELSE END FILe FOR FORWARD
    FUNCTION GOTO IF IN LABEL MOD NIL NOT OF OR PACKED PROCEDURE PROGRAM
    RECORD REPEAT SET THEN TO TYPE UNTIL VAR WHILE WITH
    OTHERS

%token DOTDOT NEQ GEQ LEQ ASSIGN

/* pseudo tokens */
%token PASCAL_KEYWORD TYPE_IDENTIFIER FIELD_IDENTIFIER
    C_RESERVED CONSTANT_IDENTIFIER FUNCTION_IDENTIFIER
    PROCEDURE_IDENTIFIER
    UNDEFINED_IDENTIFIER VARIABLE_IDENTIFIER PARAMETER_IDENTIFIER
    READ READLN WRITE WRITELN BUILD_IN_IDENTIFIER

%token <value> CHARACTER_STRING CHARACTER_CONSTANT UNSIGNED_NUMBER

%token <fvalue> REAL_CONSTANT LONG_REAL_CONSTANT

%token <sym> IDENTIFIER

%type <value> bound to_or_downto

%type <type> boolean_expression expression simple_expression term factor

%type <type_ptr> type_denoter type ordinal_type subrange_type pointer_type
    structured_type unpacked_structured_type array_type component_type
    index_type_list index_type record_type field_list fixed_part
    record_section_list record_section variant_part variant_list variant
    variant_field_list parameter_declarator_list parameter_declarator
    variable_declarator_list variable_declarator field_declarator_list
    field_declarator formal_parameter_section parameter_group
    formal_parameter_section_list function_head procedure_head
    optional_formal_parameter_section_list file_type
    program_parameter_list program_parameter

%expect 1

%start program

%%

program:
    program_heading ';' program_block '.'
    ;

program_heading:
    PROGRAM IDENTIFIER
    {
        prog_symbol = $2;
        open_name_file();
        open_header_file();
        if (!def_filename.empty())
        {
            open_def_file();
        }
    }
    optional_program_parameters
    ;

optional_program_parameters:
    /* empty */
    | '(' program_parameter_list ')'
    ;

program_block:
    {
        cppout.redir_file(H_FILE_NUM);
    }
    label_declaration_part
    {
        if (false && !def_filename.empty())
        {
            cppout.redir_file(DEF_FILE_NUM);
        }
    }
    constant_definition_part
    {
        if (false && !def_filename.empty())
        {
            cppout.redir_file(H_FILE_NUM);
        }
    }
    type_definition_part
    variable_declaration_part
    {
        if (nglobalvars > 0 && !var_struct_name.empty())
        {
            cppout.out_s("};\n");
            if (class_name.empty())
            {
                cppout.out_s("C4PEXTERN ");
            }
            cppout.out_s("struct " + var_struct_class_name + " " + var_struct_name + ";\n");
        }
        cppout.redir_file(C_FILE_NUM);
    }
    procedure_and_function_declaration_part
        {
            if (!one_c_file)
            {
                begin_new_c_file(prog_symbol->s_repr, 1);
            }
            routine_handle = 0;
            forget_fast_vars();
            if (class_name.empty())
            {
                if (dll_flag)
                {
                    cppout.redir_file(H_FILE_NUM);
                    cppout.out_s("\n#ifdef __cplusplus\nextern \"C++\" int C4PCEECALL " + entry_name + "(int argc, char* argv[]);\n#endif\n");
                    cppout.redir_file(C_FILE_NUM);
                    cppout.out_s("\nextern \"C++\" int C4PCEECALL " + entry_name + "(int argc, char* argv[])\n\n");
                }
                else
                {
                    cppout.out_s("\nvoid C4PCEECALL int main(int argc, const char* argv[])\n\n");
                }
            }
            else
            {
                cppout.redir_file(H_FILE_NUM);
                cppout.out_s("int Run(int argc, char* argv[]);\n");
                cppout.redir_file(C_FILE_NUM);
                cppout.out_s("\nint " + class_name + "::Run(int argc, char* argv[])\n\n");
            }
        }
    statement_part
        {
            if (n_fast_vars)
            {
                declare_fast_var_macro(routine_handle);
            }
            cppout.close_file(C_FILE_NUM);
            close_header_file();
            if (!def_filename.empty())
            {
                close_def_file();
            }
            close_name_file();
        }
    ;

program_parameter_list:
    program_parameter
    | program_parameter_list ',' program_parameter
    ;

program_parameter:
    IDENTIFIER
    {
    }
    ;

procedure_and_function_declaration_part:
    /* empty */
    | procedure_and_function_declarations
    ;

procedure_and_function_declarations:
    procedure_or_function_declaration
    | procedure_and_function_declarations procedure_or_function_declaration
    ;

procedure_or_function_declaration:
    procedure_declaration
    | function_declaration
    ;

function_declaration:
    function_head ';' FORWARD ';'
    {
    }
    | function_head ';'
    {
        routine_handle += 1;
        begin_routine(reinterpret_cast<prototype_node*>($1), routine_handle);
    }
    label_declaration_part
    constant_definition_part
    type_definition_part
    variable_declaration_part
    /* procedure_and_function_declaration_part (nested functions not implemented) */
    BEGIn
    {
        symbol_t* result_type = reinterpret_cast<prototype_node*>($<type_ptr>1)->result_type;
        cppout.out_s("\n" + std::string(result_type->s_translated_type != nullptr ? result_type->s_translated_type : result_type->s_repr) + " c4p_result;\n\n");
        cppout.out_s("C4P_PROC_ENTRY (" + std::to_string(routine_handle) + ");\n");
    }
    statement_sequence END ';'
    {
        cppout.out_s("\n");
        if (auto_exit_label_flag)
        {
            cppout.out_s("c4p_L" + std::to_string(auto_exit_label) + ":\n");
            auto_exit_label_flag = false;
        }
        cppout.out_s("C4P_PROC_EXIT (" + std::to_string(routine_handle) + ");\n");
        cppout.out_s("return (c4p_result);\n");
        end_routine(routine_handle);
    }
    ;

function_head:
    FUNCTION IDENTIFIER
    {
        if ($2->s_kind != FUNCTION_IDENTIFIER)
        {
            c4p_error("`%s' is not a function name", $2->s_repr);
        }
        $$ = $2->s_type_ptr;
    }
    | FUNCTION IDENTIFIER optional_formal_parameter_section_list ':' IDENTIFIER
    {
        if ($2->s_kind == FUNCTION_IDENTIFIER)
        {
            /* already declared (forward) */
            $$ = $2->s_type_ptr;
        }
        else
        {
            if ($2->s_kind != UNDEFINED_IDENTIFIER)
            {
                c4p_error("`%s' already defined", $2->s_repr);
            }
            if ($5->s_kind != TYPE_IDENTIFIER)
            {
                c4p_error("`%s' is not a type identifier", $5->s_repr);
            }
            $$ = new_type_node(PROTOTYPE_NODE, $2, $3, $5);
            $2->s_kind = FUNCTION_IDENTIFIER;
            $2->s_type = PROTOTYPE_NODE;
            $2->s_type_ptr = $$;
        }
    }
    ;

optional_formal_parameter_section_list:
    /* empty */
    {
        $$ = nullptr;
    }
    | '('
    {
        mark_symbol_table();
    }
    formal_parameter_section_list ')'
    {
        unmark_symbol_table();
        $$ = $3;
    }
    ;

procedure_declaration:
    procedure_head ';' FORWARD ';'
    {
    }
    | procedure_head ';'
    {
        routine_handle += 1;
        begin_routine(reinterpret_cast<prototype_node*>($1), routine_handle);
    }
    label_declaration_part
    constant_definition_part
    type_definition_part
    variable_declaration_part
    BEGIn
    {
        cppout.out_s("C4P_PROC_ENTRY (" + std::to_string(routine_handle) + ");\n");
    }
    statement_sequence END ';'
    {
        cppout.out_s("C4P_PROC_EXIT (" + std::to_string(routine_handle) + ");\n");
        end_routine(routine_handle);
    }
    ;

procedure_head:
    PROCEDURE IDENTIFIER optional_formal_parameter_section_list
    {
        if ($2->s_kind == PROCEDURE_IDENTIFIER)
        {
            /* already declared (forward) */
            $$ = $2->s_type_ptr;
        }
        else
        {
            if ($2->s_kind != UNDEFINED_IDENTIFIER)
            {
                c4p_error("`%s' already declared", $2->s_repr);
            }
            $$ = new_type_node(PROTOTYPE_NODE, $2, $3, 0);
            $2->s_kind = PROCEDURE_IDENTIFIER;
            $2->s_type = PROTOTYPE_NODE;
            $2->s_type_ptr = $$;
        }
    }
    ;

formal_parameter_section_list:
    formal_parameter_section
    | formal_parameter_section_list ';'
    {
        $<type_ptr>$ = last_parameter;
    }
    formal_parameter_section
    {
        reinterpret_cast<parameter_node*>($<type_ptr>3)->next = reinterpret_cast<parameter_node*>($4);
        $$ = $1;
    }
    ;

formal_parameter_section:
    {
        by_reference = false;
    }
    parameter_group
    {
        $$ = $2;
    }
    | VAR
    {
        by_reference = true;
    }
    parameter_group
    {
        $$ = $3;
    }
    /* not implemented
    | FUNCTION parameter_group
    | PROCEDURE identifier_list
    */
    ;

parameter_group:
    parameter_declarator_list ':' IDENTIFIER
    {
        parameter_node* par = reinterpret_cast<parameter_node*>($1);
        if ($3->s_kind != TYPE_IDENTIFIER)
        {
            c4p_error("`%s' is not a type identifier", $3->s_repr);
        }
        while (par != nullptr)
        {
            if ($3->s_type != ARRAY_NODE)
            {
                par->by_reference = by_reference;
            }
            else
            {
                par->by_reference = false;
            }
            par->type_symbol = $3;
            par = par->next;
        }
        $$ = $1;
    }
    ;

parameter_declarator_list:
    parameter_declarator
    | parameter_declarator_list ','
    {
        $<type_ptr>$ = last_parameter;
    }
    parameter_declarator
    {
        reinterpret_cast<parameter_node*>($<type_ptr>3)->next = reinterpret_cast<parameter_node*>($4);
        $$ = $1;
    }
    ;

parameter_declarator:
    IDENTIFIER
    {
        $$ = new_type_node(PARAMETER_NODE, $1->s_repr);
        last_parameter = reinterpret_cast<parameter_node*>($$);
    }
    ;

label_declaration_part:
    /* empty */
    | LABEL label_list ';'
    ;

label_list:
    UNSIGNED_NUMBER
    {
    }
    | label_list ',' UNSIGNED_NUMBER
    ;

constant_definition_part:
    /* empty */
    | CONST constant_definitions
    ;

constant_definitions:
    constant_definition
    | constant_definitions constant_definition
    ;

constant_definition:
    IDENTIFIER '='
    {
        cppout.out_s("static constexpr ");
        $<buf_mark>$ = cppout.get_buf_mark();
        cppout.out_s("                    " + std::string($1->s_repr) + " { ");
    }
    constant ';'
    {
        if (last_type == REAL_TYPE)
        {
            cppout.out_buf_over($<buf_mark>3, "C4P_real            ", 20);
        }
        else if (last_type == LONG_REAL_TYPE)
        {
            cppout.out_buf_over($<buf_mark>3, "C4P_longreal        ", 20);
        }
        else if (last_type == INTEGER_TYPE)
        {
            cppout.out_buf_over($<buf_mark>3, "C4P_integer         ", 20);
        }
        else if (last_type == STRING_TYPE)
        {
            cppout.out_buf_over($<buf_mark>3, "const char*         ", 20);
        }
        else if (last_type == CHARACTER_TYPE)
        {
            cppout.out_buf_over($<buf_mark>3, "char                ", 20);
        }
        cppout.out_s(" };\n");
        define_symbol($1, CONSTANT_IDENTIFIER, block_level, last_type, nullptr, &last_value);
    }
    ;

constant:
    REAL_CONSTANT
    {
        last_value.fvalue = $1;
        last_type = REAL_TYPE;
        cppout.out_s(std::to_string(last_value.fvalue));
    }
    | '+' REAL_CONSTANT
    {
        last_value.fvalue = $2;
        last_type = REAL_TYPE;
        cppout.out_s(std::to_string(last_value.fvalue));
    }
    | '-' REAL_CONSTANT
    {
        last_value.fvalue = - $2;
        last_type = REAL_TYPE;
        cppout.out_s(std::to_string(last_value.fvalue));
    }
    | LONG_REAL_CONSTANT
    {
        last_value.fvalue = $1;
        last_type = LONG_REAL_TYPE;
        cppout.out_s(std::to_string(last_value.fvalue));
    }
    | '+' LONG_REAL_CONSTANT
    {
        last_value.fvalue = $2;
        last_type = LONG_REAL_TYPE;
        cppout.out_s(std::to_string(last_value.fvalue));
    }
    | '-' LONG_REAL_CONSTANT
    {
        last_value.fvalue = - $2;
        last_type = LONG_REAL_TYPE;
        cppout.out_s(std::to_string(last_value.fvalue));
    }
    | UNSIGNED_NUMBER
    {
        last_value.ivalue = $1;
        last_type = INTEGER_TYPE;
        cppout.out_s(std::to_string(last_value.ivalue) + integer_literal_suffix);
    }
    | '+' UNSIGNED_NUMBER
    {
        last_value.ivalue = $2;
        last_type = INTEGER_TYPE;
        cppout.out_s(std::to_string(last_value.ivalue) + integer_literal_suffix);
    }
    | '-' UNSIGNED_NUMBER
    {
        last_value.ivalue = - $2;
        last_type = INTEGER_TYPE;
        cppout.out_s(std::to_string(last_value.ivalue) + integer_literal_suffix);
    }
    | IDENTIFIER
    {
        if ($1->s_kind != CONSTANT_IDENTIFIER)
        {
            c4p_error("`%s' is not a constant identifier", $1->s_repr);
        }
        last_value = $1->s_value;
        last_type = $1->s_type;
        cppout.out_s($1->s_repr);
    }
    | '+' IDENTIFIER
    {
        if ($2->s_kind != CONSTANT_IDENTIFIER)
        {
            c4p_error("`%s' is not a constant identifier", $2->s_repr);
        }
        last_value = $2->s_value;
        last_type = $2->s_type;
        cppout.out_s($2->s_repr);
    }
    | '-' IDENTIFIER
    {
        if ($2->s_kind != CONSTANT_IDENTIFIER)
        {
            c4p_error("`%s' is not a constant identifier", $2->s_repr);
        }
        /* fixme: suspicious: */
        last_value.ivalue = - $2->s_value.ivalue;
        last_type = $2->s_type;
        cppout.out_s("(-" + std::string($2->s_repr) + ")");
    }
    | CHARACTER_STRING
    {
        last_value.ivalue = $1;
        last_type = STRING_TYPE;
        cppout.out_s("\"");
        cppout.translate_string(get_string((STRING_PTR)$1));
        cppout.out_s("\"");
    }
    | CHARACTER_CONSTANT
    {
        last_value.ivalue = $1;
        last_type = CHARACTER_TYPE;
        cppout.out_s("\'");
        cppout.translate_char(static_cast<int>($1));
        cppout.out_s("\'");
    }
    ;

bound:
    UNSIGNED_NUMBER
    | '+' UNSIGNED_NUMBER
    {
        $$ = $2;
    }
    | '-' UNSIGNED_NUMBER
    {
        $$ = - $2;
    }
    | IDENTIFIER
    {
        if ($1->s_kind != CONSTANT_IDENTIFIER)
        {
            c4p_error("`%s' is not a constant identifier", $1->s_repr);
        }
        $$ = $1->s_value.ivalue;
    }
    | '+' IDENTIFIER
    {
        if ($2->s_kind != CONSTANT_IDENTIFIER)
        {
            c4p_error("`%s' is not a constant identifier", $2->s_repr);
        }
        $$ = $2->s_value.ivalue;
    }
    | '-' IDENTIFIER
    {
        if ($2->s_kind != CONSTANT_IDENTIFIER)
        {
            c4p_error("`%s' is not a constant identifier", $2->s_repr);
        }
        $$ = - $2->s_value.ivalue;
    }
    | CHARACTER_CONSTANT
    ;

type_definition_part:
    /* empty */
    | TYPE type_definitions
    ;

type_definitions:
    type_definition
    | type_definitions type_definition
    ;

type_definition:
    IDENTIFIER '='
    {
        cppout.out_s("typedef ");
    }
    type_denoter ';'
    {
        define_type($1, block_level, last_type, $4);
        cppout.out_s(";\n");
    }
    ;

type_denoter:
    IDENTIFIER
    {
        if ($1->s_kind != TYPE_IDENTIFIER)
        {
            c4p_error ("`%s' is not a type identifier", $1->s_repr);
        }
        last_type = NAMED_TYPE_NODE;
        $$ = new_type_node(NAMED_TYPE_NODE, $1);
    }
    | type
    ;

type:
    ordinal_type
    | structured_type
    | pointer_type
    ;

ordinal_type:
    subrange_type
    /* | enumerated_type (not implemnted) */
    ;

subrange_type:
    bound DOTDOT bound
    {
        last_type = SUBRANGE_NODE;
        $$ = new_type_node(SUBRANGE_NODE, $1, $3);
    }
    ;

structured_type:
    unpacked_structured_type
    | PACKED unpacked_structured_type
    {
        $$ = $2;
    }
    ;

unpacked_structured_type:
    array_type
    | record_type
    | file_type
    /* | set_type (not implemented) */
    ;

file_type:
    FILe OF component_type
    {
        $$ = new_type_node(FILE_NODE, last_type, $3);
        last_type = FILE_NODE;
    }
    ;

array_type:
    ARRAY '[' index_type_list ']' OF
    {
        $<type_ptr>$ = last_array_node;
    }
    component_type
    {
        reinterpret_cast<array_node*>($<type_ptr>6)->component_type = last_type;
        reinterpret_cast<array_node*>($<type_ptr>6)->component_type_ptr = $7;
        last_type = ARRAY_NODE;
        $$ = $3;
    }
    ;

pointer_type:
    '^' component_type
    {
        $$ = new_type_node(POINTER_NODE, last_type, $2);
        last_type = POINTER_NODE;
    }
    ;

component_type:
    type_denoter
    ;

index_type_list:
    index_type
    | index_type_list ','
    {
        $<type_ptr>$ = last_array_node;
    }
    index_type
    {
        reinterpret_cast<array_node*>($<type_ptr>3)->component_type = ARRAY_NODE;
        reinterpret_cast<array_node*>($<type_ptr>3)->component_type_ptr = $4;
        $$ = $1;
    }
    ;

index_type:
    bound DOTDOT bound
    {
        last_array_node = reinterpret_cast<array_node*>(new_type_node(ARRAY_NODE, $1, $3));
        $$ = reinterpret_cast<array_node*>(last_array_node);
    }
    | IDENTIFIER
    {
        C4P_integer lbound;
        C4P_integer ubound;
        if ($1->s_kind != TYPE_IDENTIFIER)
        {
            c4p_error("`%s' is not a type identifier", $1->s_repr);
        }
        if ($1->s_type == CHARACTER_TYPE)
        {
            if (chars_are_unsigned)
            {
                lbound = 0;
                ubound = 255;
            }
            else
            {
                lbound = -128;
                ubound = 127;
            }
        }
        else if ($1->s_type == BOOLEAN_TYPE)
        {
            lbound = 0;
            ubound = 1;
        }
        else if ($1->s_type != SUBRANGE_NODE)
        {
            c4p_error("`%s' is not a subrange type identifier", $1->s_repr);
        }
        else
        {
            lbound = (reinterpret_cast<subrange_node*>($1->s_type_ptr)->lower_bound);
            ubound = (reinterpret_cast<subrange_node*>($1->s_type_ptr)->upper_bound);
        }
        last_array_node = (reinterpret_cast<array_node*>(new_type_node(ARRAY_NODE, lbound, ubound)));
        $$ = last_array_node;
    }
    ;

record_type:
    RECORD field_list END
    {
        last_type = RECORD_NODE;
        $$ = new_type_node(RECORD_NODE, $2);
    }
    ;

field_list:
    fixed_part variant_part
    {
        $$ = new_type_node(FIELD_LIST_NODE, $1, $2);
    }
    ;

fixed_part:
    record_section_list
    ;

record_section_list:
    record_section
    | record_section_list ';'
    {
        $<type_ptr>$ = last_record_section;
    }
    record_section
    {
        reinterpret_cast<record_section_node*>($<type_ptr>3)->next = reinterpret_cast<record_section_node*>($4);
        $$ = $1;
    }
    ;

record_section:
    /* empty */
    {
        $$ = nullptr;
    }
    | field_declarator_list ':' type_denoter
    {
        $$ = new_type_node(RECORD_SECTION_NODE, $1, last_type, $3);
        last_record_section = reinterpret_cast<record_section_node*>($$);
    }
    ;

field_declarator_list:
    field_declarator
    | field_declarator_list ','
    {
        $<type_ptr>$ = last_declarator;
    }
    field_declarator
    {
        reinterpret_cast<declarator_node*>($<type_ptr>3)->next = reinterpret_cast<declarator_node*>($4);
        $$ = $1;
    }
    ;

field_declarator:
    IDENTIFIER
    {
        $$ = new_type_node(DECLARATOR_NODE, $1);
        last_declarator = reinterpret_cast<declarator_node*>($$);
    }
    ;

variant_part:
    /* empty */
    {
        $$ = nullptr;
    }
    | CASE variant_selector OF variant_list
    {
        $$ = new_type_node(VARIANT_NODE, $4);
    }
    ;

variant_selector:
    tag_type
    /* | tag_field ':' tag_type (not implemented) */
    ;

tag_type:
    IDENTIFIER
    {
    }
    ;

variant_list:
    variant
    | variant_list ';'
    {
        $<type_ptr>$ = last_variant;
    }
    variant
    {
        reinterpret_cast<variant_field_list_node*>($<type_ptr>3)->next = reinterpret_cast<variant_field_list_node*>($4);
        $$ = $1;
    }
    ;

variant:
    /* empty */
    {
        $$ = last_variant = nullptr;
    }
    |
    {
        cppout.out_stop();
    }
    case_constant_list ':' '('
    {
        cppout.out_resume();
    }
    variant_field_list ')'
    {
        $$ = $6;
        last_variant = reinterpret_cast<variant_node*>($$);
    }
    ;

variant_field_list:
    fixed_part variant_part
    {
        $$ = new_type_node(VARIANT_FIELD_LIST_NODE, $1, $2);
    }
    ;

variable_declaration_part:
    /* empty */
    | VAR variable_declarations
    ;

variable_declarations:
    variable_declaration
    | variable_declarations variable_declaration
    ;

variable_declaration:
    variable_declarator_list ':' type_denoter ';'
    {
        if (block_level == 0)
        {
            if (class_name.length() == 0 && var_struct_name.length() == 0)
            {
                cppout.out_s("C4PEXTERN ");
            }
            if (nglobalvars == 0 && !var_struct_name.empty())
            {
                cppout.out_s("struct " +  var_struct_class_name);
                if (!var_struct_base_class_name.empty())
                {
                    cppout.out_s(" : public " + var_struct_base_class_name);
                }
                cppout.out_s(" {\n");
            }
            nglobalvars += 1;
        }
        declare_var_list(reinterpret_cast<declarator_node*>($1), VARIABLE_IDENTIFIER, block_level, last_type, $3);
        cppout.out_s(";\n");
    }
    ;

variable_declarator_list:
    variable_declarator
    | variable_declarator_list ','
    {
        $<type_ptr>$ = last_declarator;
    }
    variable_declarator
    {
        reinterpret_cast<declarator_node*>($<type_ptr>3)->next = reinterpret_cast<declarator_node*>($4);
        $$ = $1;
    }
    ;

variable_declarator:
    IDENTIFIER
    {
        $$ = new_type_node(DECLARATOR_NODE, $1);
        last_declarator = reinterpret_cast<declarator_node*>($$);
    }
    ;

statement_part:
    compound_statement
    ;

compound_statement:
    BEGIn
    {
        cppout.out_s("{\n");
        curly_brace_level += 1;
        if (routine_handle == 0 && curly_brace_level == 1)
        {
            if (n_fast_vars)
            {
                cppout.out_s("C4P_FAST_VARS_0\n");
            }
            cppout.out_s("C4P_BEGIN_PROGRAM(\"" + std::string(prog_symbol->s_repr) + "\", argc, argv);\n");
        }
    }
    statement_sequence END
    {
        if (routine_handle == 0 && curly_brace_level == 1)
        {
            cppout.out_s("C4P_END_PROGRAM();\n");
        }
        curly_brace_level -= 1;
        cppout.out_s("}\n");
    }
    ;

statement_sequence:
    statement
    | statement_sequence ';' statement
    ;

statement:
    optional_label
    {
        buf_mark2 = NO_MARK;
    }
    simple_statement
    | optional_label structured_statement
    ;

optional_label:
    /* empty */
    | label
    ;

label:
    UNSIGNED_NUMBER ':'
    {
        cppout.out_s("\nc4p_L" + std::to_string($1) + ":\n");
        if (auto_exit_label >= 0 && $1 == auto_exit_label)
        {
            auto_exit_label_flag = false;
        }
    }
    ;

simple_statement:
    /* empty */
    {
        cppout.out_s(";\n");
    }
    | assignment_statement
    | procedure_statement
    | goto_statement
    ;

procedure_statement:
    READ
    {
        to_eoln = false;
    }
    read_parameter_list
    | READLN
    {
        to_eoln = true;
    }
    readln_parameter_list
    | WRITE
    {
        to_eoln = false;
    }
    write_parameter_list
    | WRITELN
    {
        to_eoln = true;
    }
    writeln_parameter_list
    | IDENTIFIER
    {
        if ($1->s_kind != PROCEDURE_IDENTIFIER)
        {
            c4p_warning("`%s' is not a procedure identifier", $1->s_repr);
        }
        cppout.out_s(std::string($1->s_repr) + " (");
        push_parameter_node (last_parameter);
        if ($1->s_kind == PROCEDURE_IDENTIFIER)
        {
            last_parameter = (reinterpret_cast<prototype_node*>($1->s_type_ptr)->formal_parameter);
        }
        else
        {
            last_parameter = 0;
        }
    }
    optional_actual_parameter_list
    {
        pop_parameter_node(last_parameter);
        cppout.out_s(");\n");
    }
    ;

readln_parameter_list:
    /* empty */
    {
        cppout.out_s("c4p_readln ();\n");
    }
    | read_parameter_list
    ;

read_parameter_list:
    '('
    {
        if (to_eoln)
        {
            cppout.out_s("C4P_READLN_BEGIN () ");
        }
        else
        {
            cppout.out_s("C4P_READ_BEGIN () ");
        }
        cppout.out_s("c4p_read_");
        buf_mark = cppout.get_buf_mark();
        cppout.out_s("_ (");
        file_specified = false;
        file_spec = "input";
    }
    read_parameter_list1 ')'
    {
        if (to_eoln)
        {
            cppout.out_s(", " + file_spec + "); C4P_READLN_END (" + file_spec + ")\n");
        }
        else
        {
            cppout.out_s(", " + file_spec + "); C4P_READ_END (" + file_spec + ")\n");
        }
    }
    ;

read_parameter_list1:
    read_parameter
    | read_parameter_list1 ','
    {
        cppout.out_s(", " + file_spec + "); c4p_read_");
        buf_mark = cppout.get_buf_mark();
        cppout.out_s("_ (");
    }
    read_parameter
    ;

read_parameter:
    variable_access
    {
        if (file_specified && file_type != CHARACTER_TYPE)
        {
            cppout.out_buf_over(buf_mark, "v", 1);
        }
        else
        {
            switch (last_type)
            {
            case FILE_NODE:
                cppout.out_buf_over(buf_mark, "f", 1);
                file_specified = true;
                cppout.get_buf_text(file_spec, buf_mark + 3, 0);
                file_type = reinterpret_cast<file_node*>(last_type_ptr)->type;
                break;
            case REAL_TYPE:
            case LONG_REAL_TYPE:
                cppout.out_buf_over(buf_mark, "r", 1);
                break;
            case INTEGER_TYPE:
                cppout.out_buf_over(buf_mark, "i", 1);
                break;
            case SUBRANGE_NODE:
                cppout.out_buf_over(buf_mark, "i", 1);
                break;
            case CHARACTER_TYPE:
                cppout.out_buf_over(buf_mark, "c", 1);
                break;
            case STRING_TYPE:
                cppout.out_buf_over(buf_mark, "s", 1);
                break;
            default:
                c4p_error("internal error: unknown read type");
            }
        }
    }
    ;

writeln_parameter_list:
    /* empty */
    {
        cppout.out_s("c4p_writeln ();\n");
    }
    | write_parameter_list
    ;

write_parameter_list:
    '('
    {
        if (to_eoln)
        {
            cppout.out_s("C4P_WRITELN_BEGIN () ");
        }
        else
        {
            cppout.out_s("C4P_WRITE_BEGIN () ");
        }
        cppout.out_s("c4p_write_");
        buf_mark = cppout.get_buf_mark();
        cppout.out_s("__ (");
        file_specified = false;
        file_spec = "output";
    }
    write_parameter_list1 ')'
    {
        if (to_eoln)
        {
            cppout.out_s(", " + file_spec + "); C4P_WRITELN_END (" + file_spec + ")\n");
        }
        else
        {
            cppout.out_s(", " + file_spec + "); C4P_WRITE_END (" + file_spec + ")\n");
        }
    }
    ;

write_parameter_list1:
    write_expression
    | write_parameter_list1 ','
    {
        cppout.out_s(", " + file_spec + "); c4p_write_");
        buf_mark = cppout.get_buf_mark();
        cppout.out_s("__ (");
    }
    write_expression
    ;

write_expression:
    expression
        {
            if (file_specified && file_type != CHARACTER_TYPE)
            {
                cppout.out_buf_over(buf_mark, "v ", 2);
            }
            else
            {
                switch (last_type)
                {
                case FILE_NODE:
                    cppout.out_buf_over(buf_mark, "f ", 2);
                    file_specified = true;
                    cppout.get_buf_text(file_spec, buf_mark + 4, 0);
                    file_type = reinterpret_cast<file_node*>(last_type_ptr)->type;
                    break;
                case REAL_TYPE:
                case LONG_REAL_TYPE:
                    cppout.out_buf_over(buf_mark, "r ", 2);
                    break;
                case INTEGER_TYPE:
                    cppout.out_buf_over(buf_mark, "i ", 2);
                    break;
                case SUBRANGE_NODE:
                    cppout.out_buf_over(buf_mark, "i ", 2);
                    break;
                case CHARACTER_TYPE:
                    cppout.out_buf_over(buf_mark, "c ", 2);
                    break;
                case ARRAY_NODE:
                case STRING_TYPE:
                    cppout.out_buf_over(buf_mark, "s ", 2);
                    break;
                default:
                    c4p_error("internal error: unknown write type: %u", last_type);
                }
            }
        }
    | expression ':'
    {
        switch (last_type)
        {
        case REAL_TYPE:
        case LONG_REAL_TYPE:
            cppout.out_buf_over(buf_mark, "r1", 2);
            break;
        case INTEGER_TYPE:
            cppout.out_buf_over(buf_mark, "i1", 2);
            break;
        case SUBRANGE_NODE:
            cppout.out_buf_over(buf_mark, "i1", 2);
            break;
        case CHARACTER_TYPE:
            cppout.out_buf_over(buf_mark, "c1", 2);
            break;
        case STRING_TYPE:
            cppout.out_buf_over(buf_mark, "s1", 2);
            break;
        default:
            c4p_error("internal error: unkown write type: %u", last_type);
        }
        cppout.out_s(", ");
    }
    output_width
    ;

output_width:
    expression
    {
    }
    | expression ':'
    {
        cppout.out_s(", ");
    }
    expression
        {
            cppout.out_buf_over(buf_mark + 1, "2", 1); break;
        }
    ;

structured_statement:
    compound_statement
    | conditional_statement
    | repetitive_statement
    /* | with_statement (not implemented) */
    ;

conditional_statement:
    if_statement
    | case_statement
    ;

/* The next rule causes a shift/reduce conflict. */
if_statement:
    IF
    {
        cppout.out_s("if (");
    }
    boolean_expression THEN
    {
        cppout.out_s(")\n");
        extra_indent += 1;
    }
    statement else_part
;

else_part:
    /* empty */
    {
        extra_indent -= 1;
    }
    | ELSE
    {
        extra_indent -= 1;
        cppout.out_s("else\n");
        extra_indent += 1;
    }
    statement
    {
        extra_indent -= 1;
    }
    ;

case_statement:
    CASE
    {
        cppout.out_s("switch (");
    }
    case_index OF
    {
        cppout.out_s(") {\n");
        curly_brace_level += 1;
    }
    case_list_element_sequence optional_semicolon END
    {
        curly_brace_level -= 1;
        cppout.out_s("}\n");
    }
    ;

optional_semicolon:
    /* empty */
    | ';'
    ;

case_index:
    expression
    {
    }
    ;

case_list_element_sequence:
    case_list_element
    | case_list_element_sequence ';' case_list_element
    ;

case_list_element:
    case_constant_list ':'
    {
        extra_indent += 1;
    }
    statement
    {
        cppout.out_s("break;\n");
        extra_indent -= 1;
    }
    ;

case_constant_list:
    case_constant
    | case_constant_list ',' case_constant
    ;

case_constant:
    {
        cppout.out_s("case ");
    }
    constant
    {
        cppout.out_s(":\n");
    }
    |
    OTHERS
    {
        cppout.out_s("default:\n");
    }
    ;

repetitive_statement:
    repeat_statement
    | while_statement
    | for_statement
    ;

while_statement:
    WHILE
    {
        cppout.out_s("while (");
    }
    expression DO
    {
        cppout.out_s(")\n");
        extra_indent += 1;
    }
    statement
    {
        extra_indent -= 1;
    }
    ;

repeat_statement:
    REPEAT
    {
        cppout.out_s("do {\n");
        curly_brace_level += 1;
    }
    statement_sequence UNTIL
    {
        curly_brace_level -= 1;
        cppout.out_s("} while (! (");
    }
    expression
    {
        cppout.out_s("));\n");
    }
    ;

/* fixme: for statement is not fully implemented */
for_statement:
    FOR IDENTIFIER ASSIGN
    {
        cppout.out_s("C4P_FOR_BEGIN (");
        if ($2->s_block_level == 0 && $2->s_kind == VARIABLE_IDENTIFIER && ! ($2->s_flags & S_PREDEFINED))
        {
            if (!var_struct_name.empty())
            {
                cppout.out_s(var_struct_name + ".");
            }
            cppout.out_s(var_name_prefix.c_str());
        }
        cppout.out_s(std::string($2->s_repr) + ", ");
    }
    initial_value to_or_downto
    {
        cppout.out_s(", " + std::string($6 > 0 ? "<=" : ">=") + ", ");
    }
    final_value DO
    {
        cppout.out_s(")\n");
        extra_indent += 1;
    }
    statement
    {
        extra_indent -= 1;
        cppout.out_s("C4P_FOR_END (");
        if ($2->s_block_level == 0 && $2->s_kind == VARIABLE_IDENTIFIER && ! ($2->s_flags & S_PREDEFINED))
        {
            if (!var_struct_name.empty())
            {
                cppout.out_s(var_struct_name + ".");
            }
            cppout.out_s(var_name_prefix);
        }
        cppout.out_s(std::string($2->s_repr) + ", " + ($6 > 0 ? "++" : "--") + ")\n");
    }
    ;

initial_value:
    expression
    {
    }
    ;

final_value:
    expression
    {
    }
    ;

to_or_downto:
    TO
    {
        $$ = 1;
    }
    | DOWNTO
    {
        $$ = -1;
    }
    ;

assignment_statement:
    variable_access
    {
        if (last_type == FUNCTION_TYPE)
        {
            cppout.out_s("c4p_result = ");
        }
        else
        {
            cppout.out_s(" = ");
        }
        }
    assign
    {
        if (buf_mark2 != NO_MARK)
        {
            cppout.out_buf_over(buf_mark2, "w", 1);
        }
    }
    expression
    {
        cppout.out_s(";\n");
    }
    ;

goto_statement:
    GOTO UNSIGNED_NUMBER
    {
        cppout.out_s("goto c4p_L" + std::to_string($2) + ";\n");
    }
    ;

assign:
    ASSIGN
    ;

variable_access:
    entire_variable
    | component_variable
    /* | identified_variable (not implemented) */
    | buffer_variable
    ;

buffer_variable:
    variable_access '^'
    {
        if (last_type != FILE_NODE)
        {
            c4p_error("left side of `^' must be a file variable");
        }
        cppout.out_s(".bufref()");
        last_type = reinterpret_cast<file_node*>(last_type_ptr)->type;
        last_type_ptr = reinterpret_cast<file_node*>(last_type_ptr)->type_ptr;
    }
    ;

entire_variable:
    variable_identifier
    ;

variable_identifier:
    IDENTIFIER
    {
        switch ($1->s_kind)
        {
            case VARIABLE_IDENTIFIER:
            case PARAMETER_IDENTIFIER:
            case CONSTANT_IDENTIFIER:
                last_type = $1->s_type;
                last_type_ptr = $1 ->s_type_ptr;
                if ($1->s_flags & S_FAST)
                {
                    cppout.out_s("c4p_fast_" + std::string($1->s_repr) + "_" + std::to_string(routine_handle));
                    remember_fast_var($1->s_repr);
                }
                else
                {
                    if ($1->s_block_level == 0 && $1->s_kind == VARIABLE_IDENTIFIER && ! ($1->s_flags & S_PREDEFINED))
                    {
                        if (!var_struct_name.empty())
                        {
                            cppout.out_s(var_struct_name + ".");
                        }
                        cppout.out_s(var_name_prefix.c_str());
                    }
                    cppout.out_s($1->s_repr);
                }
                break;
            case FUNCTION_IDENTIFIER:
                last_type = FUNCTION_TYPE;
                last_type_ptr = $1->s_type_ptr;
                break;
            default:
                c4p_error("`%s' isn't defined", $1->s_repr);
                break;
        }
    }
    ;

component_variable:
    indexed_variable
    | field_designator
    ;

indexed_variable:
    array_variable '['
    {
        if (last_type != ARRAY_NODE && last_type != POINTER_NODE)
        {
            c4p_error("left side of `[' must be an array variable");
        }
        cppout.out_s("[ ");
        $<value>$ = 0;
        if (last_type == POINTER_NODE)
        {
            push_array_type(reinterpret_cast<pointer_node*>(last_type_ptr)->array_node_ptr);
        }
        else
        {
            push_array_type(reinterpret_cast<array_node*>(last_type_ptr));
        }
    }
    index_expression_list ']'
    {
        last_type = last_array_node->component_type;
        last_type_ptr = last_array_node->component_type_ptr;
        pop_array_type();
        cppout.out_s(" ]");
    }
    ;

array_variable:
    variable_access
    ;

field_designator:
    record_variable '.' IDENTIFIER
    {
        last_type = translate_field_name($3->s_repr, reinterpret_cast<record_node*>(last_type_ptr), &last_type_ptr);
    }
    ;

record_variable:
    variable_access
    ;

index_expression_list:
    expression
    {
        if (last_array_node->lower_bound != 0)
        {
            cppout.out_s(" - (" + std::to_string(get_array_offset(last_array_node)) + ")");
        }
        last_type = last_array_node->component_type;
        last_type_ptr = last_array_node->component_type_ptr;
    }
    | index_expression_list ','
    {
        cppout.out_s(" ][ ");
    }
    expression
    {
        last_array_node = reinterpret_cast<array_node*>(last_array_node->component_type_ptr);
        cppout.out_s(" - (" + std::to_string(get_array_offset(last_array_node)) + ")");
        last_type = last_array_node->component_type;
        last_type_ptr = last_array_node->component_type_ptr;
    }
    ;

expression:
    simple_expression
    | expression relational_operator
    {
        if (relational_cast_expressions)
        {
            $<buf_mark>$ = cppout.get_buf_mark();
            cppout.out_s("(C4P_integer)     ");
        }
    }
    simple_expression
    {
        $$ = coerce($1, $4);
        if (relational_cast_expressions)
        {
            if ($1 == $4)
            {
                cppout.out_buf_over($<buf_mark>3, "                  ", 18);
            }
            else if ($$ == REAL_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_real)        ", 18);
            }
            else if ($$ == LONG_REAL_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_longreal)    ", 18);
            }
            else if ($$ == LONG_INTEGER_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_longinteger) ", 18);
            }
        }
    }
    ;

boolean_expression:
    expression
    ;

relational_operator:
    '<'
    {
        cppout.out_s(" < ");
    }
    | '>'
    {
        cppout.out_s(" > ");
    }
    | LEQ
    {
                cppout.out_s(" <= ");
    }
    | GEQ
    {
        cppout.out_s(" >= ");
    }
    | '='
    {
        cppout.out_s(" == ");
    }
    | NEQ
    {
        cppout.out_s(" != ");
    }
    ;

simple_expression:
    term
    | '-'
    {
        cppout.out_s(" - ");
        if (other_cast_expressions)
        {
        cppout.out_s("(C4P_integer) ");
        }
    }
    term
    {
        $$ = $3;
    }
    | '+' term
    {
        $$ = $2;
    }
    | simple_expression adding_operator
    {
        if (other_cast_expressions)
        {
            $<buf_mark>$ = cppout.get_buf_mark();
            cppout.out_s("(C4P_integer)     ");
        }
    }
    term
    {
        $$ = coerce($1, $4);
        if (other_cast_expressions)
        {
            if ($1 == $4)
            {
                cppout.out_buf_over($<buf_mark>3, "                  ", 18);
            }
            else if ($$ == REAL_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_real)        ", 18);
            }
            else if ($$ == LONG_REAL_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_longreal)    ", 18);
            }
            else if ($$ == LONG_INTEGER_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_longinteger) ", 18);
            }
        }
    }
    | simple_expression OR
    {
        cppout.out_s(" || ");
    }
    term
    ;

adding_operator:
    '-'
    {
        cppout.out_s(" - ");
    }
    | '+'
    {
        cppout.out_s(" + ");
    }
    ;

term:
    factor
    | term '/'
    {
        cppout.out_s(" / ");
        if (other_cast_expressions)
        {
            cppout.out_s("(C4P_real) ");
        }
    }
    factor
    | term multiplying_operator
    {
        if (other_cast_expressions)
        {
            $<buf_mark>$ = cppout.get_buf_mark();
            cppout.out_s("(C4P_integer)     ");
        }
    }
    factor
    {
        $$ = coerce($1,$4);
        if (other_cast_expressions)
        {
            if ($1 == $4)
            {
                cppout.out_buf_over($<buf_mark>3, "                  ", 18);
            }
            else if ($$ == REAL_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_real)        ", 18);
            }
            else if ($$ == LONG_REAL_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_longreal)    ", 18);
            }
            else if ($$ == LONG_INTEGER_TYPE)
            {
                cppout.out_buf_over($<buf_mark>3, "(C4P_longinteger) ", 18);
            }
        }
    }
    | term AND
    {
        cppout.out_s(" && ");
    }
    factor
    ;

multiplying_operator:
    '*'
    {
        cppout.out_s(" * ");
    }
    | DIV
    {
        cppout.out_s(" / ");
    }
    | MOD
    {
        cppout.out_s(" % ");
    }
    ;

factor:
    UNSIGNED_NUMBER
    {
        $$ = last_type = INTEGER_TYPE;
        cppout.out_s(std::to_string($1) + integer_literal_suffix);
    }
    | REAL_CONSTANT
    {
        $$ = last_type = REAL_TYPE;
        cppout.out_s(std::to_string($1));
    }
    | LONG_REAL_CONSTANT
    {
        $$ = last_type = LONG_REAL_TYPE;
        cppout.out_s(std::to_string($1));
    }
    | CHARACTER_STRING
    {
        $$ = last_type = STRING_TYPE;
        cppout.out_s("\"");
        cppout.translate_string(get_string ((STRING_PTR) $1));
        cppout.out_s("\"");
    }
    | CHARACTER_CONSTANT
    {
        $$ = last_type = CHARACTER_TYPE;
        cppout.out_s("\'");
        cppout.translate_char(static_cast<int>($1));
        cppout.out_s("\'");
    }
    | variable_access
    {
        if (last_type == FUNCTION_TYPE)
        {
            prototype_node * proto = reinterpret_cast<prototype_node*>(last_type_ptr);
            if (strcmp(proto->name->s_repr, "eoln") == 0)
            {
                cppout.out_s("eoln(input)");
            }
            else if (strcmp(proto->name->s_repr, "eof") == 0)
            {
                cppout.out_s("eof(input)");
            }
            cppout.out_s(std::string(proto->name->s_repr) + " ()");
            last_type = proto->result_type->s_type;
            last_type_ptr = proto->result_type->s_type_ptr;
        }
        $$ = last_type;
    }
    | IDENTIFIER '('
    {
        if ($1->s_kind != FUNCTION_IDENTIFIER)
        {
            c4p_warning("`%s' is not a function identifier", $1->s_repr);
        }
        else if ($1->s_type != PROTOTYPE_NODE)
        {
            c4p_error("internal error: `%1' has no type", $1->s_repr);
        }
        cppout.out_s(std::string($1->s_repr) + " (");
        push_parameter_node(last_parameter);
        if ($1->s_kind == FUNCTION_IDENTIFIER)
        {
            last_parameter = (reinterpret_cast<prototype_node*>($1->s_type_ptr)->formal_parameter);
        }
        else
        {
            last_parameter = nullptr;
        }
    }
    actual_parameter_list ')'
    {
        prototype_node * proto = reinterpret_cast<prototype_node*>($1->s_type_ptr);
        cppout.out_s(")");
        pop_parameter_node(last_parameter);
        if ($1->s_kind == FUNCTION_IDENTIFIER)
        {
            $$ = last_type = proto->result_type->s_type;
            last_type_ptr = proto->result_type->s_type_ptr;
        }
        else
        {
            $$ = last_type = INTEGER_TYPE;
            last_type_ptr = nullptr;
        }
    }
    /* | set_constructor (not implemented) */
    | '('
    {
        cppout.out_s(" (");
    }
    expression ')'
    {
        cppout.out_s(") ");
        $$ = $3;
    }
    | NOT
    {
        cppout.out_s("! ");
    }
    factor
    {
        $$ = $3;
    }
    ;

optional_actual_parameter_list:
    /* empty */
    | '(' actual_parameter_list ')'
    ;

actual_parameter_list:
    actual_parameter
    | actual_parameter_list ','
    {
        cppout.out_s(", ");
    }
    actual_parameter
    ;

actual_parameter:
    expression
    {
        if (last_parameter != nullptr)
        {
            last_parameter = last_parameter->next;
        }
    }
    ;
