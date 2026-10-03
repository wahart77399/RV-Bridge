#include "Arduino.h"
#define CUSTOM_CHAR_HEADER
#include "HomeSpan.h"
#include "DeviceFactory.h"
#include "RVConstants.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "LittleFS.h"
#include "debug.h"


DeviceFactory* DeviceFactory::instance = nullptr;
uint32_t DeviceFactory::chassisAccessoryAid = 0;
// DGN -> instance -> device; the maps do not own the devices
std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DeviceFactory::DGN2DeviceMap;

namespace {
    uint32_t accessoryCount(const String& type) {
        if (type == "Thermostat") return 3;
        if (type == "Inverter" || type == "Generator") return 4;
        if (type == "ATS") return 8;
        if (type == "Charger") return 7;
        return 1;
    }

    bool replaceJsonFile(const char* path, const char* tempPath, const char* backupPath,
                         JsonDocument& document) {
        bool ok = false;
        LittleFS.remove(tempPath);
        File output = LittleFS.open(tempPath, "w");
        if (output) {
            size_t expected = measureJson(document);
            size_t written = serializeJsonPretty(document, output);
            output.flush();
            output.close();
            if (written == expected) {
                LittleFS.remove(backupPath);
                if (LittleFS.rename(path, backupPath)) {
                    if (LittleFS.rename(tempPath, path)) {
                        LittleFS.remove(backupPath);
                        ok = true;
                    } else {
                        LittleFS.rename(backupPath, path);
                    }
                }
            }
        }
        if (!ok) LittleFS.remove(tempPath);
        return ok;
    }
}

#include "ChassisMobility.h"

#ifdef HOME_KIT_1
#include "LightDevice.h"
#include "LightDeviceView.h"

#include "DoorLock.h"
#include "DoorLockView.h"


#include "WaterPump.h"
#include "WaterPumpView.h"
#include "Thermostat.h"
#include "ThermostatView.h"
#endif


#ifdef HOME_KIT_2
#include "Generator.h"
#include "GeneratorView.h"
#include "Tanks.h"
#include "TanksView.h"
#include "AutomaticTransferSwitch.h"
#include "AutomaticTransferSwitchView.h"
#include "Battery.h"
#include "BatteryView.h"
#include "Inverter.h"
#include "InverterView.h"
#include "FloorHeat.h"
#include "FloorHeatView.h"
#include "CoverDevice.h"
#include "CoverView.h"
#include "Charger.h"
#endif

#include "ChassisMobilityView.h"
DeviceFactory* DeviceFactory::getInstance() {
    if (!DeviceFactory::instance) {
        DeviceFactory::instance = new DeviceFactory();
#ifdef HOME_KIT_1
        ChassisMobility* chassis = ChassisMobility::getInstance();
        const uint8_t defaultChassisIndex = DEFAULT_CHASSIS_INDEX;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_COMMAND][defaultChassisIndex] = chassis;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_STATUS][defaultChassisIndex] = chassis;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_STATUS_2][defaultChassisIndex] = chassis;
        SpanView::setNextAccessoryAid(chassisAccessoryAid);
        ChassisMobilityView::createChassisMobilityView((GenericDevice* )chassis, "Chassis Mobility Sensor");
#endif
    }
    return DeviceFactory::instance;
}

