//
// Created by Marcelo Lima on 09/06/2018.
//

#pragma once

#include <string>

#include <fmt/ostream.h>
#include <fmt/time.h>
#include <mfl/string.hpp>

namespace logger {

  /**
   * The verbose level for the application.
   * Any log that is emitted at a level equal or lesser will be printed
   */
  enum Level {
    NONE = -1,
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    LOG = 4,
    INFO = 5,
    DEBUG = 6
  };

  extern Level verboseLevel;

  bool inline setVerboseLevel(const char * level) {
    using namespace mfl::string::hash32;
    switch (hash(level)) {
      case "NONE"_h:
        logger::verboseLevel = logger::NONE;
        return true;
      case "FATAL"_h:
        logger::verboseLevel = logger::FATAL;
        return true;
      case "ERROR"_h:
        logger::verboseLevel = logger::ERROR;
        return true;
      case "WARN"_h:
        logger::verboseLevel = logger::WARN;
        return true;
      case "LOG"_h:
        logger::verboseLevel = logger::LOG;
        return true;
      case "INFO"_h:
        logger::verboseLevel = logger::INFO;
        return true;
      case "DEBUG"_h:
        logger::verboseLevel = logger::DEBUG;
        return true;
      default: return false;
    }
  }

  constexpr auto FORMAT = "[{:%F %T}] {:s}{:s}\n";

  template <Level level>
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
  struct LogPrepend<LOG> {
    static constexpr auto PREPEND = "LOG: ";
  };

  template <>
  struct LogPrepend<WARN> {
    static constexpr auto PREPEND = "WARN: ";
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
    if (level <= verboseLevel) {
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
    if (level <= verboseLevel) {
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
    if (level <= verboseLevel) {
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

#define LOG(level, ...) if (level <= logger::verboseLevel) { \
    if constexpr (level <= logger::WARN && level > 0) { \
      logger::errPrintln<level>(__VA_ARGS__); \
    } else { \
      logger::println<level>(__VA_ARGS__); \
    } \
  }
}
