#pragma once

struct yl_memory_file;

struct yl_memory_file *ylmf_create_with_buffer_size(long long buffer_size);
void ylmf_destroy(struct yl_memory_file **instance);

long long ylmf_get_buffer_written(struct yl_memory_file *instance);

long long ylmf_get_buffer_size(struct yl_memory_file *instance);
int ylmf_set_buffer_size(struct yl_memory_file *instance, long long buffer_size);

unsigned ylmf_read(struct yl_memory_file *instance, char *buffer, unsigned count, long long *offset);
unsigned ylmf_write(struct yl_memory_file *instance, const char *data, unsigned count, long long *offset);