bool DeviceFactory::instanceFromData(RVC_DGN dgn, uint8_t* data, uint8_t& index) {
    bool found = false;
    if (data != nullptr) {
        if ((dgn == WATER_PUMP_COMMAND) || (dgn == WATER_PUMP_STATUS)) {
#ifdef HOME_KIT_1
            index = WATER_PUMP_INDEX;
            found = true;
#endif
        } else if ((dgn == ATS_AC_STATUS_1) || (dgn == ATS_AC_STATUS_2) || (dgn == ATS_AC_STATUS_3) || (dgn == ATS_AC_STATUS_4)) {
#ifdef HOME_KIT_2
            // RV-C packs the ATS instance into the low bits of byte 0
            uint8_t tmp = data[AutomaticTransferSwitch::ATS_BYTE_0] & AutomaticTransferSwitch::ATS_STATUS_INDEX_MASK;
            if ((tmp > ATS_INSTANCE_0_INVALID) && (tmp < ATS_INSTANCE_7_INVALID)) {
                index = tmp;
                found = true;
            }
#endif
        } else if ((dgn == INVERTER_AC_STATUS_1) || (dgn == INVERTER_STATUS)) {
#ifdef HOME_KIT_2
            uint8_t tmp = data[INVERTER_LINE_INDEX];
            if (dgn == INVERTER_AC_STATUS_1) {
                tmp = tmp & INVERTER_INSTANCE_MASK;
            }
            if (tmp != INVERTER_INVALID) {
                index = tmp;
                found = true;
            }
#endif
        } else if ((dgn == GENERATOR_AC_STATUS_1) || (dgn == GENERATOR_AC_STATUS_2) || (dgn == GENERATOR_AC_STATUS_3) || (dgn == GENERATOR_AC_STATUS_4)) {
#ifdef HOME_KIT_2
            uint8_t tmp = data[Generator::GENERATOR_BYTE_0] & Generator::GENERATOR_OUTPUT_INDEX_MASK;
            if ((tmp > static_cast<uint8_t>(GeneratorInstance::GENERATOR_INSTANCE_0_INVALID)) &&
                (tmp < static_cast<uint8_t>(GeneratorInstance::GENERATOR_INSTANCE_11_INVALID))) {
                index = tmp;
                found = true;
            }
#endif
        } else {
            index = Packet::getIndex(data);
            found = true;
        }
    }
    return found;
}

GenericDevice* DeviceFactory::getDeviceByData(RVC_DGN dgn, uint8_t* data) {
    GenericDevice* result = nullptr;
    uint8_t index = 0;
    if (instanceFromData(dgn, data, index)) {
        // find() rather than [] so unknown bus traffic doesn't grow the map
        auto byDgn = DGN2DeviceMap.find(dgn);
        if (byDgn != DGN2DeviceMap.end()) {
            auto byIndex = byDgn->second.find(index);
            if (byIndex != byDgn->second.end()) {
                result = byIndex->second;
            }
        }
    }
    return result;
}

std::map<String, DeviceFactory::DeviceCreator> DeviceFactory::creators;

