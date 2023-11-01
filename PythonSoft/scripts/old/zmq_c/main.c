#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>

#define SAMPLES_PER_SECOND 14705883
#define MAX_SAMPLES (SAMPLES_PER_SECOND*5)
#define MAX_ALLOC   ((MAX_SAMPLES+MAX_SAMPLES)*8)

void *captureThread(void *vargp, uint32_t samples_to_retrieve, char* file_name) {
    uint8_t* data;
    uint8_t* sample_data;
    FILE* sample_file;
    size_t len;
    uint64_t samples_written;
    uint64_t data_written;
    uint64_t pkts_rx;
    zmq_msg_t message;
    zmq_msg_t msg_samples;

	void* context = vargp;
    void* sub = zmq_socket (context, ZMQ_SUB);

    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "ADC", 3);
    zmq_setsockopt(sub, ZMQ_RCVBUF, "", SAMPLES_PER_SECOND);

    if(zmq_connect (sub, "tcp://192.168.1.229:5556") != 0) {
        printf("Failed to Bind ZMQ to port 5556\n");
        return 0;
    }

    sample_data = malloc(MAX_ALLOC);
    if(!sample_data) {
        return NULL;
    }

    sample_file = fopen(file_name, "wb");
    if(!sample_file) {
        return NULL;
    }

    samples_written = 0;
    data_written = 0;
    pkts_rx = 0;
    while(samples_written < samples_to_retrieve) {

        zmq_msg_init (&message);
        zmq_msg_recv (&message, sub, 0);

        data = zmq_msg_data(&message);
        len = zmq_msg_size(&message);

        if(strncmp("ADC", (char*)data, 3) == 0) {
            zmq_msg_init (&msg_samples);
            zmq_msg_recv (&msg_samples, sub, 0);

            pkts_rx++;
            data = zmq_msg_data(&msg_samples);
            len = zmq_msg_size(&msg_samples);

            memcpy(&sample_data[data_written], data, len);

            data_written += len;
            samples_written += (len - 16) / 4;

            zmq_msg_close(&msg_samples);
        }
        zmq_msg_close (&message);

    }

    fwrite(sample_data, data_written, 1, sample_file);
    fclose(sample_file);

    printf("Pkts: %ld Samples: %ld SamplesPerPacket: %ld Bytes: %ld\n", pkts_rx, samples_written, (data_written / (pkts_rx * 16)) * 2, data_written);

    free(sample_data);
    zmq_close(sub);

    return NULL;
}

int main (int argc, char **argv) {
    pthread_t thread_cap_id;
    pthread_t thread_write_id;
    void *context;

    uint32_t samples_to_retrieve;
    char file_name[1024];

    context = zmq_ctx_new();

    zmq_ctx_set(context, ZMQ_IO_THREADS, 4);

    // set this to something reasonable, 1 second
    samples_to_retrieve = SAMPLES_PER_SECOND/8;
    if(argc > 1) {
        samples_to_retrieve = atoi(argv[1]);
        if(samples_to_retrieve > MAX_SAMPLES) {
            samples_to_retrieve = MAX_SAMPLES;
        }
    }

    strcpy(file_name, "sample_data.raw");
    if(argc > 2) {
        if(strlen(argv[2]) < sizeof(file_name)) {
            strncpy(file_name, argv[2], strlen(argv[2]));
        }
    }

    printf("\nNum Samples to retrieve: %d\n", samples_to_retrieve);
    printf("Writing to %s\n", file_name);

    captureThread(context, samples_to_retrieve, file_name);

    zmq_ctx_destroy (context);

    return 0;
}

/*
void convert_data(uint64_t d, int32_t *ch0_data, uint8_t *ch0_div, uint8_t *ch0_sel, int32_t *ch1_data, uint8_t *ch1_div, uint8_t *ch1_sel ) {
    *ch0_data = (int32_t)(d >> 46) & 0x3FFF;
    *ch0_div = (d >> 36) & 0x7F;
    *ch0_sel = (d >> 32) & 0xF;
    *ch1_data = ((int32_t)(d & 0x000000FFFFFFFF)) >> 14;
    *ch1_div = (d >> 4) & 0x7F;
    *ch1_sel = (d >> 0) & 0xF;
}
*/