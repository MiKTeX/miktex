/*****
 * policy.h
 * Andy Hammerlindl 2011/09/03
 *
 * Defines a low-level C interface for interacting with the interpreter and
 * its datatypes.
 *****/

// TODO: Wrap in namespace.

typedef long long int_typ;

typedef struct {} handle_base_typ;
typedef handle_base_typ *handle_typ;
typedef struct {} arguments_base_typ;
typedef arguments_base_typ *arguments_typ;
typedef struct {} state_base_typ;
typedef state_base_typ *state_typ;

typedef void (*function_typ)(state_typ, void *);

typedef struct {
  const char *buf;
  size_t length;
} string_typ;

typedef void (*error_callback_typ)(string_typ);

typedef long arg_rest_option;
#define NORMAL_ARG 45000
#define REST_ARG 45001

typedef struct {
  int_typ version;

  handle_typ (*copyHandle)(handle_typ handle);
  void (*releaseHandle)();

  handle_typ (*handleFromInt)(int_typ x);
  // For bool, O is false, 1 is true, and no other value is allowed.
  handle_typ (*handleFromBool)(int_typ x);
  handle_typ (*handleFromDouble)(double x);
  handle_typ (*handleFromString)(string_typ x);
  handle_typ (*handleFromFunction)(const char *signature,
                                   function_typ f, void *data);

  int_typ (*IntFromHandle)(handle_typ handle);
  int_typ (*boolFromHandle)(handle_typ handle);
  double (*doubleFromHandle)(handle_typ handle);
  // TODO: Note that a pointer and length are returned, but the pointer is
  // valid for a limited time only.
  string_typ (*stringFromHandle)(handle_typ handle);

#if 0
  bool (*handleIsOverloaded)(handle_typ handle);
  handle_typ (*signatureless)(handle_typ handle);
#endif

  handle_typ (*getField)(handle_typ handle, const char *name);
  handle_typ (*getCell)(handle_typ handle, handle_typ index);

  // Adds a field to a datum (possibly a module) and sets it to an initial
  // value.
  // TODO: Change name to sig.
  void (*addField)(handle_typ handle,
                   const char *name, handle_typ init);

  arguments_typ (*newArguments)();
  void (*releaseArguments)(arguments_typ args);
  void (*addArgument)(arguments_typ args, const char *name, handle_typ handle,
                      arg_rest_option at);
  handle_typ (*call)(handle_typ callee, arguments_typ args);

  handle_typ (*globals)(state_typ state);
  int_typ (*numParams)(state_typ state);
  handle_typ (*getParam)(state_typ state, int_typ index);
  void (*setReturnValue)(state_typ state, handle_typ handle);

  // Allows the user sets an error callback, which is called on any error.
  void (*setErrorCallback)(error_callback_typ callback);
} policy_typ;


