#include "SerialMessageHandler.h"

#include <iostream>
using namespace std;

DataAccess::Interface::SerialMessageHandler::~SerialMessageHandler () {
}


void DataAccess::Interface::SerialMessageHandler::print ( const std::string& str ) const {
   cout << str;
}

void DataAccess::Interface::SerialMessageHandler::print ( const int val ) const {
   cout << val;
}

void DataAccess::Interface::SerialMessageHandler::print ( const double val ) const {
   cout << val;
}


void DataAccess::Interface::SerialMessageHandler::synchronisedPrint ( const std::string& str ) const {
   cout << str;
}


void DataAccess::Interface::SerialMessageHandler::synchronisedPrint ( const int val ) const {
   cout << val;
}


void DataAccess::Interface::SerialMessageHandler::synchronisedPrint ( const double val ) const {
   cout << val;
}



void DataAccess::Interface::SerialMessageHandler::printLine ( const std::string& str ) const {
   cout << str << endl;
}


void DataAccess::Interface::SerialMessageHandler::synchronisedPrintLine ( const std::string& str ) const {
   cout << str << endl;
}



void DataAccess::Interface::SerialMessageHandler::newLine () const {
   cout << endl;
}


void DataAccess::Interface::SerialMessageHandler::synchronisedNewLine () const {
   cout << endl;
}


void DataAccess::Interface::SerialMessageHandler::flush () const {
   cout << std::flush;
}

