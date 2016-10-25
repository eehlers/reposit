/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Eric Ehlers

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
    \brief Support for Excel functions which loop on an input value
*/

#ifndef rpxl_loop_hpp
#define rpxl_loop_hpp

#include <rpxl/repositxl.hpp>
#include <rpxl/utilities/xlutilities.hpp>

// The max number of failed loop iterations to be logged.  This is for
// a 0-based array and will be displayed to user as ERROR_LIMIT+1
#define ERROR_LIMIT 9

namespace reposit {

    //! Execute one iteration of the loop function
    template<class LoopFunction, class InputType, class OutputType>
    struct LoopIteration {
        void operator()(
                LoopFunction &loopFunction, 
                XLOPER &xIn, 
                XLOPER &xOut,
                bool expandVector) {
            InputType inputItem = reposit::convert2<InputType>(ConvertOper(xIn));
            OutputType returnItem = loopFunction(inputItem);
            scalarToOper(returnItem, xOut, expandVector);
        }
    };

    //! Partial specialization for LoopIteration where return type is void
    template<class LoopFunction, class InputType>
    struct LoopIteration<LoopFunction, InputType, void> {
        void operator()(
                LoopFunction &loopFunction, 
                XLOPER &xIn, 
                XLOPER &xOut,
                bool expandVector) {
            InputType inputItem = reposit::convert2<InputType>(ConvertOper(xIn));
            loopFunction(inputItem);
            scalarToOper(true, xOut, expandVector);
        }
    };

    //! Invoke the contained function once for each item in the input vector.
    template<class LoopFunction, class InputType, class OutputType>
    void loop(
              const boost::shared_ptr<FunctionCall> &functionCall,
              LoopFunction &loopFunction, 
              OPER *xIn, 
              XLOPER &xOut) {

        // FIXME - xTemp may not be cleaned up properly in the event of an exception.
        OPER xTemp, *xMulti;
        bool excelToFree = false;
        bool xllToFree = false;

        // If the input is an array then take its address & carry on
        if (xIn->xltype == xltypeMulti) {
            xMulti = xIn;
        // If the input is a list then call split on it
        } else if (isList(xIn)) {
            splitOper(xIn, &xTemp);
            xMulti = &xTemp;
            xllToFree = true;
        // If the input is a scalar then just call the function once & return
        } else if (xIn->xltype == xltypeNum
        ||  xIn->xltype == xltypeBool
        ||  xIn->xltype == xltypeStr) {
            LoopIteration<LoopFunction, InputType, OutputType>()(
                loopFunction, *xIn, xOut, true);
            return;
        // Some other input (e.g. a reference) - try to convert to an array
        } else {
            Excel(xlCoerce, &xTemp, 2, xIn, TempInt(xltypeMulti));
            xMulti = &xTemp;
            excelToFree = true;
        }

        xOut.val.array.rows = xMulti->val.array.rows;
        xOut.val.array.columns = xMulti->val.array.columns;
        int numCells = xMulti->val.array.rows * xMulti->val.array.columns;
        xOut.val.array.lparray = new XLOPER[numCells]; 
        xOut.xltype = xltypeMulti | xlbitDLLFree;

        int errorCount = 0;
        std::ostringstream err;
        LoopIteration<LoopFunction, InputType, OutputType> loopIteration;
        for (int i=0; i<numCells; ++i) {
            try {
                loopIteration(loopFunction, 
                    xMulti->val.array.lparray[i],
                    xOut.val.array.lparray[i],
                    false);
            } catch (const std::exception &e) {
                xOut.val.array.lparray[i].xltype = xltypeErr;
                xOut.val.array.lparray[i].val.err = xlerrNum;

                if (errorCount > ERROR_LIMIT) {
                    // Limit exceeded.  Take no action.  For performance reasons we test
                    // this case first since it's most common on big loop w/many errors
                    ;
                } else if (errorCount < ERROR_LIMIT) {
                    err << std::endl << std::endl 
                        << "iteration #" << i << " - " << e.what();
                    errorCount++;
                } else { // errorCount == ERROR_LIMIT
                    err << std::endl << std::endl 
                        << "iteration #" << i << " - " << e.what()
                        << std::endl << std::endl 
                        << "Count of failed iterations in looping function hit "
                        << "limit of " << ERROR_LIMIT + 1 << " - logging discontinued";
                    errorCount++;
                }
            }
        }

        if (errorCount)
            RepositoryXL::instance().logError(err.str(), functionCall);

        // Free memory
        if (excelToFree) {
            Excel(xlFree, 0, 1, &xTemp);
        } else if (xllToFree) {
            freeOper(&xTemp);
        }

    }

}

#endif

