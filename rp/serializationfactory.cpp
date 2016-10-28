/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Eric Ehlers
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

#if defined(HAVE_CONFIG_H)     // Dynamically created by configure
    #include <rp/config.hpp>
#endif

#include <rp/serializationfactory.hpp>
#include <rp/processor.hpp>
#include <rp/range.hpp>
#include <rp/group.hpp>
#include <rp/repository.hpp>
#include <rp/conversions/getobjectvector.hpp>

//#if BOOST_VERSION > 105000
    //#define BOOST_FILESYSTEM_VERSION 3
#if BOOST_VERSION > 104601 & BOOST_VERSION < 105000
    #define BOOST_FILESYSTEM_VERSION 2
#endif

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <fstream>

namespace reposit {

    boost::shared_ptr<Object> createRange(const boost::shared_ptr<ValueObject> &valueObject) 
	{
        // FIXME - Implement ValueObject::permanent() and call that instead?
        bool permanent = 
            boost::get<bool>(valueObject->getProperty("PERMANENT"));

        std::vector<std::vector<double> > values = 
            reposit::matrix::convert<double>(valueObject->getProperty("VALUES"), "VALUES");

        boost::shared_ptr<Object> object(new Range(valueObject, values, permanent));
        return object;
    }

    boost::shared_ptr<Object> createGroup(const boost::shared_ptr<ValueObject> &valueObject) 
	{
        return boost::shared_ptr<Object>(
			new Group(
				valueObject,
				vector::convert<std::string>(valueObject->getProperty("OBJECTIDLIST"), "OBJECTIDLIST"),
				// FIXME - Implement ValueObject::permanent() and call that instead?
				boost::get<bool>(valueObject->getProperty("PERMANENT"))));
    }

    SerializationFactory *SerializationFactory::instance_;

    SerializationFactory::SerializationFactory() {
        instance_ = this;
        registerCreator("rpRange", createRange);
        registerCreator("rpGroup", createGroup);

	    ProcessorPtr processor(new DefaultProcessor());
        ProcessorFactory::instance().storeProcessor("DefaultProcessor", processor);
    }

    SerializationFactory::~SerializationFactory() {
        instance_ = 0;
    }

    SerializationFactory &SerializationFactory::instance() {
        RP_REQUIRE(instance_, "Attempt to reference uninitialized SerializationFactory object");
        return *instance_;
    }

    SerializationFactory::CreatorMap &SerializationFactory::creatorMap_() const {
        static CreatorMap creatorMap;
        return creatorMap;
    }

    void SerializationFactory::registerCreator(const std::string &className, const Creator &creator) {
        creatorMap_()[className] = creator;
    }

    boost::shared_ptr<Object> SerializationFactory::recreateObject( 
        boost::shared_ptr<reposit::ValueObject> valueObject) const {

        CreatorMap::const_iterator i = creatorMap_().find(valueObject->className());
        RP_REQUIRE(i != creatorMap_().end(), "No creator for class " << valueObject->className());
        Creator creator = i->second;
        boost::shared_ptr<reposit::Object> object = creator(valueObject);
        return object;
    }

    StrObjectPair SerializationFactory::restoreObject(
        const boost::shared_ptr<reposit::ValueObject> &valueObject,
        bool overwriteExisting) const {

        StrObjectPair object;
        object.second = recreateObject(valueObject);

        // FIXME just call ValueObject::objectId()?
        object.first = boost::get<std::string>(valueObject->getProperty("OBJECTID"));
        reposit::Repository::instance().storeObject(object.first, object.second, overwriteExisting);

        return object;
    }

