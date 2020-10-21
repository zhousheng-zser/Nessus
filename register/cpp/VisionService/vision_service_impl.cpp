#include "vision_service_impl.hpp"

#include <mutex>
#include <functional>
#include <unordered_map>

#include <longinus/retina_net.hpp>
#include <irisviel/face_service.hpp>
#include <romancia/face_alignment.hpp>
#include <gaius/feature_extractor.hpp>
#include <cassius/feature_extractor.hpp>

using namespace glasssix::gaius;
using namespace glasssix::cassius;
using namespace glasssix::romancia;
using namespace glasssix::irisviel;
using namespace glasssix::longinus;

namespace glasssix::exposing::nessus
{
	namespace
	{
		struct package_names
		{
			static constexpr utf8_string_view gaius{ u8"gaius" };
			static constexpr utf8_string_view cassius{ u8"cassius" };
			static constexpr utf8_string_view longinus{ u8"longinus" };
			static constexpr utf8_string_view romancia{ u8"romancia" };
			static constexpr utf8_string_view irisviel{ u8"irisviel" };
		};

		struct function_names final
		{
			static constexpr utf8_string_view gaius_new{ u8"gaius.new" };
			static constexpr utf8_string_view cassius_new{ u8"cassius.new" };
			static constexpr utf8_string_view longinus_new{ u8"longinus.new" };
			static constexpr utf8_string_view romancia_new{ u8"romancia.new" };
			static constexpr utf8_string_view irisviel_new{ u8"irisviel.new" };
			static constexpr utf8_string_view gaius_delete{ u8"gaius.delete" };
			static constexpr utf8_string_view cassius_delete{ u8"cassius.delete" };
			static constexpr utf8_string_view longinus_delete{ u8"longinus.delete" };
			static constexpr utf8_string_view romancia_delete{ u8"romancia.delete" };
			static constexpr utf8_string_view irisviel_delete{ u8"irisviel.delete" };
			static constexpr utf8_string_view gaius_forward{ u8"gaius.Forward" };
			static constexpr utf8_string_view cassius_forward{ u8"cassius.Forward" };
			static constexpr utf8_string_view longinus_detect{ u8"longinus.detect" };
			static constexpr utf8_string_view longinus_trace{ u8"longinus.trace" };
			static constexpr utf8_string_view romancia_align_face{ u8"romancia.alignFace" };
			static constexpr utf8_string_view romancia_antispoofing{ u8"romancia.antispoofing" };
			static constexpr utf8_string_view romancia_blur_detect{ u8"romancia.blur_detect" };
			static constexpr utf8_string_view romancia_mask_detect{ u8"romancia.mask_detect" };
			static constexpr utf8_string_view irisviel_clear{ u8"irisviel.clear" };
			static constexpr utf8_string_view irisviel_remove_all{ u8"irisviel.remove_all" };
			static constexpr utf8_string_view irisviel_load_databases{ u8"irisviel.load_databases" };
			static constexpr utf8_string_view irisviel_add_record{ u8"irisviel.add_record" };
			static constexpr utf8_string_view irisviel_add_records{ u8"irisviel.add_records" };
			static constexpr utf8_string_view irisviel_update_record{ u8"irisviel.update_record" };
			static constexpr utf8_string_view irisviel_update_records{ u8"irisviel.update_records" };
			static constexpr utf8_string_view irisviel_remove_record{ u8"irisviel.remove_record" };
			static constexpr utf8_string_view irisviel_remove_records{ u8"irisviel.remove_records" };
			static constexpr utf8_string_view irisviel_search{ u8"irisviel.search" };
		};
	}

