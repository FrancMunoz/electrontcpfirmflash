# OTA TPCClient Updater

A TCPClient to OTA update Particle Electron without cloud.

## Notes

- Just compile and run. The firwmare that's downloaded from my server only sends two strings to ```Serial``` it's about 4Kb because it's the same code without the call to update function... to have a litle big download.
- Remember to update/remove ```cellular_credentials_set```.

#### Next Steps:

- I didn't have more time to test SSL
- Update system firmware.
