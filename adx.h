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
    uint8_t info_flag;

    void (*proc)(adx_config_t* adx_conf, SF_INFO* sf_info, double* x, double** x_proc);
    int (*write)(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc);
} adx_config_t;

void set_defaults(adx_config_t* adx_conf);
int get_options(int* argc, char** argv, adx_config_t* adx_conf);
int open_file(SNDFILE** file, SF_INFO* sf_info, adx_config_t* adx_conf);
int read_file_data_double(SNDFILE* file, SF_INFO* sf_info, size_t* size, double** x);
void convert_short_buf_to_uint8(short* s16_buf, size_t size, uint8_t* u8_buf);
int read_file_data_uint8(SNDFILE* file, SF_INFO* sf_info, size_t* size, uint8_t** x);
void proc_mono(adx_config_t* adx_conf, SF_INFO* sf_info, double* x, double** x_proc);
void proc_cpy(adx_config_t* adx_conf, SF_INFO* sf_info, double* x, double** x_proc);
int mix2mono(size_t size, int channels, double* x, double** x_mono);
char* get_sndfile_major_format(SF_INFO* sf_info);
char* get_sndfile_subtype(SF_INFO* sf_info);
int output_input_file_info(SF_INFO* sf_info, adx_config_t* adx_conf);
int select_output_format(adx_config_t* adx_conf, char* str);
int write_file_csv_horizontal(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc);
int write_file_csv_vertical(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc);
int write_stdout(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc);
int write_file_bytes(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc);
int output_help();
