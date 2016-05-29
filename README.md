# TIKdevil

Fastest batch TIKs (eShop TIcKets) maker and installer on the planet, right on your 3DS!

# Features
- Doesn't anymore rely on encTitleKeys.bin nor any files, direct table parse from that title key site;
- **Complete revamp, 1 click take-them-all in less than 5 minutes**;
- **Reads already installed tickets and only generates missing ones**;
- **Direct installs generated tickets!**
- Removes temp files, leaving only table for local storage.

**Soon**
- Region filtering;
- Separate menu items for single things;
- QR to CIA for quick installing and updating TIKdevil;
- **eShop redirection**;
- More speeding and stuff :smile_cat: 

# Building
TIKdevil depends on [libctru
(https://github.com/smealum/ctrulib) and a git submodule that will need to be fetched.

LibCTRU v.1.1.0 or higher is required, this is not installed with devkitPro by default. You must compile yourself.

TinyXML2 is required, place the source in source folder before building.

The bt folder contains buildtools for the 3ds.


## Credits

Thanks to cearp and Drakia for CIAngel!
Many thanks for machinamentum and FirmwareDownloader! Thanks to make_cdn_cia!
License is GPL to comply with the usage of make_cdn_cia code.
