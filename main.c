#include "adx.h"

// TODO: Have a decrypt option if the data in the audio file are encrypted
// TODO: Have the naming scheme from the mix2mono project
// TODO: Finish the implementation where there would be file data as the audio file data.
// TODO: Check out the SFC_GET_FORMAT_SUBTYPE and SFC_GET_FORMAT_MAJOR code examples, might need to change mix2mono as well...
// INFO: Data gets casted when the encoding doesn't match the read function

int main (int argc, char** argv) {
    SF_INFO sf_info;        // File info
    SNDFILE* file;          // Pointer to file
    void* x = NULL;         // Data from audio file, considered the input signal
    void* x_proc = NULL;    // To store x after any processing. Currently only gets copied over or mix2mono.
    adx_config_t adx_conf;  // Tool config

    /* Initialise the struct */
    memset(&sf_info, 0, sizeof(SF_INFO));

    /* Set defaults to ensure certain behabiour */
    set_defaults(&adx_conf);

    /* Get the options from the CLI */
    CHECK_ERR(get_options(&argc, argv, &adx_conf));

    /* Initialise input audio file buffer and open file */
    file = NULL;
    CHECK_ERR(open_file(&file, &sf_info, &adx_conf));

    /* Output info on the inputted file */
    output_input_file_info(&sf_info, &adx_conf);

    /* Set the tool settings */
    adx_conf.set_adx(&adx_conf, &sf_info);

    /* Read the file data using the specifed settings */
    CHECK_ERR(read_file_data_raw(&adx_conf, file, &sf_info, &x));
    printf("Read file data succesfully.\n");

    /* Do some processing */
    adx_conf.proc(&adx_conf, &sf_info, x, &x_proc);

    /* Write to file */ 
    CHECK_ERR(adx_conf.write(&adx_conf, &sf_info, x_proc));
    printf("Outputted data to '%s'!\n", adx_conf.ofile);

    sf_close(file);
    free(x);
    free(x_proc);

    return 0;
}
