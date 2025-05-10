#include "adx.h"

//TODO: Have output file date/time, and input file properties naming schemes. Also include input file name to output.
// Get that from the swavgen project.
// Have a decrypt option if the data in the audio file are encrypted
// Handle treating the data as bytes (uint8) instead of double, this is for extracting files from the audio file
// Far future, treat input data as any specified data type

int main (int argc, char** argv) {
    SF_INFO sf_info;        // File info
    SNDFILE* file;          // Pointer to file
    double* x = NULL;       // Data from audio file, considered the input signal
    double* x_proc = NULL;  // To store x after any processing. Currently only gets copied over or mix2mono.
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

    /* Read the file data */
    CHECK_ERR(read_file_data_double(file, &sf_info, &adx_conf.file_size, &x));
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
