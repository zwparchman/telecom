 
#ifndef  MappedFile_INC
#define  MappedFile_INC


#if WILL_USE_BOOST
#include<boost/iostreams/device/mapped_file.hpp>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#endif

struct MappedFile{
  char * start;
  uint64_t length;

#if WILL_USE_BOOST
  boost::iostreams::mapped_file file;
#else
  ~MappedFile();
#endif

  MappedFile(const std::string &s);

  size_t size() const ;

  char * data() const ;


};

#endif   /* ----- #ifndef MappedFile_INC  ----- */
