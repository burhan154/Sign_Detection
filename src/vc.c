//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem




IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);
				
				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;
				
				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}
		
		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}
	
	return image;
}


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}
		
		fclose(file);

		return 1;
	}
	
	return 0;
}


int vc_scale_gray_to_rgb(IVC *src, IVC *dst) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 3))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;
			pos_dst = y*bytesperline_dst + x*channels_dst;

			
			if (datasrc[pos_src] <= 64) {
				datadst[pos_dst] = 0;
				datadst[pos_dst + 1] = datasrc[pos_src]*(255/64);
				datadst[pos_dst + 2] = 255;
			}
			else if (datasrc[pos_src] <= 128) {
				datadst[pos_dst] = 0;
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 255 - (datasrc[pos_src] - 64)*(255 / 64);
			}
			else if (datasrc[pos_src] <= 192) {
				datadst[pos_dst] = (datasrc[pos_src]-128) * (255 / 64);
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 0;
			}
			else {
				datadst[pos_dst] = 255;
				datadst[pos_dst + 1] = 255 - (datasrc[pos_src] - 192)*(255 / 64);
				datadst[pos_dst + 2] = 0;
			}

		}
	}
	return 1;
}


int vc_gray_histogram_show(IVC *src, IVC *dst)
{
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL)
        return 0;

    int width = src->width;
    int height = src->height;
    int x, y;

    // Check if source image is grayscale (single channel)
    if (src->channels != 1)
        return 0;

    // Check if destination image is RGB (3 channels)
    if (dst->channels != 3)
        return 0;

    int histogram[256] = {0};

    // Compute histogram of grayscale image
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            unsigned char gray_value = src->data[y * src->bytesperline + x];
            histogram[gray_value]++;
        }
    }

    // Find the maximum histogram value
    int max_value = 0;
    for (int i = 0; i < 256; i++)
    {
        if (histogram[i] > max_value)
            max_value = histogram[i];
    }

    // Normalize histogram values to fit in the destination image
    float scale_factor = (float)(dst->height - 1) / max_value;

    // Clear the destination image
    for (y = 0; y < dst->height; y++)
    {
        for (x = 0; x < dst->width; x++)
        {
            dst->data[y * dst->bytesperline + 3 * x] = 0;
            dst->data[y * dst->bytesperline + 3 * x + 1] = 0;
            dst->data[y * dst->bytesperline + 3 * x + 2] = 0;
        }
    }

    // Draw histogram on the destination image
    for (int i = 0; i < 256; i++)
    {
        int histogram_height = (int)(histogram[i] * scale_factor);

        for (int j = 0; j < histogram_height; j++)
        {
            dst->data[(dst->height - 1 - j) * dst->bytesperline + 3 * i] = 255;
            dst->data[(dst->height - 1 - j) * dst->bytesperline + 3 * i + 1] = 255;
            dst->data[(dst->height - 1 - j) * dst->bytesperline + 3 * i + 2] = 255;
        }
    }

    return 1;
}


int vc_gray_lowpass_gaussian_filter(IVC *src, IVC *dst)
{
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL)
        return 0;

    int width = src->width;
    int height = src->height;
    int x, y;

    // Check if source image is grayscale (single channel)
    if (src->channels != 1)
        return 0;

    // Check if destination image is grayscale (single channel)
    if (dst->channels != 1)
        return 0;

    // Gaussian filter kernel
    double kernel[3][3] = {
        {0.0625, 0.125, 0.0625},
        {0.125,  0.25,  0.125},
        {0.0625, 0.125, 0.0625}
    };

    // Apply Gaussian filter to the grayscale image
    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            double sum = 0.0;

            // Convolution with the Gaussian filter kernel
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    double value = (double)src->data[(y + j) * src->bytesperline + (x + i)];
                    sum += value * kernel[j + 1][i + 1];
                }
            }

            // Clamp the resulting value to the range [0, 255]
            sum = fmin(fmax(sum, 0.0), 255.0);

            // Store the filtered value in the destination image
            dst->data[y * dst->bytesperline + x] = (unsigned char)round(sum);
        }
    }

    return 1;
}


