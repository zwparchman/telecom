#include "MappedFile.hpp"


#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>



using namespace std;

MappedFile::MappedFile(const string &s){
#if WILL_USE_BOOST
  file.open(s);

  start = file.data();
  length = file.size();
#else
  int fd = open( s.c_str(), O_RDONLY);
  struct stat sb;

  if (fstat (fd, &sb) == -1) {
    perror ("fstat");
    throw("fstat");
  }
  if (!S_ISREG (sb.st_mode)) {
    throw("not a file");
  }
  length = sb.st_size;
  start = (char*) mmap (0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (start == MAP_FAILED) {
    perror ("mmap");
    throw("map failed");
  }
  if (close (fd) == -1) {
    perror ("close");
    throw("close failed");
  }
#endif
}

size_t MappedFile::size() const {
  return length;
}

char * MappedFile::data() const {
  return start;
}

#if ! WILL_USE_BOOST
MappedFile::~MappedFile(){
  munmap(start, size() );
}
#endif

