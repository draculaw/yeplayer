#include <jni.h>
#include <string.h>
#include <android/bitmap.h>
#include <SLES/OpenSLES.h>

#include "com_draculaw_yeplayer_YePlayerHelper.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#define YEPLAPYER_UNUSED(x)  (void)(x)

#define LOG_LEVEL 99

#include <android/log.h>
#define LOG_TAG "yeplayer"
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}


//typedef struct VideoState;
typedef struct VideoState {
	AVFormatContext *pFormatCtx;
	AVStream *pVideoStream;
	int videoStreamIdx;
	AVFrame *frame;  //to store the decoded frame   
	AVFrame *audioFrame;
	int fint;
    int64_t nextFrameTime;//track next frame display time    
	int frameqDequeueIdx, frameqEnqueueIdx;
	AVIndexEntry *entryIdx;
//	sem_t fqfullsem, fqemptysem;
    int status;

} VideoState;

typedef struct VideoDisplayUtil {    
	struct SwsContext *img_resample_ctx;
    AVFrame *pFrameRGBA;    
    int width, height;
    void* pBitmap;//use the bitmap as the pFrameRGBA buffer    
    int frameNum; 
} VideoDisplayUtil;

typedef struct AudioState
{
	/* data */
} AudioState;


char *gVideoFileName = NULL;
int gNextPlayTime = 0;
VideoState *gvs;
VideoDisplayUtil *gvdu;

/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naYepHello
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naYepHello
  (JNIEnv *env, jobject thiz)
{
	YEPLAPYER_UNUSED(thiz);
    return (*env)->NewStringUTF(env, "Hello from Ubuntu!");
}



/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naInit
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naInit
  (JNIEnv *pEnv, jobject pObj, jstring filename)
{
	int i;
	VideoState *vs;
	AVCodecContext *pCodecCtx;

	YEPLAPYER_UNUSED(pObj);
	YEPLAPYER_UNUSED(pEnv);
	YEPLAPYER_UNUSED(filename);

    LOGE(1, "init!");
	av_register_all();

	vs = av_mallocz(sizeof(VideoState));
	if (NULL == vs)
	{
		return -1;
	}
	gvs = vs;
	av_register_all();
	gVideoFileName = (char*)(*pEnv)->GetStringUTFChars(pEnv, filename, NULL);

	avformat_open_input(&vs->pFormatCtx, gVideoFileName, NULL, NULL);
	avformat_find_stream_info(vs->pFormatCtx, NULL);

	vs->videoStreamIdx = -1;

	for (i = 0; i < vs->pFormatCtx->nb_streams; ++i) {
		if (AVMEDIA_TYPE_VIDEO == 
				vs->pFormatCtx->streams[i]->codec->codec_type) {
			vs->videoStreamIdx = i;
			vs->pVideoStream = vs->pFormatCtx->streams[i];
			break;
		}
	}

	pCodecCtx = vs->pFormatCtx->streams[vs->videoStreamIdx]->codec;
	AVCodec *pCodec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	avcodec_open2(pCodecCtx, pCodec, NULL);

    return 0;
}

/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naGetVideoRes
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naGetVideoRes
 (JNIEnv *pEnv, jobject pObj)
{
	jintArray lRes;
	AVCodecContext *vCodecCtx = gvs->pVideoStream->codec;

	YEPLAPYER_UNUSED(pObj);
	YEPLAPYER_UNUSED(pEnv);

	lRes = (*pEnv)->NewIntArray(pEnv, 2);
	jint lVideoRes[2];
	lVideoRes[0] = vCodecCtx->width;
	lVideoRes[1] = vCodecCtx->height;
	(*pEnv)->SetIntArrayRegion(pEnv, lRes, 0, 2, lVideoRes);

    return lRes;
}

