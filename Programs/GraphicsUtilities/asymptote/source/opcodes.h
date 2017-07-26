/*****
 * opcodes.h
 * Andy Hammerlindl 2010/10/24
 * 
 * A list of the virtual machine opcodes, defined by the macro OPCODE.
 *****/

/* The first parameter is the name of the opcode.  The second parameter is a
 * character indicating what additional information (if any) is encoded with
 * the opcode:
 *   x - nothing
 *   n - integer
 *   t - item
 *   b - builtin
 *   l - lambda pointer
 *   o - instruction offset
 */

OPCODE(nop, 'x')
OPCODE(pop,'x')
OPCODE(intpush,'n')
OPCODE(constpush,'t')
OPCODE(varpush,'n')
OPCODE(varsave,'n')
OPCODE(fieldpush,'n')
OPCODE(fieldsave,'n')
OPCODE(builtin,'b')
OPCODE(jmp,'o')
OPCODE(cjmp,'o')
OPCODE(njmp,'o')
OPCODE(popcall,'x')
OPCODE(pushclosure,'x')
OPCODE(makefunc,'l')
OPCODE(ret,'x')
OPCODE(pushframe,'n')
OPCODE(popframe,'x')

OPCODE(push_default,'x')
OPCODE(jump_if_not_default,'o')

#ifdef COMBO
OPCODE(varpop,'n')
OPCODE(fieldpop,'n')

OPCODE(gejmp,'o')
#endif
