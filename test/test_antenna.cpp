#include <iostream>
#include <climits>

#include "antenna.hpp" /* provides the ngpt::antenna class */

using namespace ngpt;  /* ngpt::antenna is too long */

int main ()
{
    std::string srec { "JAV_GRANT-G3T+G" };  /* an std::string */
    char buf[]     = {"LEICA ATX1230+GNSS"}; /* a c-string     */
    
    // Construct some antenna instances, using various constructors.
    Antenna r1 { "NOV703GGG.R2   DOME" };      
    Antenna r2 { "JAVAD TR_G4    ALPHA" };
    Antenna r3 { buf };
    Antenna r4 { srec };
    Antenna r5 { std::string("LEIAR10         NONE12356022") }; /* including serial nr */

    // Print the antennas
    std::cout<<"\nAntenna r1 type: ["<< r1.to_string() << "]";
    std::cout<<"\nAntenna r2 type: ["<< r2.to_string() << "]";
    std::cout<<"\nAntenna r3 type: ["<< r3.to_string() << "]";
    std::cout<<"\nAntenna r4 type: ["<< r4.to_string() << "]";
    std::cout<<"\nAntenna r5 type: ["<< r5.to_string() << "]";

    // add some serial numbers
    r1.set_serial_nr("1440925503");
    r2.set_serial_nr("08430019");
    std::cout<<"\nAntenna r1 type: ["<< r1.to_string() << "]";
    std::cout<<"\nAntenna r2 type: ["<< r2.to_string() << "]";
    
    // Check wether some Antennas are the equal
    //std::cout<<"\nAre r1 and r2 equal ? " << ( (r1==r2)?"yes":"no" );
    //std::cout<<"\nAre r1 and r3 equal ? " << ( (r1==r3)?"yes":"no" );
    //std::cout<<"\nAre r1 and r4 equal ? " << ( (r1==r4)?"yes":"no" );
    
    // Print the size of each instance
    std::cout<<"\nSize of Antenna object = " << sizeof(r1)  
      << " bytes or " << sizeof(r1)*CHAR_BIT << " bits";

    std::cout<<"\n";
    return 0;
}
