// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/system_info_display/system_info_display_api.h"

namespace extensions {

using api::experimental_system_info_display::DisplayUnitInfo;

bool SystemInfoDisplayGetFunction::RunImpl() {
  DisplayInfoProvider::Get()->StartQueryInfo(
      base::Bind(&SystemInfoDisplayGetFunction::OnGetDisplayInfoCompleted,
                 this));
  return true;
}

void SystemInfoDisplayGetFunction::OnGetDisplayInfoCompleted(
    const DisplayInfo& info, bool success) {
  if (success) {
    results_ =
      api::experimental_system_info_display::Get::Results::Create(info);
  }
  else
    SetError("Error occurred when querying display information.");
  SendResponse(success);
}

}  // namespace extensions