	class vision_service_impl::impl
	{
	public:
		impl()
		{
			// New
			functions_.insert_or_assign(function_names::gaius_new, std::bind(&impl::gaius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::cassius_new, std::bind(&impl::cassius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_new, std::bind(&impl::longinus_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_new, std::bind(&impl::romancia_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_new, std::bind(&impl::irisviel_new, this, std::placeholders::_1));

			// Delete
			functions_.insert_or_assign(function_names::gaius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::cassius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::longinus_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::romancia_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));

			// Business
			functions_.insert_or_assign(function_names::longinus_detect, std::bind(&impl::longinus_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_trace, std::bind(&impl::longinus_trace, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_align_face, std::bind(&impl::romancia_align_face, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_antispoofing, std::bind(&impl::romancia_antispoofing, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_blur_detect, std::bind(&impl::romancia_blur_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_mask_detect, std::bind(&impl::romancia_mask_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gaius_forward, std::bind(&impl::gaius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::cassius_forward, std::bind(&impl::cassius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_clear, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_clear, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_all, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_all, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_load_databases, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_load_databases, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_add_record, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_add_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_add_records, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_add_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_update_record, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_update_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_update_records, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_update_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_record, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_records, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_search, std::bind(&impl::irisviel_search, this, std::placeholders::_1));
		}

		param_string name() const
		{
			return u8"Glasssix Vision Service";
		}

		param_string version() const
		{
			return u8"1.0.0";
		}

		param_vector<param_string> available_functions() const
		{
			auto result = make_param_vector<param_string>();

			for (const auto& [key, value] : functions_)
			{
				result.push_back(key);
			}

			return result;
		}

		param_hash_map<guid, param_string> existing_instances() const
		{
			auto result = make_param_hash_map<guid, param_string>();

			for (const auto& [key, value] : instances_)
			{
				result.add_or_update(key, std::get<param_string>(value));
			}

			return result;
		}

		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
		{
			auto iter = functions_.find(function_name);

			return iter != functions_.end() ? iter->second(params) : throw abi_key_not_found{ function_name };
		}
	private:
		unknown_object cassius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::cassius, make_exported_interface<cassius::feature_extractor>(models_directory + u8"/unicorn.racy", device));
		}

		unknown_object gaius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::gaius, make_exported_interface<gaius::feature_extractor>(models_directory + u8"/mobile_unicorn.racy", models_directory + u8"/mobile_unicorn_mask.racy", device));
		}

		unknown_object longinus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto nms = unbox<float>(params.get_value(u8"nms"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::longinus, make_exported_interface<retina_net>(models_directory + u8"/retina.racy", models_directory + u8"/pfld_small_gen_age_sim.racy", nms, device));
		}

		unknown_object romancia_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::romancia, make_exported_interface<face_alignment>(models_directory + u8"/antispoofing80x80", device));
		}

		unknown_object irisviel_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto single_database_capacity = unbox<std::int32_t>(params.get_value(u8"single_database_capacity"));
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto working_directory = unbox<param_string>(params.get_value(u8"working_directory"));

			return add_instance(package_names::irisviel, make_exported_interface<face_service>(single_database_capacity, dimension, working_directory));
		}

		unknown_object cassius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<cassius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		unknown_object gaius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<gaius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto has_mask = unbox<std::int32_t>(params.get_value(u8"has_mask"));

			return instance.get(aligned_faces, num, order, has_mask?true:false);
		}

		unknown_object longinus_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto min_size = unbox<std::int32_t>(params.get_value(u8"min_size"));
			auto threshold = unbox<float>(params.get_value(u8"threshold"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(image, channels, height, width, min_size, threshold, order);
		}

		unknown_object longinus_trace(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto face = params.get_value(u8"face").as<face_info>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto result = instance.single_trace(face, image, channels, height, width, order);

			return result;
		}

		unknown_object romancia_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(image, channels, height, width, faces, order);
		}

		unknown_object romancia_blur_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto face = params.get_value(u8"face").as<face_info>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return box(instance.blur_detect(face, image, channels, height, width, order));
		}

		unknown_object romancia_mask_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto face = params.get_value(u8"face").as<face_info>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return box(instance.mask_detect(face, image, channels, height, width, order));
		}

		unknown_object romancia_antispoofing(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto face = params.get_value(u8"face").as<face_info>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return box(instance.antispoofing(face, image, channels, height, width, order));
		}

		void irisviel_clear(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).clear();
		}

		void irisviel_remove_all(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).remove_all();
		}

