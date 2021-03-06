#include "libuvc/libuvc.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include <stdio.h>
#include <unistd.h>

using namespace cv;

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame, void *ptr)
{
    uvc_frame_t *bgr;
    uvc_error_t ret;
    enum uvc_frame_format *frame_format = (enum uvc_frame_format *)ptr;
    /* FILE *fp;
   * static int jpeg_count = 0;
   * static const char *H264_FILE = "iOSDevLog.h264";
   * static const char *MJPEG_FILE = ".jpeg";
   * char filename[16]; */

    /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
    bgr = uvc_allocate_frame(frame->width * frame->height * 3);
    if (!bgr)
    {
        printf("unable to allocate bgr frame!\n");
        return;
    }

    switch (frame->frame_format)
    {
    case UVC_FRAME_FORMAT_H264:
        /* use `ffplay H264_FILE` to play */
        /* fp = fopen(H264_FILE, "a");
     * fwrite(frame->data, 1, frame->data_bytes, fp);
     * fclose(fp); */
        break;
    case UVC_COLOR_FORMAT_MJPEG:
        /* sprintf(filename, "%d%s", jpeg_count++, MJPEG_FILE);
     * fp = fopen(filename, "w");
     * fwrite(frame->data, 1, frame->data_bytes, fp);
     * fclose(fp); */
        break;
    case UVC_COLOR_FORMAT_YUYV:
        /* Do the BGR conversion */
        ret = uvc_any2bgr(frame, bgr);
        if (ret)
        {
            uvc_perror(ret, "uvc_any2bgr");
            uvc_free_frame(bgr);
            return;
        }
        break;
    default:
        break;
    }

    //   cvSetData(cvImg, bgr->data, bgr->width * 3);
    const std::vector<int> sz = {(int)bgr->height, (int)bgr->width};
    printf("Width: %d, Height: %d\n", bgr->width, bgr->height);
    cv::Mat img = cv::Mat(sz, CV_8UC3, bgr->data);

    if (!img.empty())
    {
        cv::namedWindow("Image", WINDOW_NORMAL);
        cv::imshow("Image", img);
        cv::waitKey(20);
    }

    uvc_free_frame(bgr);
}

int main(int argc, char **argv)
{
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;
    uvc_error_t res;

    /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
    res = uvc_init(&ctx, NULL);

    if (res < 0)
    {
        uvc_perror(res, "uvc_init");
        return res;
    }

    puts("UVC initialized");

    /* Locates the first attached UVC device, stores in dev */
    res = uvc_find_device(
        ctx, &dev,
        0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */

    if (res < 0)
    {
        uvc_perror(res, "uvc_find_device"); /* no devices found */
    }
    else
    {
        puts("Device found");

        /* Try to open the device: requires exclusive access */
        res = uvc_open(dev, &devh);

        if (res < 0)
        {
            uvc_perror(res, "uvc_open"); /* unable to open device */
        }
        else
        {
            puts("Device opened");

            /* Print out a message containing all the information that libuvc
       * knows about the device */
            uvc_print_diag(devh, stderr);

            const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh);
            const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
            enum uvc_frame_format frame_format;
            int width = 640;
            int height = 481;
            int fps = 30;

            switch (format_desc->bDescriptorSubtype)
            {
            case UVC_VS_FORMAT_MJPEG:
                frame_format = UVC_COLOR_FORMAT_MJPEG;
                break;
            case UVC_VS_FORMAT_FRAME_BASED:
                frame_format = UVC_FRAME_FORMAT_H264;
                break;
            default:
                frame_format = UVC_FRAME_FORMAT_YUYV;
                break;
            }

            if (frame_desc)
            {
                width = frame_desc->wWidth;
                height = frame_desc->wHeight;
                fps = 10000000 / frame_desc->dwDefaultFrameInterval;
            }

            printf("\nFirst format: (%4s) %dx%d %dfps\n", format_desc->fourccFormat, width, height, fps);

            /* Try to negotiate first stream profile */
            res = uvc_get_stream_ctrl_format_size(
                devh, &ctrl, /* result stored in ctrl */
                frame_format,
                width, height, fps /* width, height, fps */
            );

            /* Print out the result */
            uvc_print_stream_ctrl(&ctrl, stderr);

            if (res < 0)
            {
                uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
            }
            else
            {
                /* Start the video stream. The library will call user function cb:
         *   cb(frame, (void *) 12345)
         */
                res = uvc_start_streaming(devh, &ctrl, cb, (void *)12345, 0);

                if (res < 0)
                {
                    uvc_perror(res, "start_streaming"); /* unable to start stream */
                }
                else
                {
                    puts("Streaming...");

                    uvc_set_ae_mode(devh, 1); /* e.g., turn on auto exposure */

                    sleep(10); /* stream for 10 seconds */

                    /* End the stream. Blocks until last callback is serviced */
                    uvc_stop_streaming(devh);
                    puts("Done streaming.");
                }
            }

            /* Release our handle on the device */
            uvc_close(devh);
            puts("Device closed");
        }

        /* Release the device descriptor */
        uvc_unref_device(dev);
    }

    /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
    uvc_exit(ctx);
    puts("UVC exited");

    return 0;
}
