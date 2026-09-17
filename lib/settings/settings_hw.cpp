#include "settings_hw.h"

#include "Sanitize.h"

#ifdef __AVR__
#include <avr/eeprom.h>
#endif

namespace settings {

namespace {

const uint16_t kAddrTlow = 0;
const uint16_t kAddrThigh = 1;
const uint8_t kBlank = 0xFF;

}  // namespace

SettingsHw::SettingsHw() : tlow_(kBlank), thigh_(kBlank) { sanitize(tlow_, thigh_); }

void SettingsHw::init() {
    uint8_t tlow = kBlank;
    uint8_t thigh = kBlank;
#ifdef __AVR__
    tlow = eeprom_read_byte(reinterpret_cast<const uint8_t*>(kAddrTlow));
    thigh = eeprom_read_byte(reinterpret_cast<const uint8_t*>(kAddrThigh));
#endif
    sanitize(tlow, thigh);
    tlow_ = tlow;
    thigh_ = thigh;
}

uint8_t SettingsHw::tlow() const { return tlow_; }

uint8_t SettingsHw::thigh() const { return thigh_; }

void SettingsHw::save(uint8_t tlow, uint8_t thigh) {
    // An unusable pair is cancelled: the stored one is already good, so there
    // is nothing to fall back to and no reason to touch the cells.
    if (!usable(tlow, thigh)) {
        return;
    }
    tlow_ = tlow;
    thigh_ = thigh;
#ifdef __AVR__
    // update, not write: an unchanged value costs no erase cycle. Both calls
    // block until the cell is programmed, which is why this is main-loop only.
    eeprom_update_byte(reinterpret_cast<uint8_t*>(kAddrTlow), tlow);
    eeprom_update_byte(reinterpret_cast<uint8_t*>(kAddrThigh), thigh);
#endif
}

}  // namespace settings