/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naGetFrameRate
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naGetFrameRate
  (JNIEnv *env, jobject thiz)
{
	YEPLAPYER_UNUSED(thiz);
	YEPLAPYER_UNUSED(env);

	int fr;
	VideoState *vs = gvs;

    if (vs->pVideoStream->avg_frame_rate.den 
    	&& vs->pVideoStream->avg_frame_rate.num) {     

    	fr = av_q2d(vs->pVideoStream->avg_frame_rate);

    } else if (vs->pVideoStream->r_frame_rate.den 
    	&& vs->pVideoStream->r_frame_rate.num) {     

    	fr = av_q2d(vs->pVideoStream->r_frame_rate);

  } else if(vs->pVideoStream->time_base.den 
  		&& vs->pVideoStream->time_base.num) {

  		fr = 1/av_q2d(vs->pVideoStream->time_base);

  } else if (vs->pVideoStream->codec->time_base.den 
  		&& vs->pVideoStream->codec->time_base.num) {

    	fr = 1/av_q2d(vs->pVideoStream->codec->time_base);
  }   

  return fr;
}

/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naFinish
 * Signature: (Landroid/graphics/Bitmap;)I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naFinish
  (JNIEnv *pEnv, jobject pObj, jobject pBitmap)
{
	VideoDisplayUtil* vdu = gvdu;
	//free RGBA images
	AndroidBitmap_unlockPixels(pEnv, pBitmap);
	avcodec_free_frame(&vdu->pFrameRGBA);
	VideoState* vs = gvs;
	avcodec_free_frame(&vs->frame);
	//close codec
	avcodec_close(vs->pVideoStream->codec);
	//close video file
	avformat_close_input(&vs->pFormatCtx);
	av_free(vs);
	return 0;
}


/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naGetDuration
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naGetDuration
 (JNIEnv *pEnv, jobject pObj)
{
	YEPLAPYER_UNUSED(pObj);
	YEPLAPYER_UNUSED(pEnv);

	return gvs->pFormatCtx->duration / AV_TIME_BASE;
}



/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naGetFrameRes
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naGetFrameRes
 (JNIEnv *pEnv, jobject pObj)
{
	YEPLAPYER_UNUSED(pEnv);
	YEPLAPYER_UNUSED(pObj);
    return 0;
}


/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naGetVideoFrame
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naGetVideoFrame
 (JNIEnv *pEnv, jobject pObj)
{
  	YEPLAPYER_UNUSED(pEnv);
  	YEPLAPYER_UNUSED(pObj);
  	VideoState* vs = gvs;
  	VideoDisplayUtil *vdu = gvdu;
	//read frames and decode them 
  	AVPacket packet;   
  	int framefinished;
  	int audiofinished;
  	int retVideo;
  	int retAudio;
  	LOGE(1, "wxb dur %d time_base %d ", gvs->pFormatCtx->duration, AV_TIME_BASE);
  	while ((!vs->status) && 0 <= av_read_frame(vs->pFormatCtx, &packet)) {

  		if (vs->videoStreamIdx == packet.stream_index) {    

  			retVideo = avcodec_decode_video2(
  				vs->pVideoStream->codec, vs->frame, 
  				&framefinished, &packet);          

  			retAudio = avcodec_decode_audio4(
  				vs->pVideoStream->codec, vs->audioFrame, 
  				&audiofinished, &packet);       
  			int64_t curtime = av_gettime();

  			LOGE(1, "ret Video %d finished %d \n ret audio %d finished is %d!", 
                	retVideo, framefinished, retAudio, audiofinished);
  			if (audiofinished) {
                int data_size = av_samples_get_buffer_size(  
                        vs->audioFrame->linesize,
                        vs->pVideoStream->codec->channels,  
                        vs->audioFrame->nb_samples,
                        vs->pVideoStream->codec->sample_fmt, 
                        0);  

                LOGE(1, "data size is %d!", data_size);
  			}

  			if (framefinished) {

  				sws_scale(vdu->img_resample_ctx, 
  					vs->frame->data, 
  					vs->frame->linesize, 
  					0, 
  					vs->pVideoStream->codec->height, 
  					vdu->pFrameRGBA->data, 
  					vdu->pFrameRGBA->linesize);         


  				LOGE(1, "yeplayer fint %d framenume %d vs->nextFrameTime %d curtime %d", 
  					vs->fint, vdu->frameNum, vs->nextFrameTime, curtime);     
  				if (vs->nextFrameTime - curtime > 20*1000) {
  					//usleep(vs->nextFrameTime-curtime);
  					usleep(1);
  				}
  				++vdu->frameNum;
  				vs->nextFrameTime += vs->fint*1000;    

  				if (gNextPlayTime != 0) {
  					vs->nextFrameTime = gNextPlayTime * 1000;
  					gNextPlayTime = 0;
  				}
  				LOGE(1, "yeplayer fint %d framenume %d vs->nextFrameTime-curtime %d", 
  					vs->fint, vdu->frameNum, vs->nextFrameTime-curtime);        

  				return vdu->frameNum;
  			}
  		}
  		av_free_packet(&packet);
  	} 

  return 0;
}


