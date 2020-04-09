#include <iostream>
#include "obsrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::ObservationRnx;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testObsRnx [Obs. RINEX]\n";
    return 1;
  }

  ObservationRnx rnx(argv[1]);
  rnx.print_members();
  std::cout<<"\n";
}
