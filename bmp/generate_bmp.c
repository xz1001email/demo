#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>


/* 
 * this is a example to display gray image using bmp
 * cap_raw_raw_1600x1296_3200.raw file is a bit10 gray data,
 * should convert to bit8 when using bmp protocol
 *
 * */

typedef struct tagFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfResv1;
    uint16_t bfResv2;
    uint32_t bfOffBits;
}__attribute__ ((packed)) FileHeader;

typedef struct tagInodeHeader {
    uint32_t biSize;
    uint32_t biWith;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__ ((packed)) InodeHeader;


typedef struct tagPalette {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t resv;
}__attribute__ ((packed)) PaletteT;

PaletteT g_palette[256];


#define PIXEL_BITS  8
//#define PIXEL_BITS  10
#define WIDTH       1600
#define HEIGHT      1296

//#define WIDTH       640
//#define HEIGHT      720
//#define WIDTH    640
//#define HEIGHT   480

char buf[WIDTH*HEIGHT*2];
char gray[WIDTH*HEIGHT];
//#define GRAY_RAW_FILE "./cap_raw_raw_1600x1296_3200.raw"
#define GRAY_RAW_FILE "./cap.raw"

void palette_init(void)
{
    for (int i=0; i<256; i++) {
        g_palette[i].red = i;
        g_palette[i].green = i;
        g_palette[i].blue = i;
    }
    printf("palette inited\n");
}


int fillBmp(const char *rawfile, int mode, int bits, int width, int height)
{
    FILE *fp = fopen(rawfile, "r");
    if (!fp) {
        printf("open file %s fail\n", rawfile);
        return -1;
    }

    fread(buf, 1, width*height*2, fp);
    fclose(fp);

    FILE *fw = fopen("./out.bmp", "w+");
    if (!fw) {
        printf("open file fail\n");
        return -1;
    }

    uint16_t *p = (uint16_t *)buf;


    int drop_bit = 16 - bits;

    // gray bit10 to gray bit8
    for (int h=0; h<height; h++) {
        for (int w=0; w<width; w++) {
            
            if (mode == 0) {
                //pixel 从上到下，从左到右
                gray[h*width+w] = (p[h*width + w] >> drop_bit) & 0xFF;
            } else if (mode == 1) {
                //pixel 从上到下，从右到左
                gray[h*width+w] = (p[h*width + width-1 - w] >> drop_bit) & 0xFF;

            } else if (mode ==2) {
                //pixel 从下到上，从左到右
                gray[h*width+w] = (p[width*height - 1 - (h*width+ w)] >> drop_bit) & 0xFF;

            } else if (mode == 3) {
                //pixel 从下到上，从右到左
                gray[h*width+w] = (p[width*height - 1 - (h*width+ width-1 - w)] >> drop_bit) & 0xFF;
            } else {
                //pixel 从下到上，从右到左
                gray[h*width+w] = (p[width*height - 1 - (h*width+ width-1 - w)] >> drop_bit) & 0xFF;
            }

        }
    }

    palette_init();

    FileHeader head;
    head.bfType = 0x4D42; //BM

    //all data size
    head.bfSize = sizeof(FileHeader) + sizeof(InodeHeader) + sizeof(g_palette) + sizeof(gray);
    head.bfResv1 = 0;
    head.bfResv2 = 0;

    // image offset
    head.bfOffBits = sizeof(FileHeader) + sizeof(InodeHeader) + sizeof(g_palette);


    InodeHeader node;
    node.biSize = sizeof(InodeHeader);
    node.biWith = width;
    node.biHeight = height;
    node.biPlanes = 1; //must set 1
    //node.biBitCount = 16;
    node.biBitCount = 8; // bit8 256 level gray
    node.biCompression = 0;

    //image data size
    node.biSizeImage = sizeof(gray);
    node.biXPelsPerMeter = 0x0EC4;
    node.biYPelsPerMeter = 0x0EC4;
    node.biClrUsed = 0;
    node.biClrImportant = 0;

    int ret = 0;
    ret += fwrite(&head, 1, sizeof(FileHeader), fw);
    printf("write head %d\n", ret);

    ret += fwrite(&node, 1, sizeof(InodeHeader), fw);
    printf("write node %d\n", ret);

    //write palette
    ret += fwrite(&g_palette[0], 1, sizeof(g_palette), fw);
    printf("write palette %d\n", ret);

    ret += fwrite(gray, 1, sizeof(gray), fw);
    printf("write image data %d\n", ret);
    fclose(fw);
    

    return 0;
}


const char *opt_string = "hm:f:b:W:H:";


void usage(void)
{
    fprintf(stderr, "Usage: [-h] [-f rawfile] [-m mode] [-b pixel_bit] [-W width] [-H height]\n");
    //fprintf(stderr, "./generate_bmp -f cap_raw_raw_1600x1296_3200.raw -b 10 -W 1600 -H 1296\n");
}
enum {
    POS_FILE,
    POS_BITS,
    POS_WIDTH,
    POS_HEIGHT,
    POS_MAX
};

int main(int argc, char *argv[])
{
    int mode = 3, bits=8;
    int opt;
    char file[128] = {"cap.raw"};
    int width = 1280, height=720;
    uint32_t flag = 0x0;

    while ((opt = getopt(argc, argv, opt_string)) != -1) {
        switch (opt) {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'm':
                mode = atoi(optarg);
                break;
            case 'b':
                {
                    bits = atoi(optarg);
                    flag |= (1<<POS_BITS);
                }
                break;
            case 'W':
                {
                    width = atoi(optarg);
                    flag |= (1<<POS_WIDTH);
                }
                break;
            case 'H':
                {
                    height = atoi(optarg);
                    flag |= (1<<POS_HEIGHT);
                }
                break;
            case 'f':
                {
                    snprintf(file, sizeof(file), "%s", optarg);
                    flag |= (1<<POS_FILE);
                }
                break;
            default: /* '?' */
                usage();
                exit(EXIT_FAILURE);
        }
    }

    int validbits = (1 << POS_MAX) - 1;
    if((flag & validbits) != validbits) {
        fprintf(stderr, "need args: [-f rawfile] [-b pixel_bit] [-W width] [-H height]\n");
        return -1;
    }
    
    if (width > width || height > HEIGHT) {
        printf("width max %d height max %d\n", WIDTH, HEIGHT);
        return -1;
    }

    fillBmp(file, mode, bits, width, height);


    /* Other code omitted */

    exit(EXIT_SUCCESS);
}






