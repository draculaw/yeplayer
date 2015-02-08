#include <assert.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#define LOG_TAG "mylib"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

void Java_ru_dzakhov_ffmpeg_test_MainActivity_createEngine(JNIEnv* env,
        jclass clazz) {
    avcodec_init();

    av_register_all();

}

jstring Java_ru_dzakhov_ffmpeg_test_MainActivity_loadFile(JNIEnv* env,
		jobject obj, jstring file, jbyteArray array) {

	jboolean isfilenameCopy;
	const char * filename = (*env)->GetStringUTFChars(env, file,
			&isfilenameCopy);
	int audioStreamIndex;
	AVCodec *codec;
	AVCodecContext *c = NULL;
	AVFormatContext * pFormatCtx;
	AVCodecContext * aCodecCtx;
	int out_size, len, audioStream = -1, i, err;
	FILE *f, *outfile;
	uint8_t *outbuf;
	uint8_t inbuf[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	AVPacket avpkt;
	jclass cls = (*env)->GetObjectClass(env, obj);
	jmethodID play = (*env)->GetMethodID(env, cls, "playSound", "([BI)V"); //At the begining of your main function

	LOGE("source file name is %s", filename);

	avcodec_init();

	av_register_all();

	LOGE("Stage 1");
	/* get format somthing of source file to AVFormatContext */
	int lError;
	if ((lError = av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL))
			!= 0) {
		LOGE("Error open source file: %d", lError);
		exit(1);
	}
	if ((lError = av_find_stream_info(pFormatCtx)) < 0) {
		LOGE("Error find stream information: %d", lError);
		exit(1);
	}
	LOGE("Stage 1.5");
	LOGE("audio format: %s", pFormatCtx->iformat->name);
	LOGE("audio bitrate: %d", pFormatCtx->bit_rate);

	audioStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO,
			-1, -1, &codec, 0);

	LOGE("audio codec: %s", codec->name);

	/* get codec somthing of audio stream to AVCodecContext */
	aCodecCtx = pFormatCtx->streams[audioStreamIndex]->codec;
	if (avcodec_open(aCodecCtx, codec) < 0) {
		LOGE("cannot open the audio codec!");
		exit(1);
	}

	printf("Audio decoding\n");
	LOGE("Stage 1.7");
	LOGE("S");
	codec = avcodec_find_decoder(aCodecCtx->codec_id);
	LOGE("Stage 1.8");
	if (!codec) {
		LOGE("codec not found\n");
		exit(1);
	}
	LOGE("Stage 2");
	//              c= avcodec_alloc_context();
	LOGE("Stage 3");
	/* open it */
	if (avcodec_open(aCodecCtx, codec) < 0) {
		LOGE("could upper");
		fprintf(stderr, "could not open codec\n");
		LOGE("could not open codec");

	}
	LOGE("Stage 4");
	outbuf = malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "could not open %s\n", filename);
		LOGE("could not open");
		exit(1);
	}

	/* decode until eof */
	avpkt.data = inbuf;
	avpkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);
	LOGE("Stage 5");

	while (avpkt.size > 0) {
		//                  LOGE("Stage 6");
		out_size = (AVCODEC_MAX_AUDIO_FRAME_SIZE / 3) * 2;
		len = avcodec_decode_audio3(aCodecCtx, (int16_t *) outbuf,
				&out_size, &avpkt);
		LOGE("data_size %d len %d", out_size, len);
		if (len < 0) {
			fprintf(stderr, "Error while decoding\n");
			LOGE("DECODING ERROR");
			LOGE("DECODING ERROR %d", len);
			exit(1);
		}
		//                  LOGE("Stage 7");
		if (out_size > 0) {
			/* if a frame has been decoded, output it */
			//                      LOGE("Stage 8");
			jbyte *bytes = (*env)->GetByteArrayElements(env, array, NULL);
			memcpy(bytes, outbuf, out_size); //
			(*env)->ReleaseByteArrayElements(env, array, bytes, 0);
			(*env)->CallVoidMethod(env, obj, play, array, out_size);
			//                      LOGE("DECODING ERROR5");
		}
		LOGE("Stage 9");
		avpkt.size -= len;
		avpkt.data += len;
		if (avpkt.size < AUDIO_REFILL_THRESH) {
			/* Refill the input buffer, to avoid trying to decode
			 * incomplete frames. Instead of this, one could also use
			 * a parser, or use a proper container format through
			 * libavformat. */
			memmove(inbuf, avpkt.data, avpkt.size);
			avpkt.data = inbuf;
			len = fread(avpkt.data + avpkt.size, 1,
					AUDIO_INBUF_SIZE - avpkt.size, f);
			if (len > 0)
				avpkt.size += len;
		}
	}
	LOGE("Stage 12");
	fclose(f);
	free(outbuf);

	avcodec_close(c);
	av_free(c);

}



