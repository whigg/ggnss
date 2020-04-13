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

  // read header and print info
  ObservationRnx rnx(argv[1]);
  rnx.print_members();

  // read epochs
  int status=0, numepochs=0;
  while (!status) {
    status = rnx.read_next_epoch();
    ++numepochs;
  }
  std::cout<<"\nEnded reading Rinex file; last status: "<<status
    <<" num of epochs: "<<numepochs;

  std::cout<<"\n";
}
