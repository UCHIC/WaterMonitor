This firmware is for the more power-efficient datalogger prototype based around the Atmel ATmega328 microcontroller. The corresponding hardware files can be found in CIWS-Loggers/hardware/<insert_file_path_when_created>. This source code was written in the Arduino 1.8.8 IDE.

Current Issues:
  Faulty peak-counting algorithm (probably ported wrong).
  Record Number isn't resetting (easy fix)
  Real Time Clock time/date data corrupted on power off.