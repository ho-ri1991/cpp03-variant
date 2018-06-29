#include <cassert>
#include <string>
#include "variant.hpp"

int main()
{
  using namespace my;
  using namespace type_traits;
  {
    variant_storage::local_storage< MAKE_MY_TYPE_LIST_3(int, bool, double) > s(1);
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
    variant_storage::local_storage< MAKE_MY_TYPE_LIST_3(int, bool, double) > s(false);
    assert(s.get_discriminator() == 1);
    s.destroy();
  }
  {
    variant_storage::local_storage< MAKE_MY_TYPE_LIST_3(int, bool, double) > s(1.0);
    assert(s.get_discriminator() == 2);
    s.destroy();
  }
  {
    variant_storage::local_storage< MAKE_MY_TYPE_LIST_3(int, bool, double) > s1(1.0);
    assert(s1.get_discriminator() == 2);
    variant_storage::local_storage< MAKE_MY_TYPE_LIST_3(int, bool, double) > s2(s1);
    assert(s2.get_discriminator() == s1.get_discriminator());
    assert(*s1.get_as<double>() == *s2.get_as<double>());
    variant_storage::local_storage< MAKE_MY_TYPE_LIST_3(int, bool, double) > s3(false);
    s2 = s3;
    assert(s2.get_discriminator() == s3.get_discriminator());
    assert(*s3.get_as<bool>() == *s2.get_as<bool>());
    s1.destroy();
    s2.destroy();
    s3.destroy();
  }
  {
    variant<MAKE_MY_TYPE_LIST_3(int, bool, double), variant_storage::local_storage> var(1);
    assert(::my::get<0>(var) == 1);
    assert(::my::get<int>(var) == 1);
  }
  {
    variant<MAKE_MY_TYPE_LIST_3(int, int, bool), variant_storage::local_storage> var((in_place_index_t<0>()), 1);
    assert(::my::get<0>(var) == 1);
    assert(::my::get<int>(var) == 1);
  }
  {
    variant<MAKE_MY_TYPE_LIST_3(int, int, bool), variant_storage::local_storage> var((in_place_index_t<1>()), 1);
    assert(::my::get<1>(var) == 1);
    assert(::my::get<int>(var) == 1);
  }
  {
    variant<MAKE_MY_TYPE_LIST_2(bool, std::string), variant_storage::local_storage> var("a");
    assert(::my::get<0>(var) == true);
    assert(::my::get<bool>(var) == true);
  }
  {
    variant<MAKE_MY_TYPE_LIST_2(bool, std::string), variant_storage::local_storage> var((in_place_type_t<std::string>()), "a");
    assert(::my::get<1>(var) == "a");
    assert(::my::get<std::string>(var) == "a");
  }
  {
    variant<MAKE_MY_TYPE_LIST_3(int, bool, double), variant_storage::dynamic_storage> var(1);
    assert(::my::get<0>(var) == 1);
    assert(::my::get<int>(var) == 1);
  }
  {
    variant<MAKE_MY_TYPE_LIST_3(int, int, bool), variant_storage::dynamic_storage> var((in_place_index_t<0>()), 1);
    assert(::my::get<0>(var) == 1);
    assert(::my::get<int>(var) == 1);
  }
  {
    variant<MAKE_MY_TYPE_LIST_3(int, int, bool), variant_storage::dynamic_storage> var((in_place_index_t<1>()), 1);
    assert(::my::get<1>(var) == 1);
    assert(::my::get<int>(var) == 1);
  }
  {
    variant<MAKE_MY_TYPE_LIST_2(bool, std::string), variant_storage::dynamic_storage> var("a");
    assert(::my::get<0>(var) == true);
    assert(::my::get<bool>(var) == true);
  }
  {
    variant<MAKE_MY_TYPE_LIST_2(bool, std::string), variant_storage::dynamic_storage> var((in_place_type_t<std::string>()), "a");
    assert(::my::get<1>(var) == "a");
    assert(::my::get<std::string>(var) == "a");
  }
  return 0;
}

