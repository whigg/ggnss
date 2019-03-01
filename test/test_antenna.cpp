#include <iostream>
#include <climits>
#include <cassert>
#include "antenna.hpp"

using namespace ngpt;

int main ()
{
  std::string srec {"JAV_GRANT-G3T+G"};  /* an std::string */
  char buf[]     = {"LEIATX1230+GNSS"}; /* a c-string     */

  // Construct some antenna instances, using various constructors.
  ReceiverAntenna r1 { "NOV703GGG.R2    NONE" };      
  ReceiverAntenna r2 { "JAV_RINGANT_G3T JAVD" };
  ReceiverAntenna r3 { buf };
  ReceiverAntenna r4 { srec };
  // including serial nr -- not accepted --
  ReceiverAntenna r5 { std::string("LEIAR10         NONE12356022") };
  // Print the antennas
  std::cout<<"\nAntenna r1 type: ["<< r1.__underlying_char__() << "]";
  std::cout<<"\nAntenna r2 type: ["<< r2.__underlying_char__() << "]";
  std::cout<<"\nAntenna r3 type: ["<< r3.__underlying_char__() << "]";
  std::cout<<"\nAntenna r4 type: ["<< r4.__underlying_char__() << "]";
  std::cout<<"\nAntenna r5 type: ["<< r5.__underlying_char__() << "]";
  
  // add some serial numbers
  r1.set_serial_nr("1440925503");
  r2.set_serial_nr("08430019");
  std::cout<<"\nAntenna r1 type: ["<< r1.__underlying_char__() << "]";
  std::cout<<"\nAntenna r2 type: ["<< r2.__underlying_char__() << "]";

  // Check wether some Antennas are the equal
  std::cout<<"\n> Are r1 and r2 are the same ? " << ( (r1.is_same(r2))?"yes":"no" );
  
  auto r6(r3);
  std::cout<<"\n> Checking two antennas with same model and no serials, aka: "
           << "\n\""<<r3.__underlying_char__()<<"\" and"
           << "\n\""<<r6.__underlying_char__()<<"\""
           <<"\nAre r3 and r6 the same ? " << ( (r3.is_same(r6))?"yes":"no" )
           <<"\nAre r3 and r6 of the same model ? " << ( (r3.compare_model(r6))?"yes":"no" )
           <<"\nDoes r3 have serial ? "<<r3.has_serial()
           <<"\nDoes r6 have serial ? "<<r6.has_serial();
  
  auto r7(r1);
  std::cout<<"\n> Checking two antennas with same model and same serials, aka: "
           << "\n\""<<r1.__underlying_char__()<<"\" and"
           << "\n\""<<r7.__underlying_char__()<<"\""
           <<"\nAre r1 and r7 the same ? " << ( (r1.is_same(r7))?"yes":"no" )
           <<"\nAre r1 and r7 of the same model ? " << ( (r1.compare_model(r7))?"yes":"no" )
           <<"\nDoes r1 have serial ? "<<r1.has_serial()
           <<"\nDoes r7 have serial ? "<<r7.has_serial();
  
  r7.set_serial_nr("123456789");
  std::cout<<"\n> Checking two antennas with same model and different serials, aka: "
           << "\n\""<<r1.__underlying_char__()<<"\" and"
           << "\n\""<<r7.__underlying_char__()<<"\""
           <<"\nAre r1 and r7 the same ? " << ( (r1.is_same(r7))?"yes":"no" )
           <<"\nAre r1 and r7 of the same model ? " << ( (r1.compare_model(r7))?"yes":"no" )
           <<"\nDoes r1 have serial ? "<<r1.has_serial()
           <<"\nDoes r7 have serial ? "<<r7.has_serial();
  
  ReceiverAntenna r8("NOV703GGG.R2    NONE");
  std::cout<<"\n> Checking two antennas with same model; one has serial one not, aka: "
           << "\n\""<<r1.__underlying_char__()<<"\" and"
           << "\n\""<<r8.__underlying_char__()<<"\""
           <<"\nAre r1 and r8 the same ? " << ( (r1.is_same(r8))?"yes":"no" )
           <<"\nAre r1 and r8 of the same model ? " << ( (r1.compare_model(r8))?"yes":"no" )
           <<"\nDoes r1 have serial ? "<<r1.has_serial()
           <<"\nDoes r8 have serial ? "<<r8.has_serial();
  
  std::cout<<"\n> Checking two antennas with different model; one has serial one not, aka: "
           << "\n\""<<r1.__underlying_char__()<<"\" and"
           << "\n\""<<r3.__underlying_char__()<<"\""
           <<"\nAre r1 and r3 the same ? " << ( (r1.is_same(r3))?"yes":"no" )
           <<"\nAre r1 and r3 of the same model ? " << ( (r1.compare_model(r3))?"yes":"no" )
           <<"\nDoes r1 have serial ? "<<r1.has_serial()
           <<"\nDoes r3 have serial ? "<<r3.has_serial();
  
  std::cout<<"\n> Checking two antennas with different model and different serials, aka: "
           << "\n\""<<r1.__underlying_char__()<<"\" and"
           << "\n\""<<r2.__underlying_char__()<<"\""
           <<"\nAre r1 and r2 the same ? " << ( (r1.is_same(r2))?"yes":"no" )
           <<"\nAre r1 and r2 of the same model ? " << ( (r1.compare_model(r2))?"yes":"no" )
           <<"\nDoes r1 have serial ? "<<r1.has_serial()
           <<"\nDoes r2 have serial ? "<<r2.has_serial();
  
  r2.set_serial_nr("1440925503");
  std::cout<<"\n> Checking two antennas with different model and same serials, aka: "
           << "\n\""<<r1.__underlying_char__()<<"\" and"
           << "\n\""<<r2.__underlying_char__()<<"\""
           <<"\nAre r1 and r2 the same ? " << ( (r1.is_same(r2))?"yes":"no" )
           <<"\nAre r1 and r2 of the same model ? " << ( (r1.compare_model(r2))?"yes":"no" )
           <<"\nDoes r1 have serial ? "<<r1.has_serial()
           <<"\nDoes r2 have serial ? "<<r2.has_serial();
 
  // Print the size of each instance
  std::cout<<"\nSize of Antenna object = " << sizeof(r1)  
    << " bytes or " << sizeof(r1)*CHAR_BIT << " bits";

  std::cout<<"\n";
  return 0;
}
