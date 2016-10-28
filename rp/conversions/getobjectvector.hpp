/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Eric Ehlers

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
    \brief Helper functions to facilitate retrieval of Object lists
*/

#ifndef rp_getobjectlist_hpp
#define rp_getobjectlist_hpp

#include <rp/repository.hpp>
#include <rp/group.hpp>

namespace reposit {

    //! Convert a vector of Object IDs to a vector of objects.
    template <class ObjectClass>
    std::vector<boost::shared_ptr<ObjectClass> > getObjectVector(
            const std::vector<std::string> &objectIDs,
            const int &nestingLevel = 0,
			bool includeGroups = false) {

        RP_REQUIRE(nestingLevel < 10, "getObjectVector - nesting level exceeds 10. "
            "(Possible infinite recursion?)");

        std::vector<boost::shared_ptr<ObjectClass> > ret;

        for (std::vector<std::string>::const_iterator i = objectIDs.begin();
                i != objectIDs.end(); ++i) {

            std::string objectId = *i;

            // If the object ID is a null string then silently ignore.
            // This will be revised to raise an exception in the absence of explicit
            // indication from the user to do otherwise.
            if (objectId.empty())
                continue;

            boost::shared_ptr<Object> object;
            Repository::instance().retrieveObject(object, objectId);

            boost::shared_ptr<Group> group =
                boost::dynamic_pointer_cast<Group>(object);
            if (group) {
                std::vector<boost::shared_ptr<ObjectClass> > ret2 =
                    getObjectVector<ObjectClass>(group->list(), nestingLevel + 1, includeGroups);
                ret.insert(ret.end(), ret2.begin(), ret2.end());
				if(includeGroups) {
					boost::shared_ptr<ObjectClass> objectDerived =
						boost::dynamic_pointer_cast<ObjectClass>(object);
					RP_REQUIRE(objectDerived, "Error converting Group with id '"
						<< objectId << "' - unable to convert to type '"
						<< typeid(ObjectClass).name() << "'");
					ret.push_back(objectDerived);				
				}
            } else {
                boost::shared_ptr<ObjectClass> objectDerived =
                    boost::dynamic_pointer_cast<ObjectClass>(object);
                RP_REQUIRE(objectDerived, "Error retrieving object with id '"
                    << objectId << "' - unable to convert reference to type '"
                    << typeid(ObjectClass).name() << "'");
                ret.push_back(objectDerived);
            }

        }
        return ret;
    }

    //! Convert a vector of Object IDs to a vector of library objects.
    template <class ObjectClass, class LibraryClass>
    std::vector<boost::shared_ptr<LibraryClass> > getLibraryObjectVector(
            const std::vector<std::string> &objectIDs,
            const int &nestingLevel = 0) {

        RP_REQUIRE(nestingLevel < 10, "getLibraryObjectVector - nesting level exceeds 10. "
            "(Possible infinite recursion?)");

        std::vector<boost::shared_ptr<LibraryClass> > ret;

        for (std::vector<std::string>::const_iterator i = objectIDs.begin();
                i != objectIDs.end(); ++i) {

            std::string objectId = *i;

            // If the object ID is a null string then silently ignore.
            // This will be revised to raise an exception in the absence
            // of explicit indication from the user to do otherwise.
            if (objectId.empty())
                continue;

            boost::shared_ptr<Object> object;
            Repository::instance().retrieveObject(object, objectId);

            boost::shared_ptr<Group> group =
                boost::dynamic_pointer_cast<Group>(object);
            if (group) {
                std::vector<boost::shared_ptr<LibraryClass> > ret2 =
                    getLibraryObjectVector<ObjectClass, LibraryClass>(
                        group->list(), nestingLevel + 1);
                ret.insert(ret.end(), ret2.begin(), ret2.end());
            } else {
                boost::shared_ptr<ObjectClass> objectDerived =
                    boost::dynamic_pointer_cast<ObjectClass>(object);
                RP_REQUIRE(objectDerived, "Error retrieving object with id '"
                    << objectId << "' - unable to convert reference to type '"
                    << typeid(ObjectClass).name() << "'");

                boost::shared_ptr<LibraryClass> libraryObject;
                objectDerived->getLibraryObject(libraryObject);
                ret.push_back(libraryObject);
            }

        }
        return ret;
    }
}

#endif

