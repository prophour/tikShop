# TIKdevil


Now we can build TIKs (eShop TIcKets) on the 3DS!
"Download & Generate" option is recommended for first time use, it will download lastest encTitleKeys.bin on the 3DS and generate the TIKs out of it.

*Note: it can take half hour or so, the screen can seem idle but it's processing so yeah. I will look forward to improve this*

Next time uses, for like updating, you can use "Download & Delta Generate" for generating only new TIKs.

*Note: it will save ~20 minutes, but it's still slow... looking forward to improve this too."*


## Things to do

>Install TIKs right from TIKdevil

>Check for already installed TIKs

>Region-generations (optional)

>Improve generation speed


# Building
TIKdevil depends on [libctru](https://github.com/smealum/ctrulib) and a git submodule that will need to be fetched.

When initially fetching the project, the easiest way to get the code and submodules for building is the following:

`git clone --recursive https://github.com/Kyraminol/TIKdevil.git`

If you have already checked out the code without the submodules, you can run the following command in the project directory to fetch the submodule data:

`git submodule update --init --recursive`

## Credits

Thanks to cearp and Drakia for CIAngel!
Many thanks for machinamentum and FirmwareDownloader! Thanks to make_cdn_cia!
License is GPL to comply with the usage of make_cdn_cia code.