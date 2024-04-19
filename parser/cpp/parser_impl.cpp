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
	enum class IMAGE_FORMAT
	{
		IMAGE_BGR_NCHW = 0,
		IMAGE_BGR_NHWC = 1,
		IMAGE_NV21 = 2,
		UNKNOW = 99
	};

	struct data_handler
	{
		data_handler() : data_{ nullptr }, size_{ 0 }, format_{ IMAGE_FORMAT::UNKNOW }, is_heap_allocated_{ false } {}
		data_handler(std::uint8_t* data, size_t size, IMAGE_FORMAT format, bool is_heap_allocated) : data_{ data }, size_{ size }, format_{ format }, is_heap_allocated_{ is_heap_allocated } {}
		~data_handler()
		{
			if (is_heap_allocated_)
				if (data_)
					delete[] data_;
		}
		const std::uint8_t* data_;
		size_t size_;
		IMAGE_FORMAT format_;
		bool is_heap_allocated_;
	};



	inline void convert_to_bgr(std::shared_ptr<data_handler>& src, std::shared_ptr<data_handler>& dst, int width, int height)
	{
		switch (src->format_)
		{
		case IMAGE_FORMAT::IMAGE_BGR_NCHW:
		{
			if (width * height * 3 != src->size_)
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NCHW, width * height * 3 != src->size_");
			dst = src;
		}
		case IMAGE_FORMAT::IMAGE_BGR_NHWC:
		{
			int step = 0;
			if (src->size_ != width * height * 3)
			{
				step = ((width * 3 + 3) >> 2) << 2;
				if (src->size_ != step * height)
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NHWC, src->size_ != width * height * 3 || src->size_ != (((width * 3 + 3) >> 2) << 2) * height");
			}
			else
				step = width * 3;

			if (step == width * 3)
				dst = src;
			else if (step > width * 3)
			{
				size_t size = 3 * height * width;
				std::uint8_t* dst_ptr = new std::uint8_t[size];
				const std::uint8_t* src_ptr = src->data_;
				for (size_t i = 0; i < height; i++)
					std::copy(src_ptr + i * step, src_ptr + i * step + width * 3, dst_ptr + i * width * 3);

				dst = std::make_shared<data_handler>(dst_ptr, size, IMAGE_FORMAT::IMAGE_BGR_NHWC, true);
			}
			else
			{
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "step < width * 3");
			}

			break;
		}
		case IMAGE_FORMAT::IMAGE_NV21:
		{
			size_t size = width * height * 3;
			if (src->size_ != (size >> 1))
				throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "convert_to_bgr: src->size_ != (width * height * 3 >> 1)");

			std::uint8_t* dst_ptr = new std::uint8_t[size];
			int aligned_src_width = (width + 1) & ~1;
			const uint8_t* y = src->data_;
			const uint8_t* uv = src->data_ + aligned_src_width * height;
			if (libyuv::NV21ToRGB24(y, width, uv, aligned_src_width, dst_ptr, width * 3, width, height))
				throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, "NV21ToRGB24 failed.");

			dst = std::make_shared<data_handler>(dst_ptr, size, IMAGE_FORMAT::IMAGE_BGR_NHWC, true);
			break;
		}
		default:
			throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Unsupported image format.");
			break;
		}
	}

	inline std::shared_ptr<data_handler> decode_and_convert(param_span<std::uint8_t> src, bool is_base64, IMAGE_FORMAT format, int width, int height)
	{
		if (height <= 0 || width <= 0)
			throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: height <= 0 || width <= 0");

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
		convert_to_bgr(temp, dst, width, height);
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

		param_string execute(const guid& instance_id, const param_string& str_param, const param_span<std::uint8_t> img_data, const int height, const int width, const IMAGE_FORMAT img_format, bool is_base64, const param_span<std::uint8_t> output_data)
		{
			auto frame = decode_and_convert(img_data, is_base64, img_format, width, height);
			param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

			auto input_params_map = make_param_hash_map<param_string, unknown_object>(
				{ {u8"params", box(str_param)},
				{u8"bgr_data", box(image_span)},
				{u8"height", box(height)},
				{u8"width", box(width)},
				{u8"output_data", box(output_data)} });

			return plugin_manager_.execute(instance_id, input_params_map);
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

	param_string parser_impl::execute(const guid& instance_id, const param_string& str_param, const param_span<std::uint8_t> img_data, const int height, const int width, const int data_format, bool is_base64, param_span<std::uint8_t> output_data)
	{
		return parser_impl_concret::instance().execute(instance_id, str_param, img_data, height, width, static_cast<IMAGE_FORMAT>(data_format), is_base64, output_data);
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
