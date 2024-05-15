#include "parser_impl.hpp"
#include "plugin_manager.hpp"
#include "parser_exception.hpp"
#include "json.h"
#include "libyuv.h"
#include "base64_x.hpp"
#include <singleton.hpp>
#include <fmt/format.h>

#include <filesystem.hpp>
#include <os_context.hpp>

#include <string_view>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <algorithm>

#ifdef __GNUC__
#include <malloc.h>
#endif

namespace glasssix::exposing::nessus
{
	namespace
	{
		std::vector<std::string> split(std::string_view s, std::string_view seperator)
		{
			std::vector<std::string> result;
			typedef std::string::size_type string_size;
			string_size i = 0;

			while (i != s.size())
			{
				int flag = 0;
				while (i != s.size() && flag == 0)
				{
					flag = 1;
					for (string_size x = 0; x < seperator.size(); ++x)
						if (s[i] == seperator[x])
						{
							++i;
							flag = 0;
							break;
						}
				}

				flag = 0;
				string_size j = i;
				while (j != s.size() && flag == 0)
				{
					for (string_size x = 0; x < seperator.size(); ++x)
						if (s[j] == seperator[x])
						{
							flag = 1;
							break;
						}
					if (flag == 0)
						++j;
				}
				if (i != j)
				{
					result.push_back(std::string(s.substr(i, j - i)));
					i = j;
				}
			}
			return result;
		}
	}


	//supported image format
	enum class DATA_FORMAT
	{
		IMAGE_BGR_NCHW = 0,
		IMAGE_BGR_NHWC = 1,
		IMAGE_NV21 = 2,
		UNKNOW = 99
	};

	struct data_handler
	{
		data_handler() : data_{ nullptr }, size_{ 0 }, format_{ DATA_FORMAT::UNKNOW }, is_heap_allocated_{ false } {}
		data_handler(std::uint8_t* data, size_t size, DATA_FORMAT format, bool is_heap_allocated) : data_{ data }, size_{ size }, format_{ format }, is_heap_allocated_{ is_heap_allocated } {}
		~data_handler()
		{
			if (is_heap_allocated_)
				if (data_)
					delete[] data_;
		}
		const std::uint8_t* data_;
		size_t size_;
		DATA_FORMAT format_;
		bool is_heap_allocated_;
	};



	inline void convert_to_bgr(std::shared_ptr<data_handler>& src, std::shared_ptr<data_handler>& dst, int n, int c, int h, int w)
	{
		switch (src->format_)
		{
		case DATA_FORMAT::IMAGE_BGR_NCHW:
		{
			if (src->size_ % n != 0 && src->size_ / n < c * h * w)
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NCHW, src->size_ % n != 0 && src->size_ / n < c * h * w");

			int stride = src->size_ / (n * c * h);
			if(stride == w)
				dst = src;
			else if (stride > w)
			{
				int size = n * c * h * w;
				std::uint8_t* dst_ptr = new std::uint8_t[size];
				const std::uint8_t* src_ptr = src->data_;
				for (int i = 0; i < n; i++)
					for (int j = 0; j < c; j++)
						for (int y = 0; y < h; y++)
							std::copy(src_ptr + i * c * h * stride + j * h * stride + y * stride, src_ptr + i * c * h * stride + j * h * stride + y * stride + w, dst_ptr + i * c * h * w + j * h * w + y * w);

				dst = std::make_shared<data_handler>(dst_ptr, size, DATA_FORMAT::IMAGE_BGR_NCHW, true);
			}
			else
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "stride < w");
		}
		case DATA_FORMAT::IMAGE_BGR_NHWC:
		{
			if (src->size_ % n != 0 && src->size_ / n < h * w * c)
			{
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NHWC, src->size_ % n != 0 && src->size_ / n < h * w * c");
			}

			int stride = src->size_ / (n * h);
			if (stride == c * w)
				dst = src;
			else if (stride > c * w)
			{
				int size = n * c * h * w;
				std::uint8_t* dst_ptr = new std::uint8_t[size];
				const std::uint8_t* src_ptr = src->data_;
				for (int i = 0; i < n; i++)
				{
					for (int y = 0; y < h; y++)
					{
						std::copy(src_ptr + i * h * stride + i * stride, src_ptr + i * h * stride + y * stride + c * w, dst_ptr + i * c * h * w + y * c * w);
					}
				}

				dst = std::make_shared<data_handler>(dst_ptr, size, DATA_FORMAT::IMAGE_BGR_NHWC, true);
			}
			else
			{
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "stride < c * w");
			}

			break;
		}
		case DATA_FORMAT::IMAGE_NV21:
		{
			int size = h * w * c;
			if (h & 1 || src->size_ != n * (size >> 1))
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "convert_to_bgr: h & 1 || src->size_ != n * (size >> 1)");

