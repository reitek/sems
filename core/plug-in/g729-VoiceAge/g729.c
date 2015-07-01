/*

  This file is part of SEMS, a free SIP media server.

 Copyright (c) 2007, Vadim Lebedev
 Copyright (c) 2010, Stefan Sayer
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 Notes:

 This is a wrapper for VoiceAge G729 codec.

*/

#include "../../log.h"
#include <stdio.h>
#include "amci.h"
#include "codecs.h"
#include <codecLib_if.h>
#include <stdlib.h>
#include <string.h>

static int pcm16_2_g729(unsigned char* out_buf, unsigned char* in_buf, unsigned int size, 
			unsigned int channels, unsigned int rate, long h_codec );

static int g729_2_pcm16(unsigned char* out_buf, unsigned char* in_buf, unsigned int size, 
		       unsigned int channels, unsigned int rate, long h_codec );

static long g729_create(const char* format_parameters, amci_codec_fmt_info_t* format_description);
static void g729_destroy(long h_codec);

static unsigned int g729_bytes2samples(long, unsigned int);
static unsigned int g729_samples2bytes(long, unsigned int);

#define G729_PAYLOAD_ID          18
#define G729_BYTES_PER_FRAME     10
#define G729_SAMPLES_PER_FRAME   10
#define MAX_BUFF_SIZE			1024


BEGIN_EXPORTS( "g729", AMCI_NO_MODULEINIT, AMCI_NO_MODULEDESTROY)

    BEGIN_CODECS
    CODEC( CODEC_G729, pcm16_2_g729, g729_2_pcm16, AMCI_NO_CODEC_PLC,
	   (amci_codec_init_t)g729_create, (amci_codec_destroy_t)g729_destroy,
            g729_bytes2samples, g729_samples2bytes
          )
    END_CODECS
    
    BEGIN_PAYLOADS
      PAYLOAD( G729_PAYLOAD_ID, "G729", 8000, 1, CODEC_G729, AMCI_PT_AUDIO_FRAME )
    END_PAYLOADS

    BEGIN_FILE_FORMATS
    END_FILE_FORMATS

END_EXPORTS

struct stream
{
  void*  handle;
};

struct G729_codec 
{
  struct  stream  dec, enc;
  codecOption option;
  codecProperties properties;
  codec codecType;
};


static long g729_create(const char* format_parameters, amci_codec_fmt_info_t* format_description)
{
	struct G729_codec *codec = calloc(sizeof(struct G729_codec), 1);
	INFO("Create codec %s" , format_parameters );
	if (strcmp(format_parameters,"annexb=yes") == 0)
	{
		INFO("%s --> use G729AB",format_parameters);
		codec->codecType = G729AB;
	}
	else
	{
		INFO("%s --> use G729A",format_parameters);
		codec->codecType = G729A;
	}

	int rc = codecLibQuery(NULL, codec->codecType, (codecUsage)0, 0, &codec->option, 0);
    if (rc < 0)
	{
        //----------------------
        // list available codecs
        //----------------------
        codecLib_printf("\ncodecType NOT AVAILABLE in this package!\n\n");
        return(0);
    }

	
	int ret = codecLibOpen(&codec->dec.handle, codec->codecType, DEC, 0);
	
	if (ret < 0)
	{
		ERROR("ERROR - codecLibOpen with DEC returned err:%d", ret);
		return ret;
	}
	
    ret = codecLibOpen(&codec->enc.handle, codec->codecType, ENC, 0);
	if (ret < 0)
	{
		ERROR("ERROR - codecLibOpen with ENC returned err:%d", ret);
		return ret;
	}
	
	INFO("Created a codec with id: %p, dec handle is %p, enc handle is %p, G729:%d, DEC:%d, ENC:%d", codec, codec->dec.handle, codec->enc.handle, codec->codecType,DEC,ENC);
    return (long) codec;
}


static void g729_destroy(long h_codec)
{
    struct G729_codec *codec = (struct G729_codec *) h_codec;
	
    if (!h_codec)
      return;
	
	codecLibClose(&codec->enc.handle, 0);
	codecLibClose(&codec->dec.handle, 0);
	INFO("g729_destroy handle %p",codec);
    free(codec);
}