int vc_gray_highpass_filter_enhance(IVC *src, IVC *dst, int gain)
{
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL)
        return 0;

    int width = src->width;
    int height = src->height;
    int x, y;

    // Check if source image is grayscale (single channel)
    if (src->channels != 1)
        return 0;

    // Check if destination image is grayscale (single channel)
    if (dst->channels != 1)
        return 0;

    // High-pass filter kernel
    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    // Apply high-pass filter to the grayscale image
    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            int sum = 0;

            // Convolution with the high-pass filter kernel
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    sum += src->data[(y + j) * src->bytesperline + (x + i)] * kernel[j + 1][i + 1];
                }
            }

            // Add gain to the filtered value
            sum += gain;

            // Clamp the resulting value to the range [0, 255]
            sum = sum < 0 ? 0 : (sum > 255 ? 255 : sum);

            // Store the enhanced value in the destination image
            dst->data[y * dst->bytesperline + x] = (unsigned char)sum;
        }
    }

    return 1;
}


int vc_gray_highpass_filter(IVC *src, IVC *dst)
{
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL)
        return 0;

    int width = src->width;
    int height = src->height;
    int x, y;

    // Check if source image is grayscale (single channel)
    if (src->channels != 1)
        return 0;

    // Check if destination image is grayscale (single channel)
    if (dst->channels != 1)
        return 0;

    // High-pass filter kernel
    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    // Apply high-pass filter to the grayscale image
    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            int sum = 0;

            // Convolution with the high-pass filter kernel
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    sum += src->data[(y + j) * src->bytesperline + (x + i)] * kernel[j + 1][i + 1];
                }
            }

            // Clamp the resulting value to the range [0, 255]
            sum = sum < 0 ? 0 : (sum > 255 ? 255 : sum);

            // Store the filtered value in the destination image
            dst->data[y * dst->bytesperline + x] = (unsigned char)sum;
        }
    }

    return 1;
}


int vc_gray_histogram_equalization(IVC *src, IVC *dst)
{
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL)
        return 0;

    int width = src->width;
    int height = src->height;
    int x, y;

    // Check if source image is grayscale (single channel)
    if (src->channels != 1)
        return 0;

    // Check if destination image is grayscale (single channel)
    if (dst->channels != 1)
        return 0;

    int histogram[256] = {0};

    // Compute histogram of grayscale image
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            unsigned char gray_value = src->data[y * src->bytesperline + x];
            histogram[gray_value]++;
        }
    }

    // Compute cumulative histogram
    int cumulative_histogram[256] = {0};
    cumulative_histogram[0] = histogram[0];
    for (int i = 1; i < 256; i++)
    {
        cumulative_histogram[i] = cumulative_histogram[i - 1] + histogram[i];
    }

    // Normalize cumulative histogram
    int total_pixels = width * height;
    float scale_factor = 255.0f / total_pixels;
    for (int i = 0; i < 256; i++)
    {
        cumulative_histogram[i] = (int)(cumulative_histogram[i] * scale_factor);
    }

    // Apply equalization to the grayscale image
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            unsigned char gray_value = src->data[y * src->bytesperline + x];
            unsigned char equalized_value = (unsigned char)cumulative_histogram[gray_value];
            dst->data[y * dst->bytesperline + x] = equalized_value;
        }
    }

    return 1;
}


int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th)
{
    if (src == NULL || src->data == NULL || dst == NULL || dst->data == NULL)
        return 0;

    int width = src->width;
    int height = src->height;
    int x, y;
    int max_gradient = 0;
    float gradient;

    // Prewitt kernel for horizontal and vertical edges
    int prewitt_h[3][3] = {
        {-1, 0, 1},
        {-1, 0, 1},
        {-1, 0, 1}
    };

    int prewitt_v[3][3] = {
        {-1, -1, -1},
        { 0,  0,  0},
        { 1,  1,  1}
    };

    // Apply Prewitt operator to compute gradients
    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            int sum_h = 0;
            int sum_v = 0;

            // Convolution with horizontal Prewitt kernel
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    sum_h += src->data[(y + j) * src->bytesperline + (x + i)] * prewitt_h[j + 1][i + 1];
                }
            }

            // Convolution with vertical Prewitt kernel
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    sum_v += src->data[(y + j) * src->bytesperline + (x + i)] * prewitt_v[j + 1][i + 1];
                }
            }

            // Calculate gradient magnitude
            gradient = sqrt(sum_h * sum_h + sum_v * sum_v);

            // Store maximum gradient value for thresholding
            if (gradient > max_gradient)
                max_gradient = gradient;

            // Store gradient in destination image
            dst->data[y * dst->bytesperline + x] = (unsigned char)gradient;
        }
    }

    // Thresholding
    int threshold = (int)(th * max_gradient);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (dst->data[y * dst->bytesperline + x] >= threshold)
                dst->data[y * dst->bytesperline + x] = 255;
            else
                dst->data[y * dst->bytesperline + x] = 0;
        }
    }

    return 1;
}




