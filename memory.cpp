#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <unistd.h>
const int MAX_SIZE = 100;
using std::cout;
using std::endl;


char* appendReverse(const char* original, int n) {
  std::unique_ptr<char[]> result(new char[2*n]);
  for (int i = 0; i < n; i++){
    result[n - i - 1] = original[i];
    result[n + i] = original[i];
  }
  return result.get();
}

int main (int argc, char *argv[]) {
  const char* filename = argv[1];
  const char* mappedFileName = argv[2];
  int fd_src = open(filename, O_RDONLY);
  if (fd_src < 0) {
    perror("Error in file opening");
    return 1;
  }

  int fd_dest = open(mappedFileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd_dest < 0) {
    perror("Error in file opening");
    return 1;
  }
  
  if (ftruncate(fd_dest, MAX_SIZE) == -1) {
    perror("trucating file failed");
    return 1;
  }

  // So that we can get the file length
  struct stat st;
  if (fstat(fd_src, &st) < 0) {
    perror("Error in pstat");
    return 1;
  }

  size_t len_file = st.st_size; 

  // Read content of source to memory
  void *mapped_src = mmap(0, len_file, PROT_READ, MAP_PRIVATE, fd_src, 0);
  if (mapped_src == MAP_FAILED) {
    perror("mmap failed for source file");
    return 1;
  }

  
  // The actual mapping for output. We need PROT_WRITE because 
  // we want the changes to be reflected in the original file.
  void *mapped = mmap(0, MAX_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd_dest, 0);
  if (mapped == MAP_FAILED) {
    perror("mmap failed for destination file");
    return 1;
  }

  // Do some useless stuff just to prove the content can change.
  char *content = static_cast<char*>(mapped_src);
  cout << "Input:" << content;
  const char *newContent = appendReverse(content, len_file);

  // Change the mapped memory content, and the file gets updated too.
  memcpy(mapped, newContent, 2 * len_file);

  return 0;
}

