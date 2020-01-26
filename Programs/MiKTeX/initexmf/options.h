const struct poptOption IniTeXMFApp::options[] = {

  {
    "add-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ADD_FILE,
    T_("Add a file to the file name database."),
    T_("FILE")
  },

  {
    "admin", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ADMIN,
    T_("Run in administrator mode."),
    nullptr
  },

  {
    "clean", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CLEAN,
    T_("Remove restorable data."),
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
    T_("Create the specified configuration file."),
    T_("CONFIGFILE")
  },

  {
    "csv", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CSV,
    T_("Print comma-separated values."),
    nullptr
  },

  {
    "default-paper-size", 0,
    POPT_ARG_STRING, nullptr,
    OPT_DEFAULT_PAPER_SIZE,
    T_("Set the default paper size (one of: a4, letter)."),
    T_("PAPER"),
  },

  {
    "disable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_DISABLE_INSTALLER,
    T_("Disable the package installer (do not automatically install missing files)."),
    nullptr
  },

  {
    "dump", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_DUMP,
    T_("Create memory dump files."),
    T_("KEY")
  },

  {
    "dump-by-name", 0,
    POPT_ARG_STRING, nullptr,
    OPT_DUMP_BY_NAME,
    T_("Create a memory dump file by name."),
    T_("NAME")
  },

  {
    "edit-config-file", 0,
    POPT_ARG_STRING, nullptr,
    OPT_EDIT_CONFIG_FILE,
    T_("Open the specified configuration file in an editor. See the manual, for more information."),
    T_("CONFIGFILE")
  },

  {
    "enable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ENABLE_INSTALLER,
    T_("Enable the package installer (automatically install missing files)."),
    nullptr
  },

  {
    "engine", 0,
    POPT_ARG_STRING, nullptr,
    OPT_ENGINE,
    T_("Engine to be used."),
    T_("ENGINE")
  },

  {
    "find-other-tex", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_FIND_OTHER_TEX,
    T_("Locate other TeX installations."),
    nullptr
  },

  {
    "force", 0,
    POPT_ARG_NONE, nullptr,
    OPT_FORCE,
    T_("Force --mklinks to overwrite existing files."),
    nullptr
  },

  {
    "list-formats", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_FORMATS,
    T_("List formats."),
    0
  },

  {
    "list-modes", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_MODES,
    T_("List METAFONT modes"),
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
    POPT_ARG_NONE, nullptr,
    OPT_MKLANGS,
    T_("Create language.dat, language.dat.lua and language.def."),
    nullptr
  },

  {
    "mklinks", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_MKLINKS,
    T_("Create links for formats, scripts and miktex- executables."),
    T_("CATEGORY")
  },

  {
    "mkmaps", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKMAPS,
    T_("Create font map files."),
    nullptr
  },

  {
    "modify-path", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MODIFY_PATH,
    T_("Modify the environmaent variable PATH."),
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "no-registry", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NO_REGISTRY,
    T_("Do not use the Windows Registry to store configuration settings."),
    nullptr
  },
#endif

  {
    "portable", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_PORTABLE,
    T_("Create a portable setup."),
    "DIR"
  },

  {
    "principal", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_PRINCIPAL,
    T_("Set the principal of this program invocation."),
    nullptr
  },

  {
    "print-only", 'n',
    POPT_ARG_NONE, nullptr,
    OPT_PRINT_ONLY,
    T_("Print what would be done."),
    nullptr
  },

  {
    "quiet", 'q',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Suppress screen output."),
    nullptr
  },

  {
    "recursive", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RECURSIVE,
    T_("Operate recursively."),
    nullptr
  },

  {
    "register-other-roots", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_OTHER_ROOTS,
    T_("Register TEXMF root directories of other TeX installations."),
    nullptr
  },

  {
    "register-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_REGISTER_ROOT,
    T_("Register a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "register-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_SHELL_FILE_TYPES,
    T_("Register shell file types."),
    nullptr
  },
#endif

  {
    "remove-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REMOVE_FILE,
    T_("Remove a file from the file name database."),
    T_("FILE")
  },

  {
    "remove-links", 0,
    POPT_ARG_NONE, nullptr,
    OPT_REMOVE_LINKS,
    T_("Remove linked executables."),
    nullptr
  },

  {
    "report", 0,
    POPT_ARG_NONE, nullptr,
    OPT_REPORT,
    T_("Create a configuration report."),
    nullptr
  },

  {
    "rmfndb", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RMFNDB,
    T_("Remove file name database files."),
    nullptr
  },

  {
    "set-config-value", 0,
    POPT_ARG_STRING, nullptr,
    OPT_SET_CONFIG_VALUE,
    T_("Set the specified configuration value."),
    T_("[SECTION]VALUENAME=VALUE")
  },

  {
    "show-config-value", 0,
    POPT_ARG_STRING, nullptr,
    OPT_SHOW_CONFIG_VALUE,
    T_("Show the specified configuration value."),
    T_("[SECTION]VALUENAME")
  },

  {
    "unregister-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_UNREGISTER_ROOT,
    T_("Unegister a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "unregister-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_UNREGISTER_SHELL_FILE_TYPES,
    T_("Unregister shell file types."),
    nullptr
  },
#endif

  {
    "update-fndb", 'u',
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_UPDATE_FNDB,
    T_("Update the file name database."),
    T_("ROOT")
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
    POPT_ARG_STRING, nullptr,
    OPT_USER_ROOTS,
    T_("Register user root directories."),
    T_("DIRS")
  },

  {
    "verbose", 'v',
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Print information on what is being done."),
    nullptr
  },

  {
    "version", 'V',
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};
