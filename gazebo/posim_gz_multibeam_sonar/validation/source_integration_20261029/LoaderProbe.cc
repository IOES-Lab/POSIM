#include <dlfcn.h>
#include <gz/plugin/Loader.hh>
#include <iostream>
int main(int argc, char ** argv)
{
  if (argc != 3)
  {
    return 2;
  }
  gz::plugin::Loader loader;
  dlerror();
  if (argv[2][0] == '1')
  {
    dlsym(RTLD_DEFAULT, "POSIM_deliberately_missing_optional_probe");
  }
  const auto plugins = loader.LoadLib(argv[1]);
  std::cout << "seeded_error=" << argv[2] << " loaded_plugins=" << plugins.size() << std::endl;
  return plugins.empty() ? 1 : 0;
}
