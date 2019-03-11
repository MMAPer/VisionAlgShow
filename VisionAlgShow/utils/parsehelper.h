#ifndef PARSEHELPER_H
#define PARSEHELPER_H

#include <cstdio>

//void yv12toYUV(char *outYuv, char *inYv12, int width, int height, int widthStep)
//{
//    int col, row;
//    unsigned int Y, U, V;
//    int tmp;
//    int idx;

////        printf("widthStep=%d. \n", widthStep);
//    for(row=0; row<height; row++)
//    {
//        idx = row * widthStep;
//        int rowptr = row*width;
//        for(col=0; col<width; col++)
//        {
//            tmp = (row/2) * (width/2) + (col/2);
//            Y = (unsigned int)inYv12[row*width + col];
//            U = (unsigned int)inYv12[width*height + width*height/4 + tmp];
//            V = (unsigned int)inYv12[width*height + tmp];
////                if((idx+col*3+2)>(1200*widthStep))
////                {
////                    //printf("row * widthStep=%d,idx+col*3+2=%d.\n",1200 * widthStep,idx+col*3+2);
////                }
//            outYuv[idx+col*3] = Y;
//            outYuv[idx+col*3+1] = U;
//            outYuv[idx+col*3+2] = V;
//        }
//    }
//}
#endif // PARSEHELPER_H






















