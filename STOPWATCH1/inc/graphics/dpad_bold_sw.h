#define dpad_bold_sw_width 16
#define dpad_bold_sw_height 16
static SI_SEGMENT_VARIABLE(dpad_bold_sw_bits[], const uint8_t, SI_SEG_CODE) = {
  0x00, 0x06, 0x00, 0x0F, 0x01, 0x1F, 0x01, 0xBE, 0x01, 0xFC, 0x01, 0xF8, 
  0x01, 0xF0, 0x01, 0xF8, 0x01, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};