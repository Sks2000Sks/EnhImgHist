#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hist-equ.h"
#include "time.h"

void run_cpu_color_test(PPM_IMG img_in,char *argv[]);
void run_cpu_gray_test(PGM_IMG img_in,char *argv[]);

int main(int argc, char *argv[])
{
    PGM_IMG img_ibuf_g;//gray scale image
    PPM_IMG img_ibuf_c;//color image
    
    clock_t start_time = clock(); // Start global time of the application 
    if(argc<3) printf("not sufficient arguments\n");
    else printf("The arguments are %s %s\n",argv[1],argv[2]);
   

    printf("Running contrast enhancement for color images with %d threads.\n", numThreadsPerBlock);
    img_ibuf_c = read_ppm(argv[2]);//ppm image
    run_cpu_color_test(img_ibuf_c,argv);
    free_ppm(img_ibuf_c);
           
    printf("Running contrast enhancement for gray-scale images with %d threads.\n", numThreadsPerBlock);
    img_ibuf_g = read_pgm(argv[1]);//pgm image
    run_cpu_gray_test(img_ibuf_g,argv);
    free_pgm(img_ibuf_g);
    
    clock_t stop_time = clock(); // Stop global time of the application
    float total_time = ((float)(stop_time - start_time))/(CLOCKS_PER_SEC/1000);    
    
    fprintf(stderr, "Result time: %f (ms)\n", total_time);

    return 0;
}

void run_cpu_color_test(PPM_IMG img_in,char *argv[])
{
    PPM_IMG img_obuf_hsl, img_obuf_yuv;//outbuffers for hsl and yuv images of the color image
    
    printf("Starting CPU processing...\n");
    
    clock_t start_hsl_time = clock();
    img_obuf_hsl = contrast_enhancement_c_hsl(img_in);
    clock_t stop_hsl_time = clock();
    float total_hsl_time = ((float)(stop_hsl_time - start_hsl_time))/(CLOCKS_PER_SEC/1000);
    
    printf("HSL processing time: %f (ms)\n", total_hsl_time); 
    write_ppm(img_obuf_hsl,  "out_hsl.ppm");

    clock_t start_yuv_time = clock();
    img_obuf_yuv = contrast_enhancement_c_yuv(img_in);

    clock_t stop_yuv_time = clock();
    float total_yuv_time = ((float)(stop_yuv_time - start_yuv_time))/(CLOCKS_PER_SEC/1000);

    printf("YUV processing time: %f (ms)\n", total_yuv_time);
    
    write_ppm(img_obuf_yuv, "out_yuv.ppm");
    
    free_ppm(img_obuf_hsl);
    free_ppm(img_obuf_yuv);
}

void run_cpu_gray_test(PGM_IMG img_in,char *argv[])
{
    PGM_IMG img_obuf;
    
    printf("Starting CPU processing...\n");
    
    clock_t start_time = clock();

    img_obuf = contrast_enhancement_g(img_in);

    clock_t stop_time = clock();
    float total_time = ((float)(stop_time - start_time))/(CLOCKS_PER_SEC/1000);  

    printf("Processing gray image time: %f (ms)\n", total_time);
    
    write_pgm(img_obuf,  "out.pgm");
    free_pgm(img_obuf);
}

PPM_IMG read_ppm(const char * path){
    FILE * in_file;
    char sbuf[256];
    
    char *ibuf;
    PPM_IMG result;
    int v_max, i;
    in_file = fopen(path, "r");
    if (in_file == NULL){
        printf("Input file not found!\n");
        exit(1);
    }
    /*Skip the magic number*/
    fscanf(in_file, "%s", sbuf);


    //result = malloc(sizeof(PPM_IMG));
    fscanf(in_file, "%d",&result.w);
    fscanf(in_file, "%d",&result.h);
    fscanf(in_file, "%d\n",&v_max);
    printf("Image size: %d x %d\n", result.w, result.h);
    

    result.img_r = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_g = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    result.img_b = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));
    ibuf         = (char *)malloc(3 * result.w * result.h * sizeof(char));

    
    fread(ibuf,sizeof(unsigned char), 3 * result.w*result.h, in_file);

    for(i = 0; i < result.w*result.h; i ++){
        result.img_r[i] = ibuf[3*i + 0];
        result.img_g[i] = ibuf[3*i + 1];
        result.img_b[i] = ibuf[3*i + 2];
    }
    
    fclose(in_file);
    free(ibuf);
    
    return result;
}

void write_ppm(PPM_IMG img, const char * path){
    FILE * out_file;
    int i;
    
    char * obuf = (char *)malloc(3 * img.w * img.h * sizeof(char));

    for(i = 0; i < img.w*img.h; i ++){
        obuf[3*i + 0] = img.img_r[i];
        obuf[3*i + 1] = img.img_g[i];
        obuf[3*i + 2] = img.img_b[i];
    }
    out_file = fopen(path, "wb");
    fprintf(out_file, "P6\n");
    fprintf(out_file, "%d %d\n255\n",img.w, img.h);
    fwrite(obuf,sizeof(unsigned char), 3*img.w*img.h, out_file);
    fclose(out_file);
    free(obuf);
}

void free_ppm(PPM_IMG img)
{
    free(img.img_r);
    free(img.img_g);
    free(img.img_b);
}

PGM_IMG read_pgm(const char * path){
    FILE * in_file;
    char sbuf[256];
    
    
    PGM_IMG result;
    int v_max;//, i;
    in_file = fopen(path, "r");
    if (in_file == NULL){
        printf("Input file not found!\n");
        exit(1);
    }
    
    fscanf(in_file, "%s", sbuf); /*Skip the magic number*/
    fscanf(in_file, "%d",&result.w);
    fscanf(in_file, "%d",&result.h);
    fscanf(in_file, "%d\n",&v_max);
    printf("Image size: %d x %d\n", result.w, result.h);
    

    result.img = (unsigned char *)malloc(result.w * result.h * sizeof(unsigned char));

        
    fread(result.img,sizeof(unsigned char), result.w*result.h, in_file);    
    fclose(in_file);
    
    return result;
}

void write_pgm(PGM_IMG img, const char * path){
    FILE * out_file;
    out_file = fopen(path, "wb");
    fprintf(out_file, "P5\n");
    fprintf(out_file, "%d %d\n255\n",img.w, img.h);
    fwrite(img.img,sizeof(unsigned char), img.w*img.h, out_file);
    fclose(out_file);
}

void free_pgm(PGM_IMG img)
{
    free(img.img);
}
