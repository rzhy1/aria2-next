/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 AnInsomniacy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/* copyright --> */
#include "HttpErrorPageDetector.h"

#include <algorithm>
#include <vector>

#include "util.h"

namespace aria2 {
namespace {

bool endsWithAny(const std::string& value,
                 const std::vector<std::string>& suffixes)
{
  for (const auto& suffix : suffixes) {
    if (value.size() >= suffix.size() &&
        value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
            0) {
      return true;
    }
  }
  return false;
}

bool targetLooksBinary(std::string target)
{
  util::lowercase(target);
  return endsWithAny(target,
                     {".exe", ".msi", ".msu", ".cab", ".iso", ".dmg",
                      ".pkg", ".zip", ".7z", ".rar", ".tar", ".tgz",
                      ".gz", ".xz", ".zst", ".apk", ".ipa", ".ipsw",
                      ".bin", ".img", ".torrent", ".pdf", ".mp4", ".mkv",
                      ".avi", ".mov", ".mp3", ".flac", ".wasm"});
}

bool targetLooksHtml(std::string target)
{
  util::lowercase(target);
  return endsWithAny(target, {".html", ".htm", ".xhtml", ".shtml"});
}

std::string lowerCopy(std::string value)
{
  util::lowercase(value);
  return value;
}

std::string trimBomAndSpace(std::string value)
{
  if (value.size() >= 3 &&
      static_cast<unsigned char>(value[0]) == 0xef &&
      static_cast<unsigned char>(value[1]) == 0xbb &&
      static_cast<unsigned char>(value[2]) == 0xbf) {
    value.erase(0, 3);
  }
  auto first = util::lstripIter(value.begin(), value.end());
  return std::string(first, value.end());
}

bool hasHtmlMagic(const std::string& lowerBody)
{
  auto body = trimBomAndSpace(lowerBody);
  return util::startsWith(body, "<!doctype html") ||
         util::startsWith(body, "<html") || util::startsWith(body, "<head") ||
         util::startsWith(body, "<body") || util::startsWith(body, "<script") ||
         body.find("<title") != std::string::npos ||
         body.find("<form") != std::string::npos;
}

std::string challengeReason(const std::string& lowerBody)
{
  if (lowerBody.find("uc-download-link") != std::string::npos ||
      lowerBody.find("download_warning") != std::string::npos ||
      lowerBody.find("confirm=") != std::string::npos ||
      lowerBody.find("google drive") != std::string::npos) {
    return "HTTP response looks like a Google Drive confirmation page.";
  }
  if (lowerBody.find("cf-chl-") != std::string::npos ||
      lowerBody.find("challenge-platform") != std::string::npos ||
      lowerBody.find("just a moment") != std::string::npos ||
      lowerBody.find("attention required") != std::string::npos ||
      lowerBody.find("cloudflare") != std::string::npos) {
    return "HTTP response looks like a Cloudflare challenge page.";
  }
  if (lowerBody.find("g-recaptcha") != std::string::npos ||
      lowerBody.find("captcha") != std::string::npos ||
      lowerBody.find("verify you are human") != std::string::npos) {
    return "HTTP response looks like a verification page.";
  }
  if (lowerBody.find("login") != std::string::npos ||
      lowerBody.find("sign in") != std::string::npos ||
      lowerBody.find("signin") != std::string::npos) {
    return "HTTP response looks like a login page.";
  }
  return "";
}

bool isHtmlContentType(std::string contentType)
{
  util::lowercase(contentType);
  return contentType.find("text/html") != std::string::npos ||
         contentType.find("application/xhtml+xml") != std::string::npos;
}

} // namespace

HttpErrorPageDecision detectHttpErrorPage(const std::string& targetPath,
                                          const std::string& contentType,
                                          const std::string& bodyPrefix)
{
  if (targetLooksHtml(targetPath) || !targetLooksBinary(targetPath)) {
    return {};
  }

  auto lowerBody = lowerCopy(bodyPrefix);
  auto reason = challengeReason(lowerBody);
  if (!reason.empty()) {
    return {true, reason};
  }

  if (isHtmlContentType(contentType) || hasHtmlMagic(lowerBody)) {
    return {true,
            "HTTP response looks like an HTML page, not the requested file."};
  }

  return {};
}

} // namespace aria2
