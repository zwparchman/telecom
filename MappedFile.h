 
#ifndef  MappedFile_INC
#define  MappedFile_INC



#include <sys/types.h>
#include <sys/stat.h>
#include <string>

struct MappedFile{
  char * start;
  struct stat sb;

  MappedFile(const std::string &s);

  size_t size() const ;

  char * data() const ;

  ~MappedFile();

};

#endif   /* ----- #ifndef MappedFile_INC  ----- */
