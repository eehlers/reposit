/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Eric Ehlers
 Copyright (C) 2005, 2008 Plamen Neykov

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

#include <rpxl/conversions/scalartooper.hpp>
#include <rpxl/conversions/vectortooper.hpp>
#include <rpxl/conversions/matrixtooper.hpp>
#include <vector>
#include <string>
#include <rp/property.hpp>

namespace reposit {

    DLL_API void scalarToOper(const long &value, OPER &xLong, bool expandVector) {
        xLong.xltype = xltypeNum;
        xLong.val.num = value;
    }

    DLL_API void scalarToOper(const double &value, OPER &xDouble, bool expandVector) {
        xDouble.xltype = xltypeNum;
        xDouble.val.num = value;
    }

    DLL_API void scalarToOper(const bool &value, OPER &xBoolean, bool expandVector) {
        xBoolean.xltype = xltypeBool;
        //xBoolean.val.boolean = value;
        xBoolean.val.xbool = value;
    }

    DLL_API void scalarToOper(const char *value, OPER &xChar, bool expandVector) {
		scalarToOper(std::string(value), xChar, expandVector);
    }

    DLL_API void scalarToOper(const std::string &value, OPER &xString, bool expandVector) {
        // Must use type unsigned char (BYTE) to process the 0th byte of Excel byte-counted string
        unsigned char len = __min(XL_MAX_STR_LEN - 1, value.length());
        xString.val.str = new char[len + 1];
        xString.xltype = xltypeStr | xlbitDLLFree;
        xString.val.str[0] = len;
        if (len)
            strncpy(xString.val.str + 1, value.c_str(), len);
    }

    void setError(OPER &oper, WORD val) {
        oper.xltype = xltypeErr;
        oper.val.err = val; 
    }

    class VariantToOper : public boost::static_visitor<> {
    public:
        VariantToOper(OPER &oper, bool expand) : oper_(oper), m_expand(expand) {}
        VariantToOper(const VariantToOper& op) : oper_(op.oper_), m_expand(op.m_expand) {}

        void operator()(const empty_property_tag&) { setError(oper_, xlerrNA); }

        template <typename T>
        void operator() (const T& t) { scalarToOper(t, oper_); }

        template <typename T>
        void operator()(const std::vector<T>& v) {
            if(m_expand)
                vectorToOper(v, oper_);
            else
                scalarToOper("<VECTOR>", oper_);
        }
        
        template<typename T>
        void operator()(const std::vector<std::vector<T> >& v) {
            if(m_expand)
                matrixToOper(v, oper_);
            else
                scalarToOper("<MATRIX>", oper_);
        }

    private:
        OPER &oper_;
        bool m_expand;
    };

    DLL_API void scalarToOper(const reposit::property_t &value, OPER &xVariant, bool expandVector) {
        VariantToOper variantToOper(xVariant, expandVector);
        boost::apply_visitor(variantToOper, value);
    }
}

