# Audio file Data Extraction tool (ADX)
![badge](https://badgen.net/badge/Coverage/100/blue) ![coverity](https://scan.coverity.com/projects/31872/badge.svg)

To extract the data from your audio file.

## Features
- Auto-detects the input file encoding and outputs that data to a text file.
- Can specify the encoding to use for extraction which converts between short, integer, float, and double types (not available for unsigned/signed 8-bit integers).
- Convert the audio file to a single-channel file and then extract the data from it.
- Specify the precision of the extracted data when using decimal data types.

## Installing
Currently an automatic installation exists only for Windows, and binaries are built only for Windows. For other Operating Systems you need to build from source.

### Windows
To install automatically, use the install script located in `install/` by executing the command below in a PowerShell terminal with Admin rights,

```
irm "https://raw.githubusercontent.com/ymich9963/adx/refs/heads/main/install/adx-windows-install.ps1" | iex
```

The script downloads the executable, moves it to `C:\Program Files\adx\`, and adds that path to the system environment variable. If you do not want the automated script feel free to download the executable or build from source. In case your organisation doesn't allow you to install from the script due to protected paths, download the script and change the `$ADX_install_path` variable to a location that suits you.

### macOS & Linux
Please the Building section. Use `make` to build from source.

## Usage 
Use `adx` by executing,
```
adx <audio file>
```

The `--help` option is listed below,
```
ADX options,

Basic usage, `adx <audio file>`.

        -i,     --input <file>  = Path or name of the input file.
        -o,     --output <file> = Path or name of the output file.
                --info          = Output to stdout some info about the input file.
        -f,     --output-format <format>= Choose between 'column', 'csv', 'stdout', 'stdout-csv', and 'hex-dump'.
        -p,     --precision <number>    = Enter a decimal number for the precision in the double/float outputs. Default is 5.
        -e,     --encoding <encoding>   = Force reading in a specific format. Choose between 'raw', 'uint8', 'int8', 'short', 'integer', 'float', and 'double'.
```

You may need to convert your multi-channel audio file into a single-channel (mono) audio file. If so check out [`mix2mono`](https://github.com/ymich9963/mix2mono).
 
## Building
Only dependency is [libsndfile](https://github.com/libsndfile/libsndfile). Clone the repository and go to this directory. Using `make` simply run in a command line,
```
make
```
And `adx` should be build in a new `build/` directory.

## Tests and Coverage
Running the tests or coverage can be done by running,
```
make test
```
```
make coverage
```
Testing suite used is [Unity](https://github.com/ThrowTheSwitch/Unity) and LLVM-COV for coverage.

