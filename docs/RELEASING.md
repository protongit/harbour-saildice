# Releasing

## 1. Update the version

- Bump `Version` and reset `Release` in `rpm/harbour-saildice.spec`.
- Add the release entry to `CHANGELOG.md`.
- Update package names in `README.md` and `docs/harbour-listing.md`.

## 2. Build every architecture

```bash
cd harbour-saildice
rm -f ./*.o moc_* Makefile harbour-saildice

sfdk -c target=SailfishOS-5.1.0.11-aarch64 build
sfdk -c target=SailfishOS-5.1.0.11-aarch64 check -s harbour \
  RPMS/harbour-saildice-*-1.aarch64.rpm

rm -f ./*.o moc_* Makefile harbour-saildice
sfdk -c target=SailfishOS-5.1.0.11-armv7hl build
sfdk -c target=SailfishOS-5.1.0.11-armv7hl check -s harbour \
  RPMS/harbour-saildice-*-1.armv7hl.rpm
```

Run `--selftest` and `--qmlcheck` from the i486 target, and do a visual pass on
a real device.

## 3. Commit and tag

```bash
git add -A
git commit -m "Release 1.0.0"
git tag -a v1.0.0 -m "SailDice 1.0.0"
git push origin main
git push origin v1.0.0
```

Create the GitHub release from the tag and attach both RPMs.

## 4. Submit to Harbour

1. Upload the aarch64 RPM at <https://harbour.jolla.com/>.
2. Use the details from `docs/harbour-listing.md`.
3. Upload the screenshots and fill in the privacy policy URL.
4. Submit the armv7hl RPM for the 32-bit store entry when requested by the
   Harbour review form or after the first package is accepted.
5. Answer the QA questions with the steps in the listing page.

Harbour uploads use the RPM signed by the SDK build, so always rebuild from the
tagged commit and keep the work tree clean.