int vc_rgb_to_hsv(IVC *srcdst, int tipo)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i<size; i = i + channels)
	{
		r = (float)data[i];
		g = (float)data[i + 1];
		b = (float)data[i + 2];

		// Calcula valores maximo e minimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}


		if (tipo == 0) { // moedas 10 20 50 e 1 euro para binario
			if ((hue >= 40.0f) && (hue <= 85.0f) && saturation >= 50.0f&&value >= 50.0f) {
				data[i] = 255;
				data[i + 1] = 255;
				data[i + 2] = 255;
			}
			else {
				data[i] = 0;
				data[i + 1] = 0;
				data[i + 2] = 0;
			}
		}
		else if (tipo == 1) { //moedas 1 2 5 centimos para binario
			if ((hue >= 15.0f) && (hue <= 40.0f) && (saturation >= 80.0f)) {
				data[i] = 255;
				data[i + 1] = 255;
				data[i + 2] = 255;
			}
			else {
				data[i] = 0;
				data[i + 1] = 0;
				data[i + 2] = 0;
			}
		}

	}

	
	return 1;
}




int vc_rgb_to_gray(IVC *src, IVC *dst) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 3)||(dst->channels!=1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;
			pos_dst = y*bytesperline_dst + x*channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src+1];
			bf= (float)datasrc[pos_src+2];

			datadst[pos_dst] = (unsigned char)((rf*0.229) + (gf*0.587) + (bf*0.114));
		}
	}
	return 1;
}

int vc_gray_edge_sobel(IVC *src, IVC *dst, float th) {
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int byteperline = src->width*src->channels;
	int channels = src->channels;
	int x, y;
	long int pos;
	long int posA, posB, posC, posD, posE, posF, posG, posH;
	double mag, mx, my;

	if ((width <= 0) || (height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	for (y = 1; y < height; y++)
	{
		for (x = 1; x < width; x++)
		{
			pos = y * byteperline + x * channels;

			posA = (y - 1)* byteperline + (x - 1) * channels;
			posB = (y - 1)* byteperline + (x)* channels;
			posC = (y - 1)* byteperline + (x + 1)* channels;
			posD = (y)* byteperline + (x - 1)* channels;
			posE = (y)* byteperline + (x + 1)* channels;
			posF = (y + 1)* byteperline + (x - 1)* channels;
			posG = (y + 1)* byteperline + (x)* channels;
			posH = (y + 1)* byteperline + (x + 1)* channels;

			mx = ((-1 * data[posA]) + (1 * data[posC]) + (-2 * data[posD]) + (2 * data[posE]) + (-1 * data[posF]) + (1 * data[posH])) / 3;
			my = ((-1 * data[posA]) + (1 * data[posF]) + (-2 * data[posB]) + (2 * data[posG]) + (-1 * data[posC]) + (1 * data[posH])) / 3;

			mag = sqrt((mx*mx) + (my * my));

			if (mag > th)
				dst->data[pos] = 255;
			else
				dst->data[pos] = 0;
		}
	}
	return 1;
}




int vc_gray_negative(IVC *srcdst) {
	unsigned char *data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channel = srcdst->channels;
	int x, y;
	long int pos;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))return 0;
	if (channel != 1)return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y*bytesperline + x*channel;
            if(data[pos]<20){
                data[pos] = 0;
            }else{
                data[pos] = 255 - data[pos];
            }
                
			
		}
	}

	return 1;
}


int vc_gray_to_binary(IVC *src, IVC *dst, int threshold) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;
			pos_dst = y*bytesperline_dst + x*channels_dst;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
            //else if(pos_dst>0 && pos_dst<width){
            //    if()
            //}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

	return 1;
}