void DeviceFactory::registerCreators() {

    // ============================================================
    // HOME_KIT_1
    // ============================================================
#ifdef HOME_KIT_1

    creators["DC_DimmableSwitch"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new LightDevice(c.sourceAddress, c.rvcIndex, LightKind::Dimmable);
        DGN2DeviceMap[DC_DIMMER_COMMAND][c.rvcIndex]  = d;
        DGN2DeviceMap[DC_DIMMER_STATUS_3][c.rvcIndex] = d;
        LightDeviceView::createLightDeviceView(d, c.name.c_str());
        return d;
    };

    creators["DC_Switch"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new LightDevice(c.sourceAddress, c.rvcIndex, LightKind::OnOff);   // adjust class name if different
        DGN2DeviceMap[DC_DIMMER_COMMAND][c.rvcIndex] = d;
        LightDeviceView::createLightDeviceView(d, c.name.c_str());
        return d;
    };

    creators["DoorLock"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new DoorLock(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[LOCK_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[LOCK_STATUS][c.rvcIndex]  = d;
        DoorLockView::createDoorLockView(d, c.name.c_str());
        return d;
    };

    creators["WaterPump"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        // WaterPump has a single fixed instance
        auto* d = new WaterPump(c.sourceAddress);
        DGN2DeviceMap[WATER_PUMP_COMMAND][WATER_PUMP_INDEX] = d;
        DGN2DeviceMap[WATER_PUMP_STATUS][WATER_PUMP_INDEX]  = d;
        WaterPumpView::createWaterPumpView(d, c.name.c_str());
        return d;
    };

    creators["Thermostat"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new HVAC_Thermostat(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[THERMOSTAT_COMMAND_1][c.rvcIndex] = d;
        DGN2DeviceMap[THERMOSTAT_COMMAND_2][c.rvcIndex] = d;
        DGN2DeviceMap[THERMOSTAT_STATUS_1][c.rvcIndex]  = d;
        DGN2DeviceMap[THERMOSTAT_STATUS_2][c.rvcIndex]  = d;
        ThermostatView::createThermostatView(d, c.name.c_str());
        return d;
    };

#endif // HOME_KIT_1

    // ============================================================
    // HOME_KIT_2
    // ============================================================
#ifdef HOME_KIT_2

    creators["FloorHeat"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new FloorHeat(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[FLOOR_HEAT_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[FLOOR_HEAT_STATUS][c.rvcIndex]  = d;
        FloorHeatView::createFloorHeatView(d, c.name.c_str());
        return d;
    };

    creators["Awning"] = [](const DeviceConfig& c, const CoachSpec& coach) -> GenericDevice* {
        GenericDevice* result = nullptr;
        if (c.enabled) {
            auto* d = new CoverDevice(c.sourceAddress, c.rvcIndex, CoverKind::Awning);

            uint16_t ext = static_cast<uint16_t>(c.extraUInt("extendSec", 40));
            uint16_t ret = static_cast<uint16_t>(c.extraUInt("retractSec", 45));

            CoverTiming out;

            if (coach.findCoverTiming(c.name, out)) {
                ext = out.extendSec;
                ret = out.retractSec;
            }
            d->configureTimings(ext, ret);

            DGN2DeviceMap[AWNING_COMMAND][c.rvcIndex] = d;
            DGN2DeviceMap[AWNING_STATUS][c.rvcIndex]  = d;

            CoverView::createCoverView(d, c.name.c_str());
            result = d;
        }
        return result;
    };

    creators["Shades"] = [](const DeviceConfig& c, const CoachSpec& coach) -> GenericDevice* {
        GenericDevice* result = nullptr;
        if (c.enabled) {
            auto* d = new CoverDevice(c.sourceAddress, c.rvcIndex, CoverKind::Shade);

            uint16_t ext = static_cast<uint16_t>(c.extraUInt("extendSec", 40));
            uint16_t ret = static_cast<uint16_t>(c.extraUInt("retractSec", 45));

            CoverTiming out;

            if (coach.findCoverTiming(c.name, out)) {
                ext = out.extendSec;
                ret = out.retractSec;
            }
            d->configureTimings(ext, ret);

            DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][c.rvcIndex] = d;
            DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][c.rvcIndex]  = d;

            CoverView::createCoverView(d, c.name.c_str());
            result = d;
        }
        return result;
    };

    creators["Tank"] = [](const DeviceConfig& c, const CoachSpec& coach) -> GenericDevice* {
        // Prefer the instance from the coach spec when possible
        GenericDevice* result = nullptr;
        if (c.enabled) {
            uint16_t tankSz = 0;
            if (const TankSpec* ts = coach.getTank(c.rvcIndex)) {
            tankSz = ts->capacityLiters;
            }
            auto* d = new Tanks(c.sourceAddress, c.rvcIndex, tankSz);
            DGN2DeviceMap[TANK_STATUS][c.rvcIndex] = d;
            TanksView::createTanksView(d, c.name.c_str());
            result = d;
        }
        return result;
    };

    creators["Generator"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Generator(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[GENERATOR_AC_STATUS_1][c.rvcIndex] = (PowerSensor*)d;
        d->attachView(c.name.c_str());
        return d;
    };

    creators["Inverter"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Inverter(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[INVERTER_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[INVERTER_STATUS][c.rvcIndex]      = d;
        d->attachView("Inv");  // full name is too long once the per-reading suffixes are appended
        return d;
    };

    creators["Battery"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Battery(c.sourceAddress, c.rvcIndex, 0);
        DGN2DeviceMap[DC_SOURCE_STATUS_1][c.rvcIndex] = (PowerSensor*)d;
        DGN2DeviceMap[DC_SOURCE_STATUS_2][c.rvcIndex] = (PowerSensor*)d;
        DGN2DeviceMap[DC_SOURCE_STATUS_3][c.rvcIndex] = (PowerSensor*)d;
        DGN2DeviceMap[DC_SOURCE_STATUS_4][c.rvcIndex] = (PowerSensor*)d;
        BatteryView::createBatteryView((GenericDevice*)d, c.name.c_str());
        return d;
    };

    creators["ATS"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new AutomaticTransferSwitch(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[ATS_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_4][c.rvcIndex] = d;
        d->attachView(c.name.c_str());
        return d;
    };
    creators["Charger"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Charger(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[CHARGER_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_AC_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_AC_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_AC_STATUS_4][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_DC_STATUS][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_STATUS][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_STATUS_3][c.rvcIndex] = d;
        d->attachView(c.name.c_str());
        return d;
    };

#endif // HOME_KIT_2

}

bool DeviceFactory::loadCoachSpec(const char* path, CoachSpec& out) {
    bool success = false;
    File file;

    if (path != nullptr && LittleFS.begin()) {
        file = LittleFS.open(path, "r");
        if (file) {
            JsonDocument doc;   // or StaticJsonDocument<2048> if you fix the size
            DeserializationError err = deserializeJson(doc, file);
            file.close();

            if (!err) {
                out.year      = doc["year"]      | 0;
                out.make      = doc["make"]      | "";
                out.model     = doc["model"]     | "";
                out.floorplan = doc["floorplan"] | "";
                out.coachId   = doc["coachId"] | "";
                out.chassisAid = doc["chassisAid"] | 0;

                out.tanks.clear();
                JsonArray tanks = doc["tanks"].as<JsonArray>();
                for (JsonObject t : tanks) {
                    TankSpec tank;
                    tank.instance        = t["instance"]        | 0;
                    tank.capacityLiters  = t["capacityLiters"] | 0;
                    const char* n = t["name"] | "";
                    tank.name = n;
                    RV_PRINTF("DeviceFactory::loadCoachSpec instance %d, capacityLiters %d, name %s", 
                        tank.instance, tank.capacityLiters, tank.name.c_str());
                    out.tanks.push_back(tank);
                }

                out.batteries.clear();
                JsonArray batts = doc["batteries"].as<JsonArray>();
                for(JsonObject o : batts) {
                    BatterySpec b;
                    b.instance = o["instance"] | 0;
                    b.nominalVoltage = o["nominalVoltage"] | 12.0f;
                    b.capacityAh = o["capacityAh"] | 0.0f;
                    const char* n = o["name"] | "";
                    b.name = n;
                    out.batteries.push_back(b);
                }

                out.coverTimings.clear();
                JsonObject cover = doc["coverTimes"].as<JsonObject>();
                for (JsonPair kv : cover) {
                    CoverTimingPair pair;
                    pair.key = kv.key().c_str();
                    pair.timing.extendSec  = kv.value()["extendSec"]  | 40;
                    pair.timing.retractSec = kv.value()["retractSec"] | 45;
                    out.coverTimings.push_back(pair);
                }
                success = true;
            }
        }
    }
    return success;
}


bool DeviceFactory::loadDeviceConfigs(const char* path, std::vector<DeviceConfig>& out) {
    bool         success = false;
    File         file;
    bool         fileOk  = false;
    bool         pathOk  = false;
    JsonDocument doc;
    DeserializationError err = DeserializationError::Ok;
    JsonArray    devices;
    size_t       i = 0;
    size_t       n = 0;

    out.clear();

    if (path != nullptr && path[0] != '\0') {
        pathOk = true;
    }

    if (pathOk && LittleFS.begin(false)) {
        file = LittleFS.open(path, "r");
        if (file) {
            fileOk = true;
        }
    }

    if (fileOk) {
        err = deserializeJson(doc, file);
        file.close();
    }

    if (fileOk && err == DeserializationError::Ok) {
        // Root is an array: [ { ... }, { ... } ]
        devices = doc.as<JsonArray>();
        if (!devices.isNull()) {
            n = devices.size();
            i = 0;
            while (i < n) {
                JsonObject obj = devices[i].as<JsonObject>();
                DeviceConfig cfg;   // defaults from struct

                if (!obj.isNull()) {
                    if (obj["enabled"].is<bool>()) {
                        cfg.enabled = obj["enabled"].as<bool>();
                    }
                    if (obj["type"].is<const char*>()) {
                        cfg.type = obj["type"].as<const char*>();
                    }
                    if (obj["rvcIndex"].is<int>()) {
                        cfg.rvcIndex = static_cast<uint8_t>(obj["rvcIndex"].as<int>());
                    }
                    if (obj["sourceAddress"].is<int>()) {
                        cfg.sourceAddress = static_cast<uint8_t>(obj["sourceAddress"].as<int>());
                    }
                    if (obj["name"].is<const char*>()) {
                        cfg.name = obj["name"].as<const char*>();
                    }
                    if (obj["aid"].is<uint32_t>()) {
                        cfg.aid = obj["aid"].as<uint32_t>();
                    }
                    if (obj["room"].is<const char*>()) {
                        cfg.room = obj["room"].as<const char*>();
                    }
                    if (obj["order"].is<int>()) {
                        cfg.order = static_cast<uint16_t>(obj["order"].as<int>());
                    }

                    // free-form "extra": { "awningKey": "front", "extendSec": "45", ... }
                    JsonObject extraObj = obj["extra"].as<JsonObject>();
                    if (!extraObj.isNull()) {
                        for (JsonPair kv : extraObj) {
                            ExtraPair pair;
                            pair.key = kv.key().c_str();
                            if (kv.value().is<const char*>()) {
                                pair.value = kv.value().as<const char*>();
                            } else if (kv.value().is<int>()) {
                                pair.value = String(kv.value().as<int>());
                            } else if (kv.value().is<bool>()) {
                                pair.value = kv.value().as<bool>() ? "1" : "0";
                            } else {
                                pair.value = "";
                            }
                            if (pair.key.length() > 0) {
                                cfg.extras.push_back(pair);
                            }
                        }
                    }

                    if (cfg.type.length() > 0) {
                        out.push_back(cfg);
                    }
                }
                i = i + 1;
            }
            success = (out.size() > 0);
        }
    }

    return success;
}

bool DeviceFactory::assignStableAids(std::vector<DeviceConfig>& devices, uint32_t& coachChassisAid) {
    bool hasDeviceAids = false;
    for (const DeviceConfig& device : devices) {
        hasDeviceAids = hasDeviceAids || (device.aid != 0);
    }

    uint32_t nextAid = 2;
    if (!hasDeviceAids) {
        for (DeviceConfig& device : devices) {
            if (device.enabled) {
                device.aid = nextAid;
                nextAid += accessoryCount(device.type);
            }
        }
        if (coachChassisAid == 0) {
            coachChassisAid = nextAid;
        }
        nextAid = coachChassisAid + 2;
        for (DeviceConfig& device : devices) {
            if (!device.enabled) {
                device.aid = nextAid;
                nextAid += accessoryCount(device.type);
            }
        }
    } else {
        uint32_t maxEnabledEnd = 2;
        uint32_t maxReservedEnd = (coachChassisAid > 0) ? coachChassisAid + 2 : 2;
        for (const DeviceConfig& device : devices) {
            if (device.aid > 0) {
                uint32_t endAid = device.aid + accessoryCount(device.type);
                if (endAid > maxReservedEnd) maxReservedEnd = endAid;
                if (device.enabled && (endAid > maxEnabledEnd)) maxEnabledEnd = endAid;
            }
        }
        if (coachChassisAid == 0) {
            coachChassisAid = maxEnabledEnd;
        }
        if (maxReservedEnd < coachChassisAid + 2) {
            maxReservedEnd = coachChassisAid + 2;
        }
        nextAid = maxReservedEnd;
        for (DeviceConfig& device : devices) {
            if (device.aid == 0) {
                device.aid = nextAid;
                nextAid += accessoryCount(device.type);
            }
        }
    }

    bool unique = (coachChassisAid > 1);
    for (size_t i = 0; i < devices.size(); ++i) {
        uint32_t firstEnd = devices[i].aid + accessoryCount(devices[i].type);
        if (devices[i].aid < 2 ||
            ((devices[i].aid < coachChassisAid + 2) && (firstEnd > coachChassisAid))) {
            unique = false;
        }
        for (size_t j = i + 1; j < devices.size(); ++j) {
            uint32_t secondEnd = devices[j].aid + accessoryCount(devices[j].type);
            if ((devices[i].aid < secondEnd) && (devices[j].aid < firstEnd)) {
                unique = false;
            }
        }
    }
    return unique;
}

bool DeviceFactory::saveDeviceMetadata(const char* path, const std::vector<DeviceConfig>& devices) {
    bool ok = false;
    File input = LittleFS.open(path, "r");
    if (input) {
        JsonDocument document;
        DeserializationError error = deserializeJson(document, input);
        input.close();
        JsonArray rows = document.as<JsonArray>();
        if (!error && !rows.isNull() && (rows.size() == devices.size())) {
            bool matches = true;
            bool changed = false;
            for (size_t i = 0; i < devices.size(); ++i) {
                JsonObject row = rows[i].as<JsonObject>();
                const DeviceConfig& config = devices[i];
                if (row.isNull() || (String(row["type"] | "") != config.type) ||
                    ((row["rvcIndex"] | -1) != config.rvcIndex) ||
                    ((row["sourceAddress"] | -1) != config.sourceAddress)) {
                    matches = false;
                    break;
                }
                if ((row["aid"] | 0U) != config.aid) {
                    row["aid"] = config.aid;
                    changed = true;
                }
                if (String(row["room"] | "") != config.room) {
                    row["room"] = config.room;
                    changed = true;
                }
            }
            if (matches) {
                ok = changed ? replaceJsonFile(path, "/devices.tmp", "/devices.bak", document) : true;
            }
        }
    }
    return ok;
}

bool DeviceFactory::saveChassisAid(const char* path, uint32_t aid) {
    bool ok = false;
    File input = LittleFS.open(path, "r");
    if (input) {
        JsonDocument document;
        DeserializationError error = deserializeJson(document, input);
        input.close();
        JsonObject coach = document.as<JsonObject>();
        if (!error && !coach.isNull()) {
            if ((coach["chassisAid"] | 0U) == aid) {
                ok = true;
            } else {
                coach["chassisAid"] = aid;
                ok = replaceJsonFile(path, "/coach.tmp", "/coach.bak", document);
            }
        }
    }
    return ok;
}

void DeviceFactory::createFromDeviceConfig(const std::vector<DeviceConfig>& devices,
                                           const CoachSpec& coach) {
    for (const DeviceConfig& dev : devices) {
        if (dev.enabled) {
            auto it = creators.find(dev.type);
            if (it != creators.end()) {
                SpanView::setNextAccessoryAid(dev.aid);
                if (it->second(dev, coach) != nullptr) {
                    RV_PRINTF("Created %s (%s) idx=%u\n", dev.name.c_str(), dev.type.c_str(), dev.rvcIndex);
                }
            } else {
                RV_PRINTF("Unknown device type: %s\n", dev.type.c_str());
            }
        }
    }
}


void DeviceFactory::createDevices(void) {
    CoachSpec coach;
    std::vector<DeviceConfig> devices;
    bool loaded = false;

    loaded = loadCoachSpec("/coach.json", coach);
    if (loaded) {
        loaded = loadDeviceConfigs("/devices.json", devices);
    }
    if (loaded && !devices.empty()) {
        registerCreators();
        if (!assignStableAids(devices, coach.chassisAid)) {
            RV_PRINTF("DeviceFactory: accessory AID overlap; refusing to create HomeKit accessories\n");
            return;
        }
        chassisAccessoryAid = coach.chassisAid;
        if (!saveDeviceMetadata("/devices.json", devices)) {
            RV_PRINTF("DeviceFactory: failed to persist device AIDs and rooms\n");
        }
        if (!saveChassisAid("/coach.json", chassisAccessoryAid)) {
            RV_PRINTF("DeviceFactory: failed to persist chassis AID\n");
        }
        createFromDeviceConfig(devices, coach);
    }
}
