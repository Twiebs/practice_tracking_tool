
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef enum {
  ps_type_invalid = 0,
  ps_type_uint8 = 1,
  ps_type_uint16 = 2,
  ps_type_uint32 = 3,
  ps_type_uint64 = 4,
  ps_type_int8 = 5,
  ps_type_int16 = 6,
  ps_type_int32 = 7,
  ps_type_int64 = 8,
  ps_type_float32 = 9,
  ps_type_float64 = 10,
  ps_type_block = 11,
} ps_type;

struct ps_context {
  FILE *file;
  uint8_t *buffer;
  uint64_t buffer_size;
  ps_mode mode;

  char tagBuffer[256];
  uint8_t tagLength;
  bool isErrorDetected;
} ps_ctx;

static inline void _ps_write_tag(const char *name, ps_type type) {
  ps_assert(ps_ctx.mode == ps_mode_write);
  size_t length = strlen(name);
  assert(length <= 0xFFFFFFFF);
  uint32_t write_size = length;
  fwrite(&type, 4, 1, ctx.file);
  fwrite(&write_size, 4, 1, ctx.file);
  fwrite(name, 1, length, ps_ctx.file);
}

static inline void _ps_error(const char *text) {
  ps_ctx.isErrorDetected = true;
#ifdef ps_error
  ps_error(text)
#endif//ps_error
}

int ps_open_file(const char *name, ps_mode mode) {
  const char *fopen_mode = mode == ps_mode_write ? "wb" : "rb";
  ps_ctx.file = fopen(filename, fopen_mode);
  ps_ctx.mode = mode;
  if (ps_context.mode == ps_mode_read) {
    fseek(ps_ctx.file, 0, SEEK_END);
    ps_ctx.buffer_size = ftell(file);
    fseek(ps_ctx.file, 0, SEEK_SET);
    ps_ctx.buffer = (uint8_t *)malloc(ps_ctx.buffer_size);
  }
}

void ps_close_file() {
  fclose(ps_ctx.file);
  if (ps_ctx.mode == ps_mode_read) {
    free(ps_ctx.buffer);
  }
  ps_ctx.mode = ps_mode_node;
}

bool ps_is_reading() {
  return ps_ctx.mode == ps_mode_reading;
}

bool ps_is_writing() {
  return ps_ctx.mode == ps_mode_writing;
}

void ps_begin_block(const char *name) {
  _ps_write_tag(name, ps_type_block);
}

void ps_uint32(const char *name, uint32_t *value) {
  if (ps_ctx.isErrorDetected) return;

  if (ps_is_writing()) {
    _ps_write_tag(name, ps_type_uint32);
    fwrite(value, 4, 1, ps_ctx.file);
  } else if (ps_is_reading()) {

  } else {
    ps_assert(false);
  }
}