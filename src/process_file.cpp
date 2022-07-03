#include "process_file.h"

void get_basename(std::string &base_name, std::string_view file_path, std::string_view dir_sep){
    auto pos = file_path.find_last_of(dir_sep);
    if (pos == std::string::npos){
        base_name = file_path;
    }
    else{
        base_name = file_path.substr(pos + 1);
    }
}

void get_filename(std::string &file_name){
    auto pos = file_name.find('.');
    if (pos != std::string::npos){
        file_name.erase(pos);
    }
}

void mux_video_audio(const char *output_file, const char *orig_file, const char *ascii_file){
    AVFormatContext *orig_format_context = NULL, *ascii_format_context = NULL, *output_format_context = NULL;
    AVPacket packet_orig, packet_ascii;
    int ret, orig_video_stream_index = 0;
    //ascii_video_stream_index is not used for the time being
    //int ascii_video_stream_index = 0;
    if ((ret = avformat_open_input(&orig_format_context, orig_file, NULL, NULL)) < 0){
        std::cout << "Could not open the original video" << std::endl;
    }
    if ((ret = avformat_find_stream_info(orig_format_context, NULL)) < 0){
        std::cout << "Could not find the original stream info" << std::endl;
    }
    if ((ret = avformat_open_input(&ascii_format_context, ascii_file, NULL, NULL)) < 0){
        std::cout << "Could not open the ascii video" << std::endl;
    }
    if ((ret = avformat_find_stream_info(ascii_format_context, NULL)) < 0){
        std::cout << "Could not find the ascii stream info" << std::endl;
    }
    //AVOutputFormat *outFmt = av_guess_format("mp4", NULL, NULL);
    avformat_alloc_output_context2(&output_format_context, av_guess_format("mp4", NULL, NULL), NULL, output_file);
    if (!output_format_context){
        std::cout << "Could not make output context" << std::endl;
    }
    for (int i=0; i<(int)orig_format_context->nb_streams; ++i){
        AVStream *out_stream, *orig_stream = orig_format_context->streams[i];
        AVCodecParameters *orig_codecpar = orig_stream->codecpar;
        out_stream = avformat_new_stream(output_format_context, NULL);
        if (!out_stream){
            std::cout << "Could not make output stream" << std::endl;
        }
        if (orig_codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            orig_video_stream_index = i;
            for (int j=0; j<(int)ascii_format_context->nb_streams; ++j){
                AVStream *ascii_stream = ascii_format_context->streams[j];
                AVCodecParameters *ascii_codecpar = ascii_stream->codecpar;
                if (ascii_codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
                    //ascii_video_stream_index = j;
                    ret = avcodec_parameters_copy(out_stream->codecpar, ascii_codecpar);
                }
            }
        }
        else{
            ret = avcodec_parameters_copy(out_stream->codecpar, orig_codecpar);
        }
        if (ret < 0){
            std::cout << "Could not copy codec parameters" << std::endl;
        }
    }
    av_dump_format(output_format_context, 0, output_file, 1);
    avio_open(&output_format_context->pb, output_file, AVIO_FLAG_WRITE);
    ret = avformat_write_header(output_format_context, NULL);
    if (ret < 0){
        std::cout << "Could not write file header" << std::endl;
    }
    std::cout << "muxing" << std::endl;
    while (true){
        AVStream *orig_stream, *ascii_stream, *out_stream;
        ret = av_read_frame(orig_format_context, &packet_orig);
        if (ret < 0){
            break;
        }
        orig_stream = orig_format_context->streams[packet_orig.stream_index];
        ascii_stream = ascii_format_context->streams[0];
        //ascii_stream = ascii_format_context->streams[packet_ascii.stream_index];
        out_stream = output_format_context->streams[packet_orig.stream_index];
        if (packet_orig.stream_index == orig_video_stream_index){
            // "IF" conditon is taken away for the time being because packet_orig.steam_index is overflowed for some reason
            //if (packet_ascii.stream_index == ascii_video_stream_index){
                av_read_frame(ascii_format_context, &packet_ascii);
                packet_ascii.pts = av_rescale_q_rnd(packet_ascii.pts, ascii_stream->time_base, out_stream->time_base, AVRounding::AV_ROUND_NEAR_INF);
                packet_ascii.dts = av_rescale_q_rnd(packet_ascii.dts, ascii_stream->time_base, out_stream->time_base, AVRounding::AV_ROUND_NEAR_INF);
                packet_ascii.duration = av_rescale_q(packet_ascii.duration, ascii_stream->time_base, out_stream->time_base);
                packet_ascii.pos = -1;
                packet_orig.pos = -1;
                ret = av_interleaved_write_frame(output_format_context, &packet_ascii);
                av_packet_unref(&packet_ascii);
            //}
        }
        else{
            packet_orig.pts = av_rescale_q_rnd(packet_orig.pts, orig_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
            packet_orig.dts = av_rescale_q_rnd(packet_orig.dts, orig_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
            packet_orig.duration = av_rescale_q(packet_orig.duration, orig_stream->time_base, out_stream->time_base);
            packet_orig.pos = -1;
            ret = av_interleaved_write_frame(output_format_context, &packet_orig);
        }
        if (ret < 0){
            std::cout << "Muxing error" << std::endl;
        }
        av_packet_unref(&packet_orig);
    }
    av_write_trailer(output_format_context);
    avformat_close_input(&orig_format_context);
    avformat_close_input(&ascii_format_context);
    avformat_free_context(output_format_context);
}

void reduce_video_size(const char *input_file, const char *output_file, const double &reduce){
    AVFormatContext *inputFormatContext = NULL, *outputFormatContext = NULL;
    const AVCodec *encodec = NULL, *decodec = NULL;
    AVCodecContext *encontext = NULL, *decontext = NULL;
    int ret = 0, video_stream_index = 0;
    ret = avformat_open_input(&inputFormatContext, input_file, NULL, NULL);
    if (ret < 0){
        std::cout << "Cound not open the video" << std::endl;       
    }
    ret = avformat_find_stream_info(inputFormatContext, NULL);
    if (ret < 0){
        std::cout << "Could not find the video stream info" << std::endl;
    }
    const AVOutputFormat *outFmt = av_guess_format("mp4", NULL, NULL);
    avformat_alloc_output_context2(&outputFormatContext, outFmt, NULL, output_file);
    for (int i=0; i<(int)inputFormatContext->nb_streams; ++i){
        AVStream *in_stream = inputFormatContext->streams[i];
        AVCodecParameters *in_par = in_stream->codecpar;
        if (in_par->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            AVRational fps = in_stream->r_frame_rate;
            decodec = avcodec_find_decoder(in_par->codec_id);
            decontext = avcodec_alloc_context3(decodec);
            avcodec_parameters_to_context(decontext, in_par);
            decontext->framerate = fps;
            decontext->time_base = av_inv_q(fps);
            avcodec_open2(decontext, decodec, NULL);
            AVStream *out_stream = avformat_new_stream(outputFormatContext, NULL);
            encodec = avcodec_find_encoder(in_par->codec_id);
            encontext = avcodec_alloc_context3(encodec);
            encontext->codec_id = decontext->codec_id;
            encontext->height = decontext->height;
            encontext->width = decontext->width;
            encontext->sample_aspect_ratio = decontext->sample_aspect_ratio;
            encontext->qmax = 41;
            encontext->qmin = 10;
            encontext->qcompress = 0.6;
            encontext->max_qdiff = 4;
            encontext->bit_rate = decontext->bit_rate;
            encontext->gop_size = decontext->gop_size;
            encontext->pix_fmt = decontext->pix_fmt;
            encontext->bit_rate = round(decontext->bit_rate * reduce);
            encontext->framerate = fps;
            encontext->time_base = av_inv_q(fps);
            avcodec_open2(encontext, encodec, NULL);
            avcodec_parameters_from_context(out_stream->codecpar, encontext);
        }
    }
    av_dump_format(outputFormatContext, 0, output_file, 1);
    avio_open(&outputFormatContext->pb, output_file, AVIO_FLAG_WRITE);
    ret = avformat_write_header(outputFormatContext, NULL);
    if (ret < 0){
        std::cout << "Could not write output header" << std::endl;
    }
    int res = 0;
    AVPacket *in_packet = av_packet_alloc();
    AVPacket *out_packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    while (true){
        AVStream *input_stream = inputFormatContext->streams[video_stream_index];
        AVStream *output_stream = outputFormatContext->streams[video_stream_index];
        ret = av_read_frame(inputFormatContext, in_packet);
        int64_t pts = av_rescale_q_rnd(in_packet->pts, input_stream->time_base, output_stream->time_base, AV_ROUND_NEAR_INF);
        int64_t dts = av_rescale_q_rnd(in_packet->dts, input_stream->time_base, output_stream->time_base, AV_ROUND_NEAR_INF);
        int64_t duration = av_rescale_q(in_packet->duration, input_stream->time_base, output_stream->time_base);
        if (ret < 0){
            break;
        }
        if (inputFormatContext->streams[in_packet->stream_index]->codecpar->codec_type == video_stream_index){
            res = avcodec_send_packet(decontext, in_packet);
            while (res >= 0){
                res = avcodec_receive_frame(decontext, frame);
                if (res == AVERROR(EAGAIN) || res == AVERROR_EOF){
                    break;
                }
                if (res >= 0){
                    res = avcodec_send_frame(encontext, frame);
                    while (res >= 0){
                        res = avcodec_receive_packet(encontext, out_packet);
                        if (res == AVERROR(EAGAIN) || res == AVERROR_EOF){
                            break;
                        }
                        out_packet->pts = pts;
                        out_packet->dts = dts;
                        out_packet->duration = duration;
                        res = av_interleaved_write_frame(outputFormatContext, out_packet);
                    }
                    av_frame_unref(frame);
                    av_packet_unref(out_packet);
                }
            }
        }
        av_packet_unref(in_packet);
    }
    av_frame_free(&frame);
    av_packet_free(&in_packet);
    av_packet_free(&out_packet);
    av_write_trailer(outputFormatContext);
    avformat_close_input(&inputFormatContext);
    avio_close(outputFormatContext->pb);
    avformat_free_context(inputFormatContext);
    avformat_free_context(outputFormatContext);
    avcodec_free_context(&decontext);
    avcodec_free_context(&encontext);

}