#pragma once

#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MIN_STR 255
#define MAX_STR 22 + MIN_STR
#define WELCOME_STR "Audio Data Extractor (ADX) by Yiannis Michael (ymich9963), 2025. \n\n Basic usage, `adx <Audio File>`. Use the `--help` option for details on the few options in the tool, and '--version' for version information.\n\n"
#define VERSION_STR "\nadx v0.1.0, by Yiannis Michael (ymich9963), 2025.\n\n"
#define SND_MAJOR_FORMAT_NUM 27
#define SND_SUBTYPE_NUM 36

/* Check macros */
/* Check response from sscanf */
#define	CHECK_RES(x) ({ if (!(x)) { \
							fprintf(stderr, "Argument entered was wrong...\n"); \
							return 1; \
						} \
					  })

/* Check string length */
#define	CHECK_STR_LEN(x) ({ if (strlen((x)) > MAX_STR) { \
							fprintf(stderr, "Argument string length was too large. Max is %d.\n", MAX_STR); \
							return 1; \
						} \
					  })

/* Check if an error occured to exit program */
#define	CHECK_ERR(x) ({ if ((x)) { \
							exit(EXIT_FAILURE); \
						} \
					  })

/* Check if a function returns failure */
#define	CHECK_RET(x) ({ if ((x)) { \
							return 1; \
						} \
					  })

typedef struct ADX_Config adx_config_t;

typedef struct ADX_Config {
    char ifile[MAX_STR];        // Input file max string
    char ofile[MAX_STR];        // Output file max string
    size_t input_file_size;     // Input file size
    uint8_t data_size;          // Input file data size
    char format_specifier[9];   // Format specifier to output the data
    uint8_t precision;          // Specify the output precision when using decimal types
    uint8_t info_flag;          // Set the info flag

    /* Function struct members */
    void (*set_adx)(adx_config_t* adx_conf, SF_INFO* sf_info);
    int (*read)(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);
    void (*proc)(adx_config_t* adx_conf, SF_INFO* sf_info, void* x, void** x_proc);
    void (*mix2mono)(size_t size, int channels, void* x, void** x_mono);
    void (*print)(FILE *ofile, char format_specifier[9], void* x, size_t index);
    int (*write)(adx_config_t* adx_conf, void* x_proc);
} adx_config_t;

/**
 * @brief Set some defaults to ensure correct behaviour.
 *
 * @param adx_conf ADX config struct.
 */
void set_defaults(adx_config_t* adx_conf);

/**
 * @brief Get the options specified from the CLI.
 *
 * @param argc Option count.
 * @param argv Option string array.
 * @param adx_conf ADX config struct.
 * @return Success or failure.
 */
int get_options(int argc, char** argv, adx_config_t* adx_conf);

/**
 * @brief Get the date/time string to be used in the file name.
 *
 * @return Date/time string in ddmmyyHHMMSS format.
 */
char* get_datetime_string();

/**
 * @brief Generate the output file name based on the input file name and the current date/time.
 *
 * @param ofile Output file name.
 * @param ifile Input file name.
 */
void generate_file_name(char* ofile, char* ifile);

/**
 * @brief Specify what settings to use in the tool. This is what gets used if '-r/--read-format' is specified.
 *
 * @param adx_conf ADX config struct.
 * @param strval String containing the read format selection.
 * @return Success or failure.
 */
int select_adx_settings(adx_config_t* adx_conf, char* strval);

/**
 * @brief Select the output format of the data.
 *
 * @param adx_conf ADX config struct.
 * @param strval Output format string.
 * @return Success or failure.
 */
int select_output_format(adx_config_t* adx_conf, char* strval);

/**
 * @brief Open the input file.
 *
 * @param file SNDFILE pointer to the input file.
 * @param sf_info SF_INFO type from libsndfile.
 * @param adx_conf ADX config struct.
 * @return Success or failure.
 */
int open_file(SNDFILE** file, SF_INFO* sf_info, adx_config_t* adx_conf);

