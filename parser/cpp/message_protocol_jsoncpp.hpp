#ifndef _MESSAGEPROTOCOL_JSONCPP_HPP_
#define _MESSAGEPROTOCOL_JSONCPP_HPP_

#include <libyuv.h>
#include "json.h"
#include "base64_x.hpp"
#include "plugin_interface.hpp"
#include "parser_exception.hpp"
#include "../../common/include/gungnir/hat_info.hpp"
#include "../../common/include/mjollner/box_info.hpp"
#include "../../common/include/valklyrs/result_info.hpp"
#include "../../common/include/valklyrs/vp_info.hpp"
#include "../../common/include/longinus/face_info.hpp"
#include "../../common/include/irisviel/search_result.hpp"
#include "../../common/include/irisviel/record.hpp"
#include "../../common/include/Primitives/tensor.hpp"
#include <string>
#include <memory>
#include <unordered_map>

namespace glasssix
{
    namespace exposing
    {
        namespace nessus
        {
            //supported image format
            enum class PROTOCOL_IMAGE_FORMAT
            {
                PROTOCOL_IMAGE_BGR_NCHW = 0,
                PROTOCOL_IMAGE_BGR_NHWC = 1,
                PROTOCOL_IMAGE_NV21 = 2
            };

            inline void convert_to_bgr(memory::tensor<std::uint8_t> &src, PROTOCOL_IMAGE_FORMAT src_format, memory::tensor<std::uint8_t> &dst, int width, int height)
            {
                switch (src_format)
                {
                case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NCHW:
                {
                    if (width * height * 3 != src.count())
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NCHW, width * height * 3 != src.count()");
                    dst = src;
                    break;
                }
                case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC:
                {
                    int step = 0;
                    if (src.count() != width * height * 3)
                    {
                        step = ((width * 3 + 3) >> 2) << 2;
                        if (src.count() != step * height)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NHWC, src.count() != width * height * 3 || src.count() != (((width * 3 + 3) >> 2) << 2) * height");
                    }
                    else
                        step = width * 3;

                    if (step == width * 3)
                        dst = src;
                    else if (step > width * 3)
                    {
                        dst = memory::tensor<std::uint8_t>(std::vector<int>{1, 3, height, width}, src.device(), src.order(), src.allocator());
                        std::uint8_t *dst_ptr = dst.mutable_cpu_data();
                        const std::uint8_t *src_ptr = src.cpu_data();
                        for (size_t i = 0; i < height; i++)
                            std::copy(src_ptr + i * step, src_ptr + i * step + width * 3, dst_ptr);
                    }
                    else
                    {
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "step < width * 3");
                    }

                    break;
                }
                case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_NV21:
                {
                    if (src.count() != (width * height * 3 >> 1))
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "convert_to_bgr: src.count() != (width * height * 3 >> 1)");

                    dst = memory::tensor<std::uint8_t>(std::vector<int>{1, height, width, 3}, -1, memory::NHWC /*, & memory::pool_allocator_default<std::uint8_t>::get()*/);
                    int aligned_src_width = (width + 1) & ~1;
                    const uint8_t *y = src.cpu_data();
                    const uint8_t *uv = src.cpu_data() + aligned_src_width * height;
                    if (libyuv::NV21ToRGB24(y, width, uv, aligned_src_width, dst.mutable_cpu_data(), width * 3, width, height))
                        throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, "NV21ToRGB24 failed.");

