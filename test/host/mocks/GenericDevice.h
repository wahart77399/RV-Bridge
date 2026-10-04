#pragma once
class GenericDevice {
public:
    GenericDevice() = default;
    GenericDevice(const GenericDevice&) = delete;
    GenericDevice& operator=(const GenericDevice&) = delete;
    GenericDevice(GenericDevice&&) = delete;
    GenericDevice& operator=(GenericDevice&&) = delete;
    ~GenericDevice() = default;
};
