#include "system/includes.h"
#include "wav_head.h"

int get_resample_samps(int input_samps, int input_sample_rate,
                       int output_sample_rate)
{
    float ratio = (float)output_sample_rate / (float)input_sample_rate;
    return (int)((float)input_samps * ratio);
}

void resample_audio(const short *input, int input_len, int input_channels,
                    short *output, int output_channels, int *output_len,
                    int input_sample_rate, int output_sample_rate)
{
    // 计算采样率比例因子
    float ratio = (float)output_sample_rate / (float)input_sample_rate;

    // 计算输出长度
    *output_len = (int)((float)input_len * ratio);

    // 对于每个输出样本，计算输入样本的加权平均值
    for (int i = 0; i < *output_len; i++) {
        float inIndex = (float)i / ratio;
        int index1 = (int)floorf(inIndex);
        float frac = inIndex - (float)index1;
        float weight1 = 1.0f - frac;
        float weight2 = frac;

        // 处理左声道
        float left = 0.0f;
        if (input_channels >= 1) {
            left += (float)input[index1 * input_channels + 0] * weight1;
            if (index1 < input_len - 1) {
                left += (float)input[(index1 + 1) * input_channels + 0] * weight2;
            }
        }

        // 处理右声道
        float right = 0.0f;
        if (input_channels == 1) {
            right = left;
        } else if (input_channels >= 2) {
            right += (float)input[index1 * input_channels + 1] * weight1;
            if (index1 < input_len - 1) {
                right += (float)input[(index1 + 1) * input_channels + 1] * weight2;
            }
        }

        // 将左右声道平均并转换为16位整数
        short sample = 0;
        if (output_channels == 1) {
            sample = (short)((left + right) / 2.0f);
        } else if (output_channels == 2) {
            sample = (short)left;
            output[i * output_channels + 1] = (short)right;
        }

        output[i * output_channels + 0] = sample;
    }
    if (input_channels == 1 && output_channels == 2) {
        *output_len = *output_len * 2;
    }
}

unsigned int generate_wav_header(wav_header_t *wav_header, int sample_rate, int chn, int bits, int data_size)
{
    unsigned int wav_size = sizeof(wav_header_t) + data_size;

    if (wav_header != NULL) {
        wav_header->ChunkID[0] = 'R';
        wav_header->ChunkID[1] = 'I';
        wav_header->ChunkID[2] = 'F';
        wav_header->ChunkID[3] = 'F';

        wav_header->ChunkSize = wav_size;

        wav_header->Format[0] = 'W';
        wav_header->Format[1] = 'A';
        wav_header->Format[2] = 'V';
        wav_header->Format[3] = 'E';

        wav_header->Subchunk1ID[0] = 'f';
        wav_header->Subchunk1ID[1] = 'm';
        wav_header->Subchunk1ID[2] = 't';
        wav_header->Subchunk1ID[3] = ' ';

        wav_header->Subchunk1Size = 16;
        wav_header->AudioFormat = 1;
        wav_header->NumChannels = chn;
        wav_header->SampleRate = sample_rate;
        wav_header->ByteRate = sample_rate * chn * bits / 8;
        wav_header->BlockAlign = chn * bits / 8;
        wav_header->BitsPerSample = bits;

        wav_header->Subchunk2ID[0] = 'd';
        wav_header->Subchunk2ID[1] = 'a';
        wav_header->Subchunk2ID[2] = 't';
        wav_header->Subchunk2ID[3] = 'a';

        wav_header->Subchunk2Size = data_size;
    }
    return wav_size;
}
