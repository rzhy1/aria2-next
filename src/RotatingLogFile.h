/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 The aria2-next contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/* copyright --> */
#ifndef D_ROTATING_LOG_FILE_H
#define D_ROTATING_LOG_FILE_H

#include "common.h"

#include <cstdint>
#include <memory>
#include <string>

namespace aria2 {

class BufferedFile;

class RotatingLogFile {
public:
  static const size_t MAX_FILES = 100;

  RotatingLogFile(const std::string& path, int64_t maxFileSize,
                  size_t maxFiles);
  ~RotatingLogFile();

  void open();
  void close();
  bool write(const std::string& record);
  bool isOpen() const;

private:
  std::string path_;
  int64_t maxFileSize_;
  size_t maxFiles_;
  int64_t currentSize_;
  std::unique_ptr<BufferedFile> file_;

  std::string rotatedPath(size_t index) const;
  bool openActive(const char* mode);
  bool removeFile(const std::string& path);
  bool truncateFile(const std::string& path);
  bool reconcile();
  bool rotate();
  std::string boundRecord(const std::string& record) const;
};

} // namespace aria2

#endif // D_ROTATING_LOG_FILE_H
