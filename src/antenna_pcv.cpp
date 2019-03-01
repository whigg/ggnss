#include <iostream>
#include "antenna_pcv.hpp"

using ngpt::AntennaPco;

#ifdef DEBUG
void
AntennaPco::dummy_print(std::ostream& os)
const
{
  os << ngpt::satsys_to_char(__ssys)
    << "["
    << __otype.to_string()
    << "] "
    << __dn << ", " << __de << ", " << __du;
  return;
}
#endif
