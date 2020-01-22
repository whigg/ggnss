#include <stdexcept>
#include <cstring>
#include <cassert>
#include "bern_utils.hpp"
#include "ggdatetime/datetime_read.hpp"
#ifdef DEBUG
#include <iostream>
#include "ggdatetime/datetime_write.hpp"
#endif

constexpr int MAX_SATELLIT_CHARS = 256;

/// Constructor; after this, the stream will be opened, and the start of
/// 'PART 2' of the file marked.
///
/// @param[in] fn  The SATELLIT's filename
/// @throw std::runtime_error If the file cannot be found/opened or the call
///        to BernSatellit::initialize() fails
ngpt::BernSatellit::BernSatellit(const char* fn)
  : __filename(fn)
  , __istream(fn, std::ios_base::in)
  , __part2(0)
{
  if (initialize()) {
      if (__istream.is_open()) __istream.close();
      throw std::runtime_error("[ERROR] BernSatellit::BernSatellit Failed to read SATELLIT header");
  }
}

/// Search through all the record lines in 'PART 2' block of the file, to
/// match a GLONASS satellite with the given svn for the given time interval.
/// If such a satellite is found, return the recorded frequency channel.
/// @param[in]  svn   The SVN of the GLONASS satellite
/// @param[in]  eph   The time/epoch for which we want the satellite
/// @param[out] ifrqn The frequency channel of the given satellite for the given
///                   epoch
/// @param[out] prn   The prn number of the satellite as recorded in the
///                   SATELLIT file
/// @return    -1 -> Satellite not matched in file
///             0 -> Satellite matched and ifrqn assigned 
///            >0 -> An error occured while reading the records
int
ngpt::BernSatellit::get_frequency_channel(int svn, 
  const ngpt::datetime<ngpt::seconds>& eph, int& ifrqn, int& prn)
{
  const int max_lines = 1000;
  char line[MAX_SATELLIT_CHARS];

  // The stream should be open by now!
  if (!__istream.is_open()) return 1;

  // Go to the top of the relevant field (in file)
  __istream.seekg(__part2);

  // keep reading untill we match svn and epoch
  int line_count=0, csvn;
  char* end;
  bool satellite_matched = false;
  ngpt::datetime<ngpt::seconds> start, stop;
  __istream.getline(line, MAX_SATELLIT_CHARS);
  while (!satellite_matched && line_count<max_lines) {
    // we are only interested in satellites of type: 'MW' everything else
    // is desregarded and may cause a problem when resolving; e.g. SLR records
    // have no SVN field
    if (line[5]=='M' && line[6]=='W') {
      csvn = static_cast<int>(std::strtol(line+28, &end, 10));
      if (errno == ERANGE || end==line+28) {
        errno = 0;
        return 2;
      }
      if (csvn==svn) {
        start = ngpt::strptime_ymd_hms<ngpt::seconds>(line+41);
        stop  = ngpt::datetime<ngpt::seconds>::max();
        for (int i=62; i<82; i++) {
          if (line[i] != ' ') {
            stop = ngpt::strptime_ymd_hms<ngpt::seconds>(line+62);
            break;
          }
        }
        if (eph>=start && eph<stop) {
          satellite_matched = true;
          prn = static_cast<int>(std::strtol(line, &end, 10));
          ifrqn = static_cast<int>(std::strtol(line+193, &end, 10));
          if (errno == ERANGE) {
            errno = 0;
            // throw std::runtime_error("[ERROR] Failed to resolve svn from SATELLIT file");
            return 3;
          } else {
            return 0;
          }
        }
      }
    }
    __istream.getline(line, MAX_SATELLIT_CHARS);
    ++line_count;
    // check for end of records
    if (std::strlen(line)<10 || !std::strncmp("PART 3", line, 6)) return -1;
  }

  return (line_count>=max_lines)?(-1):5;
}

/// This function should only be called once, inside the object's constructor.
/// It will try to open the file, and validate that the file is actually a
/// Bernese SATELLIT file via reading and checking the first line. After this,
/// it will try to find the line: 'PART 2: ON-BOARD SENSORS' and mark where
/// (inside the file) it is placed (aka store it in __part2).
/// @return  Anything other than 0, denotes an error
int
ngpt::BernSatellit::initialize() noexcept
{
  const int max_lines = 1000;
  const char *line1 = 
    "SATELLITE-SPECIFIC INFO FOR GPS/GLONASS/GEO/LEO/SLR, BSW5.2";
  const int line1_sz = std::strlen(line1);
  const char *line2 = 
    "PART 2: ON-BOARD SENSORS";
  const int line2_sz = std::strlen(line2);
  char line[MAX_SATELLIT_CHARS];

  // The stream should be open by now!
  if (!__istream.is_open()) return 1;

  // Go to the top of the file.
  __istream.seekg(0);
  
  // Read the first line and verify
  // ----------------------------------------------------
  __istream.getline(line, MAX_SATELLIT_CHARS);
  if (std::strncmp(line1, line, line1_sz)) {
    std::cerr<<"\n[ERROR] Failed to verify first liine of SATELIT file";
    return 10;
  }

  // Read rest of lines untill we hit
  // PART 2: ON-BOARD SENSORS
  int line_count = 0;
  while (std::strncmp(line2, line, line2_sz) && line_count<max_lines) {
    __istream.getline(line, MAX_SATELLIT_CHARS);
    ++line_count;
  }
  // verify that it is ineed the line we want (and the stream is ok)
  if (!__istream.good() || line_count>=max_lines) return 11;
  // next line is just a series of  '-' chars
  __istream.getline(line, MAX_SATELLIT_CHARS);
  // next two lines are column descriptions (for the lines that follow)
  __istream.getline(line, MAX_SATELLIT_CHARS);
  const char *hln1 =
"                                              START TIME           END TIME                 SENSOR OFFSETS (M)       SENSOR BORESIGHT VECTOR (U) SENSOR AZIMUTH VECTOR (N)";
  const int hln1_sz = std::strlen(hln1);
  if (std::strncmp(hln1, line, hln1_sz)) return 12;
  __istream.getline(line, MAX_SATELLIT_CHARS);
  const char *hln2 =
"PRN  TYPE  SENSOR NAME______SVN  NUMBER  YYYY MM DD HH MM SS  YYYY MM DD HH MM SS         DX        DY        DZ         X       Y       Z          X       Y       Z      ANTEX SENSOR NAME___  IFRQ  SIGNAL LIST___________------>";
  const int hln2_sz = std::strlen(hln2);
  if (std::strncmp(hln2, line, hln2_sz)) return 13;

  // Cool! we leave the stream here and mark this position; next line to be
  // read is an empty line and then a record line
  __istream.getline(line, MAX_SATELLIT_CHARS);
  __part2 = __istream.tellg();

  // All done !
  return 0;
}
