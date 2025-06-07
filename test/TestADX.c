#include "unity/unity.h"
#include "unity/unity_internals.h"
#include "../adx.h"


void setUp() {}

void tearDown() {
    fflush(stdout);
}

void test_set_defaults() {
    adx_config_t adx_conf;

    set_defaults(&adx_conf);

    TEST_ASSERT_EQUAL_INT(0, adx_conf.info_flag);
    TEST_ASSERT_EQUAL_INT(5, adx_conf.precision);
}

void test_output_help() {
    TEST_ASSERT_EQUAL_INT(0, output_help());
}

void test_output_file_info() {
    SF_INFO sf_info = {
        .samplerate = 1,
        .frames = 1,
        .channels = 1,
        .sections = 1,
        .seekable = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_FLOAT,
    };

    adx_config_t adx_conf = {
        .ifile = "test.wav",
        .info_flag = 1,
    };

    TEST_ASSERT_EQUAL_INT(0, output_input_file_info(&sf_info, &adx_conf));
}

void split(char* cmd, char** argv, int* argc) {
    char* token = strtok(cmd," "); 
    *argc = 0;
    while(token != NULL) {
        argv[*argc] = token; 
        token = strtok(NULL," "); 
        *argc = *argc + 1;
    }
}

void reset(char** argv, adx_config_t* adx_conf, int size) {
    for(int i = 0; i < size; i++) {
        argv[i] = "\0";
    }
    adx_config_t x = { 0 };
    *adx_conf = x;
}

void test_get_options() {
    adx_config_t adx_conf;
    int argc;
    char* argv[20];

    char cmd0[] = "first";
    split(cmd0, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &adx_conf));
    reset(argv, &adx_conf, argc);

    char cmd1[] = "first --version";
    split(cmd1, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &adx_conf));
    reset(argv, &adx_conf, argc);

    char cmd2[] = "first --help";
    split(cmd2, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &adx_conf));
    reset(argv, &adx_conf, argc);

    char cmd3[] = "first --test";
    split(cmd3, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &adx_conf));
    reset(argv, &adx_conf, argc);

    char cmd4[] = "first test.wav --input test.wav --output otest.wav --info --mono --read-format raw --precision 5 --output-format csv";
    split(cmd4, argv, &argc);
    TEST_ASSERT_EQUAL_INT(0, get_options(argc, argv, &adx_conf));
    reset(argv, &adx_conf, argc);

    char cmd5[] = "first --balls balls";
    split(cmd5, argv, &argc);
    TEST_ASSERT_EQUAL_INT(1, get_options(argc, argv, &adx_conf));
    reset(argv, &adx_conf, argc);
}

void test_open_file() {
    SNDFILE* sndfile;
    SF_INFO sf_info;
    adx_config_t adx_conf = {
        .ifile = "test.wav",
    };

    TEST_ASSERT_EQUAL_INT(0, open_file(&sndfile, &sf_info, &adx_conf));

    adx_conf.ifile[0] = 'b';
    TEST_ASSERT_EQUAL_INT(1, open_file(&sndfile, &sf_info, &adx_conf));
}

void test_select_adx_settings() {
    adx_config_t adx_conf;

    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "raw"));
    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "uint8"));
    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "int8"));
    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "short"));
    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "integer"));
    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "float"));
    TEST_ASSERT_EQUAL_INT(0, select_adx_settings(&adx_conf, "double"));
    TEST_ASSERT_EQUAL_INT(1, select_adx_settings(&adx_conf, "test"));
}

void test_set_settings() {
    SF_INFO sf_info = {
        .frames = 1,
        .channels = 1,
    };
    adx_config_t adx_conf;

    sf_info.format = SF_FORMAT_PCM_U8;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(1, adx_conf.data_size);

    sf_info.format = SF_FORMAT_ULAW;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(1, adx_conf.data_size);

    sf_info.format = SF_FORMAT_ALAW;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(1, adx_conf.data_size);

    sf_info.format = SF_FORMAT_PCM_S8;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(1, adx_conf.data_size);

    sf_info.format = SF_FORMAT_PCM_16;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(2, adx_conf.data_size);

    sf_info.format = SF_FORMAT_PCM_32;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(4, adx_conf.data_size);

    sf_info.format = SF_FORMAT_FLOAT;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(4, adx_conf.data_size);

    sf_info.format = SF_FORMAT_DOUBLE;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(8, adx_conf.data_size);

    // random format that mix2mono doesn't support
    sf_info.format = SF_FORMAT_AIFF;
    set_adx_auto(&adx_conf, &sf_info);
    TEST_ASSERT_EQUAL_INT(8, adx_conf.data_size);
}