/**
 * @brief Automatically detect the input file format and data encoding.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_auto(adx_config_t* adx_conf, SF_INFO* sf_info); 

/**
 * @brief Set ADX to read the input file as unsigned 8-bit integer.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_uint8(adx_config_t* adx_conf, SF_INFO* sf_info);

/**
 * @brief Set ADX to read the input file as signed 8-bit integer.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_int8(adx_config_t* adx_conf, SF_INFO* sf_info);

/**
 * @brief Set ADX to read the input file as signed 16-bit integer.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_short(adx_config_t* adx_conf, SF_INFO* sf_info);

/**
 * @brief Set ADX to read the input file as signed 32-bit integer.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_int(adx_config_t* adx_conf, SF_INFO* sf_info);

/**
 * @brief Set ADX to read the input file as a float.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_float(adx_config_t* adx_conf, SF_INFO* sf_info);

/**
 * @brief Set ADX to read the input file as a double.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 */
void set_adx_double(adx_config_t* adx_conf, SF_INFO* sf_info);


/**
 * @brief Reads the file data as bytes. Used for types other than short, int, float, and double.
 *
 * @param adx_conf ADX config struct.
 * @param sndfile  SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_file_data_raw(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);

/**
 * @brief Reads the file data as type short and stores it in the data buffer.
 *
 * @param adx_conf ADX config struct.
 * @param sndfile  SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_file_data_short(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);

/**
 * @brief Reads the file data as type int and stores it in the data buffer.
 *
 * @param adx_conf ADX config struct.
 * @param sndfile  SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_file_data_int(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);

/**
 * @brief Reads the file data as type float and stores it in the data buffer.
 *
 * @param adx_conf ADX config struct.
 * @param sndfile  SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_file_data_float(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);

/**
 * @brief Reads the file data as type double and stores it in the data buffer.
 *
 * @param adx_conf ADX config struct.
 * @param sndfile  SNDFILE pointer.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @return Success or failure.
 */
int read_file_data_double(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);

/**
 * @brief Set the precision of the outputed decimals.
 *
 * @param format Format specifier.
 * @param precision Precision number.
 */
void set_precision_format(char format[9], uint8_t precision);

/**
 * @brief Process function to process the data to mono.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @param x_proc Processed data buffer.
 */
void proc_mono(adx_config_t* adx_conf, SF_INFO* sf_info, void* x, void** x_proc);

/**
 * @brief Process function to just preserve the data as it is at the input. Copies the data to the new buffer.
 *
 * @param adx_conf ADX config struct.
 * @param sf_info SF_INFO type from libsndfile.
 * @param x Pointer to data buffer.
 * @param x_proc Processed data buffer.
 */
void proc_cpy(adx_config_t* adx_conf, SF_INFO* sf_info, void* x, void** x_proc);

/**
 * @brief Mix to mono function for unsigned 8-bit integer type data.
 *
 * @param size Size of data.
 * @param channels Number of channels in the input audio file.
 * @param x Input data buffer.
 * @param x_mono Output data buffer.
 */
void mix2mono_uint8(size_t size, int channels, void* x, void** x_mono);

/**
 * @brief Mix to mono function for signed 8-bit integer type data.
 *
 * @param size Size of data.
 * @param channels Number of channels in the input audio file.
 * @param x Input data buffer.
 * @param x_mono Output data buffer.
 */
void mix2mono_int8(size_t size, int channels, void* x, void** x_mono);

/**
 * @brief Mix to mono function for signed 16-bit integer type data.
 *
 * @param size Size of data.
 * @param channels Number of channels in the input audio file.
 * @param x Input data buffer.
 * @param x_mono Output data buffer.
 */
void mix2mono_short(size_t size, int channels, void* x, void** x_mono);

/**
 * @brief Mix to mono function for signed 32-bit integer type data.
 *
 * @param size Size of data.
 * @param channels Number of channels in the input audio file.
 * @param x Input data buffer.
 * @param x_mono Output data buffer.
 */
void mix2mono_int(size_t size, int channels, void* x, void** x_mono);

/**
 * @brief Mix to mono function for float.
 *
 * @param size Size of data.
 * @param channels Number of channels in the input audio file.
 * @param x Input data buffer.
 * @param x_mono Output data buffer.
 */
void mix2mono_float(size_t size, int channels, void* x, void** x_mono);

/**
 * @brief Mix to mono function for double.
 *
 * @param size Size of data.
 * @param channels Number of channels in the input audio file.
 * @param x Input data buffer.
 * @param x_mono Output data buffer.
 */
