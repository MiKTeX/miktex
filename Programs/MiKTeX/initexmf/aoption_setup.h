const struct poptOption IniTeXMFApp::aoption_setup[] = {

  {
    "add-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ADD_FILE,
    nullptr,
    nullptr
  },

  {
    "admin", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ADMIN,
    nullptr,
    nullptr
  },

  {
    "common-config", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_CONFIG,
    nullptr,
    nullptr
  },

  {
    "common-data", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_DATA,
    nullptr,
    nullptr
  },

  {
    "common-install", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_INSTALL,
    nullptr,
    nullptr
  },

  {
    "common-roots", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_ROOTS,
    nullptr,
    nullptr
  },

  {
    "create-config-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CREATE_CONFIG_FILE,
    nullptr,
    nullptr
  },

  {
    "csv", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CSV,
    nullptr,
    nullptr
  },

  {
    "default-paper-size", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DEFAULT_PAPER_SIZE,
    nullptr,
    nullptr
  },

  {
    "disable-installer", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DISABLE_INSTALLER,
    nullptr,
    nullptr
  },

  {
    "dump", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_DUMP,
    nullptr,
    nullptr
  },

  {
    "dump-by-name", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DUMP_BY_NAME,
    nullptr,
    nullptr
  },

  {
    "edit-config-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_EDIT_CONFIG_FILE,
    nullptr,
    nullptr
  },

  {
    "enable-installer", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ENABLE_INSTALLER,
    nullptr,
    nullptr
  },

  {
    "engine", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ENGINE,
    nullptr,
    nullptr
  },

  {
    "force", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_FORCE,
    nullptr,
    nullptr
  },

  {
    "list-directory", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LIST_DIRECTORY,
    nullptr,
    nullptr
  },

  {
    "list-formats", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LIST_FORMATS,
    nullptr,
    nullptr
  },

  {
    "list-modes", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LIST_MODES,
    nullptr,
    nullptr
  },

  {
    "log-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LOG_FILE,
    nullptr,
    nullptr
  },

  {
    "mklangs", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MKLANGS,
    nullptr,
    nullptr
  },

  {
    "mklinks", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_MKLINKS,
    nullptr,
    nullptr
  },

  {
    "mkmaps", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MKMAPS,
    nullptr,
    nullptr
  },

  {
    "modify-path", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MODIFY_PATH,
    nullptr,
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "no-registry", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NO_REGISTRY,
    nullptr,
    nullptr
  },
#endif

  {
    "portable", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_PORTABLE,
    nullptr,
    nullptr
  },

  {
    "print-only", 'n',
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_PRINT_ONLY,
    nullptr,
    nullptr
  },

  {
    "quiet", 'q',
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_QUIET,
    nullptr,
    nullptr
  },

  {
    "recursive", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RECURSIVE,
    nullptr,
    nullptr
  },

  {
    "register-root", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_ROOT,
    nullptr,
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "register-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_SHELL_FILE_TYPES,
    nullptr,
    nullptr
  },
#endif

  {
    "remove-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REMOVE_FILE,
    nullptr,
    nullptr
  },

  {
    "remove-links", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REMOVE_LINKS,
    nullptr,
    nullptr
  },

  {
    "report", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REPORT,
    nullptr,
    nullptr
  },

  {
    "rmfndb", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RMFNDB,
    nullptr,
    nullptr
  },

  {
    "set-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SET_CONFIG_VALUE,
    nullptr,
    nullptr
  },

  {
    "show-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SHOW_CONFIG_VALUE,
    nullptr,
    nullptr
  },

  {
    "unregister-root", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_UNREGISTER_ROOT,
    nullptr,
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "unregister-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_UNREGISTER_SHELL_FILE_TYPES,
    nullptr,
    nullptr
  },
#endif

  {
    "update-fndb", 'u',
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_UPDATE_FNDB,
    nullptr,
    nullptr
  },

  {
    "user-config", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_CONFIG,
    nullptr,
    nullptr
  },

  {
    "user-data", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_DATA,
    nullptr,
    nullptr
  },

  {
    "user-install", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_INSTALL,
    nullptr,
    nullptr
  },

  {
    "user-roots", 'r',
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_ROOTS,
    nullptr,
    nullptr
  },

  {
    "verbose", 'v',
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_VERBOSE,
    nullptr,
    nullptr
  },

  {
    "version", 'V',
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_VERSION,
    nullptr,
    nullptr
  },

  {
    "xml", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_XML,
    nullptr,
    nullptr
  },

  POPT_TABLEEND
};