	int SerializationFactory::saveObjectStream(
		std::ostream& outputStream,
        const std::vector<boost::shared_ptr<Object> > objectList)
	{
        std::vector<boost::shared_ptr<reposit::ValueObject> > valueObjects;
        std::set<std::string> seen;
        std::vector<boost::shared_ptr<reposit::Object> >::const_iterator i;
        for (i=objectList.begin(); i!=objectList.end(); ++i) {
            boost::shared_ptr<reposit::Object> object = *i;
            // FIXME just call ValueObject::objectId()?
            std::string objectID
                = boost::get<std::string>(object->properties()->getProperty("OBJECTID"));
            if (seen.find(objectID) == seen.end()) {
                valueObjects.push_back(object->properties());
                seen.insert(objectID);
            }
        }

        // Provisionally comment out this sort because
        // 1) It causes legs and schedules to appear in the wrong sequence
        // 2) In our environment we can control the sequence in which objects are saved
        // 3) I don't understand why this sort is required anyway?
        //std::stable_sort(valueObjects.begin(), valueObjects.end(), compareCategory);

        boost::archive::xml_oarchive oa(outputStream);
        register_out(oa, valueObjects);
        return valueObjects.size();
	}


	int SerializationFactory::saveObjectStream(
		std::ostream& outputStream,
		const std::vector<std::string>& handlesList,
		bool includeGroups)
	{
        std::vector<boost::shared_ptr<reposit::Object> > ObjectListObjPtr =
            reposit::getObjectVector<reposit::Object>(handlesList, 0, includeGroups);
		return saveObjectStream(outputStream, ObjectListObjPtr);
	}

	int SerializationFactory::saveObject(
		const std::vector<std::string>& handlesList,
		const std::string &path,
		bool forceOverwrite,
		bool includeGroups) 
	{
        std::vector<boost::shared_ptr<reposit::Object> > ObjectListObjPtr =
            reposit::getObjectVector<reposit::Object>(handlesList, 0, includeGroups);

		return saveObject(ObjectListObjPtr, path, forceOverwrite);
	}

    int SerializationFactory::saveObject(
        const std::vector<boost::shared_ptr<reposit::Object> >& objectList,
        const std::string &path,
        bool forceOverwrite)  {

        RP_REQUIRE(objectList.size(), "Object list is empty");

        // Create a boost path object from the char*.
        boost::filesystem::path boostPath(path);

        // If a parent directory has been specified then ensure it exists.
        if ( !boostPath.parent_path().empty() ) {
            RP_REQUIRE(boost::filesystem::exists(boostPath.branch_path()),
                       "Invalid parent path : " << path);
        }

        // If the file itself exists then ensure we can overwrite it.
        if (boost::filesystem::exists(boostPath)) {
            if (forceOverwrite) {
                try {
                    boost::filesystem::remove(boostPath);
#if BOOST_VERSION < 105000
                } catch (const boost::filesystem::basic_filesystem_error<boost::filesystem::path>&) {
#else
                } catch (const boost::filesystem::filesystem_error&) {
#endif
                    RP_FAIL("Overwrite=TRUE but overwrite failed for existing file: " << path);
                }
            } else {
                RP_FAIL("Overwrite=FALSE and the specified output file exists: " << path);
            }
        }

        std::ofstream ofs(path.c_str());
        return saveObjectStream(ofs, objectList);
    }

    /*std::string SerializationFactory::processObject(
        const boost::shared_ptr<reposit::ValueObject> &valueObject,
        bool overwriteExisting)  {

        // Code to overwrite the object ID
        //valueObject->setProperty("OBJECTID", XXX);
        CreatorMap::const_iterator i = creatorMap_().find(valueObject->className());
        RP_REQUIRE(i != creatorMap_().end(), "No creator for class " << valueObject->className());
        Creator creator = i->second;
        boost::shared_ptr<reposit::Object> object = creator(valueObject);
        std::string objectID =
            boost::any_cast<std::string>(valueObject->getProperty("OBJECTID"));
        reposit::Repository::instance().storeObject(objectID, object, overwriteExisting);
        return objectID;
    }*/

    void SerializationFactory::processPath(
        const std::string &path,
        bool overwriteExisting,
        std::vector<std::string> &processedIDs)  {

        try {

            std::ifstream ifs(path.c_str());
            boost::archive::xml_iarchive ia(ifs);
            std::vector<boost::shared_ptr<reposit::ValueObject> > valueObjects;

            register_in(ia, valueObjects);

            RP_REQUIRE(valueObjects.size(), "Object list is empty");

            std::vector<boost::shared_ptr<reposit::ValueObject> >::const_iterator i;
            int count = 0;
            for (i=valueObjects.begin(); i!=valueObjects.end(); ++i) {
                try {
                    processedIDs.push_back(
                        ProcessorFactory::instance().getProcessor(*i)->process(
                            *this, *i, overwriteExisting));
                    count++;
                } catch (const std::exception &e) {
                    RP_FAIL("Error processing item " << count << ": " << e.what());
                }
            }

        } catch (const std::exception &e) {
            RP_FAIL("Error deserializing file " << path << ": " << e.what());
        }
    }

