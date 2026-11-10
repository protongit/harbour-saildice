# Contributing

Bug reports and pull requests are welcome.

## Before opening an issue

- Check that the problem is reproducible on the latest release.
- Include the Sailfish OS version, device model and steps to reproduce.
- For rendering problems, mention whether the cover and main page both fail and
  attach a screenshot when possible.

## Pull requests

1. Keep the application dependency-free beyond Sailfish OS and Qt 5.
2. Match the existing C++/QML style and keep user-facing text in English.
3. Do not add network access or persistent personal data.
4. Run the checks below before submitting.

```bash
sfdk -c target=SailfishOS-5.1.0.11-aarch64 build
sfdk -c target=SailfishOS-5.1.0.11-aarch64 check -s harbour \
  RPMS/harbour-saildice-*.aarch64.rpm
```

Run the binary with `--selftest` and `--qmlcheck` in the SDK target. Visual
changes should be tested on a real device.

By contributing, you agree that your changes are licensed under the MIT
License.
