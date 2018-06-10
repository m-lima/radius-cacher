//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>

#include <fmt/ostream.h>
#include <fmt/time.h>

#ifndef RC_VERBOSE_LEVEL
#define RC_VERBOSE_LEVEL 3
#endif

namespace logger {

  /**
   * The verbose level for the application.
   * Any log that is emitted at a level equal or lesser will be printed
   *
   * Use:
   * cmake -DRC_VERBOSE_LEVEL=<value> (...)
   */
  enum Level {
    NONE = -1,
    FATAL = 1,
    ERROR = 2,
    LOG = 3,
    WARNING = 4,
    INFO = 5,
    DEBUG = 6
  };

  constexpr auto FORMAT = "[{:%F %T}] {:s}{:s}\n";

  template<Level level>
  struct LogPrepend {};

  template <>
  struct LogPrepend<NONE> {
    static constexpr auto PREPEND = "";
  };

  template <>
  struct LogPrepend<DEBUG> {
    static constexpr auto PREPEND = "DEBUG: ";
  };

  template <>
  struct LogPrepend<INFO> {
    static constexpr auto PREPEND = "INFO: ";
  };

  template <>
  struct LogPrepend<WARNING> {
    static constexpr auto PREPEND = "WARNING: ";
  };

  template <>
  struct LogPrepend<LOG> {
    static constexpr auto PREPEND = "LOG: ";
  };

  template <>
  struct LogPrepend<ERROR> {
    static constexpr auto PREPEND = "ERROR: ";
  };

  template <>
  struct LogPrepend<FATAL> {
    static constexpr auto PREPEND = "FATAL: ";
  };

  template <Level level, typename ... Args>
  inline void println(std::FILE * file, const char * const format, const Args & ... args) {
    if constexpr (level <= RC_VERBOSE_LEVEL) {
      auto time = std::time(nullptr);
      fmt::print(file, FORMAT, *std::localtime(&time), LogPrepend<level>::PREPEND, fmt::format(format, args...));
    }
  }

  template <Level level, typename ... Args>
  inline void println(const char * const format, const Args & ... args) {
    println<level>(stdout, format, args...);
  }

  template <Level level, typename ... Args>
  inline void errPrintln(const char * const format, const Args & ... args) {
    println<level>(stderr, format, args...);
  }

  template <Level level>
  inline void println(std::FILE * file, const std::string & string) {
    if constexpr (level <= RC_VERBOSE_LEVEL) {
      auto time = std::time(nullptr);
      fmt::print(file, FORMAT, *std::localtime(&time), LogPrepend<level>::PREPEND, string);
    }
  }

  template <Level level>
  inline void println(const std::string & string) {
    println<level>(stdout, string);
  }

  template <Level level>
  inline void errPrintln(const std::string & string) {
    println<level>(stderr, string);
  }

  template <Level level>
  inline void println(std::FILE * file) {
    if constexpr (level <= RC_VERBOSE_LEVEL) {
      auto time = std::time(nullptr);
      fmt::print(file, FORMAT, *std::localtime(&time), LogPrepend<level>::PREPEND, "");
    }
  }

  template <Level level>
  inline void println() {
    println<level>(stdout);
  }

  template <Level level>
  inline void errPrintln() {
    println<level>(stderr);
  }
}
