// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_PUSH_MESSAGING_PUSH_MESSAGING_API_FACTORY_H_
#define CHROME_BROWSER_EXTENSIONS_API_PUSH_MESSAGING_PUSH_MESSAGING_API_FACTORY_H_

#include "base/memory/singleton.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

namespace extensions {
class PushMessagingAPI;

class PushMessagingAPIFactory : public ProfileKeyedServiceFactory {
 public:
  static PushMessagingAPI* GetForProfile(Profile* profile);

  static PushMessagingAPIFactory* GetInstance();

 private:
  friend struct DefaultSingletonTraits<PushMessagingAPIFactory>;

  PushMessagingAPIFactory();
  virtual ~PushMessagingAPIFactory();

  // ProfileKeyedServiceFactory implementation.
  virtual ProfileKeyedService* BuildServiceInstanceFor(
      Profile* profile) const OVERRIDE;
  virtual bool ServiceIsNULLWhileTesting() const OVERRIDE;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_PUSH_MESSAGING_PUSH_MESSAGING_API_FACTORY_H_