int vc_gray_lowpass_median_filter(IVC *src, IVC *dst, int kernel) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	int mediana, mediaDiv = kernel ^ 2;
	kernel *= 0.5;
	int i, j,k, temp;
	int *a = (int *)malloc(sizeof(int)*mediaDiv);

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	//if ((src->channels != 1) || (dst->channels != 1))return 0;


	for (y = kernel; y < height - kernel; y++)
	{
		for (x = kernel; x < width - kernel; x++)
		{
			pos_dst = y*bytesperline_dst + x*channels_dst;


			for (k=0,y2 = y - kernel; y2 <= y + kernel; y2++)
			{
				for (x2 = x - kernel; x2 <= x + kernel; x2++)
				{
					pos_src = y2*bytesperline_src + x2*channels_src;
					a[k] = datasrc[pos_src];
				}
			}

			
				
			for (i = 0; i < (mediaDiv - 1); ++i)
			{
				for (j = i + 1; j < mediaDiv; ++j)
				{
					if (a[i] > a[j])
					{
						temp = a[j];
						a[j] = a[i];
						a[i] = temp;
					}
				}
			}
			
				mediana = (mediaDiv / 2) + 1;

				datadst[pos_dst] = a[mediana];
		}
	}


	return 1;


}

int vc_binary_close(IVC *src, IVC *dst, int kernel) {
	int checks = 1;
	IVC *dstTemp = vc_image_new(src->width, src->height, src->channels, src->levels);

	checks &= vc_binary_dilate(src, dstTemp, kernel);
	checks &= vc_binary_erode(dstTemp, dst, kernel);

	vc_image_free(dstTemp);

	return checks;
}
int vc_binary_open(IVC *src, IVC *dst, int kernel) {
	int checks=1;
	IVC *dstTemp = vc_image_new(src->width, src->height, src->channels, src->levels);

	checks &= vc_binary_erode(src, dstTemp, kernel);
	checks &= vc_binary_dilate(dstTemp, dst, kernel);

	vc_image_free(dstTemp);

	return checks;
}


int vc_binary_erode(IVC *src, IVC *dst, int kernel) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	int checks;
	kernel *= 0.5;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;


	for (y = kernel; y < height - kernel; y++)
	{
		for (x = kernel; x < width - kernel; x++)
		{
			pos_dst = y*bytesperline_dst + x*channels_dst;

			checks = 0;

			for (y2 = y - kernel; y2 <= y + kernel; y2++)
			{
				for (x2 = x - kernel; x2 <= x + kernel; x2++)
				{
					pos_src = y2*bytesperline_src + x2*channels_src;
					if (datasrc[pos_src] == 0) { checks = 1; }
				}
			}

			if (checks == 1) { datadst[pos_dst] = 0; }
			else { datadst[pos_dst] = 255; }

		}
	}


	return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel) {
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	int checks;
	kernel *= 0.5;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;


	for (y = kernel; y < height-kernel; y++)
	{
		for (x = kernel; x < width-kernel; x++)
		{
			pos_dst = y*bytesperline_dst + x*channels_dst;

			checks = 0;

			for (y2 = y-kernel; y2 <= y+kernel; y2++)
			{
				for (x2 = x-kernel; x2 <= x+kernel; x2++)
				{
					pos_src = y2*bytesperline_src + x2*channels_src;
					if (datasrc[pos_src] == 255) { checks = 1; }
				}
			}

			if (checks == 1) { datadst[pos_dst] = 255; }
			else { datadst[pos_dst] = 0; }
		}
	}

	return 1;
}


int vc_show_rb_objects(IVC* image)
{
    int blueCount =0;
    int redCount =0;

    if (image == NULL || image->channels != 3)
    {
        return 0;
    }

    int width = image->width;
    int height = image->height;
    unsigned char* data = image->data;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned char* pixel = data + (y * width + x) * 3;
            unsigned char red = pixel[2];
            unsigned char green = pixel[1];
            unsigned char blue = pixel[0];

            // Thresholding condition: check if the pixel is within the red color range
            if (red > 150 && green < 100 && blue < 100)
            {
                redCount++;
            }
            else if (red < 100 && green < 100 && blue > 90)
            {
                //blueCount++;
            }
            else
            {
                // Make non-red pixels black
                pixel[0] = 0;  // Blue channel
                pixel[1] = 0;  // Green channel
                pixel[2] = 0;  // Red channel
            }
        }
    }

    if(redCount<blueCount)
        return 1;
    return 2;
}


