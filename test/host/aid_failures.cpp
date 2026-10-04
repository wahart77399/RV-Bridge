#include "DeviceFactory.h"
#include "Preferences.h"
#include <functional>
#include <iostream>
#include <stdexcept>

namespace {
    void require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }
    DeviceConfig device(const char* type, uint32_t aid = 0, bool enabled = true) {
        DeviceConfig result;
        result.type = type;
        result.aid = aid;
        result.enabled = enabled;
        return result;
    }
    void reset() { preferencesStore = FakePreferencesStore{}; }
}

int main() {
    const std::vector<std::pair<const char*, std::function<void()>>> tests = {
        {"fresh unit reserves chassis without devices", [] {
            reset(); std::vector<DeviceConfig> devices; uint32_t chassis = 0;
            require(DeviceFactory::assignStableAids(devices, chassis), "fresh reservation failed");
            require(chassis == 2 && preferencesStore.values["sc-aids/next"] == 4, "chassis collides with bridge");
        }},
        {"legacy migration preserves enabled accessory order", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Inverter"), device("Thermostat"), device("Tank")};
            uint32_t chassis = 0;
            require(DeviceFactory::assignStableAids(devices, chassis), "legacy migration failed");
            require(devices[0].aid == 2 && devices[1].aid == 6 && devices[2].aid == 9 && chassis == 10, "legacy IDs changed");
        }},
        {"existing IDs and chassis reservation are retained", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Inverter", 2), device("Thermostat", 6), device("Tank", 60)};
            uint32_t chassis = 61;
            require(DeviceFactory::assignStableAids(devices, chassis), "existing config rejected");
            require(devices[0].aid == 2 && devices[1].aid == 6 && devices[2].aid == 60 && chassis == 61, "existing identity changed");
            require(preferencesStore.values["sc-aids/next"] == 63, "high-water mark not saved");
        }},
        {"disabled device reserves a block without shifting live devices", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Tank", 60), device("Thermostat", 0, false)};
            uint32_t chassis = 61;
            require(DeviceFactory::assignStableAids(devices, chassis), "pending reservation failed");
            require(devices[0].aid == 60 && devices[1].aid == 63, "pending device displaced live IDs");
            require(preferencesStore.values["sc-aids/next"] == 66, "pending subaccessories not reserved");
        }},
        {"deleting highest entry does not reuse its AID", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Tank", 60), device("Tank", 100)};
            uint32_t chassis = 61;
            require(DeviceFactory::assignStableAids(devices, chassis), "initial reservation failed");
            devices.pop_back(); devices.push_back(device("Tank"));
            require(DeviceFactory::assignStableAids(devices, chassis), "allocation after deletion failed");
            require(devices[1].aid == 101, "deleted ID was reused");
        }},
        {"new device list cannot displace previously reserved chassis", [] {
            reset(); std::vector<DeviceConfig> devices; uint32_t chassis = 0;
            require(DeviceFactory::assignStableAids(devices, chassis), "chassis reservation failed");
            devices.push_back(device("Thermostat")); chassis = 0;
            require(DeviceFactory::assignStableAids(devices, chassis), "subsequent allocation failed");
            require(chassis == 2 && devices[0].aid == 4, "new devices took chassis IDs");
        }},
        {"conflicting chassis identity is rejected", [] {
            reset(); preferencesStore.values["sc-aids/chassis"] = 61;
            std::vector<DeviceConfig> devices; uint32_t chassis = 2;
            require(!DeviceFactory::assignStableAids(devices, chassis), "conflicting chassis identity accepted");
        }},
        {"overlapping multi-accessory blocks are rejected", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Thermostat", 6), device("Tank", 7)};
            uint32_t chassis = 61;
            require(!DeviceFactory::assignStableAids(devices, chassis), "overlapping block accepted");
        }},
        {"device block overlapping chassis is rejected", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Thermostat", 60)}; uint32_t chassis = 61;
            require(!DeviceFactory::assignStableAids(devices, chassis), "device/chassis overlap accepted");
        }},
        {"bridge AID cannot be assigned to device", [] {
            reset(); std::vector<DeviceConfig> devices = {device("Tank", 1)}; uint32_t chassis = 61;
            require(!DeviceFactory::assignStableAids(devices, chassis), "bridge ID accepted");
        }},
        {"AID block integer overflow is rejected", [] {
            reset(); std::vector<DeviceConfig> devices = {device("ATS", UINT32_MAX - 3)}; uint32_t chassis = 61;
            require(!DeviceFactory::assignStableAids(devices, chassis), "overflowing block accepted");
        }},
        {"allocation beyond high-water integer limit is rejected", [] {
            reset(); preferencesStore.values["sc-aids/next"] = UINT32_MAX;
            preferencesStore.values["sc-aids/chassis"] = 61;
            std::vector<DeviceConfig> devices = {device("Tank")}; uint32_t chassis = 61;
            require(!DeviceFactory::assignStableAids(devices, chassis), "allocator wrapped to an old ID");
        }},
        {"HomeKit accessory count limit is enforced", [] {
            reset(); std::vector<DeviceConfig> devices(148, device("Tank")); uint32_t chassis = 0;
            require(!DeviceFactory::assignStableAids(devices, chassis), "bridge exceeded 150 accessories");
        }},
        {"unavailable NVS refuses allocation", [] {
            reset(); preferencesStore.available = false;
            std::vector<DeviceConfig> devices; uint32_t chassis = 0;
            require(!DeviceFactory::assignStableAids(devices, chassis), "unpersisted reservation accepted");
        }},
        {"failed high-water write refuses allocation", [] {
            reset(); preferencesStore.failedWrites.insert("sc-aids/next");
            std::vector<DeviceConfig> devices; uint32_t chassis = 0;
            require(!DeviceFactory::assignStableAids(devices, chassis), "failed NVS write reported success");
        }},
        {"failed chassis write refuses allocation", [] {
            reset(); preferencesStore.failedWrites.insert("sc-aids/chassis");
            std::vector<DeviceConfig> devices; uint32_t chassis = 0;
            require(!DeviceFactory::assignStableAids(devices, chassis), "failed chassis write accepted");
        }}
    };
    unsigned failures = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "PASS: " << test.first << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cout << "FAIL: " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - failures << '/' << tests.size() << " AID tests passed\n";
    return failures == 0 ? 0 : 1;
}