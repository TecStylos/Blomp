# Blocky Image Compression

Blomp is a simple image compression algorithm that compresses images by dividing them into squares of different sizes and averaging the colors in each square.

## Gettings Started

### Cloning the Repository
```bash
git clone https://github.com/TecStylos/blomp.git
```
### Compiling blomp
```bash
./build.sh [configuration]
```
Possible configurations are:
 - Release
 - Debug
 - RelWithDebInfo

The default configuration is Release.

Binaries are stored in the `bin/[configuration]` directory.

### Running blomp

```bash
./bin/[configuration]/blomp [arguments]
```

All arguments are documented in [src/BlompHelp.h](src/BlompHelp.h).

And can be viewed via the `help` argument (Without leading dashes).