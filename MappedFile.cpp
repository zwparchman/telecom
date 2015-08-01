#include "MappedFile.h"


#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>



using namespace std;

MappedFile::MappedFile(const string &s){
  int fd = open( s.c_str(), O_RDONLY);

  if (fstat (fd, &sb) == -1) {
    perror ("fstat");
    throw("fstat");
  }
  if (!S_ISREG (sb.st_mode)) {
    throw("not a file");
  }
  start = (char*) mmap (0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (start == MAP_FAILED) {
    perror ("mmap");
    throw("map failed");
  }
  if (close (fd) == -1) {
    perror ("close");
    throw("close failed");
  }
}

size_t MappedFile::size() const {
  return sb.st_size;
}

char * MappedFile::data() const {
  return start;
}

MappedFile::~MappedFile(){
  munmap(start, size() );
}

