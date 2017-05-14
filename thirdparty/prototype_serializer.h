
#include <stdint.h>

enum ps_mode {
  ps_mode_none,
  ps_mode_write,
  ps_mode_read,
};



int ps_open_file(const char *name, ps_mode mode);
void ps_close_file();
void ps_begin_block(const char *name);
void ps_end_block();
void ps_begin_block_array(const char *name);
void ps_end_block_array();
bool ps_is_reading();
bool ps_is_writing();

void ps_uint32(const char *name, uint32_t *value);
void ps_unit64(const char *name, uint64_t *value);