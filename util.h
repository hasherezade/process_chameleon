#pragma once

#include <Windows.h>

//get file name from the full path
wchar_t* get_file_name(wchar_t *full_path);

wchar_t* get_directory(IN wchar_t *full_path, OUT wchar_t *out_buf, IN const size_t out_buf_size);
