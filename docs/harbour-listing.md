# Jolla Harbour listing

Use this page when submitting SailDice to <https://harbour.jolla.com/>.

## Metadata

- **Name:** SailDice
- **Package:** `harbour-saildice`
- **Version:** 1.0.0
- **Category:** Utility
- **Developer:** protongit
- **Source code:** https://github.com/protongit/harbour-saildice
- **Privacy policy:** https://github.com/protongit/harbour-saildice/blob/main/PRIVACY.md
- **License:** MIT
- **Interface language:** English
- **Permissions:** none
- **Network access:** none

## Summary

Real 3D polyhedral dice roller for Sailfish OS.

## Description

SailDice is a native Sailfish OS dice roller built around real 3D polyhedral
dice. Choose from D4, D6, D8, D10, D12 and D20, configure up to twelve dice and
give each one its own color.

Dice are rendered with OpenGL ES 2.0 and tumble onto the face that matches their
result. Every roll shows its total and the value of each die, and the last 50
rolls stay available in the session history.

Dice configuration lives on its own page, reachable from the pull-down menu. A
cover action lets you roll and check the latest result while the application is
minimized.

SailDice works completely offline: no account, no analytics, no permissions and
no data collection.

## Changelog

Initial public release.

- Real 3D D4, D6, D8, D10, D12 and D20 dice.
- Up to twelve dice with independent type and color.
- Dedicated configuration page in the pull-down menu.
- Animated results, totals and per-die breakdown.
- Session roll history with confirmed clearing.
- Cover action for rolling and latest result.
- English interface and Sailfish launcher icons.

## Testing notes for QA

1. Open SailDice. Two D6 dice are configured by default.
2. Press **Roll**. The dice must stop showing the same values listed in the
   total and in **Roll history**.
3. Open the pull-down menu and choose **Configure dice**. Change a die type and
   color, add another die and go back.
4. Roll again and verify the new configuration and total.
5. Minimize the application. The cover shows the latest result; the cover action
   rolls again and updates it.
6. Press **Clear history** and confirm that the list becomes empty.
7. Confirm that the application works with the device offline.

## Screenshots

- `screenshots/saildice-roll.png` — main page after a roll.
- `screenshots/saildice-config.png` — dice configuration page.
- `screenshots/saildice-cover.png` — cover while the application is minimized.
- `screenshots/saildice-icon-preview.png` — launcher icon preview (optional).

## Packages

- `harbour-saildice-1.0.0-1.aarch64.rpm`
- `harbour-saildice-1.0.0-1.armv7hl.rpm`
