#include "adx.h"

void set_defaults(adx_config_t* restrict adx_conf)
{
    memset(adx_conf->ofile, '\0', MAX_STR);
    adx_conf->info_flag = 0;
    adx_conf->precision = 5;
    adx_conf->set_adx = &set_adx_auto;
    adx_conf->proc = &proc_cpy;
    adx_conf->write = &write_file_column;
}

int get_options(int argc, char** restrict argv, adx_config_t* restrict adx_conf)
{
    char strval[MAX_STR];
    uint8_t u8val = 0;

    if (argc == 1) {
        fprintf(stdout, WELCOME_STR);

        return 1;
    }

    if (argc == 2) {
        if (!(strcmp("--version", argv[1]))) {
            fprintf(stdout, VERSION_STR);

            return 1;
        }

        if (!(strcmp("--help", argv[1]))) {
            output_help();

            return 1;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && argv[i - 1][0] != '-') {
            CHECK_STR_LEN(argv[i]);
            strcpy(adx_conf->ifile, strval);
            continue;
        }

        if (!(strcmp("-i", argv[i])) || !(strcmp("--input", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(adx_conf->ifile, strval);
            i++;
            continue;
        }

        if (!(strcmp("-o", argv[i])) || !(strcmp("--output", argv[i]))) {
            CHECK_STR_LEN(argv[i + 1]);
            strcpy(adx_conf->ofile, strval);
            i++;
            continue;
        }

        if (!(strcmp("-m", argv[i])) || !(strcmp("--mono", argv[i]))) {
            adx_conf->proc = &proc_mono;
            continue;
        }

        if (!(strcmp("-r", argv[i])) || !(strcmp("--read-format", argv[i]))) {
            CHECK_RET(select_adx_settings(adx_conf, argv[i + 1]));
            i++;
            continue;
        }

        if (!(strcmp("-p", argv[i])) || !(strcmp("--precision", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%hhd", &u8val));
            adx_conf->precision = u8val;
            i++;
            continue;
        }

        if (!(strcmp("-f", argv[i])) || !(strcmp("--output-format", argv[i]))) {
            select_output_format(adx_conf, argv[i + 1]);
            i++;
            continue;
        }

        if (!(strcmp("--info", argv[i]))) {
            adx_conf->info_flag = 1;
            continue;
        }

        fprintf(stderr, "\nNo such option '%s'. Please check inputs.\n", argv[i]);

        return 1;
    }
    return 0;
}

char* get_datetime_string()
{
    time_t time_since_epoch = time(NULL);
    struct tm* tm = localtime(&time_since_epoch);
    static char s[13];
    strftime(s, sizeof(s), "%d%m%y%H%M%S", tm);

    return s;
}

void generate_file_name(char* restrict ofile, char* restrict ifile)
{
    if (ofile[0] != '\0' ) {

        return;
    }

    char ifile_no_extension[MIN_STR];

    /* Remove the extension in the input file name */
    strncpy(ifile_no_extension, ifile, strlen(ifile) - 4);

    /* Fix an issue when the copied string is not terminated correctly */
    ifile_no_extension[strlen(ifile) - 4] = '\0';

    /* Remove the path specifier */
    if (ifile_no_extension[0] == '.' && ifile_no_extension[1] == '\\') {
        memmove(ifile_no_extension, ifile_no_extension + 2, MIN_STR - 2);
    }

    sprintf(ofile, "adx-%s-%s.txt", ifile_no_extension, get_datetime_string()); 
}

int open_file(SNDFILE** restrict file, SF_INFO* restrict sf_info, adx_config_t* restrict adx_conf)
{
    *file = sf_open(adx_conf->ifile, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "\n%sPossible input audio file error.\n", sf_strerror(*file));

        return 1;
    };

    return 0;
}

int select_adx_settings(adx_config_t* restrict adx_conf, char* restrict strval)
{
    adx_conf->set_adx = NULL;

    if (!(strcmp("raw", strval))) {
        adx_conf->set_adx = &set_adx_uint8;
    }
    if (!(strcmp("uint8", strval))) {
        adx_conf->set_adx = &set_adx_uint8;
    }
    if (!(strcmp("int8", strval))) {
        adx_conf->set_adx = &set_adx_int8;
    }
    if (!(strcmp("short", strval))) {
        adx_conf->set_adx = &set_adx_short;
    }
    if (!(strcmp("integer", strval))) {
        adx_conf->set_adx = &set_adx_int;
    }
    if (!(strcmp("float", strval))) {
        adx_conf->set_adx = &set_adx_float;
    }
    if (!(strcmp("double", strval))) {
        adx_conf->set_adx = &set_adx_double;
    }

    if (!(adx_conf->set_adx)) {
        fprintf(stderr, "\nRead type '%s' not available.\n", strval); 
        return 1;
    }

    return 0;
}

int read_file_data_raw(adx_config_t* restrict adx_conf, SNDFILE* restrict sndfile, SF_INFO* restrict sf_info, void** restrict x)
{
    *x = malloc(adx_conf->input_file_size * adx_conf->data_size);
    sf_count_t sf_count = sf_read_raw(sndfile, *x, adx_conf->input_file_size * adx_conf->data_size);
    if (sf_count != adx_conf->input_file_size * adx_conf->data_size) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n", sf_count, adx_conf->input_file_size * adx_conf->data_size);

        return 1;
    }

    return 0;
}

int read_file_data_short(adx_config_t* restrict adx_conf, SNDFILE* restrict sndfile, SF_INFO* restrict sf_info, void** restrict x)
{
    *x = calloc(adx_conf->input_file_size, adx_conf->data_size);
    sf_count_t sf_count = sf_readf_short(sndfile, *x, sf_info->frames);
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

int read_file_data_int(adx_config_t* restrict adx_conf, SNDFILE* restrict sndfile, SF_INFO* restrict sf_info, void** restrict x)
{
    *x = calloc(adx_conf->input_file_size, adx_conf->data_size);
    sf_count_t sf_count = sf_readf_int(sndfile, *x, sf_info->frames);
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

int read_file_data_float(adx_config_t* restrict adx_conf, SNDFILE* restrict sndfile, SF_INFO* restrict sf_info, void** restrict x)
{
    *x = calloc(adx_conf->input_file_size, adx_conf->data_size);
    sf_count_t sf_count = sf_readf_float(sndfile, *x, sf_info->frames);
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

int read_file_data_double(adx_config_t* restrict adx_conf, SNDFILE* restrict sndfile, SF_INFO* restrict sf_info, void** restrict x)
{
    *x = calloc(adx_conf->input_file_size, adx_conf->data_size);
    sf_count_t sf_count = sf_readf_double(sndfile, *x, sf_info->frames);
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

void set_adx_uint8(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info)
{
    adx_conf->input_file_size = sf_info->frames * sf_info->channels;
    adx_conf->data_size = sizeof(uint8_t);
    strcpy(adx_conf->format_specifier, "%hhu");
    adx_conf->read = &read_file_data_raw;
    adx_conf->print = &printf_uint8;
    adx_conf->mix2mono = &mix2mono_uint8;
    fprintf(stdout,  "Set ADX settings for type 'uint8'.\n");
}

void set_adx_int8(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info)
{
    adx_conf->input_file_size = sf_info->frames * sf_info->channels;
    adx_conf->data_size = sizeof(int8_t);
    strcpy(adx_conf->format_specifier, "%hh");
    adx_conf->read = &read_file_data_raw;
    adx_conf->print = &printf_int8;
    adx_conf->mix2mono = &mix2mono_int8;
    fprintf(stdout,  "Set ADX settings for type 'int8'.\n");
}

void set_adx_short(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info)
{
    adx_conf->input_file_size = sf_info->frames * sf_info->channels;
    adx_conf->data_size = sizeof(short);
    strcpy(adx_conf->format_specifier, "%hd");
    adx_conf->read = &read_file_data_short;
    adx_conf->print = &printf_short;
    adx_conf->mix2mono = &mix2mono_short;
    fprintf(stdout,  "Set ADX settings for type 'short'.\n");
}

void set_adx_int(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info)
{
    adx_conf->input_file_size = sf_info->frames * sf_info->channels;
    adx_conf->data_size = sizeof(int);
    strcpy(adx_conf->format_specifier, "%d");
    adx_conf->read = &read_file_data_int;
    adx_conf->print = &printf_int;
    adx_conf->mix2mono = &mix2mono_int;
    fprintf(stdout,  "Set ADX settings for type 'int'.\n");
}

void set_adx_float(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info)
{
    adx_conf->input_file_size = sf_info->frames * sf_info->channels;
    adx_conf->data_size = sizeof(float);
    set_precision_format(adx_conf->format_specifier, adx_conf->precision);
    adx_conf->read = &read_file_data_float;
    adx_conf->print = &printf_float;
    adx_conf->mix2mono = &mix2mono_float;
    fprintf(stdout,  "Set ADX settings for type 'float'.\n");

}

void set_adx_double(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info)
{
    adx_conf->input_file_size = sf_info->frames * sf_info->channels;
    adx_conf->data_size = sizeof(double);
    set_precision_format(adx_conf->format_specifier, adx_conf->precision);
    adx_conf->read = &read_file_data_double;
    adx_conf->print = &printf_double;
    adx_conf->mix2mono = &mix2mono_double;
    fprintf(stdout,  "Set ADX settings for type 'double'.\n");
}

void set_adx_auto(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info) 
{
    const uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format & subtype_mask;

    fprintf(stdout, "Using auto-detection for the audio file encoding.\n");

    switch (subtype) {
        case SF_FORMAT_PCM_U8:
        case SF_FORMAT_ULAW:
        case SF_FORMAT_ALAW:
            set_adx_uint8(adx_conf, sf_info);
            break;
        case SF_FORMAT_PCM_S8:
            set_adx_int8(adx_conf, sf_info);
            break;
        case SF_FORMAT_PCM_16:
            set_adx_short(adx_conf, sf_info);
            break;
        case SF_FORMAT_PCM_32:
            set_adx_int(adx_conf, sf_info);
            break;
        case SF_FORMAT_FLOAT:
            set_adx_float(adx_conf, sf_info);
            break;
        case SF_FORMAT_DOUBLE:
            set_adx_double(adx_conf, sf_info);
            break;
        default:
            fprintf(stderr, "\nAuto-format detection for the detected format is not implemented. Resorting to reading as double.\n");
            set_adx_double(adx_conf, sf_info);
            break;
    }
}

void set_precision_format(char format[9], uint8_t precision)
{
    sprintf(format, "%%.%dlf", precision);
}

const char* get_sndfile_major_format(SF_INFO* restrict sf_info)
{
    SF_FORMAT_INFO format_info ;
    int k, count;
    const uint32_t format_mask = 0x00FF0000;
    const uint32_t major_format = sf_info->format & format_mask;

    sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int));

    for (k = 0 ; k < count ; k++) {
        format_info.format = k;
        sf_command(NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof(format_info));
        if (major_format == format_info.format) {
            return format_info.name;
        }
    }

    return "N/A";
}

const char* get_sndfile_subtype(SF_INFO* restrict sf_info)
{
    SF_FORMAT_INFO format_info ;
    int k, count;
    const uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format & subtype_mask;

    sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof (int));

    for (k = 0 ; k < count ; k++) {
        format_info.format = k;
        sf_command(NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof(format_info));
        if (subtype == format_info.format) {
            return format_info.name;
        }
    }

    return "N/A";
}

int output_input_file_info(SF_INFO* restrict sf_info, adx_config_t* restrict adx_conf)
{
    if (adx_conf->info_flag) {
        fprintf(stdout, "\n\t\t---FILE INFO---\n");
        fprintf(stdout, "\tFile Name: %s\n", adx_conf->ifile);
        fprintf(stdout, "\tSample Rate: %d\n", sf_info->samplerate);
        fprintf(stdout, "\tSamples: %lld\n", sf_info->frames);
        fprintf(stdout, "\tChannels: %d\n", sf_info->channels);
        fprintf(stdout, "\tSections: %d\n", sf_info->sections);
        fprintf(stdout, "\tSeekable: ");
        fprintf(stdout, sf_info->seekable ? "Yes\n" : "No\n");
        fprintf(stdout, "\tFormat: %s\n", get_sndfile_major_format(sf_info));
        fprintf(stdout, "\tSubtype: %s\n", get_sndfile_subtype(sf_info));
        fprintf(stdout, "\t\t---------------\n\n");
    }

    return 0;
}

void proc_mono(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info, void* restrict x, void** restrict x_proc)
{
    /* Convert file data to mono */
    adx_conf->input_file_size /= sf_info->channels;
    adx_conf->mix2mono(adx_conf->input_file_size, sf_info->channels, x, x_proc);
    printf("Mixed file to mono.\n");
}

void proc_cpy(adx_config_t* restrict adx_conf, SF_INFO* restrict sf_info, void* restrict x, void** restrict x_proc)
{
    /* Preserve the original data size */
    *x_proc = malloc(adx_conf->input_file_size * adx_conf->data_size);
    memcpy(*x_proc, x, adx_conf->input_file_size * adx_conf->data_size);
    printf("Preserved channel amount.\n");
}

void mix2mono_uint8(size_t size, int channels, void* restrict x, void** restrict x_mono)
{
    *x_mono = calloc(size, sizeof(uint8_t));
    for (uint64_t i = 0; i < size; i++) {
        for (uint16_t c = 0; c < channels; c++) {
            ((uint8_t* )*x_mono)[i] += (((uint8_t* )x)[channels * i + c]/channels);
        }
    }
}

void mix2mono_int8(size_t size, int channels, void* restrict x, void** restrict x_mono)
{
    *x_mono = calloc(size, sizeof(int8_t));
    for (uint64_t i = 0; i < size; i++) {
        for (uint16_t c = 0; c < channels; c++) {
            ((int8_t* )*x_mono)[i] += (((int8_t* )x)[channels * i + c]/channels);
        }
    }
}

void mix2mono_short(size_t size, int channels, void* restrict x, void** restrict x_mono)
{
    *x_mono = calloc(size, sizeof(int16_t));
    for (uint64_t i = 0; i < size; i++) {
        for (uint16_t c = 0; c < channels; c++) {
            ((int16_t* )*x_mono)[i] += (((int16_t* )x)[channels * i + c]/channels);
        }
    }
}

void mix2mono_int(size_t size, int channels, void* restrict x, void** restrict x_mono)
{
    *x_mono = calloc(size, sizeof(int32_t));
    for (uint64_t i = 0; i < size; i++) {
        for (uint16_t c = 0; c < channels; c++) {
            ((int32_t* )*x_mono)[i] += (((int32_t* )x)[channels * i + c]/channels);
        }
    }
}

void mix2mono_float(size_t size, int channels, void* restrict x, void** restrict x_mono)
{
    *x_mono = calloc(size, sizeof(float));
    for (uint64_t i = 0; i < size; i++) {
        for (uint16_t c = 0; c < channels; c++) {
            ((float* )*x_mono)[i] += (((float* )x)[channels * i + c]/channels);
        }
    }
}

void mix2mono_double(size_t size, int channels, void* restrict x, void** restrict x_mono)
{
    *x_mono = calloc(size, sizeof(double));
    for (uint64_t i = 0; i < size; i++) {
        for (uint16_t c = 0; c < channels; c++) {
            ((double* )*x_mono)[i] += (((double* )x)[channels * i + c]/channels);
        }
    }
}

int select_output_format(adx_config_t* restrict adx_conf, char* restrict strval)
{
    adx_conf->write = NULL;

    if (!(strcmp("csv", strval))) {
        adx_conf->write = &write_file_csv;
    }
    if (!(strcmp("column", strval))) {
        adx_conf->write = &write_file_column;
    }
    if (!(strcmp("stdout", strval))) {
        adx_conf->write = &write_stdout;
    }
    if (!(strcmp("stdout-csv", strval))) {
        adx_conf->write = &write_stdout_csv;
    }
    if (!(strcmp("hex-dump", strval))) {
        adx_conf->write = &write_file_hex_dump;
    }

    if (!(adx_conf->write)) {
        fprintf(stderr, "\nOutput format '%s' doesn't exist.\n", strval);

        return 1;
    }

    return 0;
}

void printf_uint8(FILE* restrict ofile, char format_specifier[9], void* restrict x, size_t index)
{
    fprintf(ofile, format_specifier,  ((uint8_t*)x)[index]);
}

void printf_int8(FILE* restrict ofile, char format_specifier[9], void* restrict x, size_t index)
{
    fprintf(ofile, format_specifier,  ((int8_t*)x)[index]);
}

void printf_short(FILE* restrict ofile, char format_specifier[9], void* restrict x, size_t index)
{
    fprintf(ofile, format_specifier,  ((short*)x)[index]);
}

void printf_int(FILE* restrict ofile, char format_specifier[9], void* restrict x, size_t index)
{
    fprintf(ofile, format_specifier,  ((int*)x)[index]);
}

void printf_float(FILE* restrict ofile, char format_specifier[9], void* restrict x, size_t index)
{
    fprintf(ofile, format_specifier,  ((float*)x)[index]);
}

void printf_double(FILE* restrict ofile, char format_specifier[9], void* restrict x, size_t index) {
    fprintf(ofile, format_specifier,  ((double*)x)[index]);
}

int write_file_csv(adx_config_t* restrict adx_conf, void* restrict x_proc)
{
    FILE *ofile = fopen(adx_conf->ofile, "w");
    if(!(ofile)) {
        fprintf(stderr, "\nUnable to open file for output.");

        return 1;
    };

    for (size_t i = 0; i < adx_conf->input_file_size - 1; i++){
        adx_conf->print(ofile, adx_conf->format_specifier, x_proc, i);
        fprintf(ofile, ",");
    }
    adx_conf->print(ofile, adx_conf->format_specifier, x_proc, adx_conf->input_file_size);

    fclose(ofile);

    return 0;
}

int write_file_column(adx_config_t* restrict adx_conf, void* restrict x_proc)
{
    FILE *ofile = fopen(adx_conf->ofile, "w");
    if(!(ofile)) {
        fprintf(stderr, "\nUnable to open file for output.");

        return 1;
    };

    for (size_t i = 0; i < adx_conf->input_file_size; i++){
        adx_conf->print(ofile, adx_conf->format_specifier, x_proc, i);
        fprintf(ofile, "\n");
    }

    fclose(ofile);

    return 0;
}

int write_file_hex_dump(adx_config_t* restrict adx_conf, void* restrict x_proc)
{
    FILE *ofile = fopen(adx_conf->ofile, "wb");
    if(!(ofile)) {
        fprintf(stderr, "\nUnable to open file for output.\n");

        return 1;
    };

    fwrite(x_proc, sizeof(uint8_t), adx_conf->input_file_size, ofile);

    fclose(ofile);

    return 0;
}

int write_stdout(adx_config_t* restrict adx_conf, void* restrict x_proc)
{
    for (size_t i = 0; i < adx_conf->input_file_size; i++){
        adx_conf->print(stdout, adx_conf->format_specifier, x_proc, i);
        fprintf(stdout, "\n");
    }

    return 0;
}

int write_stdout_csv(adx_config_t* restrict adx_conf, void* restrict x_proc)
{
    for (size_t i = 0; i < adx_conf->input_file_size - 1; i++){
        adx_conf->print(stdout, adx_conf->format_specifier, x_proc, i);
        fprintf(stdout, ",");
    }
    adx_conf->print(stdout, adx_conf->format_specifier, x_proc, adx_conf->input_file_size);

    return 0;
}

int output_help()
{
    printf( 
            "ADX options,\n\n"
            "Basic usage, `adx <audio file>`.\n"
            "\n\t-i,\t--input <file>\t= Path or name of the input file."
            "\n\t-o,\t--output <file>\t= Path or name of the output file."
            "\n\t\t--info\t\t= Output to stdout some info about the input file."
            "\n\t\t--mono\t\t= Convert the data to single channel data."
            "\n\t-f,\t--output-format <format>= Choose between 'column', 'csv', 'stdout', 'stdout-csv', and 'hex-dump'."
            "\n\t-p,\t--precision <number>\t= Enter a decimal number for the precision in the double/float outputs. Default is 5."
            "\n\t-r,\t--read-format <format>\t= Force reading in a specific format. Choose between 'raw', 'uint8', 'int8', 'short', 'integer', 'float', and 'double'."
            "\n"
          );

    return 0;
}

