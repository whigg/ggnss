#! /usr/bin/python

from __future__ import print_function
import datetime
import math

class Sp3:
  
  def __init__(self, filename):
    """ open file and read header """
    with open(filename, "r") as fin:
      ## First line
      line  = fin.readline()
      if line[0:2] == "#c": version_c = 'c'
      elif line[0:2] == "#d": version_c = 'd'
      else:
        raise RuntimeError("[ERROR] Sp3 file has corrupt header line #1")
      year  = int(line[3:7])
      month = int(line[8:10])
      dom   = int(line[11:13])
      hr    = int(line[14:16])
      mn    = int(line[17:19])
      sc    = float(line[20:30])
      isc   = int(sc)
      if abs(isc-sc)>1e-6:
        raise RuntimeError("[ERROR] Sp3 epoch has fractional seconds!")
      start_date = datetime.datetime(year, month, dom, hr, mn, isc)
      num_epochs = int(line[32:39])
      crd_sys    = line[46:51]
      ## Second line
      line  = fin.readline()
      interval   = float(line[24:38])
      ## Third line
      line  = fin.readline()
      num_sats   = int(line[4:6])
      ## Get the time system
      while line and line[0:2] != "%c": line = fin.readline()
      if line[0:2] != "%c":
        raise RuntimeError("[ERROR] Sp3 file has corrupt header")
      else:
        time_sys = line[9:12]
      ## that's all the info we want!
    self.version    = version_c
    self.start_date = start_date
    self.num_epochs = num_epochs
    self.crd_sys    = crd_sys
    self.interval   = interval
    self.num_sats   = num_sats
    self.time_sys   = time_sys
    self.filename   = filename

  def count_header_lines(self):
    if self.version == 'c': return 22
    elif self.version == 'd':
      num_lines = 0
      with open(self.filename, "r") as fin:
        ## read number of sats from line 3
        for i in range(0,3): line = fin.readline()
        num_lines += 3
        num_sats = int(line[3:6])
        ## each line holds 17 sats
        for i in range(0,int(num_sats/17e0)):
          line = fin.readline()
          assert line[0] == '+'
        num_lines += int(num_sats/17e0)
        for i in range(0,int(num_sats/17e0)):
          line = fin.readline()
          assert line[0:2] == '++'
        num_lines += int(num_sats/17e0)
        while line and line[0] != '*':
          line = fin.readline()
          num_lines+=1
      return num_lines-1
    else:
      raise RuntimeError("[ERROR] Sp3 file has corrupt header version")

  def read_satellite_records(self, sat_str):
    print("## Ref System: {:}, Time System {:}".format(self.crd_sys, self.time_sys))
    ## open the file and got to start of records, that is line #23
    header_lns = self.count_header_lines()
    with open(self.filename, "r") as fin:
      for i in range(0,header_lns): line = fin.readline()
      line = fin.readline()
      while line[0:3] != "EOF":
        #print("resolving line: \"{:}\"".format(line))
        t = self.__resolve_epoch_header_line__(line)
        line, dct = self.__read_sat_pos_block__(fin)
        if sat_str in dct:
            print("\"{:10s}\"{:+15.6f}{:+15.6f}{:+15.6f}{:+15.6f}".
              format(t.strftime("%Y-%m-%d %H:%M:%S"),
              dct[sat_str]['xkm'],dct[sat_str]['ykm'],dct[sat_str]['zkm'],dct[sat_str]['cms']))

  def __resolve_epoch_header_line__(self, line):
    if line[0] != '*':
      print("[ERROR] Cannot resolve epoch header line \"{:}\"".format(line.strip()))
      raise RuntimeError("[ERROR] Sp3 file has corrupt epoch header line")
    try:
      year  = int(line[3:7])
      month = int(line[8:10])
      dom   = int(line[11:13])
      hr    = int(line[14:16])
      mn    = int(line[17:19])
      sc    = float(line[20:30])
      isc   = int(sc)
      if abs(isc-sc)>1e-6:
        raise RuntimeError("[ERROR] Sp3 epoch has fractional seconds!")
      return datetime.datetime(year, month, dom, hr, mn, isc)
    except:
      raise RuntimeError("[ERROR] Sp3 file has corrupt epoch header line")
  
  def __read_sat_pos_block__(self, istream):
    pos_dict = {}
    line = istream.readline()
    #print("resolving data line: \"{:}\"".format(line))
    while line: 
      if line[0] == 'P':
        sat, sat_dict = self.__resolve_pos_n_clock_line__(line)
        pos_dict[sat] = sat_dict
      elif line[0] in ['E', 'V']:
        pass
      else:
        raise RuntimeError("[ERROR] Sp3 file has corrupt epoch data line")
      line = istream.readline()
      if line and line[0]=='*': return line, pos_dict
      if line and line[0:3] =='EOF': return line, pos_dict
    raise RuntimeError("[ERROR] could not find position data line in Sp3 file")

  def __resolve_pos_n_clock_line__(self, line):
    #print("resolving pos_n_clock from: \"{:}\"".format(line))
    sat_str = line[1:4]
    try:
      xkm     = float(line[4:18])
      ykm     = float(line[18:32])
      zkm     = float(line[32:46])
      cms     = float(line[46:60])
      #print("all vals resolved")
      return sat_str, {'xkm':xkm, 'ykm':ykm, 'zkm':zkm, 'cms':cms}
    except:
      raise RuntimeError("[ERROR] Failed to resolve position line")

import sys
if __name__ == "__main__":
  #sp3 = Sp3("../data/COD0MGXFIN_20200010000_01D_05M_ORB.SP3")
  prn="R01"
  if len(sys.argv) > 1 : prn="R"+('{:02d}'.format(int(sys.argv[1])))
  print("## Resolving satellite with PRN: {:}".format(prn))
  sp3 = Sp3("../data/COD20820.EPH_M")
  sp3.read_satellite_records(prn)
