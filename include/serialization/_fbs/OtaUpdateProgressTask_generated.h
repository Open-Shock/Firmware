// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_OTAUPDATEPROGRESSTASK_OPENSHOCK_SERIALIZATION_TYPES_H_
#define FLATBUFFERS_GENERATED_OTAUPDATEPROGRESSTASK_OPENSHOCK_SERIALIZATION_TYPES_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 25 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 24,
             "Non-compatible flatbuffers version included");

namespace OpenShock {
namespace Serialization {
namespace Types {

enum class OtaUpdateProgressTask : uint8_t {
  FetchingMetadata = 0,
  PreparingForUpdate = 1,
  FlashingFilesystem = 2,
  VerifyingFilesystem = 3,
  FlashingApplication = 4,
  MarkingApplicationBootable = 5,
  Rebooting = 6,
  MIN = FetchingMetadata,
  MAX = Rebooting
};

inline const OtaUpdateProgressTask (&EnumValuesOtaUpdateProgressTask())[7] {
  static const OtaUpdateProgressTask values[] = {
    OtaUpdateProgressTask::FetchingMetadata,
    OtaUpdateProgressTask::PreparingForUpdate,
    OtaUpdateProgressTask::FlashingFilesystem,
    OtaUpdateProgressTask::VerifyingFilesystem,
    OtaUpdateProgressTask::FlashingApplication,
    OtaUpdateProgressTask::MarkingApplicationBootable,
    OtaUpdateProgressTask::Rebooting
  };
  return values;
}

inline const char * const *EnumNamesOtaUpdateProgressTask() {
  static const char * const names[8] = {
    "FetchingMetadata",
    "PreparingForUpdate",
    "FlashingFilesystem",
    "VerifyingFilesystem",
    "FlashingApplication",
    "MarkingApplicationBootable",
    "Rebooting",
    nullptr
  };
  return names;
}

inline const char *EnumNameOtaUpdateProgressTask(OtaUpdateProgressTask e) {
  if (::flatbuffers::IsOutRange(e, OtaUpdateProgressTask::FetchingMetadata, OtaUpdateProgressTask::Rebooting)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesOtaUpdateProgressTask()[index];
}

}  // namespace Types
}  // namespace Serialization
}  // namespace OpenShock

#endif  // FLATBUFFERS_GENERATED_OTAUPDATEPROGRESSTASK_OPENSHOCK_SERIALIZATION_TYPES_H_
