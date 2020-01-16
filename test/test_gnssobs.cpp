#include <iostream>
#include <vector>
#include "gnssobs.hpp"

using ngpt::ObservationCode;

int main(/*int argc, char* argv[]*/)
{
  // RINEX 3 observation Codes
  std::vector<std::string> gps_obs = { "C1C", "C1S", "C1L", "C1X", "C1P", 
    "S1C", "S1S", "S1L", "S1X", "S1P", "C1W", "L1W", "D1W", "S1W", "C1Y", 
    "C1M", "C2C", "L1Y", "L1M", "L1N", "L2C", "D1Y", "D1M", "D1N", "D2C", 
    "S1Y", "S1M", "S1N", "S2C", "C2D", "L2D", "D2D", "S2D", "C2S", "C2L", 
    "C2X", "C2P", "L2S", "L2L", "L2X", "L2P", "D2S", "D2L", "D2X", "D2P", 
    "S2S", "S2L", "S2X", "S2P", "C2W", "L2W", "D2W", "S2W", "C2Y", "C2M", 
    "L2Y", "L2M", "L2N", "L5I", "L5Q", "L5X", "D2Y", "D2M", "D2N", "D5I", 
    "D5Q", "D5X", "S2Y", "S2M", "S2N", "S5I", "S5Q", "S5X", "C5I", "C5Q", 
    "C5X", "L1C", "D1C", "L1S", "D1S", "L1L", "D1L", "L1X", "D1X", "L1P",
    "D1P"};
  for (auto o : gps_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
    }
  }
  
  std::vector<std::string> glo_obs = { "C1C", "C1P", "L1P", "D1P", "S1P", 
    "C4B", "C4X", "L4A", "L4B", "L4X", "D4A", "D4B", "D4X", "S4A", "S4B", 
    "S4X", "C2C", "L2C", "D2C", "S2C", "C2P", "C6A", "C6B", "C6X", "C3I", 
    "C3Q", "C3X", "L2P", "L6A", "L6B", "L6X", "L3I", "L3Q", "L3X", "D2P", 
    "D6A", "D6B", "D6X", "D3I", "D3Q", "D3X", "S2P", "S6A", "S6B", "S6X", 
    "S3I", "S3Q", "S3X", "L1C", "D1C", "S1C"};
  for (auto o : glo_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
    }
  }
  
  std::vector<std::string> gal_obs = {"C1A", "C1B", "C1C", "C1X", "C1Z", 
    "C5I", "C5Q", "C5X", "C7I", "C7Q", "C7X", "C8I", "C8Q", "C8X", "C6A", 
    "C6B", "C6C", "C6X", "C6Z", "L1A", "D1A", "S1A", "L1B", "D1B", "S1B", 
    "L1C", "D1C", "S1C", "L1X", "D1X", "S1X", "L1Z", "D1Z", "S1Z", "L5I", 
    "D5I", "S5I", "L5Q", "D5Q", "S5Q", "L5X", "D5X", "S5X", "L7I", "D7I", 
    "S7I", "L7Q", "D7Q", "S7Q", "L7X", "D7X", "S7X", "L8I", "D8I", "S8I", 
    "L8Q", "D8Q", "S8Q", "L8X", "D8X", "S8X", "L6A", "D6A", "S6A", "L6B", 
    "D6B", "S6B", "L6C", "D6C", "S6C", "L6X", "D6X", "S6X", "L6Z", "D6Z", 
    "S6Z"};
  for (auto o : gal_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
    }
  }

  std::cout << "\n";
  return 0;
}
