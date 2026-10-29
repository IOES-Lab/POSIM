#include <dlfcn.h>
// Libraries legitimately probe optional symbols during dlopen constructors.
__attribute__((constructor)) static void probeOptionalSymbol()
{
  dlsym(RTLD_DEFAULT, "POSIM_deliberately_missing_optional_constructor_probe");
}
