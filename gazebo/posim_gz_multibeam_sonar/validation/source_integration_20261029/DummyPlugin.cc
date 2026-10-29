#include <gz/plugin/Register.hh>
class OptionalLookupInterface
{
public:
  virtual ~OptionalLookupInterface() = default;
  virtual int Value() const = 0;
};
class OptionalLookupPlugin : public OptionalLookupInterface
{
public:
  int Value() const override { return 1; }
};
GZ_ADD_PLUGIN(OptionalLookupPlugin, OptionalLookupInterface)
