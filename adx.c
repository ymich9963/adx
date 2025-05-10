#include "adx.h"

void set_defaults(adx_config_t* adx_conf)
{
    strcpy(adx_conf->ofile, "adx.txt");
    adx_conf->info_flag = 0;
    adx_conf->proc = &proc_cpy;
    adx_conf->write = &write_file_csv_vertical;
}

int get_options(int* restrict argc, char** restrict argv, adx_config_t* restrict adx_conf)
{
    char strval[MAX_STR];

    if (*argc == 1) {
        fprintf(stdout, WELCOME_STR);

        return 1;
    }

    if (*argc == 2) {
        if (!(strcmp("--version", argv[1]))) {
            fprintf(stdout, VERSION_STR);

            return 1;
        }

        if (!(strcmp("--help", argv[1]))) {
            output_help();

            return 1;
        }
    }

    for (int i = 1; i < *argc; i++) {
        if (argv[i][0] != '-' && argv[i - 1][0] != '-') {
            CHECK_RES(sscanf(argv[i], "%s", strval));
            strcpy(adx_conf->ifile, strval);
            continue;
        }

        if (!(strcmp("-i", argv[i])) || !(strcmp("--input", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            strcpy(adx_conf->ifile, strval);
            i++;
            continue;
        }

        if (!(strcmp("-o", argv[i])) || !(strcmp("--output", argv[i]))) {
            CHECK_RES(sscanf(argv[i + 1], "%s", strval));
            strcpy(adx_conf->ofile, strval);
            i++;
            continue;
        }

        if (!(strcmp("-m", argv[i])) || !(strcmp("--mono", argv[i]))) {
            adx_conf->proc = &proc_mono;
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

        fprintf(stderr, "\nNo such option '%s'. Please check inputs.\n\n", argv[i]);

        return 1;
    }
    return 0;
}

int open_file(SNDFILE** file, SF_INFO* sf_info, adx_config_t* adx_conf)
{
    *file = sf_open(adx_conf->ifile, SFM_READ, sf_info);
    if(!(*file)) {
        fprintf(stderr, "\nInput audio file error: %s.\n\n", sf_strerror(*file));

        return 1;
    };

    return 0;
}

int read_file_data_double(SNDFILE* file, SF_INFO* sf_info, size_t* size, double** x)
{
    *size = sf_info->frames * sf_info->channels;
    *x = calloc(*size, sizeof(double));
    sf_count_t sf_count = sf_readf_double(file, *x, sf_info->frames);
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n\n", sf_count, sf_info->frames);

        return 1;
    }

    return 0;
}

void convert_short_buf_to_uint8(short* s16_buf, size_t size, uint8_t* u8_buf)
{
    for (size_t i = 0; i < size; i++) {
        u8_buf[i] = (uint8_t) s16_buf[i];
    }
}

//TODO: Make the program able to handle this input
int read_file_data_uint8(SNDFILE* file, SF_INFO* sf_info, size_t* size, uint8_t** x)
{
    *size = sf_info->frames * sf_info->channels;
    short* s16_x = calloc(*size, sizeof(short));
    sf_count_t sf_count = sf_readf_short(file, s16_x, sf_info->frames);
    if (sf_count != sf_info->frames) {
        fprintf(stderr, "\nRead count not equal to requested frames, %lld != %lld.\n", sf_count, sf_info->frames);

        return 1;
    }

    convert_short_buf_to_uint8(s16_x, *size, *x);

    return 0;
}



char* get_sndfile_major_format(SF_INFO* sf_info)
{
    const uint32_t format_mask = 0x00FF0000;
    const uint32_t major_format = sf_info->format & format_mask;

    for (int i = 0; i < SND_MAJOR_FORMAT_NUM; i++) {
        if (major_format == snd_format_arr[i]) {
            return snd_format_arr_desc[i];
        }
    }

    return "N/A";
}

char* get_sndfile_subtype(SF_INFO* sf_info)
{
    const uint16_t subtype_mask = 0x00FF;
    const uint16_t subtype = sf_info->format & subtype_mask;

    for (int i = 0; i < SND_SUBTYPE_NUM; i++) {
        if (subtype == snd_subtype_arr[i]) {
            return snd_subtype_arr_desc[i];
        }
    }

    return "N/A";
}

int output_input_file_info(SF_INFO* sf_info, adx_config_t* adx_conf)
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

void proc_mono(adx_config_t* adx_conf, SF_INFO* sf_info, double* x, double** x_proc)
{
    /* Convert file data to mono */
    adx_conf->file_size /= sf_info->channels;
    mix2mono(adx_conf->file_size, sf_info->channels, x, x_proc);
    printf("Mixed file to mono.\n");
}

void proc_cpy(adx_config_t* adx_conf, SF_INFO* sf_info, double* x, double** x_proc)
{
    /* Preserve the original data size */
    *x_proc = malloc(adx_conf->file_size * sizeof(double));
    memcpy(*x_proc, x, adx_conf->file_size * sizeof(double));
    printf("Preserved channel amount.\n");
}

int mix2mono(size_t size, int channels, double* x, double** x_mono)
{
    *x_mono = calloc(size, sizeof(double));
    uint64_t i = 0;
    uint16_t c = 0;
    for (i = 0; i < size; i++) {
        for (c = 0; c < channels; c++) {
            (*x_mono)[i] += (x[channels * i + c]/channels);
        }
    }

    return 0;
}

int select_output_format(adx_config_t* adx_conf, char* str)
{
    adx_conf->write = NULL;

    if (!(strcmp("csv-horizontal", str))) {
        adx_conf->write = &write_file_csv_horizontal;
    }
    if (!(strcmp("csv-vertical", str))) {
        adx_conf->write = &write_file_csv_vertical;
    }
    if (!(strcmp("stdout", str))) {
        adx_conf->write = &write_stdout;
    }
    if (!(strcmp("bytes", str))) {
        adx_conf->write = &write_file_bytes;
    }

    if (!(adx_conf->write)) {
        fprintf(stderr, "\nOutput format '%s' doesn't exist.\n", str);
        
        return 1;
    }

    return 0;
}

int write_file_csv_horizontal(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc)
{
    FILE *ofile = fopen(adx_conf->ofile, "w");
    if(!(ofile)) {
        fprintf(stderr, "\nUnable to open file for output.");

        return 1;
    };

    for (size_t i = 0; i < adx_conf->file_size - 1; i++){
        fprintf(ofile, "%lf,", x_proc[i]);
    }
    fprintf(ofile, "%lf", x_proc[sf_info->frames]);

    fclose(ofile);

    return 0;
}

int write_file_csv_vertical(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc)
{
    FILE *ofile = fopen(adx_conf->ofile, "w");
    if(!(ofile)) {
        fprintf(stderr, "\nUnable to open file for output.");

        return 1;
    };

    for (size_t i = 0; i < adx_conf->file_size; i++){
        fprintf(ofile, "% lf\n", x_proc[i]);
    }

    fclose(ofile);

    return 0;
}

int write_file_bytes(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc)
{
    FILE *ofile = fopen(adx_conf->ofile, "wb");
    if(!(ofile)) {
        fprintf(stderr, "\nUnable to open file for output.");

        return 1;
    };
    
    fwrite(x_proc, sizeof(uint8_t), adx_conf->file_size, ofile);

    fclose(ofile);

    return 0;
}

int write_stdout(adx_config_t* adx_conf, SF_INFO* sf_info, double* x_proc)
{
    for (size_t i = 0; i < adx_conf->file_size; i++){
        fprintf(stdout, "% lf\n", x_proc[i]);
    }

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
            "\n\t-f,\t--output-format <file>= Choose between 'csv-vertical', 'csv-horizontal', and 'stdout'."
          );

    return 0;
}

uint32_t snd_format_arr[SND_MAJOR_FORMAT_NUM] = {
    SF_FORMAT_WAV,
    SF_FORMAT_AIFF,
    SF_FORMAT_AU,
    SF_FORMAT_RAW,
    SF_FORMAT_PAF,
    SF_FORMAT_SVX,
    SF_FORMAT_NIST,
    SF_FORMAT_VOC,
    SF_FORMAT_IRCAM,
    SF_FORMAT_W64,
    SF_FORMAT_MAT4,
    SF_FORMAT_MAT5,
    SF_FORMAT_PVF,
    SF_FORMAT_XI,
    SF_FORMAT_HTK,
    SF_FORMAT_SDS,
    SF_FORMAT_AVR,
    SF_FORMAT_WAVEX,
    SF_FORMAT_SD2,
    SF_FORMAT_FLAC,
    SF_FORMAT_CAF,
    SF_FORMAT_WVE,
    SF_FORMAT_OGG,
    SF_FORMAT_MPC2K,
    SF_FORMAT_RF64,
    SF_FORMAT_MPEG,
};

char* snd_format_arr_desc[SND_MAJOR_FORMAT_NUM] = {
    "Microsoft WAV format (little endian)",
    "Apple/SGI AIFF format (big endian)",
    "Sun/NeXT AU format (big endian)",
    "RAW PCM data",
    "Ensoniq PARIS file format",
    "Amiga IFF / SVX8 / SV16 format",
    "Sphere NIST format",
    "VOC files",
    "Berkeley/IRCAM/CARL",
    "Sonic Foundry’s 64 bit RIFF/WAV",
    "Matlab (tm) V4.2 / GNU Octave 2.0",
    "Matlab (tm) V5.0 / GNU Octave 2.1",
    "Portable Voice Format",
    "Fasttracker 2 Extended Instrument",
    "HMM Tool Kit format",
    "Midi Sample Dump Standard",
    "Audio Visual Research",
    "MS WAVE with WAVEFORMATEX",
    "Sound Designer 2",
    "FLAC lossless file format",
    "Core Audio File format",
    "Psion WVE format",
    "Xiph OGG container",
    "Akai MPC 2000 sampler",
    "RF64 WAV file",
    "MPEG-1/2 audio stream",
};

uint32_t snd_subtype_arr[SND_SUBTYPE_NUM] = {
    SF_FORMAT_PCM_S8,
    SF_FORMAT_PCM_16,
    SF_FORMAT_PCM_24,
    SF_FORMAT_PCM_32,
    SF_FORMAT_PCM_U8,
    SF_FORMAT_FLOAT,
    SF_FORMAT_DOUBLE,
    SF_FORMAT_ULAW,
    SF_FORMAT_ALAW,
    SF_FORMAT_IMA_ADPCM,
    SF_FORMAT_MS_ADPCM,
    SF_FORMAT_GSM610,
    SF_FORMAT_VOX_ADPCM,
    SF_FORMAT_NMS_ADPCM_16,
    SF_FORMAT_NMS_ADPCM_24,
    SF_FORMAT_NMS_ADPCM_32,
    SF_FORMAT_G721_32,
    SF_FORMAT_G723_24,
    SF_FORMAT_G723_40,
    SF_FORMAT_DWVW_12,
    SF_FORMAT_DWVW_16,
    SF_FORMAT_DWVW_24,
    SF_FORMAT_DWVW_N,
    SF_FORMAT_DPCM_8,
    SF_FORMAT_DPCM_16,
    SF_FORMAT_VORBIS,
    SF_FORMAT_OPUS,
    SF_FORMAT_ALAC_16,
    SF_FORMAT_ALAC_20,
    SF_FORMAT_ALAC_24,
    SF_FORMAT_ALAC_32,
    SF_FORMAT_MPEG_LAYER_I,
    SF_FORMAT_MPEG_LAYER_II,
    SF_FORMAT_MPEG_LAYER_III,
};

char* snd_subtype_arr_desc[SND_SUBTYPE_NUM] = {
    "Signed 8 bit data",
    "Signed 16 bit data",
    "Signed 24 bit data",
    "Signed 32 bit data",
    "Unsigned 8 bit data (WAV and RAW only)",
    "32 bit float data",
    "64 bit float data",
    "U-Law encoded.",
    "A-Law encoded.",
    "IMA ADPCM.",
    "Microsoft ADPCM.",
    "GSM 6.10 encoding.",
    "OKI / Dialogix ADPCM",
    "16kbs NMS G721-variant encoding.",
    "24kbs NMS G721-variant encoding.",
    "32kbs NMS G721-variant encoding.",
    "32kbs G721 ADPCM encoding.",
    "24kbs G723 ADPCM encoding.",
    "40kbs G723 ADPCM encoding.",
    "12 bit Delta Width Variable Word encoding.",
    "16 bit Delta Width Variable Word encoding.",
    "24 bit Delta Width Variable Word encoding.",
    "N bit Delta Width Variable Word encoding.",
    "8 bit differential PCM (XI only)",
    "16 bit differential PCM (XI only)",
    "Xiph Vorbis encoding.",
    "Xiph/Skype Opus encoding.",
    "Apple Lossless Audio Codec (16 bit).",
    "Apple Lossless Audio Codec (20 bit).",
    "Apple Lossless Audio Codec (24 bit).",
    "Apple Lossless Audio Codec (32 bit).",
    "MPEG-1 Audio Layer I.",
    "MPEG-1 Audio Layer II.",
    "MPEG-2 Audio Layer III.",
};

