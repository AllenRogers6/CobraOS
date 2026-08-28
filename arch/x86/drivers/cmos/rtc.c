#include "rtc.h"
#include "io.h"

#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT 0x71

static uint8_t cmos_read(uint8_t reg) {
  outb(CMOS_ADDRESS_PORT, (reg & 0x7F));
  io_wait();
  return inb(CMOS_DATA_PORT);
}

static int cmos_is_updating(void) { return cmos_read(0x0A) & 0x80; }

static void cmos_wait_ready(void) {
  while (cmos_is_updating()) {
    /*spin*/
  }
}

static uint8_t bcd_to_bin(uint8_t bcd) {
  return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

void rtc_read_datetime(struct rtc_datetime *dt) {
  cmos_wait_ready();

  uint8_t second_raw = cmos_read(0x00);
  uint8_t minute_raw = cmos_read(0x02);
  uint8_t hour_raw = cmos_read(0x04);
  uint8_t day_raw = cmos_read(0x07);
  uint8_t month_raw = cmos_read(0x08);
  uint8_t year_raw = cmos_read(0x09);
  uint8_t status_b = cmos_read(0x0B);

  dt->second = bcd_to_bin(second_raw);
  dt->minute = bcd_to_bin(minute_raw);
  dt->day = bcd_to_bin(day_raw);
  dt->month = bcd_to_bin(month_raw);
  dt->year = bcd_to_bin(year_raw);

  if (status_b & 0x02) {
    uint8_t pm = hour_raw & 0x80;
    uint8_t hour12 = bcd_to_bin(hour_raw & 0x7F);

    if (pm) {
      dt->hour = (hour12 == 12) ? 12 : (hour12 + 12);
    } else {
      dt->hour = (hour12 == 12) ? 0 : hour12;
    }
  } else {
    dt->hour = bcd_to_bin(hour_raw & 0x3F);
  }

  uint8_t century_raw = cmos_read(0x32);
  if (century_raw != 0xFF && century_raw != 0x00) {
    dt->year += bcd_to_bin(century_raw) * 100;
  } else {
    if (dt->year < 80) {
      dt->year += 2000;
    } else {
      dt->year += 1900;
    }
  }
}
