#pragma once

struct yl_memory_file;

struct yl_memory_file *ylmf_create_with_buffer_size(unsigned buffer_size);
void ylmf_destroy(struct yl_memory_file **instance);

unsigned ylmf_get_buffer_available(struct yl_memory_file *instance);

unsigned ylmf_get_buffer_size(struct yl_memory_file *instance);
int ylmf_set_buffer_size(struct yl_memory_file *instance, unsigned buffer_size);

unsigned ylmf_read(struct yl_memory_file *instance, char *buffer, unsigned count, long long *offset);
unsigned ylmf_write(struct yl_memory_file *instance, const char *data, unsigned count, long long *offset);
