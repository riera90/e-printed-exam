#ifndef SPIFFS_FILES_H
#define SPIFFS_FILES_H

void spiffs_init();
void spiffs_deinit();
bool spiffs_delete_file(const char* path);

#endif