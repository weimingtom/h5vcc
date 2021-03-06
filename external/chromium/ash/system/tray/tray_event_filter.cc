// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/tray/tray_event_filter.h"

#include "ash/root_window_controller.h"
#include "ash/shell.h"
#include "ash/shell_window_ids.h"
#include "ash/system/tray/tray_background_view.h"
#include "ash/system/tray/tray_bubble_wrapper.h"
#include "ash/system/tray/tray_constants.h"
#include "ash/system/tray/tray_event_filter.h"
#include "ash/wm/property_util.h"
#include "ash/wm/shelf_layout_manager.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/views/widget/widget.h"

namespace ash {
namespace internal {

TrayEventFilter::TrayEventFilter(TrayBubbleWrapper* wrapper)
    : wrapper_(wrapper) {
  ash::Shell::GetInstance()->AddPreTargetHandler(this);
}

TrayEventFilter::~TrayEventFilter() {
  ash::Shell::GetInstance()->RemovePreTargetHandler(this);
}

void TrayEventFilter::OnMouseEvent(ui::MouseEvent* event) {
  if (event->type() == ui::ET_MOUSE_PRESSED &&
      ProcessLocatedEvent(event)) {
    event->StopPropagation();
  }
}

void TrayEventFilter::OnTouchEvent(ui::TouchEvent* event) {
  if (event->type() == ui::ET_TOUCH_PRESSED && ProcessLocatedEvent(event))
    event->StopPropagation();
}

bool TrayEventFilter::ProcessLocatedEvent(ui::LocatedEvent* event) {
  if (event->target()) {
    aura::Window* target = static_cast<aura::Window*>(event->target());
    // Don't process events that occurred inside an embedded menu.
    ash::internal::RootWindowController* root_controller =
        ash::GetRootWindowController(target->GetRootWindow());
    if (root_controller && root_controller->GetContainer(
            ash::internal::kShellWindowId_MenuContainer)->Contains(target)) {
      return false;
    }
  }
  views::Widget* bubble_widget = wrapper_->bubble_widget();
  if (!bubble_widget)
    return false;

  gfx::Rect bounds = bubble_widget->GetWindowBoundsInScreen();
  bounds.Inset(wrapper_->bubble_view()->GetBorderInsets());
  aura::RootWindow* root = bubble_widget->GetNativeView()->GetRootWindow();
  aura::client::ScreenPositionClient* screen_position_client =
      aura::client::GetScreenPositionClient(root);
  gfx::Point screen_point(event->root_location());
  screen_position_client->ConvertPointToScreen(root, &screen_point);

  if (bounds.Contains(screen_point))
    return false;
  if (wrapper_->tray()) {
    // If the user clicks on the parent tray, don't process the event here,
    // let the tray logic handle the event and determine show/hide behavior.
    bounds = wrapper_->tray()->GetWidget()->GetClientAreaBoundsInScreen();
    if (bounds.Contains(screen_point))
      return false;
  }
  // Handle clicking outside the bubble and tray and return true if the
  // event was handled.
  return wrapper_->tray()->ClickedOutsideBubble();
}

}  // namespace internal
}  // namespace ash