    std::vector<std::string> SerializationFactory::loadObject(
        const std::string &directory,
        const std::string &pattern,
        bool recurse,
        bool overwriteExisting)  {

        boost::filesystem::path boostPath(directory);
        RP_REQUIRE(boost::filesystem::exists(boostPath) && boost::filesystem::is_directory(boostPath),
            "The specified directory is not valid : " << directory);

        std::vector<std::string> returnValue;
        bool fileFound = false;
        boost::regex r(pattern, boost::regex::perl | boost::regex::icase);

        if (recurse) {

            for (boost::filesystem::recursive_directory_iterator itr(boostPath);
                itr != boost::filesystem::recursive_directory_iterator(); ++itr) {
#if BOOST_VERSION < 105000
                    if (regex_match(itr->path().leaf(), r) &&
#else
                    if (regex_match(itr->path().leaf().string(), r) &&
#endif
                                    boost::filesystem::is_regular(itr->status())) {
                        fileFound = true;
                        processPath(itr->path().string(), overwriteExisting, returnValue);
                    }
            }

        } else {

            for (boost::filesystem::directory_iterator itr(boostPath);
                itr != boost::filesystem::directory_iterator(); ++itr) {
#if BOOST_VERSION < 105000
                    if (regex_match(itr->path().leaf(), r) &&
#else
                    if (regex_match(itr->path().leaf().string(), r) &&
#endif
                                    boost::filesystem::is_regular(itr->status())) {
                        fileFound = true;
                        processPath(itr->path().string(), overwriteExisting, returnValue);
                    }
            }

        }

        RP_REQUIRE(fileFound, "Found no files matching pattern '" << pattern << "' in directory '"
            << directory << "' with recursion = " << std::boolalpha << recurse);

        // processPath() will already have thrown if empty files were detected
        // so the following is a redundant sanity check.
        RP_REQUIRE(!returnValue.empty(), "No objects loaded from directory : " << directory);

        ProcessorFactory::instance().postProcess();

        return returnValue;
    }

    std::string SerializationFactory::saveObjectString(
        const std::vector<boost::shared_ptr<reposit::Object> > &objectList,
        bool forceOverwrite /* TODO : we need to remove this arg */) {

        RP_REQUIRE(objectList.size(), "Object list is empty");
        std::ostringstream os;
		saveObjectStream(os, objectList);
		return os.str();
    }

	std::vector<std::string> SerializationFactory::loadObjectString(
        const std::string &xml,
        bool overwriteExisting) {
        std::istringstream xmlStream(xml);
		return loadObjectStream(xmlStream, overwriteExisting);
	}

    std::vector<std::string> SerializationFactory::loadObjectStream(
        std::istream& xmlStream,
        bool overwriteExisting) {

        std::vector<std::string> returnValue;

        try {
            boost::archive::xml_iarchive ia(xmlStream);

            std::vector<boost::shared_ptr<reposit::ValueObject> > valueObjects;
            register_in(ia, valueObjects);

            RP_REQUIRE(valueObjects.size(), "Object list is empty");

            std::vector<boost::shared_ptr<reposit::ValueObject> >::const_iterator i;
            int count = 0;
            for (i=valueObjects.begin(); i!=valueObjects.end(); ++i) {
                try {
                    returnValue.push_back(
                        ProcessorFactory::instance().getProcessor(*i)->process(
                            *this, *i, overwriteExisting));
                    count++;
                } catch (const std::exception &e) {
                    RP_FAIL("Error processing item " << count << ": " << e.what());
                }
            }
            ProcessorFactory::instance().postProcess();

        } catch (const std::exception &e) {
            RP_FAIL("Error deserializing xml : " << e.what());
        }

        RP_REQUIRE(!returnValue.empty(), "No objects loaded from xml");

        return returnValue;
    }

}