void mix2mono_double(size_t size, int channels, void* x, void** x_mono);

/**
 * @brief Get the SNDFILE major format string. Same as descriptions given in the documentation.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @return Major format string
 */
const char* get_sndfile_major_format(SF_INFO* sf_info);

/**
 * @brief Get the SNDFILE subtype string. Same as subtypes given in the documentation.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @return Subtype string.
 */
const char* get_sndfile_subtype(SF_INFO* sf_info);

/**
 * @brief Output the input file info to stdout. Specified with --info.
 *
 * @param sf_info Pointer to SF_INFO variable containing file information.
 * @param adx_conf ADX config struct.
 * @return Success or failure.
 */
int output_input_file_info(SF_INFO* sf_info, adx_config_t* adx_conf);

/**
 * @brief Prints an unsigned 8-bit integer to the specified FILE buffer.
 *
 * @param ofile Output FILE buffer.
 * @param format_specifier Format specifier used in the printf function.
 * @param x Data buffer.
 * @param index Index to data buffer.
 */
void printf_uint8(FILE *ofile, char format_specifier[9], void* x, size_t index);

/**
 * @brief Prints an signed 8-bit integer to the specified FILE buffer.
 *
 * @param ofile Output FILE buffer.
 * @param format_specifier Format specifier used in the printf function.
 * @param x Data buffer.
 * @param index Index to data buffer.
 */
void printf_int8(FILE *ofile, char format_specifier[9], void* x, size_t index);

/**
 * @brief Prints a short-type to the specified FILE buffer.
 *
 * @param ofile Output FILE buffer.
 * @param format_specifier Format specifier used in the printf function.
 * @param x Data buffer.
 * @param index Index to data buffer.
 */
void printf_short(FILE *ofile, char format_specifier[9], void* x, size_t index);

/**
 * @brief Prints an int-type to the specified FILE buffer.
 *
 * @param ofile Output FILE buffer.
 * @param format_specifier Format specifier used in the printf function.
 * @param x Data buffer.
 * @param index Index to data buffer.
 */
void printf_int(FILE *ofile, char format_specifier[9], void* x, size_t index);

/**
 * @brief Prints a float-type to the specified FILE buffer.
 *
 * @param ofile Output FILE buffer.
 * @param format_specifier Format specifier used in the printf function.
 * @param x Data buffer.
 * @param index Index to data buffer.
 */
void printf_float(FILE *ofile, char format_specifier[9], void* x, size_t index);

/**
 * @brief Prints a double-type to the specified FILE buffer.
 *
 * @param ofile Output FILE buffer.
 * @param format_specifier Format specifier used in the printf function.
 * @param x Data buffer.
 * @param index Index to data buffer.
 */
void printf_double(FILE *ofile, char format_specifier[9], void* x, size_t index);

/**
 * @brief Write the processed data buffer to the specified output file in CSV format.
 *
 * @param adx_conf ADX config struct.
 * @param x_proc Processed data buffer.
 * @return Success or failure.
 */
int write_file_csv(adx_config_t* adx_conf, void* x_proc);

/**
 * @brief Write the processed data buffer to the specified output file in a single column.
 *
 * @param adx_conf ADX config struct.
 * @param x_proc Processed data buffer.
 * @return Success or failure.
 */
int write_file_column(adx_config_t* adx_conf, void* x_proc);

/**
 * @brief Write the processed data buffer to stdout in a single column.
 *
 * @param adx_conf ADX config struct.
 * @param x_proc Processed data buffer.
 * @return Success or failure.
 */
int write_stdout(adx_config_t* adx_conf, void* x_proc);

/**
 * @brief Write the processed data buffer to the stdout as CSV.
 *
 * @param adx_conf ADX config struct.
 * @param x_proc Processed data buffer.
 * @return Success or failure.
 */
int write_stdout_csv(adx_config_t* adx_conf, void* x_proc);

/**
 * @brief Write the processed data buffer to the specified output file as a hex dump.
 *
 * @param adx_conf ADX config struct.
 * @param x_proc Processed data buffer.
 * @return Success or failure.
 */
int write_file_hex_dump(adx_config_t* adx_conf, void* x_proc);

/**
 * @brief Output the --help option for ADX.
 *
 * @return Success or failure.
 */
int output_help();
