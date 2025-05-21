#include <sndfile.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_STR 255
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

extern char* snd_subtype_arr_desc[SND_SUBTYPE_NUM];
extern uint32_t snd_subtype_arr[SND_SUBTYPE_NUM];
extern char* snd_format_arr_desc[SND_MAJOR_FORMAT_NUM];
extern uint32_t snd_format_arr[SND_MAJOR_FORMAT_NUM];

typedef struct ADX_Config adx_config_t;

typedef struct ADX_Config {
    char ifile[MAX_STR];
    char ofile[MAX_STR];
    size_t file_size;
    uint8_t data_size;
    char format_specifier[9];
    uint8_t precision;
    uint8_t info_flag;

    void (*set_adx)(adx_config_t* adx_conf, SF_INFO* sf_info);
    void (*proc)(adx_config_t* adx_conf, SF_INFO* sf_info, void* x, void** x_proc);
    void (*mix2mono)(size_t size, int channels, void* x, void** x_mono);
    void (*print)(FILE *ofile, char format_specifier[9], void* x, size_t index);
    int (*write)(adx_config_t* adx_conf, SF_INFO* sf_info, void* x_proc);
} adx_config_t;

void set_defaults(adx_config_t* adx_conf);
int get_options(int* argc, char** argv, adx_config_t* adx_conf);
int select_adx_settings(adx_config_t* adx_conf, char* strval);
int select_output_format(adx_config_t* adx_conf, char* str);
int open_file(SNDFILE** file, SF_INFO* sf_info, adx_config_t* adx_conf);
int read_file_data_raw(adx_config_t* adx_conf, SNDFILE* sndfile, SF_INFO* sf_info, void** x);
void set_adx_auto(adx_config_t* adx_conf, SF_INFO* sf_info); 
void set_adx_uint8(adx_config_t* adx_conf, SF_INFO* sf_info);
void set_adx_int8(adx_config_t* adx_conf, SF_INFO* sf_info);
void set_adx_short(adx_config_t* adx_conf, SF_INFO* sf_info);
void set_adx_int(adx_config_t* adx_conf, SF_INFO* sf_info);
void set_adx_float(adx_config_t* adx_conf, SF_INFO* sf_info);
void set_adx_double(adx_config_t* adx_conf, SF_INFO* sf_info);
void set_precision_format(char format[9], uint8_t precision);
void convert_short_buf_to_uint8(short* s16_buf, size_t size, uint8_t* u8_buf);
void proc_mono(adx_config_t* adx_conf, SF_INFO* sf_info, void* x, void** x_proc);
void proc_cpy(adx_config_t* adx_conf, SF_INFO* sf_info, void* x, void** x_proc);
void mix2mono_uint8(size_t size, int channels, void* x, void** x_mono);
void mix2mono_int8(size_t size, int channels, void* x, void** x_mono);
void mix2mono_short(size_t size, int channels, void* x, void** x_mono);
void mix2mono_int(size_t size, int channels, void* x, void** x_mono);
void mix2mono_float(size_t size, int channels, void* x, void** x_mono);
void mix2mono_double(size_t size, int channels, void* x, void** x_mono);
const char* get_sndfile_major_format(SF_INFO* sf_info);
const char* get_sndfile_subtype(SF_INFO* sf_info);
int output_input_file_info(SF_INFO* sf_info, adx_config_t* adx_conf);
void printf_uint8(FILE *ofile, char format_specifier[9], void* x, size_t index);
void printf_int8(FILE *ofile, char format_specifier[9], void* x, size_t index);
void printf_short(FILE *ofile, char format_specifier[9], void* x, size_t index);
void printf_int(FILE *ofile, char format_specifier[9], void* x, size_t index);
void printf_float(FILE *ofile, char format_specifier[9], void* x, size_t index);
void printf_double(FILE *ofile, char format_specifier[9], void* x, size_t index);
int write_file_csv_horizontal(adx_config_t* adx_conf, SF_INFO* sf_info, void* x_proc);
int write_file_csv_vertical(adx_config_t* adx_conf, SF_INFO* sf_info, void* x_proc);
int write_stdout(adx_config_t* adx_conf, SF_INFO* sf_info, void* x_proc);
int write_file_hex_dump(adx_config_t* adx_conf, SF_INFO* sf_info, void* x_proc);
int output_help();
