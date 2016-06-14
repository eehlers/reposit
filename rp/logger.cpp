/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007 Eric Ehlers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#if defined(HAVE_CONFIG_H)     // Dynamically created by configure
    #include <rp/config.hpp>
#endif
#include <rp/rpdefines.hpp>
#include <rp/logger.hpp>
#include <rp/exception.hpp>
#include <ostream>
#include <iostream>

#ifdef RP_INCLUDE_BOOST_LOGGING

#define BOOST_LOG_DYN_LINK

#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>

boost::shared_ptr< boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > > sinkStdout;
boost::shared_ptr< boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > > sinkFile;

void reposit::Logger::setFile(const std::string &logFileName,
                     const int &logLevel) {

    filename_ = std::string();
    // Create a boost path object from the std::string.
    boost::filesystem::path path(logFileName);

    // If a parent directory has been specified then ensure it exists.
    if (!path.parent_path().empty()) {
        RP_REQUIRE(boost::filesystem::exists(path.parent_path()),
                   "Invalid parent path : " << logFileName);
    }

    try {
        boost::log::core::get()->remove_sink(sinkFile);
        sinkFile = boost::log::add_file_log(logFileName);
        setLevel(logLevel);
        filename_ = logFileName;
    } catch (const std::exception &e) {
        RP_FAIL("Logger::logSetFile: unable to set logfile - " << logFileName << " - " << e.what());
    }
}

void reposit::Logger::setConsole(
    const int &console,
    const int &logLevel) {
    try {
        boost::log::core::get()->remove_sink(sinkStdout);
        if (console)
            sinkStdout = boost::log::add_console_log(std::cout);
        setLevel(logLevel);
    } catch (const std::exception &e) {
        RP_FAIL("Logger::logSetFile: unable to set console - " << e.what());
    }
}

void reposit::Logger::setLevel(const int &logLevel) {
    try {
        level_ = -1;
        switch (logLevel) {
            case 0:
                boost::log::core::get()->set_logging_enabled(false);
                break;
            case 1:
                boost::log::core::get()->set_logging_enabled(true);
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::fatal);
                break;
            case 2:
                boost::log::core::get()->set_logging_enabled(true);
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);
                break;
            case 3:
                boost::log::core::get()->set_logging_enabled(true);
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
                break;
            case 4:
                boost::log::core::get()->set_logging_enabled(true);
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
                break;
            case 5:
                boost::log::core::get()->set_logging_enabled(true);
                boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
                break;
            default:
                RP_FAIL("Logger::setLogLevel: invalid logLevel: " << logLevel);
        }
        level_ = logLevel;
    } catch (const std::exception &e) {
        RP_FAIL("Logger::logSetFile: unable to set log level - " << e.what());
    }
}

void reposit::Logger::writeMessage(const std::string &message,
                          const int &level) {
        // client applications call this function from within their
        // catch() clauses so this function must not throw.
        try {
            
            switch (level) {
            case 1:
                BOOST_LOG_TRIVIAL(fatal) << message;
                break;
            case 2:
                BOOST_LOG_TRIVIAL(error) << message;
                break;
            case 3:
                BOOST_LOG_TRIVIAL(warning) << message;
                break;
            case 4:
                BOOST_LOG_TRIVIAL(info) << message;
                break;
            case 5:
                BOOST_LOG_TRIVIAL(debug) << message;
                break;
            }
        } catch (...) {}
}

//get log file name
const std::string reposit::Logger::file() const {
    return filename_;
}

//get log level
const int reposit::Logger::level() const {
    return level_;
}

#else

void reposit::Logger::setFile(const std::string &logFileName,
                     const int &logLevel) {
}

void reposit::Logger::setConsole(
    const int &console,
    const int &logLevel) {
}

void reposit::Logger::setLevel(const int &logLevel) {
}

void reposit::Logger::writeMessage(const std::string &message,
                          const int &level) {
    std::cout << message << std::endl;
}

const std::string reposit::Logger::file() const {
    return std::string();
}

const int reposit::Logger::level() const {
    return 0;
}

#endif

