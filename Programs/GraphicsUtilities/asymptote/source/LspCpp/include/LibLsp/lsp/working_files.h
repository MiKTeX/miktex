#pragma once

#include "LibLsp/lsp/lsp_diagnostic.h"
#include "LibLsp/lsp/AbsolutePath.h"
#include "LibLsp/lsp/textDocument/did_change.h"
#include "LibLsp/lsp/textDocument/did_close.h"
#include "LibLsp/lsp/textDocument/did_open.h"
#include <mutex>
#include <string>
#include <memory>
#include "Directory.h"

struct WorkingFiles;
struct WorkingFilesData;
struct WorkingFile {

    int version = 0;
    AbsolutePath filename;
    Directory directory;
    WorkingFiles& parent;
    std::atomic<long long> counter;
    WorkingFile(WorkingFiles& ,const AbsolutePath& filename, const std::string& buffer_content);
    WorkingFile(WorkingFiles&, const AbsolutePath& filename, std::string&& buffer_content);
    const std::string&  GetContentNoLock() const
    {
        return  buffer_content;
    }
protected:
    friend struct WorkingFiles;
    std::string buffer_content;
};

struct WorkingFiles {

  WorkingFiles();
  ~WorkingFiles();

  void  CloseFilesInDirectory(const std::vector<Directory>&);
  std::shared_ptr<WorkingFile>  OnOpen(lsTextDocumentItem& open);
  std::shared_ptr<WorkingFile>  OnChange(const lsTextDocumentDidChangeParams& change);
  bool  OnClose(const lsTextDocumentIdentifier& close);
  std::shared_ptr<WorkingFile>  OnSave(const lsTextDocumentIdentifier& _save);

  bool GetFileBufferContent(const AbsolutePath& filename, std::wstring& out)
  {
      auto  file = GetFileByFilename(filename);
  	  if(!file)
          return false;
	return GetFileBufferContent(file, out);
  }
  bool  GetFileBufferContent(const AbsolutePath& filename,std::string& out)
  {
      auto  file = GetFileByFilename(filename);
      if (!file)
          return false;
      return GetFileBufferContent(file, out);
  }
  bool  GetFileBufferContent(std::shared_ptr<WorkingFile>&, std::string& out);
  bool  GetFileBufferContent(std::shared_ptr<WorkingFile>&, std::wstring& out);


  // Find the file with the given filename.
  std::shared_ptr<WorkingFile>   GetFileByFilename(const AbsolutePath& filename);

  void Clear();
private:
  std::shared_ptr<WorkingFile>  GetFileByFilenameNoLock(const AbsolutePath& filename);

  WorkingFilesData* d_ptr;


};
