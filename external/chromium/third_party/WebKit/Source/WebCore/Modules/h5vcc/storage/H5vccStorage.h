/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef H5vccStorage_h
#define H5vccStorage_h

#include "config.h"
#include <wtf/RefCounted.h>

namespace H5vcc {

class Storage {
 public:
  virtual ~Storage() {}
  virtual void ClearCookies() = 0;
  virtual void Flush(bool sync) = 0;
  virtual bool GetCookiesEnabled() = 0;
  virtual void SetCookiesEnabled(bool value) = 0;
};

}  // namespace H5vcc

namespace WebCore {

class H5vccStorage : public RefCounted<H5vccStorage> {
 public:
  H5vccStorage() {}
  virtual ~H5vccStorage() {}

  static void clearCookies();
  static void flush(bool sync);
  static bool getCookiesEnabled();
  static void setCookiesEnabled(bool value);

 private:
  static void init();

  static H5vcc::Storage *impl_;
};

}  // namespace WebCore

#endif  // H5vccStorage_h
