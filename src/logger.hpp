//
// Created by Marcelo Lima on 6/9/18.
//

#pragma once

#include <string>

#include <fmt/ostream.h>

#ifndef VERBOSE_LEVEL
#define VERBOSE_LEVEL 0
#endif

namespace logger {

  enum Level {
    FATAL = 0,
    ERROR = 1,
    WARNING = 2,
    INFO = 3,
    DEBUG = 4,
    ALL = 5
  };

  template <int Level>
  struct LogPrepend {
    static constexpr auto PREPEND = "";
  };

  template <>
  struct LogPrepend<4> {
    static constexpr auto PREPEND = "DEBUG: ";
  };

  template <>
  struct LogPrepend<3> {
    static constexpr auto PREPEND = "INFO: ";
  };

  template <>
  struct LogPrepend<2> {
    static constexpr auto PREPEND = "WARNING: ";
  };

  template <>
  struct LogPrepend<1> {
    static constexpr auto PREPEND = "ERROR: ";
  };

  template <>
  struct LogPrepend<0> {
    static constexpr auto PREPEND = "FATAL: ";
  };

  template <int Level, typename ... Args>
  inline void println(const char * const format, const Args & ... args) {
    if constexpr (Level <= VERBOSE_LEVEL) {
      std::string logFormat{LogPrepend<Level>::PREPEND};
      logFormat.append(format);
      logFormat.append("\n");
      fmt::print(logFormat, args...);
    }
  }

  template <int Level, typename ... Args>
  inline void errPrintln(const char * const format, const Args & ... args) {
    if constexpr (Level <= VERBOSE_LEVEL) {
      std::string logFormat{LogPrepend<Level>::PREPEND};
      logFormat.append(format);
      logFormat.append("\n");
      fmt::print(stderr, logFormat, args...);
    }
  }

  template <int Level>
  inline void println(const std::string & string) {
    if constexpr (Level <= VERBOSE_LEVEL) {
      std::string logFormat{LogPrepend<Level>::PREPEND};
      logFormat.append(string);
      logFormat.append("\n");
      fmt::print(logFormat);
    }
  }

  template <int Level>
  inline void errPrintln(const std::string & string) {
    if constexpr (Level <= VERBOSE_LEVEL) {
      std::string logFormat{LogPrepend<Level>::PREPEND};
      logFormat.append(string);
      logFormat.append("\n");
      fmt::print(stderr, logFormat);
    }
  }

  template <int Level>
  inline void println() {
    if constexpr (Level <= VERBOSE_LEVEL) {
      fmt::print("\n");
    }
  }

  template <int Level>
  inline void errPrintln() {
    if constexpr (Level <= VERBOSE_LEVEL) {
      fmt::print("\n");
    }
  }
}
