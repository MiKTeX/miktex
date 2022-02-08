//
// Created by jamie on 8/23/21.
//

#include "EXRFiles.h"
#include "locate.h"

namespace camp
{
using std::cout;
using std::cerr;
using std::endl;

IEXRFile::IEXRFile(const string& File)
{
  char const* err;
  int ret;
  string file=settings::locateFile(File);
  string image=settings::getSetting<string>("image");
  if(file.empty()) {
    cerr << "EXR file not found: " << File << endl
         << "Precomputed image directories like ibl/" << image << " can be "
         << "downloaded into the Asymptote search path like this:"
         << endl << endl
         << "wget -q --show-progress -nH -np -r --cut-dirs=1 "
         << settings::getSetting<string>("imageURL") << "/refl.exr" << endl
         << "wget -q --show-progress -nH -np -r --cut-dirs=1 "
         << settings::getSetting<string>("imageURL") << "/" << image
         << endl;
    exit(-1);
  }
  const char *filename=file.c_str();
  if((ret=LoadEXR(&data,&width,&height,filename,&err)) != TINYEXR_SUCCESS)
    {
      cerr << "TinyEXR Error: " << err << " " << filename << endl;
      FreeEXRErrorMessage(err);
      exit(-1);
    }
}

IEXRFile::~IEXRFile()
{
  if(data) {
    free(data);
    data=nullptr;
  }
}

}
