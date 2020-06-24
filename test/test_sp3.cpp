#include <iostream>
#include <map>
#include "sp3c.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::Sp3c;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testSp3 [Sp3c|d]\n";
    return 1;
  }

  // read header and print info
  Sp3c sp3(argv[1]);
  sp3.print_members();

  auto vec = sp3.allocate_epoch_vector();

  int j, nsats, epochs=0;
  ngpt::datetime<ngpt::microseconds> t;
  do {
    j = sp3.get_next_epoch(t, vec, nsats);
    ++epochs;
  } while (!j);

  /*
  ngpt::LagrangeSp3Interpolator<3600> Lgrng(sp3);
  j = Lgrng.initialize();
  */

  std::cout<<"\nRead "<<epochs<<" epochs.";
  if (j>0) std::cout<<"\n[ERROR] While reading s3 file; error code #"<<j;
  if (j<0) std::cout<<"\nOK, EOF encountered in sp3";

  std::cout<<"\n";
  return 0;
}