static int pcm16_2_g729(unsigned char* out_buf, unsigned char* in_buf, unsigned int size, 
						unsigned int channels, unsigned int rate, long h_codec )
{
	struct G729_codec *codec = (struct G729_codec *) h_codec;
	if (!h_codec)
	{
		ERROR("ERROR - passed a NULL handle %ld",h_codec);
		return -1;
	}
	//INFO("pcm16_2_g729 with size %d, channe ls %d, rate %d, h_codec: %lx, enc_handle: %p", size,channels,rate,h_codec, codec->enc.handle);
	
	
	div_t blocks = div(size, G729_BYTES_PER_FRAME);
	if (blocks.rem)
	{
		ERROR("pcm16_2_G729: number of blocks should be integral (block size = %d)\n",
			G729_BYTES_PER_FRAME);
		return -1;
	}

	int out_size = 0;

	while(size >= G729_BYTES_PER_FRAME) 
	{
		Word16* inBuffer;
		int		inSize;
		Word8*	outBuffer;
		int		outSize;

		/* Set input stream params */
		inSize = size;
		inBuffer = (Word16*)in_buf;
		
		/* Set output buffer */
		outBuffer = (Word8*)out_buf;
		
		/* Encode a frame  */
		//INFO("CodecOptions is %p and properties(%p)",(void*) &codec->option, (void*)&codec->properties);
		int res = codecLibQuery(&codec->enc.handle,codec->codecType, ENC, &codec->properties, &codec->option, 3);
		
		//INFO("CodecOptions is %p and properties(%p)",(void*) &codec->option, (void*)&codec->properties);
		if (res < 0)
		{
			ERROR("ERROR - codecLibQuery returned an error (%d) while codecLibQuery ENC", res);
			return res;
		}
		Word32 fromLen = inSize/2; //num of words
		Word32 toLen	= MAX_BUFF_SIZE; //MAX buffer
		//INFO("Calling codecLibEncode with handle %p, inSize %u and outSize %u",codec->enc.handle,(unsigned) fromLen,(unsigned)toLen);
		
		int ret = codecLibEncode(&codec->enc.handle, inBuffer,&fromLen, outBuffer,&toLen, &codec->option, 3);
		if (ret <0 )
		{
			ERROR("codecLibEncode returned %d",ret);
			return ret;
		}
		//INFO("codecLibEncode returned: inSize %u outSize %u",(unsigned) fromLen,(unsigned) toLen);
		
		inSize = fromLen * 2;  //size in bytes
		outSize = toLen;
		
		size -= inSize;
		in_buf += inSize;

		out_buf += outSize;
		out_size += outSize;
	}

	return out_size;
}

static int g729_2_pcm16(unsigned char* out_buf, unsigned char* in_buf, unsigned int size, 
						unsigned int channels, unsigned int rate, long h_codec )
{
	//INFO("g729_2_pcm16 with size %d, channels %d, rate %d", size,channels,rate);
	
	unsigned int out_size = 0;
	int frameSize = G729_BYTES_PER_FRAME;
	int x;
	struct G729_codec *codec = (struct G729_codec *) h_codec;

	if (!h_codec)
	{
		ERROR("Invalid codec handle %ld", h_codec);
		return -1;
	}

	Word8 *inBuffer = (Word8*) in_buf;
	Word32 inSize = size;
	Word16 *outBuffer = ( Word16*) out_buf ;
	Word32 outSize  = MAX_BUFF_SIZE; //MAX_BUFF_SIZE n_words
	
	for(x = 0; x < size; x += frameSize) 
	{
		int res = codecLibQuery(&codec->dec.handle,codec->codecType, DEC, &codec->properties, &codec->option, 3);
		if (res < 0)
		{
			ERROR("ERROR - codecLibQuery returned an error (%d) while codecLibQuery DEC", res);
			return res;
		}

		Word32 toLen = outSize/2;  //Num of words
		//INFO("Calling codecLibDecode with handle %p, inSize %d, outSize %d", codec->dec.handle, inSize, toLen);
		int ret = codecLibDecode(&codec->dec.handle, inBuffer, &inSize, outBuffer, &toLen, &codec->option, 3);
		if (ret <0 )
		{
			ERROR("codecLibDecode returned %d",ret);
			return ret;
		}

		//DBG("codecLibDecode returned: inSize %d outSize %d",inSize,toLen);
		inBuffer += inSize;
		frameSize = inSize;

		outBuffer += toLen;
		out_size += toLen*2;
	}

	return out_size;
}

static unsigned int g729_bytes2samples(long h_codec, unsigned int num_bytes) {
  return  (G729_SAMPLES_PER_FRAME * num_bytes) / G729_BYTES_PER_FRAME;
}

static unsigned int g729_samples2bytes(long h_codec, unsigned int num_samples) {
  return G729_BYTES_PER_FRAME * num_samples /  G729_SAMPLES_PER_FRAME; 
}