/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naNextTime
 * Signature: (Landroid/graphics/Bitmap;II)I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naNextTime
  (JNIEnv *pEnv, jobject pObj, jint nextTime)
{
	gNextPlayTime = nextTime;

	return 0;
}

/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naPrepareDisplay
 * Signature: (Landroid/graphics/Bitmap;II)I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naPrepareDisplay
  (JNIEnv *pEnv, jobject pObj, jobject pBitmap, jint width, jint height)
{
	VideoState* vs = gvs;
	VideoDisplayUtil* vdu = av_mallocz(sizeof(VideoDisplayUtil));
	gvdu = vdu;
	vs->frame = avcodec_alloc_frame();
	vdu->frameNum = 0;
	vdu->width = width;
	vdu->height = height;
	vdu->pFrameRGBA = avcodec_alloc_frame();
	AndroidBitmapInfo linfo;
	int lret;
	//1. retrieve information about the bitmap
	if ((lret = AndroidBitmap_getInfo(pEnv, pBitmap, &linfo)) < 0) {
		LOGE(1, "AndroidBitmap_getinfo failed! error = %d", lret);
		return -1;
	}
	if (linfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE(1, "bitmap format is not rgba_8888!");
		return- 1;
	}
	//2. lock the pixel buffer and retrieve a pointer to it
	if ((lret = AndroidBitmap_lockPixels(pEnv, pBitmap, &vdu->pBitmap)) < 0) {
		LOGE(1, "AndroidBitmap_lockpixels() failed! error = %d", lret);
		return -1;
	}
	//for android, we use the bitmap buffer as the buffer for pFrameRGBA
	avpicture_fill((AVPicture*)vdu->pFrameRGBA, vdu->pBitmap, PIX_FMT_RGBA, width, height);
	vdu->img_resample_ctx = sws_getContext(
			vs->pVideoStream->codec->width, 
			vs->pVideoStream->codec->height, 
			vs->pVideoStream->codec->pix_fmt, 
			width, height, 
			PIX_FMT_RGBA, SWS_BICUBIC, 
			NULL, NULL, NULL);
	if (NULL == vdu->img_resample_ctx) {
		LOGE(1, "error initialize the video frame conversion context");
		return -1;
	}
	//LOGE(1, "%s %d", __FILE__, __LINE__);
	vs->nextFrameTime = av_gettime() + 50*1000;	//introduce 50 milliseconds of initial delay
	return 0;
}



/*
 * Class:     com_draculaw_yeplayer_YePlayerHelper
 * Method:    naStartDecodeVideo
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_draculaw_yeplayer_YePlayerHelper_naStartDecodeVideo
 (JNIEnv *pEnv, jobject pObj)
{
  	YEPLAPYER_UNUSED(pEnv);
  	YEPLAPYER_UNUSED(pObj);
    return 0;
}



#ifdef __cplusplus
}
#endif

