#pragma once

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include "RE/Starfield.h"
#include "SFSE/SFSE.h"

#include "REL/THook.h"
#include "RE/IDs_VTABLE.h"

#include "RE/A/Actor.h"
#include "RE/I/INIPrefSettingCollection.h"
#include "RE/I/INISettingCollection.h"
#include "RE/G/GameSettingCollection.h"
#include "RE/M/MenuTopicManager.h"
#include "RE/N/NiCamera.h"
#include "RE/N/NiAVObject.h"
#include "RE/N/NiNode.h"
#include "RE/N/NiUpdateData.h"
#include "RE/P/PlayerCamera.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TESDataHandler.h"
#include "RE/T/TESTopicInfo.h"
#include "RE/T/TESResponse.h"
#include "RE/A/AIProcess.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/U/UI.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstring>
#include <format>
#include <mutex>
#include <limits>
#include <optional>
#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
