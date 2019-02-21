#include <iostream>
#include "antex.hpp"

using ngpt::Antex;
using ngpt::Antenna;
using ngpt::Satellite;
using ngpt::AntennaPcoList;
using ngpt::seconds;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr<<"\n[ERROR] Run as: $>testAntex [antex]\n";
        return 1;
    }

    Antenna ant;
    Antex atx (argv[1]);

    int j0, j1;
    do {
        j0 = atx.read_next_antenna_typeD(ant);
        if (j0>0) {
            std::cerr<<"\n[ERROR] Failed while reading next antenna; code: "<<j0<<"\n";
            return j0;
        } else if (j0<0) {
            std::cout<<"\nEOF encountered; exiting";
            break;
        }
        j1 = atx.skip_rest_of_antennaD();
        if (j1) {
            std::cerr<<"\n[ERROR] Failed while skipping antenna; code: "<<j1<<"\n";
            return j1;
        }
        std::cout<<"\nSuccessefuly read and skipped antenna: \""<<ant.to_string()<<"\"";
    } while (j0+j1==0);

    Antenna an1("ASH701945G_M    SCIS");
    Antenna an2("TRMR10-2        NONE");
    AntennaPcoList pco;
    j0 = atx.get_antenna_pco(an1, pco);
    std::cout<<"\nCollected PCO for "<<an1.to_string()<<" status: "<<j0;
    for (const auto p : pco.__vecref__()) { 
        std::cout<<"\n\t";
        p.dummy_print(std::cout);
    }
    j0 = atx.get_antenna_pco(an2, pco);
    std::cout<<"\nCollected PCO for "<<an2.to_string()<<" status: "<<j0;
    for (const auto p : pco.__vecref__()) { 
        std::cout<<"\n\t";
        p.dummy_print(std::cout);
    }

    Satellite sat (ngpt::satellite_system::galileo);
    sat.prn() = 12;
    auto dt = ngpt::datetime<seconds>(ngpt::year(2017),
                                 ngpt::month(10),
                                 ngpt::day_of_month(4), seconds(0));
    
    //std::ifstream::pos_type pos;
    j0 = atx.get_antenna_pco(sat.prn(), sat.system(), dt, pco);
    std::cout<<"\nfind_satellite_antenna returned: "<<j0;
    for (const auto p : pco.__vecref__()) { 
        std::cout<<"\n\t";
        p.dummy_print(std::cout);
    }
    
    j0 = atx.get_antenna_pco(1204, sat.system(), dt, pco);
    std::cout<<"\nfind_satellite_antenna returned: "<<j0;

    return 0;
}
