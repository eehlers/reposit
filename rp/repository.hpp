/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Eric Ehlers
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
    \brief Class Repository - Maintain a store of Objects
*/

#ifndef rp_repository_hpp
#define rp_repository_hpp

#include <rp/objectwrapper.hpp>
#include <rp/rpdefines.hpp>
#include <rp/iless.hpp>
#include <map>

//! reposit
/*! Namespace for reposit functionality.
*/
namespace reposit {

	//! Forward declarations
	class Group;

    //! Maintain a store of Objects.
    /*! The client application may store, retrieve, and delete Objects in
        the Repository.

        This class is implemented using a Singleton design pattern.  Rather
        than the Meyers Singleton used elsewhere, Repository uses a specialized
        Singleton supporting inheritance so that the Repository can be customized
        for specific platforms.

        This class is designed so that it can be exported across DLL
        boundaries on the Windows platform.
    */
    class DLL_API Repository {
    public:
        //! \name Structors and static members
        //@{
        //! Constructor - initialize the singleton.
        Repository();
        //! Destructor - de-initialize the singleton.
        virtual ~Repository();
        //! Client applications access the global object via a call to Repository::instance().
        static Repository &instance();
        //@}

        //! \name Object Management
        //@{
        //! Store an Object with the given ID.
        /*! Any existing Object with that ID is deleted.

            Storage of objects uses "case-preserving" behavior:
            \code
                storeObject("MyObject")         // store "MyObject"
                retrieveObject("MyObJeCt")      // retrieve "MyObject"
                storeObject("MYOBJECT")         // overwrite "MyObject"
            \endcode
        */
        virtual std::string storeObject(const std::string &objectID,
                                        const boost::shared_ptr<Object> &object,
                                        bool overwrite = false,
                                        boost::shared_ptr<ValueObject> valueObject = boost::shared_ptr<ValueObject>());

        //! Template member function to retrieve the Object with given ID.
        /*! Retrieve the object with the given ID and downcast it to the desired type.
            Throw an exception if no Object exists with that ID.
            This template passes the work off to function retrieveObjectImpl which
            may be overridden in derived classes.
        */
        template <class T>
        void retrieveObject(boost::shared_ptr<T> &ret,
                            const std::string &id) {
            boost::shared_ptr<Object> object = retrieveObjectImpl(id);
            ret = boost::dynamic_pointer_cast<T>(object);
            RP_REQUIRE(ret, "Error retrieving object with id '"
                << id << "' - unable to convert reference to type '"
                << typeid(T).name() << "' found instead '"
                << typeid(*object).name() << "'");
        }

        //! Override of template function retrieveObject.
        /*! Specialized for the case where the client has requested a reference to
            class Object and no downcast is necessary.
        */
        void retrieveObject(boost::shared_ptr<Object> &ret,
                            const std::string &id);

        //! Default implementation of retrieveObjectImpl.
        /*! Retrieves the Object with the given ID.
            Throws an exception if no Object exists with that ID.
            This member function may be overridden in base classes which customize
            the Repository class for platform-specific functionality.
        */
        virtual boost::shared_ptr<Object> retrieveObjectImpl(const std::string &objectID);
        
        //! Delete the object with the given ID.
        /*! Delete the object regardless of whether or not it is permanent.
            Take no action if no object exists with that ID.
        */
        virtual void deleteObject(const std::string &objectID);

        //! Delete a list of objects with the given IDs.
        /*! Delete the objects regardless of whether or not they are permanent.
            Take no action for IDs that do not correspond to valid objects.
        */
        virtual void deleteObject(const std::vector<std::string> &objectIDs);

        //! Delete all of the Objects in the Repository.
        /*! Take no action if the Repository is already empty.
        */
        virtual void deleteAllObjects(const bool &deletePermanent = false);
        //@}

        //! \name Logging
        //@{
        //! Log the indicated Object to the given stream.
        /*! If no Object exists with that ID then an appropriate message is written
            to the stream.
        */
        virtual void dumpObject(const std::string &objectID, std::ostream&);
        //! Log dump of the Repository.
        /*! Write all of the Objects in the Repository to the given output stream.
        */
        virtual void dump(std::ostream&);
        //@}

        //! \name Utilities
        //@{
        //! Count of all the Objects in the Repository.
        virtual int objectCount();

        //! List the IDs of all the Objects in the Repository.
        /*! Returns an empty list if the Repository is empty.
        */
        virtual const std::vector<std::string> listObjectIDs(
            const std::string &regex = "");

        //! Indicate whether an Object with the given ID is found in the Repository.
        virtual std::vector<bool> objectExists(const std::vector<std::string> &objectList);
        //@}

        //! Define the type of the structure used to store the Objects.
        /*! The Repository class cannot declare a private data member of type
            ObjectMap, because std::map cannot be exported across DLL boundaries
            on the Windows platform.  Instead the map is declared as a static
            variable in the cpp file.
        */
        typedef std::map<std::string, boost::shared_ptr<ObjectWrapper>, my_iless> ObjectMap;

        //! \name Precedent object IDs and timestamps
        //@{
        //! Retrieve the list of IDs of precedent objects
        virtual const std::vector<std::string> precedentIDs(const std::string &objectID);
        //! The object's initial creation time
        virtual std::vector<double> creationTime(const std::vector<std::string> &objectList);
        //! The time of the object's last update
        virtual std::vector<double> updateTime(const std::vector<std::string> &objectList);
        //@}

        //! get the object's permanent proterty
        virtual std::vector<bool> isPermanent(const std::vector<std::string> &objectList);
        //! get the object's name
        virtual const std::vector<std::string> className(const std::vector<std::string> &objectList);
        //@}


    protected:
        //! A pointer to the Repository instance, used to support the Singleton pattern.
        static Repository *instance_;
        //! Get the object ObjectWrapper from ObjectMap
        virtual const boost::shared_ptr<ObjectWrapper>& getObjectWrapper(const std::string &objectID) const;

        //! Register an ObjectWrapper as an Observer of its precedents
        /*! The given ObjectWrapper is registered as an Observer of all of its
            precedent ObjectWrappers, which in this case act as Observables.
            If any of the precedents changes, then the Observer is notified.
        */
        virtual void registerObserver( 
            boost::shared_ptr<ObjectWrapper> objWrapper);

        //! Convert Excel-format Object IDs into the format recognized by the base Repository class
        /*! The functiong will be used in derived class(e.g in class
            repositoryXL it will change the objectID custom_#0001 into custom);
        */
        virtual std::string formatID(const std::string &objectID);

        //! Indicate whether an Object with the given ID is found in the Repository.
        virtual bool objectExists(const std::string &objectID) const;

        //! Retrieve the list of IDs of precedent objects containde in this group
		virtual const std::vector<std::string> precedentIDs(const boost::shared_ptr<Group>& group);

    };

}

#endif

