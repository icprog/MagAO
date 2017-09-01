/*
 * 						*** ATTENTION ***
 * This file contains boost intializations that must be done only once!
 * PLEASE include it in the object containing the main.
 */

#ifndef AOCOMMANDSEXPORT_H_INCLUDE
#define AOCOMMANDSEXPORT_H_INCLUDE

// ALL COMMANDS ARE DEFINED HERE
#include "arblib/aoArb/AOCommands.h"

using namespace Arcetri::Arbitrator;

#include <boost/serialization/export.hpp>

// Requests
BOOST_CLASS_EXPORT_GUID(AdjustGain, "AdjustGain")
BOOST_CLASS_EXPORT_GUID(AdjustIntTime, "AdjustIntTime")


// Commands
BOOST_CLASS_EXPORT_GUID(PowerOnWfs, "PowerOnWfs")
BOOST_CLASS_EXPORT_GUID(PowerOffWfs, "PowerOffWfs")
BOOST_CLASS_EXPORT_GUID(PowerOnAdSec, "PowerOnAdSec")
BOOST_CLASS_EXPORT_GUID(PowerOffAdSec, "PowerOffAdSec")
BOOST_CLASS_EXPORT_GUID(SetAOMode, "SetAOMode")

BOOST_CLASS_EXPORT_GUID(PresetFlat, "PresetFlat")
BOOST_CLASS_EXPORT_GUID(PresetAO, "PresetAO")
BOOST_CLASS_EXPORT_GUID(AcquireRefAO, "AcquireRefAO")
BOOST_CLASS_EXPORT_GUID(CheckRefAO, "CheckRefAO")
BOOST_CLASS_EXPORT_GUID(CorrectModes, "CorrectModes")
BOOST_CLASS_EXPORT_GUID(SetZernikes, "SetZernikes")
BOOST_CLASS_EXPORT_GUID(ModifyAO, "ModifyAO")
BOOST_CLASS_EXPORT_GUID(OffsetXY, "OffsetXY")
BOOST_CLASS_EXPORT_GUID(OffsetZ, "OffsetZ")
BOOST_CLASS_EXPORT_GUID(Pause, "Pause")
BOOST_CLASS_EXPORT_GUID(RefineAO, "RefineAO")
BOOST_CLASS_EXPORT_GUID(Resume, "Resume")
BOOST_CLASS_EXPORT_GUID(StartAO, "StartAO")
BOOST_CLASS_EXPORT_GUID(MirrorSet, "MirrorSet")
BOOST_CLASS_EXPORT_GUID(MirrorRest, "MirrorRest")
BOOST_CLASS_EXPORT_GUID(FaultRecovery, "FaultRecovery")
BOOST_CLASS_EXPORT_GUID(StandaloneFail, "StandaloneFail")
BOOST_CLASS_EXPORT_GUID(SeeingLmtdFail, "SeeingLmtdFail")

BOOST_CLASS_EXPORT_GUID(Stop, "Stop")
BOOST_CLASS_EXPORT_GUID(RecoverAdSecFailure, "RecoverAdSecFailure")
BOOST_CLASS_EXPORT_GUID(RecoverWfsFailure, "RecoverWfsFailure")

BOOST_CLASS_EXPORT_GUID(UserPanic, "UserPanic")
BOOST_CLASS_EXPORT_GUID(RequestAOStatus, "RequestAOStatus")

#endif
