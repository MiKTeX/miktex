# MiKTeX Source Code

## Directory structure

### `BuildUtilities`

#### `c4p`

This is the Pascal-to-C translator for the MiKTeX project.

### `Libraries`

#### `MiKTeX/App`

The MiKTeX App library exports the [Application class](@ref MiKTeX::App::Application) which most programs are based on.

#### `MiKTeX/Core`

The MiKTeX Core Library provides basic utilities which help to implement programs in a portable way

- [Cfg](@ref MiKTeX::Core::Cfg): Parse and write INI files.
- [Directory](@ref MiKTeX::Core::Directory): Access and control file system directories.
- [DirectoryLister](@ref MiKTeX::Core::DirectoryLister): Read the contents of a file system directory.
- [File](@ref MiKTeX::Core::File): Access and control files.
- [LockFile](@ref MiKTeX::Core::LockFile): Create lock files.
- [MemoryMappedFile](@ref MiKTeX::Core::MemoryMappedFile): Access memory mapped files.
- [Process](@ref MiKTeX::Core::Process): Create and control child processes.
- [Session](@ref MiKTeX::Core::Session): An interface for searching files and managing the MiKTeX configuration.

#### `MiKTeX/Extractor`

#### `MiKTeX/KPathSeaEmulation`

#### `MiKTeX/PackageManager`

#### `MiKTeX/Setup`

#### `MiKTeX/TeXAndFriends`

#### `MiKTeX/Trace`

#### `MiKTeX/UI/Qt`

#### `MiKTeX/Util`

#### `MiKTeX/Web2CWmulation`

### `Programs`

#### `MiKTeX/Console`

#### `MiKTeX/initexmf`

#### `MiKTeX/mkfntmap`

#### `MiKTeX/PackageManager/mpm`

#### `MiKTeX/Setup/miktexsetup`
