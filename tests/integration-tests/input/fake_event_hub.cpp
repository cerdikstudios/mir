#include "mir_test/fake_event_hub.h"

using android::AxisInfo;
using android::InputDeviceIdentifier;
using android::PropertyMap;
using android::Vector;
using android::String8;
using android::RawAbsoluteAxisInfo;
using android::RawEvent;
using android::sp;
using android::status_t;
using android::KeyCharacterMap;
using android::VirtualKeyDefinition;

namespace mir
{

FakeEventHub::FakeEventHub()
{
}

FakeEventHub::~FakeEventHub()
{
}

uint32_t FakeEventHub::getDeviceClasses(int32_t deviceId) const
{
    if (deviceId == android::BUILT_IN_KEYBOARD_ID)
    {
        return android::INPUT_DEVICE_CLASS_KEYBOARD;
    }
    auto fake_device_iterator = device_from_id.find(deviceId);

    if (fake_device_iterator != device_from_id.end())
    {
        return fake_device_iterator->second.classes;
    }
    else
    {
        return 0;
    }
}

InputDeviceIdentifier FakeEventHub::getDeviceIdentifier(int32_t deviceId) const
{
    auto fake_device_iterator = device_from_id.find(deviceId);

    if (fake_device_iterator != device_from_id.end())
    {
        return fake_device_iterator->second.identifier;
    }
    else
    {
        return InputDeviceIdentifier();
    }
}

void FakeEventHub::getConfiguration(int32_t deviceId, PropertyMap* outConfiguration) const
{
    (void)deviceId;
    (void)outConfiguration;
}

status_t FakeEventHub::getAbsoluteAxisInfo(int32_t deviceId, int axis,
        RawAbsoluteAxisInfo* outAxisInfo) const
{
    (void)deviceId;
    (void)axis;
    (void)outAxisInfo;
    return -1;
}

bool FakeEventHub::hasRelativeAxis(int32_t deviceId, int axis) const
{
    (void)deviceId;
    (void)axis;
    return false;
}

bool FakeEventHub::hasInputProperty(int32_t deviceId, int property) const
{
    auto fake_device_iterator = device_from_id.find(deviceId);

    if (fake_device_iterator != device_from_id.end())
    {
        auto property_iterator =
            fake_device_iterator->second.input_properties.find(property);

        if (property_iterator != fake_device_iterator->second.input_properties.end())
        {
            return property_iterator->second;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

status_t FakeEventHub::mapKey(int32_t deviceId, int32_t scanCode, int32_t usageCode,
                              int32_t* outKeycode, uint32_t* outFlags) const
{
    (void)deviceId;
    (void)usageCode;
    *outKeycode = scanCode;
    *outFlags = 0;
    return android::OK;
}

status_t FakeEventHub::mapAxis(int32_t deviceId, int32_t scanCode,
                               AxisInfo* outAxisInfo) const
{
    (void)deviceId;
    (void)scanCode;
    (void)outAxisInfo;
    return -1;
}

void FakeEventHub::setExcludedDevices(const Vector<String8>& devices)
{
    (void)devices;
}

size_t FakeEventHub::getEvents(int timeoutMillis, RawEvent* buffer, size_t bufferSize)
{
    (void) timeoutMillis;
    size_t num_events_obtained = 0;

    for (size_t i = 0; i < bufferSize && events_available.size() > 0; ++i)
    {
        buffer[i] = events_available.front();
        events_available.pop_front();
        ++num_events_obtained;
    }

    return num_events_obtained;
}

int32_t FakeEventHub::getScanCodeState(int32_t deviceId, int32_t scanCode) const
{
    (void)deviceId;
    (void)scanCode;
    return AKEY_STATE_UNKNOWN;
}

int32_t FakeEventHub::getKeyCodeState(int32_t deviceId, int32_t keyCode) const
{
    (void)deviceId;
    (void)keyCode;
    return AKEY_STATE_UNKNOWN;
}

int32_t FakeEventHub::getSwitchState(int32_t deviceId, int32_t sw) const
{
    (void)deviceId;
    (void)sw;
    return AKEY_STATE_UNKNOWN;
}

status_t FakeEventHub::getAbsoluteAxisValue(int32_t deviceId, int32_t axis,
        int32_t* outValue) const
{
    (void)deviceId;
    (void)axis;
    (void)outValue;
    return -1;
}

bool FakeEventHub::markSupportedKeyCodes(int32_t deviceId, size_t numCodes,
        const int32_t* keyCodes,
        uint8_t* outFlags) const
{
    (void)deviceId;
    (void)numCodes;
    (void)keyCodes;
    (void)outFlags;
    return true;
}

bool FakeEventHub::hasScanCode(int32_t deviceId, int32_t scanCode) const
{
    (void)deviceId;
    (void)scanCode;
    return false;
}

bool FakeEventHub::hasLed(int32_t deviceId, int32_t led) const
{
    (void)deviceId;
    (void)led;
    return false;
}

void FakeEventHub::setLedState(int32_t deviceId, int32_t led, bool on)
{
    (void)deviceId;
    (void)led;
    (void)on;
}

void FakeEventHub::getVirtualKeyDefinitions(int32_t deviceId,
        Vector<VirtualKeyDefinition>& outVirtualKeys) const
{
    (void)deviceId;
    (void)outVirtualKeys;
}

sp<KeyCharacterMap> FakeEventHub::getKeyCharacterMap(int32_t deviceId) const
{
    (void)deviceId;
    return sp<KeyCharacterMap>();
}

bool FakeEventHub::setKeyboardLayoutOverlay(int32_t deviceId,
        const sp<KeyCharacterMap>& map)
{
    (void)deviceId;
    (void)map;
    return true;
}

void FakeEventHub::vibrate(int32_t deviceId, nsecs_t duration)
{
    (void)deviceId;
    (void)duration;
}

void FakeEventHub::cancelVibrate(int32_t deviceId)
{
    (void)deviceId;
}

void FakeEventHub::requestReopenDevices()
{
}

void FakeEventHub::wake()
{
}

void FakeEventHub::dump(android::String8& dump)
{
    (void)dump;
}

void FakeEventHub::monitor()
{
}

void FakeEventHub::synthesize_builtin_keyboard_added()
{
    RawEvent event;
    event.when = 0;
    event.deviceId = android::BUILT_IN_KEYBOARD_ID;
    event.type = EventHubInterface::DEVICE_ADDED;
    events_available.push_back(event);

    event.when = 0;
    event.type = EventHubInterface::FINISHED_DEVICE_SCAN;
    events_available.push_back(event);
}

void FakeEventHub::synthesize_key_event(int keycode)
{
    RawEvent event;
    event.when = 0;
    event.type = EV_KEY;
    event.code = keycode;
    event.value = 1;
    event.deviceId = android::BUILT_IN_KEYBOARD_ID;
    events_available.push_back(event);
}

} // namespace mir
