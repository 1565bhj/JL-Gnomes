#ifndef __WAV_H__
#define __WAV_H__


typedef struct {
    // The "RIFF" chunk descriptor
    unsigned char ChunkID[4];
    int ChunkSize;
    unsigned char Format[4];
    // The "fmt" sub-chunk
    unsigned char Subchunk1ID[4];
    int Subchunk1Size;
    short AudioFormat;
    short NumChannels;
    int SampleRate;
    int ByteRate;
    short BlockAlign;
    short BitsPerSample;
    // The "data" sub-chunk
    unsigned char Subchunk2ID[4];
    int Subchunk2Size;
} wav_header_t;

unsigned int generate_wav_header(wav_header_t *wav_header, int sample_rate, int chn,
                                 int bits, int data_size);

int get_resample_samps(int input_samps, int input_sample_rate,
                       int output_sample_rate);
void resample_audio(const short *input, int input_len, int input_channels,
                    short *output, int output_channels, int *output_len,
                    int input_sample_rate, int output_sample_rate);

#endif
