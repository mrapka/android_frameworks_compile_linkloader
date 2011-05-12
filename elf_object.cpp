#include "elf_object.h"

#include "utils/serialize.h"
#include "elf_header.h"
#include "elf_strtab.h"
#include "elf_section_header.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace boost;
using namespace serialization;
using namespace std;

template <typename Archiver>
inline void elf_object::read_header(Archiver &AR) {
  header = elf_header::read(AR);
}

template <typename Archiver>
inline void elf_object::read_section_header_table(Archiver &AR) {
  elf_header const &header = get_header();

  AR.seek(header.get_section_header_table_offset(), true);
  for(size_t i = 0; i < header.get_section_header_num(); ++i) {
    sh_table.push_back(elf_section_header::read(AR, *this));

    // FIXME: AR should be seeked if sizeof(Elf_Shdr) != shentsize.
  }
}

template <typename Archiver>
inline void elf_object::read_section_header_str_tab(Archiver &AR) {
  section_header_str_tab =
    elf_strtab::read(AR, get_section_header(
                         get_header().get_str_section_index()));
}

template <typename archiver>
void elf_object::read_internal(archiver &AR) {
  read_header(AR);
  read_section_header_table(AR);
  read_section_header_str_tab(AR);
}

shared_ptr<elf_object> elf_object::read(string const &filename) {
  int fd;
  unsigned char const *image;
  size_t size;

  if (!open_map_file(filename, fd, image, size)) {
    // Unable to open and map the file
    return shared_ptr<elf_object>();
  }

  if (size < EI_NIDENT) {
    // File size is smallar than EI_NIDENT, impossible to be correct.
    close_map_file(fd, image, size);
    return shared_ptr<elf_object>();
  }

  // Read the ELF object
  shared_ptr<elf_object> result(new elf_object());

  if (image[EI_DATA] == ELFDATA2LSB) {
    archive_reader_le AR(image, size);
    result->read_internal(AR);

    if (!AR) {
      // Something got wrong while reading.
      close_map_file(fd, image, size);
      return shared_ptr<elf_object>();
    }
  } else {
    archive_reader_be AR(image, size);
    result->read_internal(AR);

    if (!AR) {
      // Something got wrong while reading.
      close_map_file(fd, image, size);
      return shared_ptr<elf_object>();
    }
  }

  // Close the file
  close_map_file(fd, image, size);

  return result;
}

void elf_object::print() const{
  // Print elf header
  get_header().print();

  // Print elf section header
  elf_section_header::print_header();
  for(int i=0; i<get_header().get_section_header_num(); ++i) {
    get_section_header(i).print();
  }
  elf_section_header::print_footer();
}

bool elf_object::open_map_file(std::string const &filename,
                               int &fd,
                               unsigned char const *&image,
                               size_t &size) {
  // Open the file in readonly mode
  fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    return false;
  }

  // Query the file size
  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    close(fd);
    return false;
  }

  size = (size_t)sb.st_size;

  // Map the file image
  image = static_cast<unsigned char const *>(
    mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0));

  if (image == NULL || image == MAP_FAILED) {
    close(fd);
    return false;
  }

  return true;
}

void elf_object::close_map_file(int fd,
                                unsigned char const *ptr,
                                size_t size) {
  if (ptr) { munmap((void *)ptr, size); }
  if (fd >= 0) { close(fd); }
}
