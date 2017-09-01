/*
 * 						*** ATTENTION ***
 * This file contains boost intializations that must be done only once!
 * PLEASE include it in the object containing the main.
 */

#ifndef ALERTSEXPORT_H_INCLUDE
#define ALERTSEXPORT_H_INCLUDE

#include "arblib/base/Alerts.h"

using namespace Arcetri::Arbitrator;
#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_GUID(Warning, "Warning")
BOOST_CLASS_EXPORT_GUID(Error, "Error")
BOOST_CLASS_EXPORT_GUID(Panic, "Panic")
BOOST_CLASS_EXPORT_GUID(HighOrderOffload, "HighOrderOffload")
BOOST_CLASS_EXPORT_GUID(DumpSaved, "DumpSaved")

#endif
