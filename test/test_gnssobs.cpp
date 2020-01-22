#include <iostream>
#include <vector>
#include "gnssobs.hpp"
#include "gnssobsrv.hpp"

using ngpt::ObservationCode;

int main(/*int argc, char* argv[]*/)
{
  // RINEX 3 (actually 3.04) observation Codes
  // make sure they can all be resolved as ObservationCodes (for every SatSys)
  //
  
  int EXIT_STATUS = 0;
  
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
        EXIT_STATUS += 1;
      }
      ngpt::GnssObservable gobs (ngpt::SATELLITE_SYSTEM::gps, obs);
      std::cout<<"\ngps::"<<o<<" freuency: "<<gobs.frequency()<<" MHz";
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
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
        EXIT_STATUS += 1;
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
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
        EXIT_STATUS += 1;
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
    }
  }

  std::vector<std::string> sbs_obs = {"C1C", "C5I", "C5Q", "C5X", "L1C", 
    "D1C", "S1C", "L5I", "D5I", "S5I", "L5Q", "D5Q", "S5Q", "L5X", "D5X", 
    "S5X"};
  for (auto o : sbs_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
        EXIT_STATUS += 1;
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
    }
  }

  std::vector<std::string> qzs_obs = {"C1C", "C1S", "C1L", "C1X", "C1Z", 
    "C2S", "C2L", "C2X", "C5I", "C5Q", "C5X", "C5D", "C5P", "C5Z", "C6S", 
    "C6L", "C6X", "C6E", "C6Z", "L1C", "D1C", "L1S", "D1S", "L1L", "D1L", 
    "L1X", "D1X", "L1Z", "D1Z", "L2S", "D2S", "L2L", "D2L", "L2X", "D2X", 
    "L5I", "D5I", "L5Q", "D5Q", "L5X", "D5X", "L5D", "D5D", "L5P", "D5P", 
    "L5Z", "D5Z", "L6S", "D6S", "L6L", "D6L", "L6X", "D6X", "L6E", "D6E", 
    "L6Z", "D6Z", "S1C", "S1S", "S1L", "S1X", "S1Z", "S2S", "S2L", "S2X", 
    "S5I", "S5Q", "S5X", "S5D", "S5P", "S5Z", "S6S", "S6L", "S6X", "S6E", 
    "S6Z"};
  for (auto o : qzs_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
        EXIT_STATUS += 1;
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
    }
  }

  std::vector<std::string> bds_obs = {"C2I", "L2I", "D2I", "C2Q", "L2Q", 
    "D2Q", "C2X", "L2X", "D2X", "C1D", "L1D", "D1D", "C1P", "L1P", "D1P", 
    "C1X", "L1X", "D1X", "C1A", "L1A", "D1A", "L1N", "D1N", "C5D", "L5D", 
    "D5D", "C5P", "L5P", "D5P", "C5X", "L5X", "D5X", "C7I", "L7I", "D7I", 
    "C7Q", "L7Q", "D7Q", "C7X", "L7X", "D7X", "C7D", "L7D", "D7D", "C7P", 
    "L7P", "D7P", "C7Z", "L7Z", "D7Z", "C8D", "L8D", "D8D", "C8P", "L8P", 
    "D8P", "C8X", "L8X", "D8X", "C6I", "L6I", "D6I", "C6Q", "L6Q", "D6Q", 
    "C6X", "L6X", "D6X", "C6A", "L6A", "D6A", "S2I", "S2Q", "S2X", "S1D", 
    "S1P", "S1X", "S1A", "S1N", "S5D", "S5P", "S5X", "S7I", "S7Q", "S7X", 
    "S7D", "S7P", "S7Z", "S8D", "S8P", "S8X", "S6I", "S6Q", "S6X", "S6A"};
  for (auto o : bds_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
        EXIT_STATUS += 1;
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
    }
  }
  
  std::vector<std::string> irn_obs = {"C5A", "L5A", "D5A", "C5B", "L5B", 
    "D5B", "C5C", "L5C", "D5C", "C5X", "L5X", "D5X", "C9A", "L9A", "D9A", 
    "C9B", "L9B", "D9B", "C9C", "L9C", "D9C", "C9X", "L9X", "D9X", "S5A", 
    "S5B", "S5C", "S5X", "S9A", "S9B", "S9C", "S9X"};
  for (auto o : irn_obs) {
    try {
      ObservationCode obs (o.c_str());
      if (obs.to_string()!=o) {
        std::cerr<<"\n[ERROR] Observation \""<<o<<"\" resolved to \""<<obs.to_string();
        EXIT_STATUS += 1;
      }
    } catch (std::runtime_error& e) {
      std::cerr<<"\n[ERROR] Failed to resolve ObservationCode: \""<<o<<"\"";
      EXIT_STATUS += 1;
    }
  }

  std::cout << "\n";
  return 0;
}
