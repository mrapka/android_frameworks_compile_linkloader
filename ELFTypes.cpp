#include "ELFTypes.h"
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Format.h>

namespace detail {

#define ELF_TYPE_PRINT_OPERATOR(TYPE, FORMAT_WIDTH)                         \
  std::ostream &operator<<(std::ostream &os, TYPE const &val) {             \
    char prev_fill = os.fill('0');                                          \
    std::ios_base::fmtflags prev_flags = os.flags();                        \
    os.setf(std::ios_base::hex, std::ios_base::basefield);                  \
    os.setf(std::ios_base::right);                                          \
    os.width(FORMAT_WIDTH);                                                 \
    os << val.value;                                                        \
    os.flags(prev_flags);                                                   \
    os.fill(prev_fill);                                                     \
    return os;                                                              \
  }

  ELF_TYPE_PRINT_OPERATOR(ELF32Address, 8)
  ELF_TYPE_PRINT_OPERATOR(ELF32Offset,  8)
  ELF_TYPE_PRINT_OPERATOR(ELF64Address, 16)
  ELF_TYPE_PRINT_OPERATOR(ELF64Offset,  16)

#undef ELF_TYPE_PRINT_OPERATOR

#define ELF_TYPE_PRINT_OPERATOR(TYPE, FORMAT_WIDTH)                         \
  llvm::raw_ostream &operator<<(llvm::raw_ostream &os, TYPE const &val) {   \
    os << llvm::format("%0*x", FORMAT_WIDTH, val.value);                    \
    return os;                                                              \
  }

  ELF_TYPE_PRINT_OPERATOR(ELF32Address, 8)
  ELF_TYPE_PRINT_OPERATOR(ELF32Offset,  8)
  ELF_TYPE_PRINT_OPERATOR(ELF64Address, 16)
  ELF_TYPE_PRINT_OPERATOR(ELF64Offset,  16)

#undef ELF_TYPE_PRINT_OPERATOR

} // end namespace detail