void test_read_file_data() {
    SNDFILE* sndfile;
    SF_INFO sf_info;
    adx_config_t adx_conf = {
        .ifile = "test.wav",
    };
    void* x;
    open_file(&sndfile, &sf_info, &adx_conf);
    int input_file_size = sf_info.frames * sf_info.channels;

    TEST_ASSERT_EQUAL_INT(0, read_file_data_raw(&adx_conf, sndfile, &sf_info, &x));
    adx_conf.input_file_size = 1;
    adx_conf.data_size = 1;
    TEST_ASSERT_EQUAL_INT(1, read_file_data_raw(&adx_conf, sndfile, &sf_info, &x));

    open_file(&sndfile, &sf_info, &adx_conf);
    adx_conf.input_file_size = input_file_size;
    adx_conf.data_size = sizeof(short);
    TEST_ASSERT_EQUAL_INT(0, read_file_data_short(&adx_conf, sndfile, &sf_info, &x));
    TEST_ASSERT_EQUAL_INT(1, read_file_data_short(&adx_conf, (SNDFILE*) NULL, &sf_info, &x));

    open_file(&sndfile, &sf_info, &adx_conf);
    adx_conf.input_file_size = input_file_size;
    adx_conf.data_size = sizeof(int);
    TEST_ASSERT_EQUAL_INT(0, read_file_data_int(&adx_conf, sndfile, &sf_info, &x));
    TEST_ASSERT_EQUAL_INT(1, read_file_data_int(&adx_conf, NULL, &sf_info, &x));

    open_file(&sndfile, &sf_info, &adx_conf);
    adx_conf.input_file_size = input_file_size;
    adx_conf.data_size = sizeof(float);
    TEST_ASSERT_EQUAL_INT(0, read_file_data_float(&adx_conf, sndfile, &sf_info, &x));
    TEST_ASSERT_EQUAL_INT(1, read_file_data_float(&adx_conf, NULL, &sf_info, &x));

    open_file(&sndfile, &sf_info, &adx_conf);
    adx_conf.input_file_size = input_file_size;
    adx_conf.data_size = sizeof(double);
    TEST_ASSERT_EQUAL_INT(0, read_file_data_double(&adx_conf, sndfile, &sf_info, &x));
    TEST_ASSERT_EQUAL_INT(1, read_file_data_double(&adx_conf, NULL, &sf_info, &x));
}

void test_set_precision_format() {
    char format[9];

    set_precision_format(format, 5);

    TEST_ASSERT_EQUAL_STRING("%.5lf", format);
}

void test_get_sndfile_major_format() {
    SF_INFO sf_info = {
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_S8,
    };

    TEST_ASSERT_EQUAL_STRING("WAV (Microsoft)", get_sndfile_major_format(&sf_info));

    sf_info.format = 0xFFFFFF;
    TEST_ASSERT_EQUAL_STRING("N/A", get_sndfile_major_format(&sf_info));
}

void test_get_sndfile_subtype() {
    SF_INFO sf_info = {
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_S8,
    };

    TEST_ASSERT_EQUAL_STRING("Signed 8 bit PCM", get_sndfile_subtype(&sf_info));

    sf_info.format = 0xFFFFFF;
    TEST_ASSERT_EQUAL_STRING("N/A", get_sndfile_subtype(&sf_info));
}

void test_get_datetime_string() {
    TEST_ASSERT_EQUAL_INT(12, strlen(get_datetime_string()));
}

void test_generate_file_name() {
    char ofile[MAX_STR];
    char ifile[MIN_STR] = ".\\test.wav";

    memset(ofile, '\0', MAX_STR);
    generate_file_name(ofile, ifile);

    TEST_ASSERT_EQUAL_INT(0, strncmp(ofile, "adx-test-", 9));
    TEST_ASSERT_EQUAL_INT(0, strncmp(strrev(ofile), "txt.", 4));
    TEST_ASSERT_EQUAL_INT(strlen("adx-test-ddmmyyHHMMSS.txt"), strlen(ofile));

    strcpy(ofile, "test");
    generate_file_name(ofile, ifile);
    TEST_ASSERT_EQUAL_STRING("test", ofile);
}

void test_proc() {
    int x[] = {2,2,2,2};
    void* x_proc1;
    void* x_proc2;
    SF_INFO sf_info;

    adx_config_t adx_conf = {
        .input_file_size = 4,
        .data_size = sizeof(int)
    };

    proc_cpy(&adx_conf, &sf_info, x, &x_proc1);

    TEST_ASSERT_EQUAL_INT_ARRAY(x, x_proc1, 4);

    adx_conf.mix2mono = &mix2mono_int;
    sf_info.channels = 2;
    proc_mono(&adx_conf, &sf_info, x, &x_proc2);
    TEST_ASSERT_EQUAL_INT_ARRAY(x, x_proc2, 2);
}

void test_select_output_format() {
    adx_config_t adx_conf;

    TEST_ASSERT_EQUAL_INT(0, select_output_format(&adx_conf, "csv"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&adx_conf, "column"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&adx_conf, "stdout"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&adx_conf, "stdout-csv"));
    TEST_ASSERT_EQUAL_INT(0, select_output_format(&adx_conf, "hex-dump"));
    TEST_ASSERT_EQUAL_INT(1, select_output_format(&adx_conf, "test"));
}