void Java_com_rohaupt_RRD2_player_Example(JNIEnv* env, jobject obj,jstring file,jbyteArray array)
{
	jboolean            isfilenameCopy;
	const char *        filename = (*env)->GetStringUTFChars(env, file, &isfilenameCopy);
	AVCodec *codec;
	AVCodecContext *c= NULL;
	int out_size, len;
	FILE *f, *outfile;
	uint8_t *outbuf;
	uint8_t inbuf[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	AVPacket avpkt;
	jclass              cls = (*env)->GetObjectClass(env, obj);
	jmethodID           play = (*env)->GetMethodID(env, cls, "playSound", "([BI)V");//At the begining of your main function

	av_init_packet(&avpkt);

	printf("Audio decoding\n");

	/* find the mpeg audio decoder */
	codec = avcodec_find_decoder(CODEC_ID_MP3);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}

	c= avcodec_alloc_context();

	/* open it */
	if (avcodec_open(c, codec) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}

	outbuf = malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "could not open %s\n", filename);
		exit(1);
	}

	/* decode until eof */
	avpkt.data = inbuf;
	avpkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);

	while (avpkt.size > 0) {
		out_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		len = avcodec_decode_audio3(c, (short *)outbuf, &out_size, &avpkt);
		if (len < 0) {
			fprintf(stderr, "Error while decoding\n");
			exit(1);
		}
		if (out_size > 0) {
			/* if a frame has been decoded, output it */
			jbyte *bytes = (*env)->GetByteArrayElements(env, array, NULL);
			memcpy(bytes, outbuf, out_size); //
			(*env)->ReleaseByteArrayElements(env, array, bytes, 0);
			(*env)->CallVoidMethod(env, obj, play, array, out_size);

		}
		avpkt.size -= len;
		avpkt.data += len;
		if (avpkt.size < AUDIO_REFILL_THRESH) {
			/* Refill the input buffer, to avoid trying to decode
			 * incomplete frames. Instead of this, one could also use
			 * a parser, or use a proper container format through
			 * libavformat. */
			memmove(inbuf, avpkt.data, avpkt.size);
			avpkt.data = inbuf;
			len = fread(avpkt.data + avpkt.size, 1,
					AUDIO_INBUF_SIZE - avpkt.size, f);
			if (len > 0)
				avpkt.size += len;
		}
	}

	fclose(f);
	free(outbuf);

	avcodec_close(c);
	av_free(c);
}


package com.rohaupt.RRD2;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.SystemClock;

public class player extends Activity
{
    private AudioTrack track;
    private FileOutputStream os;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        createEngine();

        MediaPlayer mp = new MediaPlayer();
        mp.start();

        int bufSize = AudioTrack.getMinBufferSize(32000,
                                                  AudioFormat.CHANNEL_CONFIGURATION_STEREO, 
                                                  AudioFormat.ENCODING_PCM_16BIT);


        track = new AudioTrack(AudioManager.STREAM_MUSIC, 
                               32000, 
                               AudioFormat.CHANNEL_CONFIGURATION_STEREO, 
                               AudioFormat.ENCODING_PCM_16BIT, 
                               bufSize,
                               AudioTrack.MODE_STREAM);

        byte[] bytes = new byte[bufSize];

