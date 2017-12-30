
/*
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

#include <rpxl/repositxl.hpp>
#include <rpxl/utilities/xlutilities.hpp>
#include <ExampleObjects/accountexample.hpp>

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#ifdef BOOST_MSVC
#  define BOOST_LIB_DIAGNOSTIC
#  include <rp/auto_link.hpp>
#  include <xlsdk/auto_link.hpp>
#  undef BOOST_LIB_DIAGNOSTIC
#endif
#include <sstream>

DLLEXPORT int xlAutoOpen() {

	//Instantiate the Processor Factory
	static reposit::ProcessorFactory processorFactory;
    // Instantiate the Serialization Factory
    static AccountExample::SerializationFactory factory;

    static XLOPER xDll;

    try {

        AccountExample::registerEnumeratedTypes();

        Excel(xlGetName, &xDll, 0);

        Excel(xlfRegister, 0, 7, &xDll,
            TempStrNoSize("\x14""addin1CreateCustomer"),// function code name
            TempStrNoSize("\x06""CCCNL#"),              // parameter codes
            TempStrNoSize("\x14""addin1CreateCustomer"),// function display name
            TempStrNoSize("\x1B""ObjectID,Name,Age,Permanent"), // comma-delimited list of parameters
            TempStrNoSize("\x01""1"),                   // function type (0 = hidden function, 1 = worksheet function, 2 = command macro)
            TempStrNoSize("\x07""Example"));            // function category

        Excel(xlfRegister, 0, 7, &xDll,
            TempStrNoSize("\x13""addin1CreateAccount"), // function code name
            TempStrNoSize("\x08""CCCCNPL#"),            // parameter codes
            TempStrNoSize("\x13""addin1CreateAccount"), // function display name
            TempStrNoSize("\x2F""ObjectID,Customer,Type,Number,Balance,Permanent"), // comma-delimited list of parameters
            TempStrNoSize("\x01""1"),                   // function type (0 = hidden function, 1 = worksheet function, 2 = command macro)
            TempStrNoSize("\x07""Example"));            // function category

        Excel(xlfRegister, 0, 7, &xDll,
            TempStrNoSize("\x10""addin1SetBalance"),    // function code name
            TempStrNoSize("\x04""LCE#"),                // parameter codes
            TempStrNoSize("\x10""addin1SetBalance"),    // function display name
            TempStrNoSize("\x10""ObjectID,Balance"),    // comma-delimited list of parameters
            TempStrNoSize("\x01""1"),                   // function type (0 = hidden function, 1 = worksheet function, 2 = command macro)
            TempStrNoSize("\x07""Example"));            // function category

        Excel(xlfRegister, 0, 7, &xDll,
            TempStrNoSize("\x05""func1"),               // function code name
            TempStrNoSize("\x04""CCP#"),                // parameter codes
            TempStrNoSize("\x05""func1"),               // function code name
            TempStrNoSize("\x10""ObjectID,Trigger"),    // comma-delimited list of parameters
            TempStrNoSize("\x01""1"),                   // function type (0 = hidden function, 1 = worksheet function, 2 = command macro)
            TempStrNoSize("\x07""Example"));            // function category

        Excel(xlFree, 0, 1, &xDll);
        return 1;

    } catch (const std::exception &e) {

        std::ostringstream err;
        err << "Error loading ExampleXllDynamic1: " << e.what();
        Excel(xlcAlert, 0, 1, TempStrStl(err.str()));
        Excel(xlFree, 0, 1, &xDll);
        return 0;

    } catch (...) {

        Excel(xlFree, 0, 1, &xDll);
        return 0;

    }
}

DLLEXPORT int xlAutoClose() {

    try {

        AccountExample::unregisterEnumeratedTypes();
        return 1;

    } catch (const std::exception &e) {

        std::ostringstream err;
        err << "Error unloading ExampleXllDynamic1: " << e.what();
        Excel(xlcAlert, 0, 1, TempStrStl(err.str()));
        return 0;

    } catch (...) {

        return 0;

    }

}

DLLEXPORT void xlAutoFree(XLOPER *px) {

    freeOper(px);

}

DLLEXPORT char *addin1CreateCustomer(
        char *objectID,
        char *name,
        long *age,
        bool *permanent) {

    boost::shared_ptr<reposit::FunctionCall> functionCall;

    try {

        functionCall = boost::shared_ptr<reposit::FunctionCall>
            (new reposit::FunctionCall("addin1CreateCustomer"));

        boost::shared_ptr<reposit::ValueObject> valueObject(
            new AccountExample::CustomerValueObject(objectID, name, *age, *permanent));

        boost::shared_ptr<reposit::Object> object(
            new AccountExample::CustomerObject(valueObject, name, *age, *permanent));

        std::string returnValue = 
            reposit::RepositoryXL::instance().storeObject(objectID, object);

        static char ret[XL_MAX_STR_LEN];
        reposit::stringToChar(returnValue, ret);
        return ret;

    } catch (const std::exception &e) {

        reposit::RepositoryXL::instance().logError(e.what(), functionCall);
        return 0;

    }
}

DLLEXPORT char *addin1CreateAccount(
        char *objectID,
        char *customer,
        char *type,
        long *number,
        OPER *balance,
        bool *permanent) {

    boost::shared_ptr<reposit::FunctionCall> functionCall;

    try {

        functionCall = boost::shared_ptr<reposit::FunctionCall>
            (new reposit::FunctionCall("addin1CreateAccount"));

        RP_GET_REFERENCE(customerRef, customer,
            AccountExample::CustomerObject, AccountExample::Customer)

        double balanceDouble = reposit::convert<double>(
            reposit::ConvertOper(*balance), "balance", 100.00);

        reposit::property_t balanceProperty =
            reposit::convert<reposit::property_t>(
            reposit::ConvertOper(*balance), "balance");

        AccountExample::Account::Type typeEnum =
            reposit::Create<AccountExample::Account::Type>()(type);

        boost::shared_ptr<reposit::ValueObject> valueObject(
            new AccountExample::AccountValueObject(objectID, customer, type, *number, balanceProperty, *permanent));

        boost::shared_ptr<reposit::Object> object(
            new AccountExample::AccountObject(valueObject, customerRef, typeEnum, *number, balanceDouble, *permanent));

        std::string returnValue = 
            reposit::RepositoryXL::instance().storeObject(objectID, object, true);

        static char ret[XL_MAX_STR_LEN];
        reposit::stringToChar(returnValue, ret);
        return ret;

    } catch (const std::exception &e) {

        reposit::RepositoryXL::instance().logError(e.what(), functionCall);
        return 0;

    }
}

DLLEXPORT short int *addin1SetBalance(char *objectID, double *balance) {

    boost::shared_ptr<reposit::FunctionCall> functionCall;

    try {

        functionCall = boost::shared_ptr<reposit::FunctionCall>
            (new reposit::FunctionCall("addin1SetBalance"));

        RP_GET_OBJECT(accountObject, objectID, AccountExample::AccountObject)

        accountObject->setBalance(*balance);
        static short int ret = TRUE;
        return &ret;

    } catch (const std::exception &e) {

        reposit::RepositoryXL::instance().logError(e.what(), functionCall);
        return 0;

    }
}

DLLEXPORT char *func1(char *objectID, OPER *trigger) {

    boost::shared_ptr<reposit::FunctionCall> functionCall;

    try {

        functionCall = boost::shared_ptr<reposit::FunctionCall>
            (new reposit::FunctionCall("func1"));

        AccountExample::Account::Type accountTypeEnum =
            reposit::Create<AccountExample::Account::Type>()(objectID);

        std::ostringstream s;
        s << accountTypeEnum;
        static char ret[XL_MAX_STR_LEN];
        reposit::stringToChar(s.str(), ret);
        return ret;

    } catch (const std::exception &e) {

        reposit::RepositoryXL::instance().logError(e.what(), functionCall);
        return 0;

    }
}
