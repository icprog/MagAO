/*
 * 						*** ATTENTION ***
 * This file contains boost intializations that must be done only once!
 * PLEASE include it in the object containing the main.
 */

#ifndef COMMANDSEXPORT_H_INCLUDE
#define COMMANDSEXPORT_H_INCLUDE

#include "arblib/base/Commands.h"

//using namespace Arcetri::Wfs_Arbitrator;

#include <boost/serialization/export.hpp>

using namespace Arcetri::Arbitrator;

// Commands
BOOST_CLASS_EXPORT_GUID(Ping, "Ping")
BOOST_CLASS_EXPORT_GUID(RequestFeedback, "RequestFeedback")
BOOST_CLASS_EXPORT_GUID(RequestStatus, "RequestStatus")
BOOST_CLASS_EXPORT_GUID(Arcetri::Arbitrator::RecoverFailure, "RecoverFailure")   // Namespace specified because AdSecArbitrator::RecoverFailure also exists...
BOOST_CLASS_EXPORT_GUID(RecoverPanic, "RecoverPanic")

#endif
