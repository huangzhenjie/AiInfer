#include <opencv2/opencv.hpp>
#include "common/arg_parsing.hpp"
#include "application/tensorrt_demo/trt_yolov8_cuda/yolov8_face_detect.hpp"
#include "application/tensorrt_demo/trt_yolov8_cpp/yolov8_face_detect.hpp"
#define DEVICE_NUM 1

void trt_cuda_inference(ai::arg_parsing::Settings *s)
{
    ai::utils::Timer timer; // 创建
    tensorrt_infer::yolov8_cuda::YOLOv8Detect yolov8_obj;
    yolov8_obj.initParameters(s->model_path, s->score_thr);

    // 判断图片路径是否存在
    if (!ai::utils::file_exist(s->image_path))
    {
        INFO("Error: image path is not exist!!!");
        exit(0);
    }

    // 加载要推理的数据
    std::vector<cv::Mat> images;
    for (int i = 0; i < s->batch_size; i++)
        images.push_back(cv::imread(s->image_path));
    std::vector<ai::cvUtil::Image> yoloimages(images.size());
    std::transform(images.begin(), images.end(), yoloimages.begin(), ai::cvUtil::cvimg_trans_func);

    // 模型预热，如果要单张推理，请调用yolov8_obj.forward
    for (int i = 0; i < s->number_of_warmup_runs; ++i)
        auto warmup_batched_result = yolov8_obj.forwards(yoloimages);

    ai::cvUtil::BatchBoxArray batched_result;
    // 模型推理
    timer.start();
    for (int i = 0; i < s->loop_count; ++i)
        batched_result = yolov8_obj.forwards(yoloimages);
    timer.stop(ai::utils::path_join("Batch=%d, iters=%d,run infer mean time:", s->batch_size, s->loop_count).c_str(), s->loop_count);

    if (!s->output_dir.empty())
    {
        ai::utils::rmtree(s->output_dir);
        yolov8_obj.draw_batch_rectangle(images, batched_result, s->output_dir);
    }
}

void trt_cpp_inference(ai::arg_parsing::Settings *s)
{
    ai::utils::Timer timer; // 创建
    tensorrt_infer::yolov8_cpp::YOLOv8Detect yolov8_obj;
    yolov8_obj.initParameters(s->model_path, s->score_thr);

    // 判断图片路径是否存在
    if (!ai::utils::file_exist(s->image_path))
    {
        INFO("Error: image path is not exist!!!");
        exit(0);
    }

    // 加载要推理的数据
    std::vector<cv::Mat> images;
    for (int i = 0; i < s->batch_size; i++)
        images.push_back(cv::imread(s->image_path));
    std::vector<ai::cvUtil::Image> yoloimages(images.size());
    std::transform(images.begin(), images.end(), yoloimages.begin(), ai::cvUtil::cvimg_trans_func);

    // 模型预热，如果要单张推理，请调用yolov8_obj.forward
    for (int i = 0; i < s->number_of_warmup_runs; ++i)
        auto warmup_batched_result = yolov8_obj.forwards(yoloimages);

    ai::cvUtil::BatchBoxArray batched_result;
    // 模型推理
    timer.start();
    for (int i = 0; i < s->loop_count; ++i)
        batched_result = yolov8_obj.forwards(yoloimages);
    timer.stop(ai::utils::path_join("Batch=%d, iters=%d,run infer mean time:", s->batch_size, s->loop_count).c_str(), s->loop_count);

    if (!s->output_dir.empty())
    {
        ai::utils::rmtree(s->output_dir);
        yolov8_obj.draw_batch_rectangle(images, batched_result, s->output_dir);
    }
}

int main(int argc, char *argv[])
{
    ai::arg_parsing::Settings s;
    if (parseArgs(argc, argv, &s) == RETURN_FAIL)
    {
        INFO("Failed to parse the args\n");
        return RETURN_FAIL;
    }
    ai::arg_parsing::printArgs(&s);
    if (!strcmp(s.device_type.c_str(), "gpu"))
    {
        CHECK(cudaSetDevice(s.device_id)); // 设置你用哪块gpu
        if (!strcmp(s.backend.c_str(), "tensorrt"))
            trt_cuda_inference(&s);
    }
    else if (!strcmp(s.device_type.c_str(), "cpu"))
    {
        if (!strcmp(s.backend.c_str(), "tensorrt"))
            trt_cpp_inference(&s);
    }
    else
    {
        INFO("Device Type can be [gpu/cpu], can't be %s", s.device_type);
        return RETURN_FAIL;
    }

    return RETURN_SUCCESS;
}