                    break;
                }
                default:
                    throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Unsupported image format.");
                    break;
                }
            }

            inline memory::tensor<std::uint8_t> decode_and_convert(param_span<std::uint8_t> src, bool is_base64, PROTOCOL_IMAGE_FORMAT format, int width, int height)
            {
                if (height <= 0 || width <= 0)
                    throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: height <= 0 || width <= 0");

                if (src.size() <= 0)
                    throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: src.size() <= 0");

                memory::orderType order;
                switch (format)
                {
                case glasssix::exposing::nessus::PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NCHW:
                    order = memory::NCHW;
                    break;
                case glasssix::exposing::nessus::PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC:
                    order = memory::NHWC;
                    break;
                default:
                    order = memory::NHWC;
                    break;
                }

                memory::tensor<std::uint8_t> temp;
                if (is_base64)
                {
                    int current_image_str_len = TB64DECLEN(src.size());

                    temp = memory::tensor<std::uint8_t>(current_image_str_len, -1, order /*, &memory::pool_allocator_default<std::uint8_t>::get()*/);
                    tb64xdec(reinterpret_cast<const std::uint8_t *>(src.data()), src.size(), temp.mutable_cpu_data());
                }
                else
                {
                    temp = memory::tensor<std::uint8_t>(src.size(), -1, order /*, &memory::pool_allocator_default<std::uint8_t>::get()*/);
                    std::copy(src.begin(), src.end(), temp.mutable_cpu_data());
                }

                memory::tensor<std::uint8_t> dst;

                convert_to_bgr(temp, format, dst, width, height);

                return dst;
            }

            inline Json::Value Longinus_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    //float nms = static_cast<float>(root["nms"].asDouble());
                    float nms = 0.4f;
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"nms", box(nms)},
                         {u8"models_directory", box(std::string_view(models_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"longinus.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Longinus_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"longinus.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            inline Json::Value Longinus_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    int min_size = root["min_size"].asInt();
                    float threshold = root["threshold"].asFloat();
                    bool do_attributing = root["do_attributing"].asBool();

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"min_size", box(min_size)},
                         {u8"threshold", box(threshold)},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"do_attributing", box(do_attributing)},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"longinus.detect", param).as<param_vector<longinus::face_info>>();

                    Json::Value jarray_rect = Json::Value(Json::arrayValue);

                    for (auto obj : result)
                    {
                        Json::Value jobj_face;
                        jobj_face["x"] = Json::Int(obj.x());
                        jobj_face["y"] = Json::Int(obj.y());
                        jobj_face["width"] = Json::Int(obj.width());
                        jobj_face["height"] = Json::Int(obj.height());
                        jobj_face["confidence"] = Json::Value(obj.confidence());

                        if (do_attributing)
                        {
                            jobj_face["attributes"]["yaw"] = Json::Value(obj.yaw());
                            jobj_face["attributes"]["pitch"] = Json::Value(obj.pitch());
                            jobj_face["attributes"]["roll"] = Json::Value(obj.roll());
                            jobj_face["attributes"]["glass_index"] = Json::Int(obj.glass_index());
                            jobj_face["attributes"]["mask_index"] = Json::Int(obj.mask_index());
                        }
                        else
                            jobj_face["attributes"] = Json::Value(Json::nullValue);

                        Json::Value jarray_landmark;

                        for (const auto &pt : obj.pts())
                        {

                            Json::Value jobj_point;
                            jobj_point["x"] = Json::Int((int)pt.key());
                            jobj_point["y"] = Json::Int((int)pt.value());
                            jarray_landmark.append(jobj_point);
                        }
                        jobj_face["landmark"] = jarray_landmark;
                        jarray_rect.append(jobj_face);
                    }

                    value["facerectwithfaceinfo_list"] = jarray_rect;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Longinus_trace_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto face = make_exported_interface<longinus::face_info>();
                    face.set_x(root["face"]["x"].asFloat());
                    face.set_y(root["face"]["y"].asFloat());
                    face.set_width(root["face"]["width"].asFloat());
                    face.set_height(root["face"]["height"].asFloat());

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"face", face},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});
                    auto result = plugin.execute(u8"longinus.trace", param).as<longinus::face_info>();

                    Json::Value jobj_face;
                    if (result.confidence() > 0.1f)
                    {
                        value["trace_success"] = Json::Value(true);
                        jobj_face["x"] = Json::Int(result.x());
                        jobj_face["y"] = Json::Int(result.y());
                        jobj_face["width"] = Json::Int(result.width());
                        jobj_face["height"] = Json::Int(result.height());
                        jobj_face["confidence"] = Json::Value(result.confidence());

                        jobj_face["attributes"]["glass_index"] = Json::Int(result.glass_index());
                        jobj_face["attributes"]["mask_index"] = Json::Int(result.mask_index());
                        jobj_face["attributes"]["yaw"] = Json::Value(result.yaw());
                        jobj_face["attributes"]["pitch"] = Json::Value(result.pitch());
                        jobj_face["attributes"]["roll"] = Json::Value(result.roll());

                        Json::Value jarray_landmark;

                        for (const auto &pt : result.pts())
                        {

                            Json::Value jobj_point;
                            jobj_point["x"] = Json::Int((int)pt.key());
                            jobj_point["y"] = Json::Int((int)pt.value());
                            jarray_landmark.append(jobj_point);
                        }
                        jobj_face["landmark"] = jarray_landmark;
                    }
                    else
                    {
                        value["trace_success"] = Json::Value(false);
                    }

                    value["facerectwithfaceinfo"] = jobj_face;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            constexpr int longinus_align_aligned_base64_buffer_len = TB64ENCLEN(3 * 128 * 128);
            inline Json::Value Longinus_center_scale_alignFace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    bool save2extrenal = false;
                    if (external.size() >= root["image_infos"].size() * 128 * 128 * 3)
                        save2extrenal = true;

                    value["aligned_images"] = Json::Value(Json::arrayValue);

                    int read_byte_size = 0;
                    for (size_t i = 0; i < root["image_infos"].size(); i++)
                    {
                        int format = root["image_infos"][Json::Int(i)]["format"].asInt();
                        int height = root["image_infos"][Json::Int(i)]["height"].asInt();
                        int width = root["image_infos"][Json::Int(i)]["width"].asInt();
                        float scale = root["image_infos"][Json::Int(i)]["scale"].asFloat();
                        int image_byte_size = root["image_infos"][Json::Int(i)]["image_byte_size"].asInt();

                        param_span<std::uint8_t> data_i(data.data() + read_byte_size, image_byte_size);
                        read_byte_size += image_byte_size;
                        auto frame = decode_and_convert(data_i, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                        param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

                        auto param = make_param_hash_map<param_string, unknown_object>(
                            { {u8"image", box(image_span)},
                             {u8"height", box(height)},
                             {u8"width", box(width)},
                             {u8"scale", box(scale)},
                             {u8"order", box(static_cast<int>(frame.order()))},
                             {u8"object_id", box(instance)} });

                        auto result = plugin.execute(u8"longinus.center_scale_alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();
                        if (save2extrenal)
                        {
                            std::copy(begin(result[0]), end(result[0]), external.begin() + i * 128 * 128 * 3);
                        }
                        else
                        {
                            std::vector<std::uint8_t> temp(longinus_align_aligned_base64_buffer_len, 0);
                            std::uint8_t* ptr = temp.data();
                            std::vector<std::uint8_t> buffer(begin(result[0]), end(result[0]));

                            tb64xenc(buffer.data(), buffer.size(), ptr);

                            value["aligned_images"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + longinus_align_aligned_base64_buffer_len));
                        }
                    }
                    
                    value["format"] = Json::Value(0);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Gungnir_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"models_directory", box(std::string_view(models_directory))}});
                    instance = unbox<guid>(plugin.execute(u8"gungnir.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Gungnir_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"gungnir.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            inline Json::Value Gungnir_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"gungnir.detect", param).as<param_vector<gungnir::hat_info>>();

                    Json::Value jarray_rect = Json::Value(Json::arrayValue);

                    for (auto obj : result)
                    {
                        Json::Value jobj_face;
                        jobj_face["x"] = Json::Int(obj.x());
                        jobj_face["y"] = Json::Int(obj.y());
                        jobj_face["width"] = Json::Int(obj.width());
                        jobj_face["height"] = Json::Int(obj.height());
                        jobj_face["prob"] = Json::Value(obj.prob());
                        jobj_face["label"] = Json::Int(obj.label());

                        jarray_rect.append(jobj_face);
                    }

                    value["hatrectwithhatinfo_list"] = jarray_rect;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Mjollner_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"models_directory", box(std::string_view(models_directory))}});
                    instance = unbox<guid>(plugin.execute(u8"mjollner.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Mjollner_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"mjollner.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            inline Json::Value Mjollner_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();

                    Json::Value roi = root.get("roi", Json::Value());
                    bool flag = roi.empty();
                    int x = flag ? 0 : roi["x"].asInt();
                    int y = flag ? 0 : roi["y"].asInt();
                    int roi_width = flag ? width : roi["width"].asInt();
                    int roi_height = flag ? height : roi["height"].asInt();
                    
                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {
                            {u8"image", box(image_span)},
                            {u8"height", box(height)},
                            {u8"width", box(width)},
                            {u8"order", box(static_cast<int>(frame.order()))},
                            {u8"object_id", box(instance)},
                            {u8"x", box(x)},
                            {u8"y", box(y)},
                            {u8"roi_width", box(roi_width)},
                            {u8"roi_height", box(roi_height)},
                        });

                    auto result = plugin.execute(u8"mjollner.detect", param).as<param_vector<mjollner::box_info>>();
                    Json::Value jarray_boxes = Json::Value(Json::arrayValue);
                    for (auto box : result)
                    {
                        Json::Value jobj_box;
                        Json::Value jarray_points = Json::Value(Json::arrayValue);
                        auto location = box.location();
                        for (size_t i = 0; i < 4; i++)
                        {
                            Json::Value point;
                            point["x"] = Json::Value(location[i * 2]);
                            point["y"] = Json::Value(location[i * 2 + 1]);

                            jarray_points.append(point);
                        }
                        jobj_box["location"] = jarray_points;
                        jobj_box["strinfo"] = Json::Value(exposing::to_narrow_string(box.strinfo()));

                        jarray_boxes.append(jobj_box);
                    }

                    value["strinfo_list"] = jarray_boxes;

                    // value["strinfo_list"] = "hello world";
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Valklyrs_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"models_directory", box(std::string_view(models_directory))}});
                    instance = unbox<guid>(plugin.execute(u8"valklyrs.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Valklyrs_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"valklyrs.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            inline Json::Value Valklyrs_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"valklyrs.detect", param).as<param_vector<valklyrs::result_info>>();

                    Json::Value jbox;
                    Json::Value jvehicle_list = Json::Value(Json::arrayValue);
                    Json::Value jperson_list = Json::Value(Json::arrayValue);

                    auto vehicle_list = result[0].vehicle_list();
                    auto person_list = result[0].person_list();
                    for (auto &&vp_info : vehicle_list)
                    {
                        Json::Value vp_obj;
                        Json::Value coordinate_obj;
                        Json::Value attribute_obj;
                        auto coordinates = vp_info.coordinates();
                        auto attributes = vp_info.attributes();
                        coordinate_obj["x"] = Json::Int(coordinates[0]);
                        coordinate_obj["y"] = Json::Int(coordinates[1]);
                        coordinate_obj["width"] = Json::Int(coordinates[2]);
                        coordinate_obj["height"] = Json::Int(coordinates[3]);
                        attribute_obj["color"] = Json::Value(exposing::to_narrow_string(attributes[0]));
                        attribute_obj["orientation"] = Json::Value(exposing::to_narrow_string(attributes[1]));
                        attribute_obj["car_type"] = Json::Value(exposing::to_narrow_string(attributes[2]));
                        vp_obj["coordinates"] = coordinate_obj;
                        vp_obj["attributes"] = attribute_obj;
                        jvehicle_list.append(vp_obj);
                    }

                    for (auto &&vp_info : person_list)
                    {
                        Json::Value vp_obj;
                        Json::Value coordinate_obj;
                        Json::Value attribute_obj;
                        auto coordinates = vp_info.coordinates();
                        auto attributes = vp_info.attributes();
                        coordinate_obj["x"] = Json::Int(coordinates[0]);
                        coordinate_obj["y"] = Json::Int(coordinates[1]);
                        coordinate_obj["width"] = Json::Int(coordinates[2]);
                        coordinate_obj["height"] = Json::Int(coordinates[3]);
                        attribute_obj["gender"] = Json::Value(exposing::to_narrow_string(attributes[0]));
                        attribute_obj["age"] = Json::Value(exposing::to_narrow_string(attributes[1]));
                        attribute_obj["ori"] = Json::Value(exposing::to_narrow_string(attributes[2]));
                        attribute_obj["hat"] = Json::Value(exposing::to_narrow_string(attributes[3]));
                        attribute_obj["glass"] = Json::Value(exposing::to_narrow_string(attributes[4]));
                        attribute_obj["handbag"] = Json::Value(exposing::to_narrow_string(attributes[5]));
                        attribute_obj["shoulderbag"] = Json::Value(exposing::to_narrow_string(attributes[6]));
                        attribute_obj["backpack"] = Json::Value(exposing::to_narrow_string(attributes[7]));
                        attribute_obj["sleeve"] = Json::Value(exposing::to_narrow_string(attributes[8]));
                        attribute_obj["texture"] = Json::Value(exposing::to_narrow_string(attributes[9]));
                        attribute_obj["lower_type"] = Json::Value(exposing::to_narrow_string(attributes[10]));
                        vp_obj["coordinates"] = coordinate_obj;
                        vp_obj["attributes"] = attribute_obj;
                        jperson_list.append(vp_obj);
                    }

                    jbox["vehicle_list"] = jvehicle_list;
                    jbox["person_list"] = jperson_list;
                    value["results"] = jbox;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Romancia_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"models_directory", box(std::string_view(models_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"romancia.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Romancia_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"romancia.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            constexpr int romancia_align_aligned_base64_buffer_len = TB64ENCLEN(3 * 128 * 128);
            inline Json::Value Romancia_alignFace_128_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto jarray_rect = root["facerectwithfaceinfo_list"];
                    auto faces = exposing::make_param_vector<longinus::face_info>();
                    for (auto i : jarray_rect)
                    {
                        auto face = exposing::make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_height(i["height"].asFloat());
                        face.set_width(i["width"].asFloat());

                        auto landmark_list = i["landmark"];
                        if (landmark_list.size() != 5)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

                        auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
                        for (auto j : landmark_list)
                        {
                            auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
                            landmark.push_back(pair);
                        }
                        face.set_pts(landmark);

                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"romancia.alignFace128", param).as<param_vector<param_vector<std::uint8_t>>>();

                    value["aligned_images"] = Json::Value(Json::arrayValue);
                    std::vector<std::uint8_t> temp(romancia_align_aligned_base64_buffer_len, 0);
                    std::uint8_t *ptr = temp.data();
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        std::vector<std::uint8_t> buffer(begin(result[i]), end(result[i]));

                        tb64xenc(buffer.data(), buffer.size(), ptr);

                        value["aligned_images"].append(Json::Value(reinterpret_cast<char *>(ptr), reinterpret_cast<char *>(ptr) + romancia_align_aligned_base64_buffer_len));
                    }
                    value["format"] = Json::Value(0);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Romancia_alignFace_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto jarray_rect = root["facerectwithfaceinfo_list"];
                    auto faces = exposing::make_param_vector<longinus::face_info>();
                    for (auto i : jarray_rect)
                    {
                        auto face = exposing::make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_height(i["height"].asFloat());
                        face.set_width(i["width"].asFloat());

                        auto landmark_list = i["landmark"];
                        if (landmark_list.size() != 5)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

                        auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
                        for (auto j : landmark_list)
                        {
                            auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
                            landmark.push_back(pair);
                        }
                        face.set_pts(landmark);

                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"romancia.alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();

                    value["aligned_images"] = Json::Value(Json::arrayValue);
                    std::vector<std::uint8_t> temp(romancia_align_aligned_base64_buffer_len, 0);
                    std::uint8_t *ptr = temp.data();
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        std::vector<std::uint8_t> buffer(begin(result[i]), end(result[i]));

                        tb64xenc(buffer.data(), buffer.size(), ptr);

                        value["aligned_images"].append(Json::Value(reinterpret_cast<char *>(ptr), reinterpret_cast<char *>(ptr) + romancia_align_aligned_base64_buffer_len));
                    }
                    value["format"] = Json::Value(0);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Romancia_blur_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    Json::Value facerect_list = root["facerect_list"];
                    auto faces = make_param_vector<longinus::face_info>();
                    for (auto &i : facerect_list)
                    {
                        auto face = make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_width(i["width"].asFloat());
                        face.set_height(i["height"].asFloat());
                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"romancia.blur_detect", param).as<param_vector<double>>();

                    Json::Value clarity = Json::Value(Json::arrayValue);
                    for (auto i : result)
                        clarity.append(i);
                    value["clarity"] = Json::Value(clarity);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Romancia_mask_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto jarray_rect = root["facerectwithfaceinfo_list"];
                    auto faces = exposing::make_param_vector<longinus::face_info>();
                    for (auto &i : jarray_rect)
                    {
                        auto face = exposing::make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_height(i["height"].asFloat());
                        face.set_width(i["width"].asFloat());

                        auto landmark_list = i["landmark"];
                        if (landmark_list.size() != 5)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

                        auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
                        for (auto &j : landmark_list)
                        {
                            auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
                            landmark.push_back(pair);
                        }
                        face.set_pts(landmark);

                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    Json::Value mask_value = Json::Value(Json::arrayValue);
                    auto result = plugin.execute(u8"romancia.mask_detect", param).as<param_vector<double>>();
                    for (auto i : result)
                        mask_value.append(i);

                    value["mask_value"] = Json::Value(mask_value);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Romancia_antispoofing_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();

                    Json::Value facerect_list = root["facerect_list"];
                    auto faces = make_param_vector<longinus::face_info>();
                    for (auto &i : facerect_list)
                    {
                        auto face = make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_width(i["width"].asFloat());
                        face.set_height(i["height"].asFloat());
                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"romancia.antispoofing", param).as<param_vector<bool>>();
                    Json::Value is_alive = Json::Value(Json::arrayValue);
                    for (auto i : result)
                        is_alive.append(i);

                    value["is_alive"] = Json::Value(is_alive);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Gaius_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    bool use_int8 = root["use_int8"].asBool();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"use_int8", box(use_int8 ? 1 : 0)},
                         {u8"models_directory", box(std::string_view(models_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"gaius.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Gaius_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"gaius.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            constexpr int gaius_forward_aligned_buffer_len = 3 * 128 * 128;
            inline Json::Value Gaius_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    bool has_mask = root["has_mask"].asBool();
                    if (format < 0 || format > 1)
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

                    auto aligned_face_array = root["aligned_images"];
                    std::vector<uint8_t> aligned_faces_vec;
                    int num = 0;
                    std::vector<std::uint8_t> temp(gaius_forward_aligned_buffer_len, 0);
                    std::uint8_t *ptr = temp.data();
                    for (auto i : aligned_face_array)
                    {
                        std::string aligned_face_base64_str = i.asString();
                        if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

                        size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t *>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
                        if (aligned_face_decode_len != gaius_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

                        aligned_faces_vec.insert(aligned_faces_vec.end(), ptr, ptr + gaius_forward_aligned_buffer_len);
                        num++;
                    }

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
                         {u8"num", box(num)},
                         {u8"order", box(format)},
                         {u8"has_mask", box(has_mask ? 1 : 0)},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"gaius.forward", param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features = Json::Value(Json::arrayValue);
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < result[i].size(); j++)
                            jarray_feature["feature"].append(result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Gaius_make_mask_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                std::uint8_t mask[64 * 128] = {0};
                try
                {
                    int format = root["format"].asInt();
                    if (format < 0 || format > 1)
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

                    auto aligned_face_array = root["aligned_images"];
                    std::vector<uint8_t> aligned_faces_vec;
                    int num = 0;
                    std::vector<std::uint8_t> temp(gaius_forward_aligned_buffer_len, 0);
                    std::uint8_t *ptr = temp.data();
                    for (auto i : aligned_face_array)
                    {
                        std::string aligned_face_base64_str = i.asString();
                        if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

                        size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t *>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
                        if (aligned_face_decode_len != gaius_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

                        if (format == 0)
                        {
                            for (size_t j = 0; j < 3; j++)
                            {
                                std::copy(mask, mask + 64 * 128, ptr + j * 128 * 128 + 64 * 128);
                            }
                        }
                        else
                        {
                            for (size_t j = 0; j < 3; j++)
                            {
                                std::copy(mask, mask + 64 * 128, ptr + 3 * 64 * 128 + j * 64 * 128);
                            }
                        }

                        aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
                        num++;
                    }

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
                         {u8"num", box(num)},
                         {u8"order", box(format)},
                         {u8"has_mask", box(1)},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"gaius.forward", param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features;
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < result[i].size(); j++)
                            jarray_feature["feature"].append(result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Cassius_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    int model_type = root["model_type"].asInt();
                    bool use_int8 = root["use_int8"].asBool();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"model_type", box(model_type)},
                         {u8"use_int8", box(use_int8 ? 1 : 0)},
                         {u8"models_directory", box(std::string_view(models_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"cassius.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Cassius_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"cassius.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            constexpr int cassius_forward_aligned_buffer_len = 3 * 128 * 128;
            inline Json::Value Cassius_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int format = root["format"].asInt();
                    if (format < 0 || format > 1)
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

                    int num = 0;
                    exposing::param_hash_map<param_string, unknown_object> param;
                    param_vector<param_vector<float>> result;
                    if (data.size() && (data.size() % cassius_forward_aligned_buffer_len == 0))
                    {
                        num = data.size() / cassius_forward_aligned_buffer_len;
                        param = make_param_hash_map<param_string, unknown_object>(
                            { {u8"aligned_faces", box(data)},
                             {u8"num", box(num)},
                             {u8"order", box(format)},
                             {u8"object_id", box(instance)} });

                        result = plugin.execute(u8"cassius.forward", param).as<param_vector<param_vector<float>>>();
                    }
                    else
                    {
                        auto aligned_face_array = root["aligned_images"];
                        std::vector<uint8_t> aligned_faces_vec;
                        std::vector<uint8_t> temp(cassius_forward_aligned_buffer_len, 0);
                        std::uint8_t* ptr = temp.data();
                        for (auto i : aligned_face_array)
                        {
                            std::string aligned_face_base64_str = i.asString();
                            if (aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len))
                                throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len)");

                            size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
                            if (aligned_face_decode_len != cassius_forward_aligned_buffer_len)
                                throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != cassius_forward_aligned_buffer_len");

                            aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
                            num++;
                        }
                        
                        param = make_param_hash_map<param_string, unknown_object>(
                            { {u8"aligned_faces", box(exposing::param_span<std::uint8_t>{ aligned_faces_vec.data(), aligned_faces_vec.size() })},
                             {u8"num", box(num)},
                             {u8"order", box(format)},
                             {u8"object_id", box(instance)} });
                        result = plugin.execute(u8"cassius.forward", param).as<param_vector<param_vector<float>>>();
                    }

                    bool save2external = false;
                    if (external.size() >= num * 512 * sizeof(float))
                        save2external = true;

                    Json::Value jobj_features = Json::Value(Json::arrayValue);
                    if (save2external)
                    {
                        for (size_t i = 0; i < result.size(); i++)
                            std::copy(begin(result[i]), end(result[i]), reinterpret_cast<float*>(external.begin()) + i * 512);
                    }
                    else
                    {
                        for (size_t i = 0; i < result.size(); i++)
                        {
                            Json::Value jarray_feature;
                            for (size_t j = 0; j < result[i].size(); j++)
                                jarray_feature["feature"].append(result[i][j]);
                            jobj_features.append(jarray_feature);
                        }
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Selene_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int model_type = root["model_type"].asInt();
                    int device = root["device"].asInt();
                    bool use_int8 = root["use_int8"].asBool();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"model_type", box(model_type)},
                         {u8"device", box(device)},
                         {u8"use_int8", box(use_int8 ? 1 : 0)},
                         {u8"models_directory", box(std::string_view(models_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"selene.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Selene_new_test_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int model_type = root["model_type"].asInt();
                    int device = root["device"].asInt();
                    bool use_int8 = root["use_int8"].asBool();
                    std::string model_path = root["model_path"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        { {u8"model_type", box(model_type)},
                          {u8"device", box(device)},
                          {u8"use_int8", box(use_int8 ? 1 : 0)},
                          {u8"model_path", box(std::string_view(model_path))} });

                    instance = unbox<guid>(plugin.execute(u8"selene.new.test", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error& ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Selene_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"selene.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            constexpr int selene_forward_aligned_buffer_len = 3 * 128 * 128;
            inline Json::Value Selene_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    if (format < 0 || format > 1)
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

                    auto aligned_face_array = root["aligned_images"];
                    std::vector<uint8_t> aligned_faces_vec;
                    int num = 0;
                   std::vector<std::uint8_t> temp(selene_forward_aligned_buffer_len, 0);
                    std::uint8_t *ptr = temp.data();
                    for (auto i : aligned_face_array)
                    {
                        std::string aligned_face_base64_str = i.asString();
                        if (aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len))
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len)");

                        size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t *>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
                        if (aligned_face_decode_len != selene_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != selene_forward_aligned_buffer_len");

                        aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
                        num++;
                    }

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
                         {u8"num", box(num)},
                         {u8"order", box(format)},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"selene.forward", param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features = Json::Value(Json::arrayValue);
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < result[i].size(); j++)
                            jarray_feature["feature"].append(result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Selene_make_mask_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                std::uint8_t mask[64 * 128] = {0};
                try
                {
                    int format = root["format"].asInt();
                    if (format < 0 || format > 1)
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

                    auto aligned_face_array = root["aligned_images"];
                    std::vector<uint8_t> aligned_faces_vec;
                    int num = 0;
                    std::vector<std::uint8_t> temp(selene_forward_aligned_buffer_len, 0);
                    std::uint8_t *ptr = temp.data();
                    for (auto i : aligned_face_array)
                    {
                        std::string aligned_face_base64_str = i.asString();
                        if (aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len))
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

                        size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t *>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
                        if (aligned_face_decode_len != selene_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

                        if (format == 0)
                        {
                            for (size_t j = 0; j < 3; j++)
                            {
                                std::copy(mask, mask + 64 * 128, ptr + j * 128 * 128 + 64 * 128);
                            }
                        }
                        else
                        {
                            for (size_t j = 0; j < 3; j++)
                            {
                                std::copy(mask, mask + 64 * 128, ptr + 3 * 64 * 128 + j * 64 * 128);
                            }
                        }

                        aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
                        num++;
                    }

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
                         {u8"num", box(num)},
                         {u8"order", box(format)},
                         {u8"object_id", box(instance)}});

                    std::int32_t model_type = unbox<std::int32_t>(plugin.execute(u8"selene.get_model_type", param));
                    if (model_type != 2)
                        throw parser_exception(parser_exception::parser_exception_code::INVALID_OPERATION, "Illegal operation. model_type != 2");

                    auto result = plugin.execute(u8"selene.forward", param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features;
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < result[i].size(); j++)
                            jarray_feature["feature"].append(result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Damocles_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    int device = root["device"].asInt();
                    bool use_int8 = root["use_int8"].asBool();
                    std::string models_directory = root["models_directory"].asString();
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"device", box(device)},
                         {u8"use_int8", box(use_int8 ? 1 : 0)},
                         {u8"models_directory", box(std::string_view(models_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"damocles.new", param));
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Damocles_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"damocles.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }

            inline Json::Value Damocles_spoofing_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();

                    Json::Value facerect_list = root["facerect_list"];
                    auto faces = make_param_vector<longinus::face_info>();
                    for (auto &i : facerect_list)
                    {
                        auto face = make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_width(i["width"].asFloat());
                        face.set_height(i["height"].asFloat());
                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(instance)}});

                    auto result = plugin.execute(u8"damocles.spoofing_detect", param).as<param_vector<param_vector<float>>>();
                    Json::Value jobj_results = Json::Value(Json::arrayValue);
                    for (size_t i = 0; i < result.size(); i++)
                    {
                        Json::Value jarray_prob;
                        for (size_t j = 0; j < result[i].size(); j++)
                            jarray_prob["prob"].append(result[i][j]);
                        jobj_results.append(jarray_prob);
                    }

                    value["spoofing_result"] = Json::Value(jobj_results);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Irisviel_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    //int single_database_capacity = root["single_database_capacity"].asInt();
                    int single_database_capacity = 1000;
                    int dimension = root["dimension"].asInt();
                    std::string working_directory = root["working_directory"].asString();

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"single_database_capacity", box(single_database_capacity)},
                         {u8"dimension", box(dimension)},
                         {u8"working_directory", box(std::string_view(working_directory))}});

                    instance = unbox<guid>(plugin.execute(u8"irisviel.new", param));

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.delete", param);

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }
            inline Json::Value Irisviel_search_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    value["result"] = Json::Value(Json::arrayValue);

                    param_vector<float> feature = make_param_vector<float>();

                    auto jarray_feature = root["feature"];
                    for (auto i : jarray_feature)
                        feature.push_back(i.asFloat());

                    auto assuming_top = root.get("top", Json::nullValue);
                    auto assuming_min_similarity = root.get("min_similarity", Json::nullValue);
                    bool has_top = assuming_top.isIntegral();
                    bool has_min_similarity = assuming_min_similarity.isNumeric();

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"feature", feature},
                         {u8"object_id", box(instance)}});

                    if (has_top)
                    {
                        param.add_or_update(u8"top", box(static_cast<std::uint32_t>(assuming_top.asUInt())));
                    }

                    if (has_min_similarity)
                    {
                        param.add_or_update(u8"min_similarity", box(assuming_min_similarity.asFloat()));
                    }

                    auto result = plugin.execute(u8"irisviel.search", param).as<param_vector<irisviel::search_result>>();

                    for (const auto &item : result)
                    {
                        Json::Value jobj_result;
                        Json::Value jobj_data;

                        auto feature = item.feature();
                        for (size_t j = 0; j < feature.size(); j++)
                            jobj_data["feature"].append(Json::Value(feature[j]));

                        jobj_data["key"] = Json::Value(to_narrow_string(item.key()));

                        jobj_result["data"] = jobj_data;
                        jobj_result["similarity"] = Json::Value(item.similarity());

                        value["result"].append(jobj_result);
                    }

                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_clear_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});
                    plugin.execute(u8"irisviel.clear", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }
                return value;
            }
            inline Json::Value Irisviel_remove_all_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});
                    plugin.execute(u8"irisviel.remove_all", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_load_databases_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;
                try
                {
                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"object_id", box(instance)}});
                    plugin.execute(u8"irisviel.load_databases", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_remove_records_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    auto keys = make_param_vector<param_string>();
                    auto jarray_keys = root["keys"];
                    for (auto i : jarray_keys)
                        keys.push_back(to_param_string(i.asString()));

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"keys", keys},
                         {u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.remove_records", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_remove_record_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    std::string key = root["key"].asString();

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"key", box(std::string_view(key))},
                         {u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.remove_record", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_add_record_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    param_vector<float> feature = make_param_vector<float>();
                    auto jarray_feature = root["data"]["feature"];
                    for (auto i : jarray_feature)
                        feature.push_back(i.asFloat());

                    int dimension = static_cast<int>(feature.size());

                    std::string key = root["data"]["key"].asString();

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"dimension", box(dimension)},
                         {u8"key", box(std::string_view(key))},
                         {u8"feature", feature},
                         {u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.add_record", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_add_records_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    auto jarray_data = root["data"];

                    auto vec = make_param_vector<param_hash_map<param_string, unknown_object>>();
                    for (auto i : jarray_data)
                    {
                        param_vector<float> feature = make_param_vector<float>();
                        auto jarray_feature = i["feature"];
                        for (auto j : jarray_feature)
                            feature.push_back(j.asFloat());

                        int dimension = static_cast<int>(feature.size());

                        std::string key = i["key"].asString();

                        auto data = make_param_hash_map<param_string, unknown_object>(
                            {{u8"dimension", box(dimension)},
                             {u8"key", box(std::string_view(key))},
                             {u8"feature", feature}});

                        vec.push_back(data);
                    }

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"records", vec},
                         {u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.add_records", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_update_record_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    param_vector<float> feature = make_param_vector<float>();
                    auto jarray_feature = root["data"]["feature"];
                    for (auto i : jarray_feature)
                        feature.push_back(i.asFloat());

                    int dimension = static_cast<int>(feature.size());

                    std::string key = root["data"]["key"].asString();

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"dimension", box(dimension)},
                         {u8"key", box(std::string_view(key))},
                         {u8"feature", feature},
                         {u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.update_record", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
            inline Json::Value Irisviel_update_records_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    auto jarray_data = root["data"];

                    auto vec = make_param_vector<param_hash_map<param_string, unknown_object>>();
                    for (auto i : jarray_data)
                    {
                        param_vector<float> feature = make_param_vector<float>();
                        auto jarray_feature = i["feature"];
                        for (auto j : jarray_feature)
                            feature.push_back(j.asFloat());

                        int dimension = static_cast<int>(feature.size());

                        std::string key = i["key"].asString();

                        auto data = make_param_hash_map<param_string, unknown_object>(
                            {{u8"dimension", box(dimension)},
                             {u8"key", box(std::string_view(key))},
                             {u8"feature", feature}});

                        vec.push_back(data);
                    }

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"records", vec},
                         {u8"object_id", box(instance)}});

                    plugin.execute(u8"irisviel.update_records", param);
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Fusion_Romancia_alignFace128_Gaius_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, std::vector<guid> &guids, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto jarray_rect = root["facerectwithfaceinfo_list"];
                    bool has_mask = root["has_mask"].asBool();

                    auto faces = exposing::make_param_vector<longinus::face_info>();
                    for (auto i : jarray_rect)
                    {
                        auto face = exposing::make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_height(i["height"].asFloat());
                        face.set_width(i["width"].asFloat());

                        auto landmark_list = i["landmark"];
                        if (landmark_list.size() != 5)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");
                        auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
                        for (auto j : landmark_list)
                        {
                            auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
                            landmark.push_back(pair);
                        }
                        face.set_pts(landmark);

                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto romancia_param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(guids[0])}});

                    auto romancia_result = plugin.execute(u8"romancia.alignFace128", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

                    std::vector<std::uint8_t> buffer;
                    for (size_t i = 0; i < romancia_result.size(); i++)
                    {
                        if (romancia_result[i].size() != gaius_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != gaius_forward_aligned_buffer_len");

                        buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
                    }

                    auto gaius_param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
                         {u8"num", box(static_cast<int>(romancia_result.size()))},
                         {u8"order", box(0)},
                         {u8"has_mask", box(has_mask ? 1 : 0)},
                         {u8"object_id", box(guids[1])}});

                    auto gaius_result = plugin.execute(u8"gaius.forward", gaius_param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features;
                    for (size_t i = 0; i < gaius_result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < gaius_result[i].size(); j++)
                            jarray_feature["feature"].append(gaius_result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Fusion_Romancia_alignFace_Selene_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, std::vector<guid> &guids, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto jarray_rect = root["facerectwithfaceinfo_list"];

                    auto faces = exposing::make_param_vector<longinus::face_info>();
                    for (auto i : jarray_rect)
                    {
                        auto face = exposing::make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_height(i["height"].asFloat());
                        face.set_width(i["width"].asFloat());

                        auto landmark_list = i["landmark"];
                        if (landmark_list.size() != 5)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");
                        auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
                        for (auto j : landmark_list)
                        {
                            auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
                            landmark.push_back(pair);
                        }
                        face.set_pts(landmark);

                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto romancia_param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(guids[0])}});

                    auto romancia_result = plugin.execute(u8"romancia.alignFace", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

                    std::vector<std::uint8_t> buffer;
                    for (size_t i = 0; i < romancia_result.size(); i++)
                    {
                        if (romancia_result[i].size() != selene_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != selene_forward_aligned_buffer_len");

                        buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
                    }

                    auto selene_param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
                         {u8"num", box(static_cast<int>(romancia_result.size()))},
                         {u8"order", box(0)},
                         {u8"object_id", box(guids[1])}});

                    auto selene_result = plugin.execute(u8"selene.forward", selene_param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features;
                    for (size_t i = 0; i < selene_result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < selene_result[i].size(); j++)
                            jarray_feature["feature"].append(selene_result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }

            inline Json::Value Fusion_Romancia_alignFace_Cassius_forward_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, std::vector<guid> &guids, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    auto jarray_rect = root["facerectwithfaceinfo_list"];

                    auto faces = exposing::make_param_vector<longinus::face_info>();
                    for (auto i : jarray_rect)
                    {
                        auto face = exposing::make_exported_interface<longinus::face_info>();
                        face.set_x(i["x"].asFloat());
                        face.set_y(i["y"].asFloat());
                        face.set_height(i["height"].asFloat());
                        face.set_width(i["width"].asFloat());

                        auto landmark_list = i["landmark"];
                        if (landmark_list.size() != 5)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");
                        auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
                        for (auto j : landmark_list)
                        {
                            auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
                            landmark.push_back(pair);
                        }
                        face.set_pts(landmark);

                        faces.push_back(face);
                    }

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame.cpu_data()), frame.count());

                    auto romancia_param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"faces", faces},
                         {u8"order", box(static_cast<int>(frame.order()))},
                         {u8"object_id", box(guids[0])}});

                    auto romancia_result = plugin.execute(u8"romancia.alignFace", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

                    std::vector<std::uint8_t> buffer;
                    for (size_t i = 0; i < romancia_result.size(); i++)
                    {
                        if (romancia_result[i].size() != cassius_forward_aligned_buffer_len)
                            throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != cassius_forward_aligned_buffer_len");

                        buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
                    }

                    auto cassius_param = make_param_hash_map<param_string, unknown_object>(
                        {{u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
                         {u8"num", box(static_cast<int>(romancia_result.size()))},
                         {u8"order", box(0)},
                         {u8"object_id", box(guids[1])}});

                    auto cassius_result = plugin.execute(u8"cassius.forward", cassius_param).as<param_vector<param_vector<float>>>();

                    Json::Value jobj_features;
                    for (size_t i = 0; i < cassius_result.size(); i++)
                    {
                        Json::Value jarray_feature;
                        for (size_t j = 0; j < cassius_result[i].size(); j++)
                            jarray_feature["feature"].append(cassius_result[i][j]);
                        jobj_features.append(jarray_feature);
                    }

                    value["features"] = jobj_features;
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                }
                catch (const parser_exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
                }
                catch (const Json::Exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
                }
                catch (const std::exception &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what());
                    value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
                }
                catch (const abi_error &ex)
                {
                    value["status"]["message"] = Json::Value(ex.what_to_narrow());
                    value["status"]["code"] = Json::Int(ex.result());
                }

                return value;
            }
        }
    }
}
#endif
