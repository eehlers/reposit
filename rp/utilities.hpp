/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007, 2010 Eric Ehlers
 Copyright (C) 2006 Plamen Neykov
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2008 Nazcatech sprl Belgium

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

/*! \file
    \brief reposit utilities
*/

#ifndef rp_utilities_hpp
#define rp_utilities_hpp

#include <rp/rpdefines.hpp>
#include <rp/exception.hpp>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace reposit {

    //! \name Diagnostics
    //@{
    //! Return the Boost version number as a string.
    std::string boostVersion();
    //! Return the reposit version number as a string.
    std::string version();
    //@}

    /*! \name Logging
        These functions wrap calls to the Logger class
        simplifying the syntax for the client application
        and alleviating the need for the client application
        to #include log4cxx headers.
    */
    //@{
    //! Start logging to file of given name.
    /*! Wraps function Logger::instance().logSetFile().
    */
    std::string logSetFile(const std::string &logFileName,
                           const int &logLevel = 4);
    //! Write a message to the log file.
    /*! Wraps function Logger::instance().logMessage().
    */
    DLL_API void logWriteMessage(const std::string &message,
                                 const int &level = 4);
    //! Set the logging threshold.
    /*! Wraps function Logger::instance().setLogLevel().

        It is seldom necessary to call this function as the default
        logging level is suitable for most purposes.
    */
    void logSetLevel(const int &logLevel);
    //! Fork log messages to stdout.
    /*! Wraps function Logger::instance().setConsole().

        This function is used in command-line environments and causes
        messages to appear both on the terminal and in the log file.
    */
    void logSetConsole(const int &console = 0,
                       const int &logLevel = 4);
    //! Write Object with given ID to log file.
    /*! Writes a warning message to the log file
        if no Object is found with the given ID.
    */
    void logObject(const std::string &objectID);
    //! Write all Objects to the log file.
    /*! Takes no action if the reposit
        repository is empty.
    */
    void logAllObjects();
    //@}

    //! \name Strings
    //@{
    //! Split a string into substrings using the given delimiter.
    std::vector<std::string> split(const std::string& line,
                                   const std::string& delim,
                                   bool token_compress);
    //! Concatenate substrings into a single string using the given delimiter.
    std::string concatenate(const std::vector<std::string>& symbols,
                            const std::string& delim);

    inline bool isNumeric(const std::string &s) {
        try{
            boost::lexical_cast<double>(s);
            return true;
        } catch(...){
            return false;
        }
    }
    //@}

    //! \name Time
    //@{
    //! Retrieve the current time as a number.
    double getTime();
    //! Return the given time as a string in format HH:MM:SS.
    std::string formatTime(double tm);
    //@}

    /** \name logFile and logLevel
    *  Miscellaneous utility functions.
    */
    //@{
    //! Wraps function Logger::instance().LogFile()
    const std::string logFile();
    //! Wraps function Logger::instance().logLevel()
    const int logLevel();
    //@}
}

#endif

