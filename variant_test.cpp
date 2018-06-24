#include "variant.hpp"
#include <cassert>

int main()
{
  using namespace my;
  using namespace type_traits;
  {
    variant_storage::local_storage<type_list<int, type_list<bool, null_type> > > s(1);
    assert(s.get_discriminator() == 0);
    s.destory();
  }
  {
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s(false);
    assert(s.get_discriminator() == 1);
    s.destory();
  }
  {
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s(1.0);
    assert(s.get_discriminator() == 2);
    s.destory();
  }
  return 0;
}
