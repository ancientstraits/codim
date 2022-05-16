#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#define DURATION 10

int main(int argc, char **argv) {
	const char *filename = argc > 1 ? argv[1] : "out.mp4";
	AVFormatContext *fc;
	avformat_alloc_output_context2(&fc, NULL, NULL, filename);
	AVOutputFormat *of = fc->oformat;

	AVCodec *vc = avcodec_find_encoder(of->video_codec);
	AVPacket *vp = av_packet_alloc();
	AVStream *vs = avformat_new_stream(fc, NULL);
	vs->id = fc->nb_streams - 1;
	AVCodecContext *vcc = avcodec_alloc_context3(vc);
	vcc->width = 1920;
	vcc->height = 1080;
	vcc->time_base = (AVRational){1, 24};
	vcc->pix_fmt = AV_PIX_FMT_YUV420P;
	avcodec_open2(vcc, vc, NULL);
	AVFrame *vf = av_frame_alloc();
	vf->format = AV_PIX_FMT_YUV420P;
	vf->width = vcc->width;
	vf->height = vcc->height;
	av_frame_get_buffer(vf, 0);
	avcodec_parameters_from_context(vs->codecpar, vcc);

	AVCodec *ac = avcodec_find_encoder(of->audio_codec);
	AVPacket *ap = av_packet_alloc();
	AVStream *as = avformat_new_stream(fc, NULL);
	as->id = fc->nb_streams - 1;
	AVCodecContext *acc = avcodec_alloc_context3(ac);
	acc->sample_fmt = AV_SAMPLE_FMT_FLTP;
	acc->sample_rate = 44100;
	acc->channels = av_get_channel_layout_nb_channels(acc->channel_layout);
	acc->channel_layout = AV_CH_LAYOUT_STEREO;
	as->time_base = (AVRational){1, acc->sample_rate};

	// if (of->flags & AVFMT_GLOBALHEADER) {
	// 	vcc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	// 	acc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	// }

	avcodec_open2(acc, ac, NULL);
	float at = 0;
	float atincr = 2 * M_PI * 110.0 / acc->sample_rate;
	float atincr2 = atincr / acc->sample_rate;
	int anbs;
	if (ac->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		anbs = 10000;
	else
		anbs = acc->frame_size;
	AVFrame *af1 = av_frame_alloc();
	af1->format = AV_SAMPLE_FMT_S16;
	af1->channel_layout = acc->channel_layout;
	af1->sample_rate = acc->sample_rate;
	af1->nb_samples = anbs;
	av_frame_get_buffer(af1, 0);
	AVFrame *af2 = av_frame_alloc();
	af2->format = AV_SAMPLE_FMT_FLTP;
	af2->channel_layout = acc->channel_layout;
	af2->sample_rate = acc->sample_rate;
	af2->nb_samples = anbs;
	av_frame_get_buffer(af2, 0);
	avcodec_parameters_from_context(as->codecpar, acc);
	// samples count
	int asc = 0;

	// struct SwsContext *vconv =
	// 	sws_getContext(vf1->width, vf1->height, vf1->format, vf2->width,
	// 				   vf2->height, vf2->format, SWS_BICUBIC, NULL, NULL, NULL);

	struct SwrContext *aconv = swr_alloc();
	av_opt_set_int(aconv, "in_channel_count", acc->channels, 0);
	av_opt_set_int(aconv, "in_sample_rate", acc->sample_rate, 0);
	av_opt_set_sample_fmt(aconv, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(aconv, "out_channel_count", acc->channels, 0);
	av_opt_set_int(aconv, "out_sample_rate", acc->sample_rate, 0);
	av_opt_set_sample_fmt(aconv, "out_sample_fmt", acc->sample_fmt, 0);
	swr_init(aconv);

	av_dump_format(fc, 0, filename, 1);

	if (!(of->flags & AVFMT_NOFILE))
		avio_open(&fc->pb, filename, AVIO_FLAG_WRITE);

	if (avformat_write_header(fc, NULL) < 0)
		return 1;

	int apts = 0, aenc = 1;
	int vpts = 0, venc = 1;

	int video_times = 0, audio_times = 0;

	while (aenc || venc) {
		if (venc && (!aenc || av_compare_ts(vpts, vcc->time_base, apts,
											acc->time_base) <= 0)) {
			printf("video\n");
			// video
			if (av_compare_ts(vpts, vcc->time_base,
				DURATION, (AVRational){1, 1}) > 0) {
				vf = NULL;
			} else {
				av_frame_make_writable(vf);
				for (int i = 0; i < vf->height; i++) {
					for (int j = 0; j < vf->width; j++) {
						vf->data[0][(i * vf->linesize[0]) + j] = 0;
					}
				}
				for (int i = 0; i < vf->height / 2; i++) {
					for (int j = 0; j < vf->width / 2; j++) {
						vf->data[1][(i * vf->linesize[1]) + j] = j;
						vf->data[2][(i * vf->linesize[2]) + j] = i;
					}
				}
				vf->pts = vpts++;
			}

			avcodec_send_frame(vcc, vf);

			int ret;
			while ((ret = avcodec_receive_packet(vcc, vp)) >= 0) {
				av_packet_rescale_ts(vp, vcc->time_base, vs->time_base);
				vp->stream_index = vs->index;
				ret = av_interleaved_write_frame(fc, vp);
			}
			venc = ret != AVERROR_EOF;

			video_times++;
		} else {
			// audio
			printf("audio\n");
			int end_of_stream = av_compare_ts(apts, acc->time_base,
				DURATION, (AVRational){1, 1}) > 0;

			if (end_of_stream) {
				af2 = NULL;
			} else {
				int16_t *data = (int16_t *)af1->data[0];
				for (int i = 0; i < af1->nb_samples; i++) {
					int v = sin(at) * 10000;
					for (int j = 0; j < acc->channels; j++)
						*data++ = v;
					at += atincr;
					atincr += atincr2;
				}
				// err could be here (af1 or af2)?
				af1->pts = apts;
				apts += af1->nb_samples;

				int anbsf = av_rescale_rnd(
					swr_get_delay(aconv, acc->sample_rate) + af1->nb_samples,
					acc->sample_rate, acc->sample_rate, AV_ROUND_UP);
				av_frame_make_writable(af1);
				// av_frame_make_writable(af2);
				// swr_convert_frame(aconv, af1, af2); // this could work...
				swr_convert(aconv, af2->data, anbsf, (const uint8_t **)af1->data,
							af1->nb_samples);
				af2->pts = av_rescale_q(asc, (AVRational){1, acc->sample_rate},
										acc->time_base);
				asc += anbsf;
			}

			avcodec_send_frame(acc, af2);
			int ret;
			while ((ret = avcodec_receive_packet(acc, ap)) >= 0) {
				av_packet_rescale_ts(ap, acc->time_base, as->time_base);
				ap->stream_index = as->index;
				ret = av_interleaved_write_frame(fc, ap);
			}
			aenc = ret != AVERROR_EOF;

			audio_times++;
		}
	}

	av_write_trailer(fc);

	avcodec_free_context(&vcc);
	av_frame_free(&vf);
	av_packet_free(&vp);

	avcodec_free_context(&acc);
	av_frame_free(&af1);
	av_frame_free(&af2);
	av_packet_free(&ap);
	swr_free(&aconv);

	if (!(of->flags & AVFMT_NOFILE))
		avio_closep(&fc->pb);

	avformat_free_context(fc);

	printf("video: %d, audio: %d\n", video_times, audio_times);

	return 0;
}