			std::uint8_t* dst_ptr = new std::uint8_t[n * size];
			for (int i = 0; i < n; n++)
			{
				int aligned_src_width = (w + 1) & ~1;
				const uint8_t* y = src->data_ + i * (size >> 1);
				const uint8_t* vu = src->data_ + i * (size >> 1) + aligned_src_width * h;
				if (libyuv::NV21ToRGB24(y, w, vu, aligned_src_width, dst_ptr + i * size, w * c, w, h))
					throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, "NV21ToRGB24 failed.");
			}

			dst = std::make_shared<data_handler>(dst_ptr, size * n, DATA_FORMAT::IMAGE_BGR_NHWC, true);
			break;
		}
		default:
			throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Unsupported image format.");
			break;
		}
	}

	inline std::shared_ptr<data_handler> decode_and_convert(param_span<std::uint8_t> src, bool is_base64, DATA_FORMAT format, int n, int c, int h, int w)
	{
		if (h <= 0 || w <= 0)
			throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: h <= 0 || w <= 0");

		if (src.size() <= 0)
			throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: src.size() <= 0");

		std::shared_ptr<data_handler> temp;
		if (is_base64)
		{
			int current_image_str_len = TB64DECLEN(src.size());

			std::uint8_t* decoded_data = new std::uint8_t[current_image_str_len];
			tb64xdec(reinterpret_cast<const std::uint8_t*>(src.data()), src.size(), decoded_data);
			temp = std::make_shared<data_handler>(decoded_data, current_image_str_len, format, true);
		}
		else
		{
			temp = std::make_shared<data_handler>(src.data(), src.size(), format, false);
		}

		std::shared_ptr<data_handler> dst;
		convert_to_bgr(temp, dst, n, c, h, w);
		return dst;
	}


	/// <summary>
	/// An implementation of the standard plugin manager.
	/// </summary>
	class parser_impl_concret : public singleton<parser_impl_concret>
	{
	public:
		guid create_instance(const param_string& qualified_name, const param_string& str_param)
		{
			return plugin_manager_.create_algo_instance(qualified_name, str_param);
		}

		param_string execute(const guid& instance_id, const param_string& str_param, const param_span<std::uint8_t> input_data, const param_span<std::uint8_t> reserve_output_data)
		{
			Json::Reader reader(Json::Features::strictMode());
			Json::FastWriter writer;
			Json::Value root;
			if (!reader.parse(exposing::to_narrow_string(str_param), root))
				throw parser_exception(parser_exception::parser_exception_code::JSON_EXCEPTION, "parse json failed");

			if (root["data_params"].empty())
			{
				auto input_params_map = make_param_hash_map<param_string, unknown_object>(
					{ {u8"params", box(exposing::to_param_string(writer.write(root.get("algo_params", Json::Value(Json::nullValue)))))},
					{u8"input_data", box(input_data)},
					{u8"output_data", box(reserve_output_data)} });

				return plugin_manager_.execute(instance_id, input_params_map);
			}
			else
			{
				int n = root["data_params"].get("num", Json::Int(1)).asInt();
				int c = root["data_params"].get("channels", Json::Int(3)).asInt();
				int h = root["data_params"]["height"].asInt();
				int w = root["data_params"]["width"].asInt();
				int format = root["data_params"].get("format", Json::Int(1)).asInt();

				auto frames = decode_and_convert(input_data, false, static_cast<DATA_FORMAT>(format), n, c, h, w);
				param_span<std::uint8_t> data_span(const_cast<std::uint8_t*>(frames->data_), frames->size_);

				param_vector<int> data_shape = make_param_vector<int>();
				data_shape.push_back(n);
				data_shape.push_back(c);
				data_shape.push_back(h);
				data_shape.push_back(w);

				auto input_params_map = make_param_hash_map<param_string, unknown_object>(
					{ {u8"params", box(exposing::to_param_string(writer.write(root.get("algo_params", Json::Value(Json::nullValue)))))},
					{u8"input_data", box(data_span)},
					{u8"order", box(static_cast<int>(frames->format_))},
					{u8"data_shape", data_shape},
					{u8"output_data", box(reserve_output_data)} });

				return plugin_manager_.execute(instance_id, input_params_map);
			}
		}

		const char* nessus_version_ = "1.0.0";

		void release_instance(const guid& instance_id)
		{
			plugin_manager_.release_algo_instance(instance_id);
#if defined(__GNUC__) && !defined(ANDROID)
			::malloc_trim(0);
#endif
		}

		void init_plugin(const param_string& config_file_path)
		{
			static bool ready = false;
			static std::string message;
			static std::once_flag flag;
			std::call_once(flag, [&]
				{
					std::ifstream f_config{ std::string(config_file_path.begin(), config_file_path.end()) };
					std::string buffer(std::istreambuf_iterator<char>{ f_config }, std::istreambuf_iterator<char>{});

					Json::Reader reader(Json::Features::strictMode());
					Json::Value config;
					if (!reader.parse(buffer, config))
						throw parser_exception(parser_exception::parser_exception_code::JSON_EXCEPTION, "parse json failed");

					for (auto lib_item : config["plugin_list"])
					{
						bool ret = get_component_loader().add_module_by_name(to_param_string(std::string_view(lib_item.asString())));
						if (!ret)
						{
							ready = false;
							message = fmt::format(R"({"load module '{}' failed"})", lib_item.asString());
							throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, message);
						}
					}

					plugin_manager_ = exposing::make_exported_interface<plugin_manager>();
					ready = true;
				});

			if(!ready)
				throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, message);
		}

	private:
		plugin_manager plugin_manager_;
	};

	parser_impl::parser_impl()
	{
	}
	parser_impl::~parser_impl()
	{
	}

	guid parser_impl::create_instance(const param_string& qualified_name, const param_string& str_param)
	{
		return parser_impl_concret::instance().create_instance(qualified_name, str_param);
	}

	param_string parser_impl::execute(const guid& instance_id, const param_string& str_param, const param_span<std::uint8_t> input_data, const param_span<std::uint8_t> reserve_output_data)
	{
		return parser_impl_concret::instance().execute(instance_id, str_param, input_data, reserve_output_data);
	}

	void parser_impl::release_instance(const guid& instance_id)
	{
		parser_impl_concret::instance().release_instance(instance_id);
	}

	void parser_impl::init_plugin(const param_string& config_file_path)
	{
		return parser_impl_concret::instance().init_plugin(config_file_path);
	}
}
