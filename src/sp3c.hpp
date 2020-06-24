#ifndef __SP3C_IGS_FILE__
#define __SP3C_IGS_FILE__

#include <fstream>
#include <limits>
#include <array>
#include <vector>
#include <algorithm>
#include "ggdatetime/dtcalendar.hpp"
#include "satsys.hpp"
#ifdef DEBUG
#include "ggdatetime/datetime_write.hpp"
#endif

namespace ngpt
{

constexpr double SP3_MISSING_POS_VALUE = 0e0;
constexpr double SP3_MISSING_CLK_VALUE = 999999e0; /// actually any value larger 
                                                   /// than this, not only this

enum class Sp3Event : unsigned int { 
  bad_abscent_position=0, 
  bad_abscent_clock, 
  clock_event, 
  clock_prediction, 
  maneuver, 
  orbit_prediction
};
static_assert(std::numeric_limits<unsigned char>::digits 
            > static_cast<unsigned int>(Sp3Event::orbit_prediction));

struct Sp3Flag {
  unsigned char bits_{0};
  void set(Sp3Event e) noexcept {
    bits_ |= (1 << static_cast<unsigned char>(e));
  }
  void clear(Sp3Event e) noexcept {
    bits_ &= (~(1 << static_cast<unsigned char>(e)));
  }
  void reset() noexcept { bits_=0; }
  bool is_set(Sp3Event e) const noexcept {
    return ((bits_ >> static_cast<unsigned char>(e)) & 1);
  }
};

struct Sp3EpochSvRecord {
  ngpt::SATELLITE_SYSTEM s_{};
  int prn_{};
  std::array<double,4> vals_{};
  Sp3Flag flag_;
};

class Sp3c
{
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;
  
  /// @brief Constructor from filename
  explicit
  Sp3c(const char*);
  
  /// @brief Destructor (closing the file is not mandatory, but nevertheless)
  ~Sp3c() noexcept 
  { 
    if (__istream.is_open()) __istream.close();
  }
  
  /// @brief Copy not allowed !
  Sp3c(const Sp3c&) = delete;

  /// @brief Assignment not allowed !
  Sp3c& operator=(const Sp3c&) = delete;
  
  /// @brief Move Constructor.
  Sp3c(Sp3c&& a)
  noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /// @brief Move assignment operator.
  Sp3c& operator=(Sp3c&& a) 
  noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  /// @brief Set the stream to end of header or anywhere else
  void
  rewind(pos_type pos=-1) noexcept;
  
  int
  get_next_epoch(ngpt::datetime<ngpt::microseconds>& t, std::vector<Sp3EpochSvRecord>&, int&) noexcept;

  auto
  allocate_epoch_vector() const noexcept {
    return std::vector<Sp3EpochSvRecord>(num_sats__, Sp3EpochSvRecord());
  }

  auto
  interval() const noexcept {return interval__;}
  
  auto
  num_sats() const noexcept {return num_sats__;}

#ifdef DEBUG
  void
  print_members() const noexcept
  {
    std::cout<<"\nfilename     :"<<__filename
             <<"\nVersion      :"<<version__
             <<"\nStart Epoch  :"<<ngpt::strftime_ymd_hms(start_epoch__)
             <<"\n# Epochs     :"<<num_epochs__
             <<"\nCoordinate S :"<<crd_sys__
             <<"\nOrbit Type   :"<<orb_type__
             <<"\nAgency       :"<<agency__
             <<"\nTime System  :"<<time_sys__
             <<"\nInterval     :"<<interval__.to_fractional_seconds();
   }
#endif

private:
  /// @brief Read sp3c header; assign info
  int
  read_header() noexcept;

  /// @brief clear the instance stream and return the exit_status
  int
  clear_stream(int exit_status) noexcept;
  
  int
  get_next_position(char* line, ngpt::SATELLITE_SYSTEM& s, int& prn,
                        std::array<double,4>& state, Sp3Flag& flag)
  noexcept;


  std::string            __filename;    ///< The name of the file
  std::ifstream          __istream;     ///< The infput (file) stream
  char                   version__;     ///< the version 'c' or 'd'
  ngpt::datetime<ngpt::microseconds>
                         start_epoch__; ///< Start epoch
  int                    num_epochs__,  ///< Number of epochs in file
                         num_sats__;    ///< Number od SVs in file
  std::string            crd_sys__,     ///< Coordinate system
                         orb_type__,    ///< Orbit type
                         agency__,      ///< Agency
                         time_sys__;    ///< Time system
  ngpt::microseconds     interval__;    ///< Epoch interval
  SATELLITE_SYSTEM       __satsys;      ///< satellite system
  pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field
};// Sp3c

template<int SEC>
class LagrangeSp3Interpolator {
public:
  LagrangeSp3Interpolator(Sp3c& sp3) noexcept
    : sp3_(&sp3),
      K((2*SEC*1000000L)/sp3_->interval().as_underlying_type()+1),
      running_sv(0)
    {
      svec_.reserve(sp3_->num_sats());
      for (int i=0; i<sp3_->num_sats(); i++) {
        // svec_.emplace_back(std::vector<Sp3EpochSvRecord>(2*K, {}));
        svec_[i] = std::vector<Sp3EpochSvRecord>{1,{}};
        svec_[i].reserve(2*K);
      }
      tvec_.reserve(2*K);
    };

    int
    initialize() {
      sp3_->rewind();
      int j,nsats,k=0;
      ngpt::datetime<ngpt::microseconds> t;
      auto vec = sp3_->allocate_epoch_vector();
      while (!j && k<K+1) {
        j = sp3_->get_next_epoch(t, vec, nsats);
        if (j) return j;
        tvec_.push_back(t);
        if (!k) {
          for (int i=0; i<nsats; i++) svec_[i][0] = vec[i];
          running_sv = nsats;
        } else {
          SATELLITE_SYSTEM s;
          int prn;
          for (int i=0; i<nsats; i++) {
            s = vec[i].s_;
            prn = vec[i].prn_;
            auto it = std::find_if(svec_.begin(), svec_.begin()+running_sv, 
                      [=](const std::vector<Sp3EpochSvRecord>& e)
                        {return e[0].s_==s && e[0].prn_==prn;}
                      );
            if (it==svec_.begin()+running_sv) {
              svec_[running_sv][0]=vec[i];
              ++running_sv;
            } else {
              it->emplace_back(vec[i]);
            }
          }
        }
        ++k;
      }
      return 0;
    }

private:
  Sp3c* sp3_;
  int K, running_sv;
  std::vector<std::vector<Sp3EpochSvRecord>> svec_;
  std::vector<ngpt::datetime<ngpt::microseconds>> tvec_;
};

}// ngpt

#endif