int vc_show_red_objects(IVC* image)
{
    if (image == NULL || image->channels != 3)
    {
        return 0;
    }

    int width = image->width;
    int height = image->height;
    unsigned char* data = image->data;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned char* pixel = data + (y * width + x) * 3;
            unsigned char red = pixel[2];
            unsigned char green = pixel[1];
            unsigned char blue = pixel[0];

            // Thresholding condition: check if the pixel is within the red color range
            if (!(red > 150 && green < 100 && blue < 100))
            {
                // Make non-red pixels black
                pixel[0] = 0;  // Blue channel
                pixel[1] = 0;  // Green channel
                pixel[2] = 0;  // Red channel
            }
        }
    }
    return 1;
}
int vc_show_blue_objects(IVC* image)
{
    if (image == NULL || image->channels != 3)
    {
        return 0;
    }

    int width = image->width;
    int height = image->height;
    unsigned char* data = image->data;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned char* pixel = data + (y * width + x) * 3;
            unsigned char red = pixel[2];
            unsigned char green = pixel[1];
            unsigned char blue = pixel[0];

            // Thresholding condition: check if the pixel is within the red color range
            if (!(red < 100 && green < 100 && blue > 90))
            {
                // Make non-red pixels black
                pixel[0] = 0;  // Blue channel
                pixel[1] = 0;  // Green channel
                pixel[2] = 0;  // Red channel
            }
        }
    }
    return 1;
}

void vc_sobel_edge_detection(IVC* image)
{
    if (image == NULL || image->data == NULL || image->width <= 0 || image->height <= 0 || image->channels != 3)
        return;

    int width = image->width;
    int height = image->height;
    unsigned char* data = image->data;

    unsigned char* grayscale = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    unsigned char* edges = (unsigned char*)malloc(width * height * sizeof(unsigned char));

    // Convert image to grayscale
    for (int i = 0; i < width * height; i++)
    {
        unsigned char* pixel = data + i * 3;
        unsigned char gray = (unsigned char)(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
        grayscale[i] = gray;
    }

    // Apply Sobel operator for edge detection
    int sobelX[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    int sobelY[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            float gx = 0, gy = 0;

            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    int index = (y + j) * width + (x + i);
                    unsigned char pixel = grayscale[index];

                    gx += sobelX[(j + 1) * 3 + (i + 1)] * pixel;
                    gy += sobelY[(j + 1) * 3 + (i + 1)] * pixel;
                }
            }

            float magnitude = sqrt(gx * gx + gy * gy);
            edges[y * width + x] = (magnitude > 128) ? 255 : 0;
        }
    }

    // Update the image data with the edges
    for (int i = 0; i < width * height; i++)
    {
        unsigned char* pixel = data + i * 3;
        unsigned char edge = edges[i];

        pixel[0] = edge;
        pixel[1] = edge;
        pixel[2] = edge;
    }

    free(grayscale);
    free(edges);
}



int convertToColorImage(IVC* grayImage, IVC* colorImage)
{
    if (grayImage == NULL || grayImage->data == NULL || grayImage->channels != 1)
        return 0;

    int width = grayImage->width;
    int height = grayImage->height;
    unsigned char* grayData = grayImage->data;

    // Copy the grayscale values into each channel of the color image
    for (int i = 0; i < width * height; i++) {
        unsigned char grayValue = grayData[i];
        //printf("%c",grayValue);
        colorImage->data[i * 3] = grayValue;
        colorImage->data[i * 3 + 1] = grayValue;
        colorImage->data[i * 3 + 2] = grayValue;
    }

    return 1;
}





void drawBoundingBox(IVC* image, int x, int y, int width, int height)
{
    if (image == NULL || image->data == NULL || image->channels != 3)
        return;

    unsigned char* data = image->data;

    // Top border
    for (int i = x; i < x + width; i++) {
        data[(y * image->width + i) * 3] = 0;
        data[(y * image->width + i) * 3 + 1] = 255;
        data[(y * image->width + i) * 3 + 2] = 0;
    }

    // Bottom border
    for (int i = x; i < x + width; i++) {
        data[((y + height - 1) * image->width + i) * 3] = 0;
        data[((y + height - 1) * image->width + i) * 3 + 1] = 255;
        data[((y + height - 1) * image->width + i) * 3 + 2] = 0;
    }

    // Left border
    for (int i = y; i < y + height; i++) {
        data[(i * image->width + x) * 3] = 0;
        data[(i * image->width + x) * 3 + 1] = 255;
        data[(i * image->width + x) * 3 + 2] = 0;
    }

    // Right border
    for (int i = y; i < y + height; i++) {
        data[(i * image->width + x + width - 1) * 3] = 0;
        data[(i * image->width + x + width - 1) * 3 + 1] = 255;
        data[(i * image->width + x + width - 1) * 3 + 2] = 0;
    }
}


OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels,int color)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

				// checksção de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

													// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++){
            blobs[a].label = labeltable[a];
            blobs[a].color = color;
        } 
	}
	else return NULL;

	return blobs;
}




int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// checksção de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / max(blobs[i].area, 1);
		blobs[i].yc = sumy / max(blobs[i].area, 1);
	}

	return 1;
}


int makeBlack(IVC* img) {

    // Check if the image pointer is valid
    if (img == NULL || img->data == NULL) {
        return 0;  // Invalid image pointer
    }

    //make black
    for (int y = 0; y < img->height; y++)
	{
		for (int x = 0; x < img->width; x++)
		{
            img->data[y*img->width+x ] = 0;
        }
    }
    return 1; 
}



int drawBox(IVC* colorImage, int x, int y, int width, int height) {
    int i, j;
    IVC *img = vc_image_new(colorImage->width, colorImage->height, 1, 255);


    if (colorImage == NULL || colorImage->data == NULL || colorImage->channels != 3)
        return 0;

    
    // Check if the coordinates are within the image boundaries
    if (x < 0 || x >= img->width || y < 0 || y >= img->height || width <= 0 || height <= 0) {
        return 0;  // Invalid coordinates or dimensions
    }
    

    //make black
    makeBlack(img);

    // Draw top horizontal line
    for (i = x; i < x + width; i++) {
        img->data[y * img->width + i] = 255;  // Set the pixel value to white (255)
    }
    
    // Draw bottom horizontal line
    for (i = x; i < x + width; i++) {
        img->data[(y + height - 1) * img->width + i] = 255;
    }
    
    // Draw left vertical line
    for (j = y; j < y + height; j++) {
        img->data[j * img->width + x] = 255;
    }
    
    // Draw right vertical line
    for (j = y; j < y + height; j++) {
        img->data[j * img->width + x + width - 1] = 255;
    }
    
    unsigned char* grayData = img->data;
    // Copy the grayscale values into each channel of the color image
    for (int i = 0; i < img->width * img->height; i++) {

        if(grayData[i]==255){
            colorImage->data[i * 3] = 0;
            colorImage->data[i * 3 + 1] = 128;
            colorImage->data[i * 3 + 2] = 0;
        }
    }


    return 1;  // Box successfully drawn
}



int detect_sign(OVC*blobs,int blobId,IVC *tempGray)
{
	int pos, x, y;
	//printf("%d ",blobs[blobId].color);
	if(blobs[blobId].color==RED)
	{
		x = blobs[blobId].width * 0.36 + blobs[blobId].x;
		y = blobs[blobId].height * 0.36 + blobs[blobId].y;
		pos = y * tempGray->bytesperline + x * tempGray->channels;

		if (tempGray->data[pos]>200) {
			return STOP;
		}
		return FORBIDDEN;
	}
	else if(blobs[blobId].color==BLUE)
	{
		x = blobs[blobId].width * 0.05 + blobs[blobId].x;
		y = blobs[blobId].width * 0.05 + blobs[blobId].y;
		pos = y * tempGray->bytesperline + x * tempGray->channels;

		if (tempGray->data[pos]>200) {
			x = blobs[blobId].width * 0.27 + blobs[blobId].x;
			y = blobs[blobId].width * 0.34 + blobs[blobId].y;
			pos = y * tempGray->bytesperline + x * tempGray->channels;

			if (tempGray->data[pos]>200) {
				return ARROWLEFT;
			}
			return ARROWRIGHT;
		}
		else{
			x = blobs[blobId].width * 0.30 + blobs[blobId].x;
			y = blobs[blobId].width * 0.82 + blobs[blobId].y;
			pos = y * tempGray->bytesperline + x * tempGray->channels;

			if (tempGray->data[pos]>200) {
				return HIGHWAY;
			}
			return CAR;
		}
	}

    return -1;
}