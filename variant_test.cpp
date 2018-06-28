#include "variant.hpp"
#include <cassert>

int main()
{
  using namespace my;
  using namespace type_traits;
  {
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s(1);
    assert(s.get_discriminator() == 0);
    assert(*s.get_as<int>() == 1);
    s.assign(2.5);
    assert(s.get_discriminator() == 2);
    s.assign(true);
    assert(s.get_discriminator() == 1);
    assert(*s.get_as<bool>() == true);
    s.destroy();
  }
  {
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s(false);
    assert(s.get_discriminator() == 1);
    s.destroy();
  }
  {
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s(1.0);
    assert(s.get_discriminator() == 2);
    s.destroy();
  }
  {
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s1(1.0);
    assert(s1.get_discriminator() == 2);
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s2(s1);
    assert(s2.get_discriminator() == s1.get_discriminator());
    assert(*s1.get_as<double>() == *s2.get_as<double>());
    variant_storage::local_storage<type_list<int, type_list<bool, type_list<double, null_type> > > > s3(false);
    s2 = s3;
    assert(s2.get_discriminator() == s3.get_discriminator());
    assert(*s3.get_as<bool>() == *s2.get_as<bool>());
    s1.destroy();
    s2.destroy();
    s3.destroy();
  }
  return 0;
}
