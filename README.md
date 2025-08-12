# EventReminder v2 (Qt) - Updated

Features added in v2:
- System tray integration and background runner (app stays in tray and checks reminders).
- Desktop notifications via QSystemTrayIcon (cross-platform).
- Auto-start toggle:
  - Windows: registry Run key (HKCU).
  - Linux: .desktop autostart file in config/autostart.
- Search bar and sort (ascending/descending).
- Highlight recently passed events (red if within 24h).
- Reminder popups for events within 5 minutes and when due.

## Build & Run
1. Install Qt 6 and Qt Creator.
2. Open `EventReminder_v2/EventReminder.pro` in Qt Creator.
3. Build and Run.

## Notes
- `events.txt` is used for storage in the working directory.
- Auto-start creation writes files to user config; ensure permissions are available.
