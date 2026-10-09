#ifndef CHASSIS_MOBILITY_VIEW_H
#define CHASSIS_MOBILITY_VIEW_H

#include "SpanView.h"
#define CUSTOM_CHAR_HEADER
#include "HomeSpan.h"
#include "RVConstants.h"
#include "DGN.h"

class ChassisMobility;

typedef enum {
    NOT_IN_MOTION = 0,
    IN_MOTION = 1
} MOTION_STATE;

class ChassisMobilityView : public SpanView {
private:
    friend class ChassisMobility;

    struct ChassisMobilityMotion : Service::MotionSensor {
        ChassisMobility* model;
        ChassisMobilityView* view;
        SpanCharacteristic* inMotion;

        ChassisMobilityMotion(ChassisMobilityView* hostView,
                              ChassisMobility* hostModel,
                              const char* spanDeviceName)
            : Service::MotionSensor()
            , model(hostModel)
            , view(hostView)
            , inMotion(nullptr) {
            (void)spanDeviceName;
            inMotion = new Characteristic::MotionDetected(NOT_IN_MOTION);
            inMotion->setDescription("Chassis Mobility Motion Sensor");
        }

        ChassisMobilityMotion(const ChassisMobilityMotion&) = delete;
        ChassisMobilityMotion& operator=(const ChassisMobilityMotion&) = delete;
        ChassisMobilityMotion(ChassisMobilityMotion&&) = delete;
        ChassisMobilityMotion& operator=(ChassisMobilityMotion&&) = delete;

        void publish(MOTION_STATE value) {
            if (inMotion != nullptr) {
                inMotion->setVal(static_cast<uint8_t>(value));
            }
        }

        boolean isInMotion() const {
            boolean moving = false;
            if (inMotion != nullptr) {
                moving = (inMotion->getVal() == IN_MOTION);
            }
            return moving;
        }
    };

    ChassisMobilityMotion* chassisMobilitySensor_;

    void sensor(ChassisMobilityMotion* value);
    ChassisMobilityMotion* sensor() const;

    ChassisMobilityView(GenericDevice* model, const char* spanDevName);

    ChassisMobilityView(const ChassisMobilityView&) = delete;
    ChassisMobilityView& operator=(const ChassisMobilityView&) = delete;
    ChassisMobilityView(ChassisMobilityView&&) = delete;
    ChassisMobilityView& operator=(ChassisMobilityView&&) = delete;

public:
    virtual ~ChassisMobilityView() override;

    virtual bool updateView() override;

    static void createChassisMobilityView(GenericDevice* model, const char* spanDevName);
};

#endif