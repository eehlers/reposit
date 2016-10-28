/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Plamen Neykov
 Copyright (C) 2007, 2008, 2010 Eric Ehlers
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
    \brief Class ValueObject - Captures the inputs to an Object
*/

#ifndef rp_valueobject_hpp
#define rp_valueobject_hpp

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <rp/property.hpp>
#include <rp/utilities.hpp>
#include <boost/serialization/access.hpp>

namespace reposit {

    //! Capture the values of the arguments passed to the Object constructor.
    /*! For each class derived from Object there is a corresponding ValueObject
        class.  The source code of classes derived from ValueObject is generated
        automatically by gensrc.  Each Object holds a reference to its
        ValueObject.

        The ValueObject captures a snapshot of the input arguments that were
        passed to the Object's constructor.  These values can be used later
        for serialization or run-time interrogation of the Object.

        This class's member functions are implemented inline in this header
        file.  This is necessary to work around problems on the Windows platform
        when exporting this class across DLL boundaries.
    */
    class ValueObject {
    public:
        //! \name Structors
        //@{
        //! Default constructor, required by boost::serialization.
        /*! Member variables are initialized with null values, these will be
            populated during deserialization.
        */
        ValueObject() : objectId_(""), className_(""), permanent_(false) {}
        //! Standard constructor called by derived classes.
        ValueObject(
            const std::string &objectId,
            const std::string &className,
            bool permanent)
            : objectId_(objectId), className_(className), permanent_(permanent) {}
        //! Empty virtual destructor.
        virtual ~ValueObject() {}
        //@}

        /*! \name Properties
            System properties are set when the ValueObject is constructed - a snapshot
            of the inputs to the ValueObject's constructor.

            User properties are additional values which are attached to the ValueObject
            after its creation.
        */
        //@{
        //! Retrieve the property names as a std::set.
        std::set<std::string> getPropertyNames() const;
        //! Retrieve the property names as a std::vector.
        virtual std::vector<std::string> getPropertyNamesVector() const = 0;
        //! Retrieve the names of the system properties stored in the ValueObject.
        virtual const std::set<std::string>& getSystemPropertyNames() const = 0;
        //! Retrieve the value of a property given its name.
        property_t getProperty(const std::string& name) const;
        //! Retrieve the value of a system property given its name.
        virtual property_t getSystemProperty(const std::string& name) const = 0;
        //! Determine whether the given user property is present.
        bool hasProperty(const std::string& name) const;
        //! Set the value of the given property.
        void setProperty(const std::string& name, const property_t& value);
        //! Set the value of the given system property.
        virtual void setSystemProperty(const std::string& name, const property_t& value) = 0;
        //@}

        //! \name Inspectors
        //@{
        //! Retrieve the ID of the underlying Object in the Repository.
        const std::string& objectId() const { return objectId_; }
        //! Retrieve the list of precedent Object IDs.
        const std::set<std::string>& getPrecedentObjects() { return precedentIDs_;}
        //! Name of this ValueObject's class.
        const std::string &className() const { return className_; }
        //@}

        //! \name processorName
        //@{
        //! the name of the processor that is required for that ValueObject;
        /*
            For ValueObject classes which do not require special processing, 
            processorName() returns DefaultProcessor.
        */
        virtual std::string processorName()  { return "DefaultProcessor"; }
        //@}

        /*! \name Precedent Object IDs
            Functions used in managing the list of precedent Objects.

            If any of the precedent Objects changes then the Object associated
            with this ValueObject must also be recreated.
        */
        //@{
        //! Store the object ID in the list of precedents.
        void processPrecedentID(const std::string& precedentID);
        //! Extract the Object ID from the property_t and pass it to processPrecedentID()
        void processPrecedentID(property_t variantID);
        //! Extract the Object IDs from the property_t vector and pass them to processPrecedentID()
        void processPrecedentID(const std::vector<property_t>& vecVariantID);
        //! Extract the Object IDs from the property_t matrix and pass them to processPrecedentID()
        void processPrecedentID(const std::vector<std::vector<property_t> >& vecVariantIDs);
        //@}
    protected:
        //! ID of the Object associated with this ValueObject.
        std::string objectId_;
        //! A string identifiying the class of this ValueObject.
        std::string className_;
        //! Boolean indicating whether the associated Object is permanent.
        /*! \sa Object::permanent() */
        bool permanent_;
        //! Name/Value pair map representing the user defined properties.
        std::map<std::string, property_t> userProperties;
        //! List of precedent Object IDs
        std::set<std::string> precedentIDs_;
    };

    inline void ValueObject::processPrecedentID(const std::string& precedentID) {
        if (!precedentID.empty())
            precedentIDs_.insert(precedentID);
    }

    // FIXME This function should take a const ref:
    //inline void ValueObject::processPrecedentID(const property_t& variantID){
    inline void ValueObject::processPrecedentID(property_t variantID){
        std::string *objectID;
        if ((objectID = boost::get<std::string>(&variantID)) && !isNumeric(*objectID))
            processPrecedentID(*objectID);
    }

    inline void ValueObject::processPrecedentID(const std::vector<property_t>& vecVariantID){
        std::vector<property_t>::const_iterator iterator = vecVariantID.begin();
        for(; iterator != vecVariantID.end(); ++iterator)
            processPrecedentID(*iterator);
    }

    inline void ValueObject::processPrecedentID(const std::vector<std::vector<property_t> >& vecVariantIDs){
        std::vector<std::vector<property_t> >::const_iterator iterator = vecVariantIDs.begin();
        for(; iterator != vecVariantIDs.end(); ++iterator){
            processPrecedentID(*iterator);
        }
    }

    inline std::set<std::string> ValueObject::getPropertyNames() const {
        std::set<std::string> ret(getSystemPropertyNames());
        for(std::map<std::string, property_t>::const_iterator i
            = userProperties.begin(); i != userProperties.end(); ++i)
            ret.insert(i->first);
        return ret;
    }

    inline property_t ValueObject::getProperty(const std::string& name) const {
        if(userProperties.find(name) != userProperties.end())
            return userProperties.find(name)->second;
        else
            return getSystemProperty(name);
    }

    inline bool ValueObject::hasProperty(const std::string& name) const {
        return userProperties.find(name) != userProperties.end();
    }

    inline void ValueObject::setProperty(const std::string& name, const property_t& value) {
        const std::set<std::string>& sysNames = getSystemPropertyNames();
        if(sysNames.find(name) != sysNames.end())
            setSystemProperty(name, value);
        else
            userProperties[name] = value;
    }
}

#endif