        try {
            os = new FileOutputStream("/sdcard/a.out",false);
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        String result = loadFile("/sdcard/a.mp3",bytes);

        try {
            os.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    void playSound(byte[] buf, int size) {  
        //android.util.Log.v("ROHAUPT", "RAH Playing");
        if(track.getPlayState()!=AudioTrack.PLAYSTATE_PLAYING)
            track.play();
        track.write(buf, 0, size);

        try {
            os.write(buf,0,size);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }


    private native void createEngine();
    private native String loadFile(String file, byte[] array);

    /** Load jni .so on initialization*/ 
    static {
         System.loadLibrary("avutil"); 
         System.loadLibrary("avcore"); 
         System.loadLibrary("avcodec");
         System.loadLibrary("avformat");
         System.loadLibrary("avdevice");
         System.loadLibrary("swscale");
         System.loadLibrary("avfilter");
         System.loadLibrary("ffmpeg");
    }
}

#include <assert.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#define DEBUG_TAG "ROHAUPT"  
//http://stackoverflow.com/questions/6228008/decoding-audio-via-android-using-ffmpeg

void Java_com_rohaupt_RRD2_player_createEngine(JNIEnv* env, jclass clazz)
{
	avcodec_init();

	av_register_all();


}

jstring Java_com_rohaupt_RRD2_player_loadFile(JNIEnv* env, jobject obj,jstring file,jbyteArray array)
{   
	jboolean            isCopy;  
	int                 i;
	int                 audioStream=-1;
	int                 res;
	int                 decoded = 0;
	int                 out_size;
	AVFormatContext     *pFormatCtx;
	AVCodecContext      *aCodecCtx;
	AVCodecContext      *c= NULL;
	AVCodec             *aCodec;
	AVPacket            packet;
	jclass              cls = (*env)->GetObjectClass(env, obj);
	jmethodID           play = (*env)->GetMethodID(env, cls, "playSound", "([BI)V");//At the begining of your main function
	const char *        szfile = (*env)->GetStringUTFChars(env, file, &isCopy);
	int16_t *           pAudioBuffer = (int16_t *) av_malloc (AVCODEC_MAX_AUDIO_FRAME_SIZE*2+FF_INPUT_BUFFER_PADDING_SIZE);
	int16_t *           outBuffer = (int16_t *) av_malloc (AVCODEC_MAX_AUDIO_FRAME_SIZE*2+FF_INPUT_BUFFER_PADDING_SIZE);


	__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, "RAH28 Starting");
	res = av_open_input_file(&pFormatCtx, szfile, NULL, 0, NULL);
	if(res!=0)
	{
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH opening input failed with result: [%d]", res);
		return file;
	}

	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH getting stream info");
	res = av_find_stream_info(pFormatCtx);
	if(res<0)
	{
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH getting stream info failed with result: [%d]", res);
		return file;
	}

	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH getting audio stream");
	for(i=0; i < pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
				audioStream < 0) {
			audioStream=i;
		}
	}


	if(audioStream==-1)
	{
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH couldn't find audio stream");
		return file;
	}
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio stream found with result: [%d]", res);


	aCodecCtx=pFormatCtx->streams[audioStream]->codec;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio codec info loaded");

	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio codec info [%d]", aCodecCtx->codec_id);

	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	if(!aCodec) {
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio codec unsupported");
		return file;
	}
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio codec info found");


	res = avcodec_open(aCodecCtx, aCodec);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio codec loaded [%d] [%d]",aCodecCtx->sample_fmt,res);

	//c=avcodec_alloc_context();
	av_init_packet(&packet);


	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH channels [%d] sample rate [%d] sample format [%d]",aCodecCtx->channels,aCodecCtx->sample_rate,aCodecCtx->sample_fmt);


	int x,y;
	x=0;y=0;
	while (av_read_frame(pFormatCtx, &packet)>= 0) {
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH frame read: [%d] [%d]",x++,y);

		if (aCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
			__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH audio ready");
			int data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE*2+FF_INPUT_BUFFER_PADDING_SIZE;
			int size=packet.size;
			y=0;
			decoded = 0;
			__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH packet size: [%d]", size);
			while(size > 0) {

				__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH decoding: [%d] [%d]",x,y++);
				int len = avcodec_decode_audio3(aCodecCtx, pAudioBuffer, &data_size, &packet);



				__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH 1 size [%d] len [%d] data_size [%d] out_size [%d]",size,len,data_size,out_size);
				jbyte *bytes = (*env)->GetByteArrayElements(env, array, NULL);

				memcpy(bytes + decoded, pAudioBuffer, len); //


				__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH 2");

				(*env)->ReleaseByteArrayElements(env, array, bytes, 0);
				__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH 3");

				(*env)->CallVoidMethod(env, obj, play, array, len);

				__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH 4");


				size -= len;
				decoded += len;

			}
			av_free_packet(&packet);
		}

	}

	// Close the video file
	av_close_input_file(pFormatCtx);

	//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "RAH Finished Running result: [%d]", res);
	(*env)->ReleaseStringUTFChars(env, file, szfile);   
	return file;  
}


{
AudioTrack track;
bufferSize = AudioTrack.getMinBufferSize(44100,AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT)
track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, mode);

//Play audio clip
track.play();

while(stream_is_over){
//Copy the decoded raw buffer from native code to "buffer" .....
//............
track.write(buffer, 0, readBytes);
}
}

// https://ffmpeg.org/doxygen/trunk/libavcodec_2avcodec_8h.html
// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html