		void irisviel_load_databases(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).load_databases();
		}

		void irisviel_add_record(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_record_helper(params, false);
		}

		void irisviel_add_records(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_records_helper(params, false);
		}

		void irisviel_update_record(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_record_helper(params, true);
		}

		void irisviel_update_records(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_records_helper(params, true);
		}

		void irisviel_remove_record(const param_hash_map<param_string, unknown_object>& params)
		{
			auto key = unbox<param_string>(params.get_value(u8"key"));

			get_instance<face_service>(params).remove_record(key);
		}

		void irisviel_remove_records(const param_hash_map<param_string, unknown_object>& params)
		{
			auto keys = params.get_value(u8"keys").as<param_vector<param_string>>();

			get_instance<face_service>(params).remove_records(keys);
		}

		unknown_object irisviel_search(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();
			auto top = unbox<std::int32_t>(params.get_value(u8"top"));

			return instance.search(feature, top);
		}

		record irisviel_create_record_helper(const param_hash_map<param_string, unknown_object>& params)
		{
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto key = unbox<param_string>(params.get_value(u8"key"));
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();
			auto result = make_exported_interface<record>(dimension);

			result.key(key);
			result.feature(feature);

			return result;
		}

		void irisviel_add_or_update_record_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			auto instance = get_instance<face_service>(params);
			auto record = irisviel_create_record_helper(params);

			if (update)
			{
				instance.update_record(record);
			}
			else
			{
				instance.add_record(record);
			}
		}

		void irisviel_add_or_update_records_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			auto instance = get_instance<face_service>(params);
			auto param_records = params.get_value(u8"records").as<param_vector<param_hash_map<param_string, unknown_object>>>();
			auto records = make_param_vector<record>();

			for (const auto& item : param_records)
			{
				records.push_back(irisviel_create_record_helper(item));
			}

			if (update)
			{
				instance.update_records(records);
			}
			else
			{
				instance.add_records(records);
			}
		}

		unknown_object add_instance(utf8_string_view package_name, const unknown_object& instance)
		{
			auto id = create_guid_from_bytes(meta::to_array(reinterpret_cast<std::size_t>(get_abi(instance))));
			{
				std::scoped_lock lock{ mutex_ };

				return (instances_.insert_or_assign(id, std::tuple{ package_name, instance }), box(id));
			}
		}

		void delete_instance_by_id(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };

			if (auto iter = instances_.find(id); iter != instances_.end())
			{
				instances_.erase(iter);
			}
		}

		void delete_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			delete_instance_by_id(get_instance_id(params));
		}

		static guid get_instance_id(const param_hash_map<param_string, unknown_object>& params)
		{
			return unbox<guid>(params.get_value(u8"object_id"));
		}

		template<typename T>
		T get_instance(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };
			auto iter = instances_.find(id);

			return iter != instances_.end() ? std::get<unknown_object>(iter->second).as<T>() : throw abi_key_not_found{ exposing::format(u8"Cannot find instance: {}.", to_param_string(id)) };
		}

		template<typename T>
		T get_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			return get_instance<T>(unbox<guid>(params.get_value(u8"object_id")));
		}

		std::mutex mutex_;
		std::unordered_map<guid, std::tuple<param_string, unknown_object>> instances_;
		std::unordered_map<param_string, std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>> functions_;
	};

	vision_service_impl::vision_service_impl() : impl_{ new impl }
	{
	}

	vision_service_impl::~vision_service_impl()
	{
		if (impl_)
		{
			delete impl_;
			impl_ = nullptr;
		}
	}

	param_string vision_service_impl::name() const
	{
		return impl_->name();
	}

	param_string vision_service_impl::version() const
	{
		return impl_->version();
	}

	param_vector<param_string> vision_service_impl::available_functions() const
	{
		return impl_->available_functions();
	}

	param_hash_map<guid, param_string> vision_service_impl::existing_instances() const
	{
		return impl_->existing_instances();
	}

	unknown_object vision_service_impl::execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
	{
		return impl_->execute(function_name, params);
	}
}