void test_printf() {
    int x[] = {1,1,1,1};

    // Can't think of a way to test them so just printing to stdout.    
    printf_uint8(stdout, "%hhu",x, 0);
    printf_int8(stdout, "%hhd",x, 0);
    printf_short(stdout, "%hd",x, 0);
    printf_int(stdout, "%d",x, 0);
    printf_float(stdout, "%.1f",x, 0);
    printf_double(stdout, "%.1lf",x, 0);
    printf("\n");

    TEST_ASSERT_EQUAL_INT(1, x[0]);
}

void test_mix2mono() {
    // uint8
    uint8_t x_uint8[] = {2,2,2,2,2,2,2,2};
    void* x_mono_uint8;
    mix2mono_uint8(8, 2, x_uint8, &x_mono_uint8);
    uint8_t result_uint8[] = {2,2,2,2};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_UINT8(result_uint8[i], ((uint8_t*)x_mono_uint8)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_UINT8(1, ((uint8_t*)x_mono_uint8)[5]);

    // int8
    int8_t x_int8[] = {2,2,2,2,2,2,2,2};
    void* x_mono_int8;
    mix2mono_int8(8, 2, x_int8, &x_mono_int8);
    int8_t result_int8[] = {2,2,2,2};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT8(result_int8[i], ((int8_t*)x_mono_int8)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_INT8(1, ((int8_t*)x_mono_int8)[5]);

    // short/int16
    int16_t x_int16[] = {2,2,2,2,2,2,2,2};
    void* x_mono_int16;
    mix2mono_short(8, 2, x_int16, &x_mono_int16);
    int16_t result_int16[] = {2,2,2,2};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT16(result_int16[i], ((int16_t*)x_mono_int16)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_INT16(1, ((int16_t*)x_mono_int16)[5]);

    // int/int32
    int32_t x_int32[] = {2,2,2,2,2,2,2,2};
    void* x_mono_int32;
    mix2mono_int(8, 2, x_int32, &x_mono_int32);
    int result_int32[] = {2,2,2,2};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT(result_int32[i], ((int32_t*)x_mono_int32)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_INT(1, ((int32_t*)x_mono_int32)[5]);

    // float
    float x_float[] = {1,1,1,1,1,1,1,1};
    void* x_mono_float;
    mix2mono_float(8, 2, x_float, &x_mono_float);
    float result_float[] = {1,1,1,1};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_FLOAT(result_float[i], ((float*)x_mono_float)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_FLOAT(1, ((float*)x_mono_float)[5]);

    // double
    double x_double[] = {1,1,1,1,1,1,1,1};
    void* x_mono_double;
    mix2mono_double(8, 2, x_double, &x_mono_double);
    double result_double[] = {1,1,1,1};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_FLOAT(result_double[i], ((double*)x_mono_double)[i]);
    }
    TEST_ASSERT_NOT_EQUAL_FLOAT(1, ((double*)x_mono_double)[5]);
}

void test_write_file() {
    SNDFILE *sndfile;
    double x[] = {1,1,1,1};
    adx_config_t adx_conf = {
        .print = &printf_int,
        .format_specifier = "%d",
        .input_file_size = 4,
        .ofile = "otest.txt",
    };

    TEST_ASSERT_EQUAL_INT(0, write_file_csv(&adx_conf, x));
    TEST_ASSERT_EQUAL_INT(0, write_file_column(&adx_conf, x));
    TEST_ASSERT_EQUAL_INT(0, write_file_hex_dump(&adx_conf, x));
    TEST_ASSERT_EQUAL_INT(0, write_stdout(&adx_conf, x));
    TEST_ASSERT_EQUAL_INT(0, write_stdout_csv(&adx_conf, x));
    printf("\n");

    adx_conf.ofile[0] = '?';
    TEST_ASSERT_EQUAL_INT(1, write_file_csv(&adx_conf, x));
    TEST_ASSERT_EQUAL_INT(1, write_file_column(&adx_conf, x));
    TEST_ASSERT_EQUAL_INT(1, write_file_hex_dump(&adx_conf, x));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_set_defaults);
    RUN_TEST(test_output_help);
    RUN_TEST(test_output_file_info);
    RUN_TEST(test_get_options);
    RUN_TEST(test_open_file);
    RUN_TEST(test_select_adx_settings);
    RUN_TEST(test_set_settings);
    RUN_TEST(test_read_file_data);
    RUN_TEST(test_set_precision_format);
    RUN_TEST(test_get_sndfile_major_format);
    RUN_TEST(test_get_sndfile_subtype);
    RUN_TEST(test_get_datetime_string);
    RUN_TEST(test_generate_file_name);
    RUN_TEST(test_proc);
    RUN_TEST(test_select_output_format);
    RUN_TEST(test_printf);
    RUN_TEST(test_mix2mono);
    RUN_TEST(test_write_file);
    return UNITY_END();
}
